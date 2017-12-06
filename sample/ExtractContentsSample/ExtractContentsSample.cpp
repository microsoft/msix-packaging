#include <cstdlib>
#include <cwchar> 
#include <string>
#include <locale>
#include <codecvt>
#include <iostream>
#include <iomanip>
#include <functional>
#include <map>

#ifdef WIN32
    #define UNICODE
    #include <windows.h>
#else
    // required posix-specific headers
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <unistd.h>
#endif

#include "AppxPackaging.hpp"
#include "AppxWindows.hpp"

// Stripped down ComPtr provided for those platforms that do not already have a ComPtr class.
template <class T>
class ComPtr
{
public:
    // default ctor
    ComPtr() = default;
    ComPtr(T* ptr) : m_ptr(ptr) { InternalAddRef(); }

    ~ComPtr() { InternalRelease(); }
    inline T* operator->() const { return m_ptr; }
    inline T* Get() const { return m_ptr; }

    inline T** operator&()
    {   InternalRelease();
        return &m_ptr;
    }

protected:
    T* m_ptr = nullptr;

    inline void InternalAddRef() { if (m_ptr) { m_ptr->AddRef(); } }
    inline void InternalRelease()
    {   
        T* temp = m_ptr;
        if (temp)
        {   m_ptr = nullptr;
            temp->Release();
        }
    }    
};

std::string utf16_to_utf8(const std::wstring& utf16string)
{
    auto converted = std::wstring_convert<std::codecvt_utf8<wchar_t>>{}.to_bytes(utf16string.data());
    std::string result(converted.begin(), converted.end());
    return result;
}

std::wstring utf8_to_utf16(const std::string& utf8string)
{
    // see https://connect.microsoft.com/VisualStudio/feedback/details/1403302/unresolved-external-when-using-codecvt-utf8
    #ifdef WIN32
    auto converted = std::wstring_convert<std::codecvt_utf8_utf16<unsigned short>, unsigned short>{}.from_bytes(utf8string.data());
    #else
    auto converted = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(utf8string.data()); 
    #endif
    std::wstring result(converted.begin(), converted.end());
    return result;
}

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
    bool CreatePackageSubfolder()
    {
        unpackOptions = static_cast<APPX_PACKUNPACK_OPTION>(unpackOptions | APPX_PACKUNPACK_OPTION::APPX_PACKUNPACK_OPTION_CREATEPACKAGESUBFOLDER);
        return true;
    }

    bool SkipManifestValidation()
    {
        validationOptions = static_cast<APPX_VALIDATION_OPTION>(validationOptions | APPX_VALIDATION_OPTION::APPX_VALIDATION_OPTION_SKIPAPPXMANIFEST);
        return true;
    }

    bool SkipSignature()
    {
        validationOptions = static_cast<APPX_VALIDATION_OPTION>(validationOptions | APPX_VALIDATION_OPTION::APPX_VALIDATION_OPTION_SKIPSIGNATURE);
        return true;
    }

    bool AllowSignatureOriginUnknown()
    {
        validationOptions = static_cast<APPX_VALIDATION_OPTION>(validationOptions | APPX_VALIDATION_OPTION::APPX_VALIDATION_OPTION_ALLOWSIGNATUREORIGINUNKNOWN);
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

    std::wstring packageName;
    std::wstring directoryName;
    APPX_VALIDATION_OPTION validationOptions = APPX_VALIDATION_OPTION::APPX_VALIDATION_OPTION_FULL;
    APPX_PACKUNPACK_OPTION unpackOptions     = APPX_PACKUNPACK_OPTION::APPX_PACKUNPACK_OPTION_NONE;
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

#ifdef WIN32
    // TODO: paths coming in SHOULD have platform-appropriate path separators
    void replace(std::wstring& input, const wchar_t oldchar, const wchar_t newchar)
    {
        std::size_t found = input.find_first_of(oldchar);
        while (found != std::string::npos)
        {
            input[found] = newchar;
            found = input.find_first_of(oldchar, found+1);
        }
    }

    int mkdirp(std::wstring& utf16Path)
    {
        replace(utf16Path, L'/', L'\\');
        for (std::size_t i = 0; i < utf16Path.size(); i++)
        {
            if (utf16Path[i] == L'\0')
            {
                break;
            }
            else if (utf16Path[i] == L'\\') 
            {
                // Temporarily set string to terminate at the '\' character
                // to obtain name of the subdirectory to create
                utf16Path[i] = L'\0';

                if (!CreateDirectory(utf16Path.c_str(), nullptr))
                {
                    int lastError = static_cast<int>(GetLastError());

                    // It is normal for CreateDirectory to fail if the subdirectory
                    // already exists.  Other errors should not be ignored.
                    if (lastError != ERROR_ALREADY_EXISTS)
                    {
                        return lastError;
                    }
                }
                // Restore original string
                utf16Path[i] = L'\\'; /* TODO: paths coming in SHOULD have platform-appropriate path separators */
            }
        }   
        return 0;     
    }
#else     
    // not all POSIX implementations provide an implementation of mkdirp
    int mkdirp(std::wstring& utf16Path)
    {
        std::string utf8Path = utf16_to_utf8(utf16Path);
        auto lastSlash = utf8Path.find_last_of("/");
        std::string path = utf8Path.substr(0, lastSlash);
        char* p = const_cast<char*>(path.c_str());
        if (*p == '/') { p++; }
        while (*p != '\0')
        {
            while (*p != '\0' && *p != '/') { p++; }
            
            char v = *p;
            *p = '\0';
            if (-1 == mkdir(path.c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) && errno != EEXIST)
            {
                return errno;
            }
            *p = v;
            p++;
        }
        return 0;
    }
#endif

struct FootprintFilesType
{
    APPX_FOOTPRINT_FILE_TYPE fileType;
    const char* description;
    bool isRequired;
};

// Types of footprint files in an app package
const int FootprintFilesCount = 4;
FootprintFilesType footprintFilesType[FootprintFilesCount] = {
    {APPX_FOOTPRINT_FILE_TYPE_MANIFEST, "manifest", true },
    {APPX_FOOTPRINT_FILE_TYPE_BLOCKMAP, "block map", true },
    {APPX_FOOTPRINT_FILE_TYPE_SIGNATURE, "digital signature", true },
    {APPX_FOOTPRINT_FILE_TYPE_CODEINTEGRITY, "CI catalog", false }, // this is ONLY required iff there exists 1+ PEs 
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
    HRESULT hr = S_OK;
    const int MaxFileNameLength = 200;
    #ifdef WIN32
    std::wstring fullFileName = path + std::wstring(L"\\") + fileName;
    #else
    std::wstring fullFileName = path + std::wstring(L"/") + fileName;
    #endif

    hr = HRESULT_FROM_WIN32(mkdirp(fullFileName));
    // Create stream for writing the file
    if (SUCCEEDED(hr))
    {
        hr = CreateStreamOnFileUTF16(fullFileName.c_str(), false, stream);
    }
    return hr;
}

// Or you can use what-ever allocator/deallocator is best for your platform...
LPVOID STDMETHODCALLTYPE MyAllocate(SIZE_T cb)  { return std::malloc(cb); }
void STDMETHODCALLTYPE MyFree(LPVOID pv)        { std::free(pv); }

//
// Creates a cross-plat app package.
//
// Parameters:
//   inputFileName  
//     The fully-qualified name of the app package (.appx file) to be opened.
//   reader 
//     On success, receives the created instance of IAppxPackageReader.
//
HRESULT GetPackageReader(State& state, IAppxPackageReader** package)
{
    HRESULT hr = S_OK;
    ComPtr<IAppxFactory> appxFactory;
    ComPtr<IStream> inputStream;

    hr = CreateStreamOnFileUTF16(state.packageName.c_str(), true, &inputStream);
    if (SUCCEEDED(hr))
    {
        // On Win32 platforms CoCreateAppxFactory defaults to CoTaskMemAlloc/CoTaskMemFree
        // On non-Win32 platforms CoCreateAppxFactory will return 0x80070032 (e.g. HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED))
        // So on all platforms, it's always safe to call CoCreateAppxFactoryWithHeap, just be sure to bring your own heap!
        hr = CoCreateAppxFactoryWithHeap(
            MyAllocate,
            MyFree,
            state.validationOptions,
            &appxFactory);

        // Create a new package reader using the factory.  For 
        // simplicity, we don't verify the digital signature of the package.
        if (SUCCEEDED(hr))
        {
            hr = appxFactory->CreatePackageReader(inputStream.Get(), package);
        }
    }

    return hr;
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
        std::printf("\nFile name: %s\n" , utf16_to_utf8(fileName).c_str());
        std::printf("Content type: %s\n", utf16_to_utf8(contentType).c_str());
        std::printf("Size: %llu bytes\n", fileSize);
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

    // You must free string buffers obtained from the packaging APIs,
    // the heap that you use is specified via CoCreateAppxFactoryWithHeap
    std::free(fileName);
    std::free(contentType);
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
HRESULT ExtractFootprintFiles(IAppxPackageReader* package, LPCWSTR outputPath)
{
    HRESULT hr = S_OK;
    std::printf("\nExtracting footprint files from the package...\n");

    for (int i = 0; SUCCEEDED(hr) && (i < FootprintFilesCount); i++)
    {
        ComPtr<IAppxFile> footprintFile;
        hr = package->GetFootprintFile(footprintFilesType[i].fileType, &footprintFile);
        if (SUCCEEDED(hr) && footprintFile.Get())
        {
            hr = ExtractFile(footprintFile.Get(), outputPath);
        }
        else if (footprintFilesType[i].isRequired)
        {
            std::printf("The package does not contain a %s.\n", footprintFilesType[i].description);
        }
        else
        {
            hr = S_OK; // the footprint file was not required.
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
HRESULT ExtractPayloadFiles(IAppxPackageReader* package, LPCWSTR outputPath)
{
    HRESULT hr = S_OK;
    ComPtr<IAppxFilesEnumerator> files;
    std::printf("\nExtracting payload files from the package...\n");

    // Get an enumerator of all payload files from the package reader and iterate
    // through all files.
    hr = package->GetPayloadFiles(&files);

    if (SUCCEEDED(hr))
    {
        BOOL hasCurrent = FALSE;
        hr = files->GetHasCurrent(&hasCurrent);

        while (SUCCEEDED(hr) && hasCurrent)
        {
            ComPtr<IAppxFile> file;
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
        hr = ExtractFootprintFiles(package.Get(), state.directoryName.c_str());
    }

    // Print information about all payload files, and extract them to disk
    if (SUCCEEDED(hr))
    {
        hr = ExtractPayloadFiles(package.Get(), state.directoryName.c_str());
    }
    return static_cast<int>(hr);
}

int main(int argc, char* argv[])
{
    HRESULT hr = S_OK;

    State state;
    std::map<std::string, Option> options = {
        { "-p", Option(true, "REQUIRED, specify input package name.",
        [&](const std::string& name) { return state.SetPackageName(name); })
        },
        { "-d", Option(true, "REQUIRED, specify output directory name.",
            [&](const std::string& name) { return state.SetDirectoryName(name); })
        },
        { "-pfn", Option(false, "Unpacks all files to a subdirectory under the specified output path, named after the package full name.",
            [&](const std::string&) { return state.CreatePackageSubfolder(); })
        },
        { "-mv", Option(false, "Skips manifest validation.  By default manifest validation is enabled.",
            [&](const std::string&) { return state.SkipManifestValidation(); })
        },
        { "-sv", Option(false, "Skips signature validation.  By default signature validation is enabled.",
            [&](const std::string&) { return state.AllowSignatureOriginUnknown(); })
        },
        { "-ss", Option(false, "Skips enforcement of signed packages.  By default packages must be signed.",
            [&](const std::string&) 
            {   footprintFilesType[2].isRequired = false;
                return state.SkipSignature();
            })
        },
        { "-?", Option(false, "Displays this help text.",
            [&](const std::string&) { return false; })
        }
    };

    auto result = ParseAndRun(options, state, argc, argv);
    if (result != 0)
    {
        std::cout << "Error: " << std::hex << result << " while extracting the appx package" <<std::endl;
    }
    return result;
}