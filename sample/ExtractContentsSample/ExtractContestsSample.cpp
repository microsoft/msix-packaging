#include <stdio.h>
#include <windows.h>
#include <strsafe.h>
#include <shlwapi.h>
#include "xPlatAppxPackaging.h"
#include <wrl/client.h>

using namespace Microsoft::WRL;

// Types of footprint files in an app package
const int FootprintFilesCount = 4;
const XPLATAPPX_FOOTPRINT_FILE_TYPE FootprintFilesType[FootprintFilesCount] = {
    XPLATAPPX_FOOTPRINT_FILE_TYPE_MANIFEST,
    XPLATAPPX_FOOTPRINT_FILE_TYPE_BLOCKMAP,
    XPLATAPPX_FOOTPRINT_FILE_TYPE_SIGNATURE,
    XPLATAPPX_FOOTPRINT_FILE_TYPE_CODEINTEGRITY
};
const LPCWSTR FootprintFilesName[FootprintFilesCount] = {
    L"manifest",
    L"block map",
    L"digital signature",
    L"CI catalog"
};

//
// Helper function to create a writable IStream over a file with the specified name
// under the specified path.  This function will also create intermediate
// subdirectories if necessary.  For simplicity, file names including path are
// assumed to be 200 characters or less.  A real application should be able to
// handle longer names and allocate the necessary buffer dynamically.
//
// Parameters:
// path - Path of the folder containing the file to be opened.  This should NOT
//        end with a slash ('\') character.
// fileName - Name, not including path, of the file to be opened
// stream - Output parameter pointing to the created instance of IStream over
//          the specified file when this function succeeds.
//
HRESULT GetOutputStream(_In_ LPCWSTR path, _In_ LPCWSTR fileName, _Outptr_ IStream** stream)
{
    HRESULT hr = S_OK;
    const int MaxFileNameLength = 200;
    WCHAR fullFileName[MaxFileNameLength + 1];

    // Create full file name by concatenating path and fileName
    hr = StringCchCopyW(fullFileName, MaxFileNameLength, path);

    if (SUCCEEDED(hr))
    {
        hr = StringCchCat(fullFileName, MaxFileNameLength, L"\\");
    }
    if (SUCCEEDED(hr))
    {
        hr = StringCchCat(fullFileName, MaxFileNameLength, fileName);
    }

    // Search through fullFileName for the '\' character which denotes
    // subdirectory and create each subdirectory in order of depth.
    for (int i = 0; SUCCEEDED(hr) && (i < MaxFileNameLength); i++)
    {
        if (fullFileName[i] == L'\0')
        {
            break;
        }
        else if (fullFileName[i] == L'\\')
        {
            // Temporarily set string to terminate at the '\' character
            // to obtain name of the subdirectory to create
            fullFileName[i] = L'\0';

            if (!CreateDirectory(fullFileName, nullptr))
            {
                DWORD lastError = GetLastError();

                // It is normal for CreateDirectory to fail if the subdirectory
                // already exists.  Other errors should not be ignored.
                if (lastError != ERROR_ALREADY_EXISTS)
                {
                    hr = HRESULT_FROM_WIN32(lastError);
                }
            }

            // Restore original string
            fullFileName[i] = L'\\';
        }
    }

    // Create stream for writing the file
    if (SUCCEEDED(hr))
    {
        hr = SHCreateStreamOnFileEx(
            fullFileName,
            STGM_CREATE | STGM_WRITE | STGM_SHARE_EXCLUSIVE,
            0, // default file attributes
            TRUE, // create new file if it does not exist
            nullptr, // no template
            stream);
    }
    return hr;
}

//
// Creates a cross-plat app package.
//
// Parameters:
//   inputFileName  
//     The fully-qualified name of the app package (.appx file) to be opened.
//   reader 
//     On success, receives the created instance of IAppxPackageReader.
//
HRESULT GetPackageReader(_In_ LPWSTR inputFileName, _Outptr_ IXplatAppxPackage** package)
{
    HRESULT hr = S_OK;
    ComPtr<IXplatAppxFactory> appxFactory;
    ComPtr<IStream> inputStream;

    // TODO: Get the Factory

    // Create a new package reader using the factory.  For 
    // simplicity, we don't verify the digital signature of the package.
    if (SUCCEEDED(hr))
    {
        hr = appxFactory->CreatePackageFromFile(inputFileName, XPLATAPPX_VALIDATION_OPTION_SKIPAPPXMANIFEST, package);
    }

    return hr;
}

//
// Prints basic info about a footprint or payload file and writes the file to disk.
//
// Parameters:
//   file 
//      The IXplatAppxFile interface that represents a footprint or payload file in the package.
//   outputPath 
//      The path of the folder for the extracted files.
//
HRESULT ExtractFile(_In_ IXplatAppxFile* file, _In_ LPCWSTR outputPath)
{
    HRESULT hr = S_OK;
    LPWSTR fileName = nullptr;
    LPWSTR contentType = nullptr;
    UINT64 fileSize = 0;
    ComPtr<IStream> fileStream;
    ComPtr<IStream> outputStream;
    ULARGE_INTEGER fileSizeLargeInteger = { 0 };

    // Get basic info about the file
    hr = file->GetName(&fileName);

    if (SUCCEEDED(hr))
    {
        hr = file->GetContentType(&contentType);
    }
    if (SUCCEEDED(hr))
    {
        hr = file->GetSize(&fileSize);
        fileSizeLargeInteger.QuadPart = fileSize;
    }
    if (SUCCEEDED(hr))
    {
        wprintf(L"\nFile name: %s\n", fileName);
        wprintf(L"Content type: %s\n", contentType);
        wprintf(L"Size: %llu bytes\n", fileSize);
    }

    // Write the file to disk
    if (SUCCEEDED(hr))
    {
        hr = file->GetStream(&fileStream);
    }
    if (SUCCEEDED(hr))
    {
        hr = GetOutputStream(outputPath, fileName, &outputStream);
    }
    if (SUCCEEDED(hr))
    {
        hr = fileStream->CopyTo(outputStream.Get(), fileSizeLargeInteger, nullptr, nullptr);
    }

    // You must free string buffers obtained from the packaging APIs
    CoTaskMemFree(fileName);
    CoTaskMemFree(contentType);
    return hr;
}

//
// Extracts all footprint files from a package.
//
// Parameters:
//   packageReader 
//      The package reader for the app package.
//   outputPath 
//      The path of the folder for the extracted footprint files.
//
HRESULT ExtractFootprintFiles(_In_ IXplatAppxPackage* package, _In_ LPCWSTR outputPath)
{
    HRESULT hr = S_OK;
    wprintf(L"\nExtracting footprint files from the package...\n");

    for (int i = 0; SUCCEEDED(hr) && (i < FootprintFilesCount); i++)
    {
        ComPtr<IXplatAppxFile> footprintFile;
        hr = package->GetFootprintFile(FootprintFilesType[i], &footprintFile);
        if (footprintFile)
        {
            hr = ExtractFile(footprintFile.Get(), outputPath);
        }
        else
        {
            wprintf(L"\nThe package does not contain a %s.\n", FootprintFilesName[i]);
        }
    }
    return hr;
}

//
// Extracts all payload files from a package.
//
// Parameters:
//   packageReader 
//      The package reader for the app package.
//   outputPath 
//      The path of the folder for the extracted payload files.
//
HRESULT ExtractPayloadFiles(_In_ IXplatAppxPackage* package, _In_ LPCWSTR outputPath)
{
    HRESULT hr = S_OK;
    ComPtr<IEnumXplatAppxFile> files;
    wprintf(L"\nExtracting payload files from the package...\n");

    // Get an enumerator of all payload files from the package reader and iterate
    // through all files.
    hr = package->GetFiles(&files);

    if (SUCCEEDED(hr))
    {
        boolean hasCurrent = FALSE;
        hr = files->GetHasCurrent(&hasCurrent);

        while (SUCCEEDED(hr) && hasCurrent)
        {
            ComPtr<IXplatAppxFile> file;
            hr = files->GetCurrent(&file);
            if (SUCCEEDED(hr))
            {
                hr = ExtractFile(file.Get(), outputPath);
            }
            if (SUCCEEDED(hr))
            {
                hr = files->MoveNext(&hasCurrent);
            }
        }
    }
    return hr;
}

int wmain(_In_ int argc, _In_count_(argc) wchar_t** argv)
{
    HRESULT hr = S_OK;

    // Specify the appropriate COM threading model
    hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);

    if (SUCCEEDED(hr))
    {
        // Create a package using the file name in argv[1] 
        ComPtr<IXplatAppxPackage> package;
        hr = GetPackageReader(argv[1], &package);

        // Print information about all footprint files, and extract them to disk
        if (SUCCEEDED(hr))
        {
            hr = ExtractFootprintFiles(package.Get(), argv[2]);
        }

        // Print information about all payload files, and extract them to disk
        if (SUCCEEDED(hr))
        {
            hr = ExtractPayloadFiles(package.Get(), argv[2]);
        }
    }

    if (FAILED(hr))
    {
        // TODO: Tell a more specific reason why the faiulre occurred. 
        wprintf(L"\nAn error occurred while extracting the appx package\n");
    }

    return 0;
}

