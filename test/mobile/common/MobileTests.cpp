#include <cstdlib>
#include <string>
#include <codecvt>
#include <locale>
#include <sys/stat.h>
#include <stdio.h>
#include <dirent.h>
#include <iostream>
#include <fstream>

#include "MobileTests.hpp"

// Used for test results
bool g_TestFailed = false;

std::string utf16_to_utf8(const std::wstring& utf16string)
{
    auto converted = std::wstring_convert<std::codecvt_utf8<wchar_t>>{}.to_bytes(utf16string.data());
    std::string result(converted.begin(), converted.end());
    return result;
}

std::wstring utf8_to_utf16(const std::string& utf8string)
{
    auto converted = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(utf8string.data());
    std::wstring result(converted.begin(), converted.end());
    return result;
}

// not all POSIX implementations provide an implementation of mkdirp
int mkdirp(std::wstring& utf16Path)
{
	std::string utf8Path = utf16_to_utf8(utf16Path);
	auto lastSlash = utf8Path.find_last_of("/");
	std::string path = utf8Path.substr(0, lastSlash);
	char* p = const_cast<char*>(path.c_str());
	if (*p == '/') { p++; }
	while (*p != '\0')
	{   while (*p != '\0' && *p != '/') { p++; }
		char v = *p;
		*p = '\0';
		if (-1 == mkdir(path.c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) && errno != EEXIST)
		{   return errno;
		}
		*p = v;
		p++;
	}
	return 0;
}

// Tracks the state of the current parse operation as well as implements input validation
struct State
{
    bool CreatePackageSubfolder()
    {
        unpackOptions = static_cast<APPX_PACKUNPACK_OPTION>(unpackOptions | APPX_PACKUNPACK_OPTION::APPX_PACKUNPACK_OPTION_CREATEPACKAGESUBFOLDER);
        return true;
    }

    bool SetValidationOptions(APPX_VALIDATION_OPTION flags)
    {
        validationOptions = static_cast<APPX_VALIDATION_OPTION>(validationOptions | flags);
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

// Cleans a directory
void RemoveContent(std::string subPath)
{
    DIR *dir;
    if ((dir = opendir(subPath.data())))
    {   struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) 
        {   if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
            {   std::string path = subPath + "/" + std::string(entry->d_name);
                if (entry->d_type == DT_DIR) { RemoveContent(path.data());}
                std::remove(path.data());
            }
        }
        closedir(dir);
    }
}

// Types of footprint files in an app package
struct FootprintFilesType
{
    APPX_FOOTPRINT_FILE_TYPE fileType;
    const char* description;
    bool isRequired;
};

const int FootprintFilesCount = 4;
FootprintFilesType footprintFilesType[FootprintFilesCount] = {
    {APPX_FOOTPRINT_FILE_TYPE_MANIFEST, "manifest", true },
    {APPX_FOOTPRINT_FILE_TYPE_BLOCKMAP, "block map", true },
    {APPX_FOOTPRINT_FILE_TYPE_SIGNATURE, "digital signature", true },
    {APPX_FOOTPRINT_FILE_TYPE_CODEINTEGRITY, "CI catalog", false }, // this is ONLY required iff there exists 1+ PEs 
};

HRESULT GetOutputStream(LPCWSTR path, LPCWSTR fileName, IStream** stream)
{
    HRESULT hr = S_OK;
    const int MaxFileNameLength = 200;
    std::wstring fullFileName = path + std::wstring(L"/") + fileName;
    std::replace(fullFileName.begin(), fullFileName.end(), '\\', '/' );

    hr = HRESULT_FROM_WIN32(mkdirp(fullFileName));
    // Create stream for writing the file
    if (SUCCEEDED(hr))
    {   hr = CreateStreamOnFileUTF16(fullFileName.c_str(), false, stream);
    }
    return hr;
}

HRESULT ExtractFile(IAppxFile* file, LPCWSTR outputPath)
{
    HRESULT hr = S_OK;
    LPWSTR fileName = nullptr;
    LPWSTR contentType = nullptr;
    UINT64 fileSize = 0;
    ComPtr<IStream> fileStream;
    ComPtr<IStream> outputStream;
    ULARGE_INTEGER fileSizeLargeInteger = { 0 };

    hr = file->GetName(&fileName);
    if (SUCCEEDED(hr))
    {   hr = file->GetContentType(&contentType);
    }
    if (SUCCEEDED(hr))
    {   hr = file->GetSize(&fileSize);
        fileSizeLargeInteger.QuadPart = fileSize;
    }

    // Write the file to disk
    if (SUCCEEDED(hr))
    {   hr = file->GetStream(&fileStream);
    }
    if (SUCCEEDED(hr))
    {   hr = GetOutputStream(outputPath, fileName, &outputStream);
    }
    if (SUCCEEDED(hr))
    {   hr = fileStream->CopyTo(outputStream.Get(), fileSizeLargeInteger, nullptr, nullptr);
    }

    // You must free string buffers obtained from the packaging APIs,
    // the heap that you use is specified via CoCreateAppxFactoryWithHeap
    std::free(fileName);
    std::free(contentType);
    return hr;
}

HRESULT ExtractFootprintFiles(IAppxPackageReader* package, LPCWSTR outputPath)
{
    HRESULT hr = S_OK;
    for (int i = 0; SUCCEEDED(hr) && (i < FootprintFilesCount); i++)
    {   ComPtr<IAppxFile> footprintFile;
        hr = package->GetFootprintFile(footprintFilesType[i].fileType, &footprintFile);
        if (SUCCEEDED(hr) && footprintFile.Get())
        {   hr = ExtractFile(footprintFile.Get(), outputPath);
        }
        else if (!footprintFilesType[i].isRequired)
        {   hr = S_OK;
        }
    }
    return hr;
}

HRESULT ExtractPayloadFiles(IAppxPackageReader* package, LPCWSTR outputPath)
{
    HRESULT hr = S_OK;
    ComPtr<IAppxFilesEnumerator> files;
    hr = package->GetPayloadFiles(&files);
    if (SUCCEEDED(hr))
    {   BOOL hasCurrent = FALSE;
        hr = files->GetHasCurrent(&hasCurrent);
        while (SUCCEEDED(hr) && hasCurrent)
        {   ComPtr<IAppxFile> file;
            hr = files->GetCurrent(&file);
            if (SUCCEEDED(hr))
            {   hr = ExtractFile(file.Get(), outputPath);
            }
            if (SUCCEEDED(hr))
            {   hr = files->MoveNext(&hasCurrent);
            }
        }
    }
    return hr;
}

// allocator/deallocator for non-Windows
LPVOID STDMETHODCALLTYPE MyAllocate(SIZE_T cb)  { return std::malloc(cb); }
void STDMETHODCALLTYPE MyFree(LPVOID pv)        { std::free(pv); }

class Text
{
public:
    char** operator&() { return &content; }
    ~Text() { Cleanup(); }

    char* content = nullptr;
protected:
    void Cleanup() { if (content) { std::free(content); content = nullptr; } }
};

HRESULT GetPackageReader(State& state, IAppxPackageReader** package)
{
    HRESULT hr = S_OK;
    ComPtr<IAppxFactory> appxFactory;
    ComPtr<IStream> inputStream;

    hr = CreateStreamOnFileUTF16(state.packageName.c_str(), true, &inputStream);
    if (SUCCEEDED(hr))
    {   hr = CoCreateAppxFactoryWithHeap(MyAllocate, MyFree, state.validationOptions, &appxFactory);
        if (SUCCEEDED(hr))
        {   hr = appxFactory->CreatePackageReader(inputStream.Get(), package);
        }
    }

    return hr;
}

HRESULT RunTest(std::string packageName, std::string unpackFolder, APPX_VALIDATION_OPTION flags, HRESULT expectedResult)
{
    HRESULT hr = S_OK;
    State state;
    state.SetPackageName(packageName);
    state.SetDirectoryName(unpackFolder);
    state.SetValidationOptions(flags);
    RemoveContent(unpackFolder);

    // Signature is not required for this test
    if(flags & APPX_VALIDATION_OPTION::APPX_VALIDATION_OPTION_SKIPSIGNATURE)
    {   footprintFilesType[2].isRequired = false;
    }

    std::cout << "------------------------------------------------------" << std::endl;
    std::cout << "Package: " << packageName << std::endl;
    std::cout << "Validation Options: " << std::hex << flags << std::endl;

    ComPtr<IAppxPackageReader> package;
    hr = GetPackageReader(state, &package);
    if(SUCCEEDED(hr))
    {   hr = ExtractFootprintFiles(package.Get(), state.directoryName.c_str());
    }
    if (SUCCEEDED(hr))
    {   hr = ExtractPayloadFiles(package.Get(), state.directoryName.c_str());
    }

    if(FAILED(hr))
    {
        std::cout << "Error: " << std::hex << hr << std::endl;
        Text text;
        auto logResult = GetLogTextUTF8(MyAllocate, &text);
        if (0 == logResult)
        {   std::cout << "LOG:" << std::endl << text.content << std::endl;
        }
        else
        {   std::cout << "UNABLE TO GET LOG WITH HR=" << std::hex << logResult << std::endl;
        }
    }

    std::cout << "Expected: " << std::hex << expectedResult << ", Got: " << std::hex << hr << std::endl;
    if(expectedResult == hr)
    {   std::cout << "Succeeded" << std::endl;
    }
    else
    {   std::cout << "Failed" << std::endl;
        g_TestFailed = false;
    }

    // Clean up
    if(flags & APPX_VALIDATION_OPTION::APPX_VALIDATION_OPTION_SKIPSIGNATURE)
    {   footprintFilesType[2].isRequired = true;
    }

    return hr;
}

HRESULT RunTests(std::string path)
{
    HRESULT hr = S_OK;

    // Create output directory
    std::string unpackFolder = path + "unpack";
    if (-1 == mkdir(unpackFolder.c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) && errno != EEXIST)
    {   return HRESULT_FROM_WIN32(errno);
    }

    std::ofstream results(path + "testResults.txt");
    auto oldcout = std::cout.rdbuf(results.rdbuf());

    // Reference from other tests
    // -sv = APPX_VALIDATION_OPTION::APPX_VALIDATION_OPTION_ALLOWSIGNATUREORIGINUNKNOWN
    // -ss = APPX_VALIDATION_OPTION::APPX_VALIDATION_OPTION_SKIPSIGNATURE

    hr = RunTest(path + "Empty.appx", unpackFolder, APPX_VALIDATION_OPTION::APPX_VALIDATION_OPTION_ALLOWSIGNATUREORIGINUNKNOWN, 0x8bad0002);
    hr = RunTest(path + "HelloWorld.appx", unpackFolder, APPX_VALIDATION_OPTION::APPX_VALIDATION_OPTION_SKIPSIGNATURE, S_OK);
    hr = RunTest(path + "SignatureNotLastPart-ERROR_BAD_FORMAT.appx", unpackFolder, APPX_VALIDATION_OPTION::APPX_VALIDATION_OPTION_FULL, 0x8bad0042);
    hr = RunTest(path + "SignedTamperedBlockMap-TRUST_E_BAD_DIGEST.appx", unpackFolder, APPX_VALIDATION_OPTION::APPX_VALIDATION_OPTION_FULL, 0x8bad0042);
    hr = RunTest(path + "SignedTamperedBlockMap-TRUST_E_BAD_DIGEST.appx", unpackFolder, APPX_VALIDATION_OPTION::APPX_VALIDATION_OPTION_ALLOWSIGNATUREORIGINUNKNOWN, 0x8bad0041);
    hr = RunTest(path + "SignedTamperedCD-TRUST_E_BAD_DIGEST.appx", unpackFolder, APPX_VALIDATION_OPTION::APPX_VALIDATION_OPTION_FULL, 0x8bad0042);
    hr = RunTest(path + "SignedTamperedCodeIntegrity-TRUST_E_BAD_DIGEST.appx", unpackFolder, APPX_VALIDATION_OPTION::APPX_VALIDATION_OPTION_FULL, 0x8bad0042);
    hr = RunTest(path + "SignedTamperedContentTypes-TRUST_E_BAD_DIGEST.appx", unpackFolder, APPX_VALIDATION_OPTION::APPX_VALIDATION_OPTION_FULL, 0x8bad0042);
    hr = RunTest(path + "SignedUntrustedCert-CERT_E_CHAINING.appx", unpackFolder, APPX_VALIDATION_OPTION::APPX_VALIDATION_OPTION_FULL, 0x8bad0042);
    hr = RunTest(path + "StoreSigned_Desktop_x64_MoviesTV.appx", unpackFolder, APPX_VALIDATION_OPTION::APPX_VALIDATION_OPTION_FULL, S_OK);
    hr = RunTest(path + "TestAppxPackage_Win32.appx", unpackFolder, APPX_VALIDATION_OPTION::APPX_VALIDATION_OPTION_SKIPSIGNATURE, S_OK);
    hr = RunTest(path + "TestAppxPackage_x64.appx", unpackFolder, APPX_VALIDATION_OPTION::APPX_VALIDATION_OPTION_SKIPSIGNATURE, S_OK);
    hr = RunTest(path + "UnsignedZip64WithCI-APPX_E_MISSING_REQUIRED_FILE.appx", unpackFolder, APPX_VALIDATION_OPTION::APPX_VALIDATION_OPTION_FULL, 0x8bad0012);
    hr = RunTest(path + "FileDoesNotExist.appx", unpackFolder, APPX_VALIDATION_OPTION::APPX_VALIDATION_OPTION_SKIPSIGNATURE, 0x8bad0001);
    hr = RunTest(path + "BlockMap/Missing_Manifest_in_blockmap.appx", unpackFolder, APPX_VALIDATION_OPTION::APPX_VALIDATION_OPTION_SKIPSIGNATURE, 0x8bad0051);
    hr = RunTest(path + "BlockMap/ContentTypes_in_blockmap.appx", unpackFolder, APPX_VALIDATION_OPTION::APPX_VALIDATION_OPTION_SKIPSIGNATURE, 0x8bad0051);
    hr = RunTest(path + "BlockMap/Invalid_Bad_Block.appx", unpackFolder, APPX_VALIDATION_OPTION::APPX_VALIDATION_OPTION_SKIPSIGNATURE, 0x8bad0041);
    hr = RunTest(path + "BlockMap/Size_wrong_uncompressed.appx", unpackFolder, APPX_VALIDATION_OPTION::APPX_VALIDATION_OPTION_SKIPSIGNATURE, S_OK);
    hr = RunTest(path + "BlockMap/HelloWorld.appx", unpackFolder, APPX_VALIDATION_OPTION::APPX_VALIDATION_OPTION_SKIPSIGNATURE, S_OK);
    hr = RunTest(path + "BlockMap/Extra_file_in_blockmap.appx", unpackFolder, APPX_VALIDATION_OPTION::APPX_VALIDATION_OPTION_SKIPSIGNATURE, 0x80070002);
    hr = RunTest(path + "BlockMap/File_missing_from_blockmap.appx", unpackFolder, APPX_VALIDATION_OPTION::APPX_VALIDATION_OPTION_SKIPSIGNATURE, 0x8bad0051);
    hr = RunTest(path + "BlockMap/No_blockmap.appx", unpackFolder, APPX_VALIDATION_OPTION::APPX_VALIDATION_OPTION_SKIPSIGNATURE, 0x80070002);
    hr = RunTest(path + "BlockMap/Bad_Namespace_Blockmap.appx", unpackFolder, APPX_VALIDATION_OPTION::APPX_VALIDATION_OPTION_SKIPSIGNATURE, 0x8bad1003);
    hr = RunTest(path + "BlockMap/Duplicate_file_in_blockmap.appx", unpackFolder, APPX_VALIDATION_OPTION::APPX_VALIDATION_OPTION_SKIPSIGNATURE, 0x8bad0051);

    std::cout << "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=" << std::endl;
    if(g_TestFailed)
    {   std::cout << "                           FAILED                                 " << std::endl;
    }
    else
    {   std::cout << "                           passed                                 " << std::endl;
    }

    std::cout.rdbuf(oldcout);

    return S_OK;
}