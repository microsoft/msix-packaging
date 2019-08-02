//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
//

#include <cstdlib>
#include <cwchar>
#include <string>
#include <locale>
#include <codecvt>
#include <iostream>
#include <iomanip>
#include <functional>
#include <map>
#include <algorithm>

#include "AppxPackaging.hpp"
#include "MSIXWindows.hpp"

#include "Helpers.hpp"

using namespace MsixSample::Helper;

// describes an option to a command that the user may specify
struct Option
{
    using CBF = std::function<bool(const std::string& value)>;

    Option(bool param, const std::string& help, CBF callback): Help(help), Callback(callback), TakesParameter(param)
    {}

    bool        TakesParameter;
    std::string Name;
    std::string Help;
    CBF         Callback;
};

// Tracks the state of the current parse operation as well as implements input validation
struct State
{
    bool SkipSignature()
    {
        validationOptions = static_cast<MSIX_VALIDATION_OPTION>(validationOptions | MSIX_VALIDATION_OPTION::MSIX_VALIDATION_OPTION_SKIPSIGNATURE);
        return true;
    }

    bool AllowSignatureOriginUnknown()
    {
        validationOptions = static_cast<MSIX_VALIDATION_OPTION>(validationOptions | MSIX_VALIDATION_OPTION::MSIX_VALIDATION_OPTION_ALLOWSIGNATUREORIGINUNKNOWN);
        return true;
    }

    bool SetPackageName(const std::string& name)
    {
        if (!packageName.empty() || name.empty()) { return false; }
        packageName = utf8_to_utf16(name);
        return true;
    }

    bool SetDirectoryName(const std::string& name)
    {
        if (!directoryName.empty() || name.empty()) { return false; }
        directoryName = utf8_to_utf16(name);
        return true;
    }

    bool SetRecursiveUnpack()
    {
        recursiveUnpack = true;
        return true;
    }

    std::wstring packageName;
    std::wstring directoryName;
    bool recursiveUnpack = false;
    MSIX_VALIDATION_OPTION validationOptions = MSIX_VALIDATION_OPTION::MSIX_VALIDATION_OPTION_FULL;
};

// Displays contextual formatted help to the user.
int Help(char* toolName, std::map<std::string, Option>& options)
{
    std::cout << std::endl;
    std::cout << "Usage:" << std::endl;
    std::cout << "------" << std::endl;
    std::cout << "\t" << toolName << " -p <package> -d <directory> [options] " << std::endl;
    std::cout << std::endl;
    std::cout << "Description:" << std::endl;
    std::cout << "------------" << std::endl;
    std::cout << "\tExtracts all files within an app package at the input <package> name to the" << std::endl;
    std::cout << "\tspecified output <directory>.  The output has the same directory structure " << std::endl;
    std::cout << "\tas the package." << std::endl;
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "--------" << std::endl;

    for (const auto& option : options)
    {
        std::cout << "\t" << std::left << std::setfill(' ') << std::setw(5) <<
            option.first << ": " << option.second.Help << std::endl;
    }
    return 0;
}

// error text if the user provided underspecified input
void Error(char* toolName)
{
    std::cout << toolName << ": error : Missing required options.  Use '-?' for more details." << std::endl;
}

template<typename Type>
struct FootprintFilesType
{
    Type fileType;
    const char* description;
    bool isRequired;
};

// Types of footprint files in an app package
const int FootprintFilesCount = 4;
FootprintFilesType<APPX_FOOTPRINT_FILE_TYPE> footprintFilesType[FootprintFilesCount] = {
    {APPX_FOOTPRINT_FILE_TYPE_MANIFEST, "manifest", true },
    {APPX_FOOTPRINT_FILE_TYPE_BLOCKMAP, "block map", true },
    {APPX_FOOTPRINT_FILE_TYPE_SIGNATURE, "digital signature", true },
    {APPX_FOOTPRINT_FILE_TYPE_CODEINTEGRITY, "CI catalog", false }, // this is ONLY required if there exists 1+ PEs
};

// Types of footprint files in a bundle package
const int BundleFootprintFilesCount = 3;
FootprintFilesType<APPX_BUNDLE_FOOTPRINT_FILE_TYPE> bundleFootprintFilesType[FootprintFilesCount] = {
    {APPX_BUNDLE_FOOTPRINT_FILE_TYPE_MANIFEST, "manifest", true },
    {APPX_BUNDLE_FOOTPRINT_FILE_TYPE_BLOCKMAP, "block map", true },
    {APPX_BUNDLE_FOOTPRINT_FILE_TYPE_SIGNATURE, "digital signature", true },
};

//
// Helper function to create a writable IStream over a file with the specified name
// under the specified path.  This function will also create intermediate
// subdirectories if necessary.
//
// Parameters:
// path - Path of the folder containing the file to be opened.  This should NOT
//        end with a slash ('\') character.
// fileName - Name, not including path, of the file to be opened
// stream - Output parameter pointing to the created instance of IStream over
//          the specified file when this function succeeds.
//
HRESULT GetOutputStream(LPCWSTR path, LPCWSTR fileName, IStream** stream)
{
    const int MaxFileNameLength = 200;
    #ifdef WIN32
    std::wstring fullFileName = path + std::wstring(L"\\") + fileName;
    #else
    std::wstring fullFileName = path + std::wstring(L"/") + fileName;
    std::replace(fullFileName.begin(), fullFileName.end(), '\\', '/' );
    #endif

    RETURN_IF_FAILED(HRESULT_FROM_WIN32(mkdirp(fullFileName)));
    // Create stream for writing the file
    RETURN_IF_FAILED(CreateStreamOnFileUTF16(fullFileName.c_str(), false, stream));
    return S_OK;
}

//
// Creates a cross-plat app package.
//
// Parameters:
//   state
//     Contains the information for package name and validation options.
//   reader
//     On success, receives the created instance of IAppxPackageReader.
//
HRESULT GetPackageReader(State& state, IAppxPackageReader** package)
{
    ComPtr<IAppxFactory> appxFactory;
    ComPtr<IStream> inputStream;
    *package = nullptr;
    RETURN_IF_FAILED(CreateStreamOnFileUTF16(state.packageName.c_str(), true, &inputStream));

    // On Win32 platforms CoCreateAppxFactory defaults to CoTaskMemAlloc/CoTaskMemFree
    // On non-Win32 platforms CoCreateAppxFactory will return 0x80070032 (e.g. HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED))
    // So on all platforms, it's always safe to call CoCreateAppxFactoryWithHeap, just be sure to bring your own heap!
    RETURN_IF_FAILED(CoCreateAppxFactoryWithHeap(
            MyAllocate,
            MyFree,
            state.validationOptions,
            &appxFactory));

    // Create a new package reader using the factory.  For
    // simplicity, we don't verify the digital signature of the package.
    RETURN_IF_FAILED(appxFactory->CreatePackageReader(inputStream.Get(), package));
    return S_OK;
}

//
// Prints basic info about a footprint or payload file and writes the file to disk.
//
// Parameters:
//   file
//      The IAppxFile interface that represents a footprint or payload file in the package.
//   outputPath
//      The path of the folder for the extracted files.
//
HRESULT ExtractFile(IAppxFile* file, LPCWSTR outputPath)
{
    Text<WCHAR> fileName;
    UINT64 fileSize = 0;
    ComPtr<IStream> fileStream;
    ComPtr<IStream> outputStream;
    ULARGE_INTEGER fileSizeLargeInteger = { 0 };

    // Get basic info about the file
    RETURN_IF_FAILED(file->GetName(&fileName));
    RETURN_IF_FAILED(file->GetSize(&fileSize));
    fileSizeLargeInteger.QuadPart = fileSize;

    std::printf("\tFile name: %s\n" , utf16_to_utf8(fileName.Get()).c_str());
    std::printf("\tSize: %llu bytes\n", fileSize);
    APPX_COMPRESSION_OPTION compressionOpt;
    RETURN_IF_FAILED(file->GetCompressionOption(&compressionOpt));
    std::printf("\tCompression option: %d\n\n", compressionOpt);

    // Write the file to disk
    RETURN_IF_FAILED(file->GetStream(&fileStream));
    RETURN_IF_FAILED(GetOutputStream(outputPath, fileName.Get(), &outputStream));
    RETURN_IF_FAILED(fileStream->CopyTo(outputStream.Get(), fileSizeLargeInteger, nullptr, nullptr));
    return S_OK;
}

//
// Extracts all footprint files from a package.
//
// Parameters:
//   package
//      The package reader for the app package.
//   outputPath
//      The path of the folder for the extracted footprint files.
//
HRESULT ExtractFootprintFiles(IAppxPackageReader* package, LPCWSTR outputPath)
{
    std::printf("Extracting footprint files from the package...\n");
    for (int i = 0; i < FootprintFilesCount; i++)
    {
        ComPtr<IAppxFile> footprintFile;
        HRESULT hr = package->GetFootprintFile(footprintFilesType[i].fileType, &footprintFile);
        if (SUCCEEDED(hr) && footprintFile.Get())
        {
            RETURN_IF_FAILED(ExtractFile(footprintFile.Get(), outputPath));
        }
        else if (footprintFilesType[i].isRequired)
        {
            std::printf("The package does not contain a %s.\n", footprintFilesType[i].description);
            return hr;
        }
    }
    return S_OK;
}

//
// Extracts all payload files from a package.
//
// Parameters:
//   package
//      The package reader for the app package.
//   outputPath
//      The path of the folder for the extracted payload files.
//
HRESULT ExtractPayloadFiles(IAppxPackageReader* package, LPCWSTR outputPath)
{
    ComPtr<IAppxFilesEnumerator> files;
    std::printf("Extracting payload files from the package...\n");

    // Get an enumerator of all payload files from the package reader and iterate
    // through all files.
    RETURN_IF_FAILED(package->GetPayloadFiles(&files));

    BOOL hasCurrent = FALSE;
    RETURN_IF_FAILED(files->GetHasCurrent(&hasCurrent));

    while (hasCurrent)
    {
        ComPtr<IAppxFile> file;
        RETURN_IF_FAILED(files->GetCurrent(&file));
        RETURN_IF_FAILED(ExtractFile(file.Get(), outputPath));
        RETURN_IF_FAILED(files->MoveNext(&hasCurrent));
    }
    return S_OK;
}

//
// Extracts all files from a package.
//
// Parameters:
//   package
//      The package reader for the app package.
//   outputPath
//      The path of the folder for the extracted payload files.
//
HRESULT ExtractPackage(IAppxPackageReader* package, LPCWSTR outputPath)
{
    // Print information about all footprint files, and extract them to disk
    RETURN_IF_FAILED(ExtractFootprintFiles(package, outputPath));
    // Print information about all payload files, and extract them to disk
    RETURN_IF_FAILED(ExtractPayloadFiles(package, outputPath));
    return S_OK;
}

//
// Extracts all footprint files from a bundle.
//
// Parameters:
//   bundle
//      The bundle reader for the app bundle.
//   outputPath
//      The path of the folder for the extracted footprint files.
//
HRESULT ExtractFootprintFilesForBundle(IAppxBundleReader* bundle, LPCWSTR outputPath)
{
    std::printf("Extracting footprint files from the bundle...\n");
    for (int i = 0; i < BundleFootprintFilesCount; i++)
    {
        ComPtr<IAppxFile> footprintFile;
        HRESULT hr = bundle->GetFootprintFile(bundleFootprintFilesType[i].fileType, &footprintFile);
        if (SUCCEEDED(hr) && footprintFile.Get())
        {
            RETURN_IF_FAILED(ExtractFile(footprintFile.Get(), outputPath));
        }
        else if (footprintFilesType[i].isRequired)
        {
            std::printf("The package does not contain a %s.\n", footprintFilesType[i].description);
            return hr;
        }
    }
    return S_OK;
}

//
// Extracts all payload packages from a bundle.
//
// Parameters:
//   bundle
//      The bundle reader for the app bundle.
//   outputPath
//      The path of the folder for the extracted payload packages.
HRESULT ExtractPayloadPackages(IAppxBundleReader* bundle, State& state)
{
    ComPtr<IAppxFilesEnumerator> packages;
    std::printf("Extracting payload files from the package...\n");

    // Get an enumerator of all payload packages from the bundle reader and iterate through all files.
    RETURN_IF_FAILED(bundle->GetPayloadPackages(&packages));
    BOOL hasCurrent = FALSE;
    RETURN_IF_FAILED(packages->GetHasCurrent(&hasCurrent));
    while (hasCurrent)
    {
        ComPtr<IAppxFile> package;
        RETURN_IF_FAILED(packages->GetCurrent(&package));
        if (state.recursiveUnpack)
        {
            ComPtr<IStream> packageStream;
            ComPtr<IAppxPackageReader> packageReader;
            Text<WCHAR> fileName;
            RETURN_IF_FAILED(package->GetStream(&packageStream));
            RETURN_IF_FAILED(package->GetName(&fileName));

            std::printf("Extracting package from bundle: %s\n" , utf16_to_utf8(fileName.Get()).c_str());
            // Pass MSIX_PACKUNPACK_OPTION_CREATEPACKAGESUBFOLDER to avoid overwritting files.
            RETURN_IF_FAILED(UnpackPackageFromStream(MSIX_PACKUNPACK_OPTION::MSIX_PACKUNPACK_OPTION_CREATEPACKAGESUBFOLDER,
                            state.validationOptions, packageStream.Get(), const_cast<char*>(utf16_to_utf8(state.directoryName).c_str())));
        }
        else
        {
            RETURN_IF_FAILED(ExtractFile(package.Get(), state.directoryName.c_str()));
        }
        RETURN_IF_FAILED(packages->MoveNext(&hasCurrent));
    }
    return S_OK;
}

//
// Extracts all files from a bundle.
//
// Parameters:
//   bundle
//      The package reader for the app package.
//   outputPath
//      The path of the folder for the extracted payload files.
//
HRESULT ExtractBundle(IAppxBundleReader* bundle, State& state)
{
    // Print information about all footprint files, and extract them to disk
    RETURN_IF_FAILED(ExtractFootprintFilesForBundle(bundle, state.directoryName.c_str()));
    // Print information about all payload bundles, and extract them to disk
    RETURN_IF_FAILED(ExtractPayloadPackages(bundle, state));
    return S_OK;
}

// Parses argc/argv input via commands into state, and extract the package.
int ParseAndRun(std::map<std::string, Option>& options, State& state, int argc, char* argv[])
{
    auto ParseInput = [&]()->bool {
        int index = 1;
        while (index < argc)
        {
            auto option = options.find(argv[index]);
            if (option == options.end()) { return false; }
            char const *parameter = "";
            if (option->second.TakesParameter)
            {
                if (++index == argc) { break; }
                parameter = argv[index];
            }
            if (!option->second.Callback(parameter)) { return false; }
            ++index;
        }
        return true;
    };

    if (!ParseInput()) { return Help(argv[0], options); }
    if (state.packageName.empty() || state.directoryName.empty())
    {   Error(argv[0]);
        return -1;
    }

    HRESULT hr = S_OK;
    // Create a package using the file name in argv[1]
    ComPtr<IAppxPackageReader> package;
    hr = GetPackageReader(state, &package);
    // Print information about all footprint files, and extract them to disk
    if (SUCCEEDED(hr))
    {
        // See if the package is a bundle;
        ComPtr<IAppxBundleReader> bundle;
        hr = package->QueryInterface(UuidOfImpl<IAppxBundleReader>::iid, reinterpret_cast<void**>(&bundle));
        if(SUCCEEDED(hr))
        {
            // This is an bundle
            hr = ExtractBundle(bundle.Get(), state);
        }
        else if(hr == E_NOINTERFACE)
        {
            // This is a package
            hr = ExtractPackage(package.Get(), state.directoryName.c_str());
        }
    }
    return static_cast<int>(hr);
}

int main(int argc, char* argv[])
{
    State state;
    std::map<std::string, Option> options = {
        { "-p", Option(true, "REQUIRED, specify input package name.",
        [&](const std::string& name) { return state.SetPackageName(name); })
        },
        { "-d", Option(true, "REQUIRED, specify output directory name.",
            [&](const std::string& name) { return state.SetDirectoryName(name); })
        },
        { "-sv", Option(false, "Skips signature validation.  By default signature validation is enabled.",
            [&](const std::string&) { return state.AllowSignatureOriginUnknown(); })
        },
        { "-ss", Option(false, "Skips enforcement of signed packages.  By default packages must be signed.",
            [&](const std::string&)
            {   footprintFilesType[2].isRequired = false;
                bundleFootprintFilesType[2].isRequired = false;
                return state.SkipSignature();
            })
        },
        { "-r", Option(false, "Recursive unpacking (only applicable for bundles). By default this is turn off.",
            [&](const std::string&) { return state.SetRecursiveUnpack(); })
        },
        { "-?", Option(false, "Displays this help text.",
            [&](const std::string&) { return false; })
        }
    };

    auto result = ParseAndRun(options, state, argc, argv);
    if (result != 0)
    {
        std::cout << "Error: " << std::hex << result << " while extracting the appx package" <<std::endl;
        Text<char> text;
        auto logResult = GetLogTextUTF8(MyAllocate, &text);
        if (0 == logResult)
        {   std::cout << "LOG:" << std::endl << text.content << std::endl;
        }
        else
        {   std::cout << "UNABLE TO GET LOG WITH HR=" << std::hex << logResult << std::endl;
        }
    }
    return result;
}
