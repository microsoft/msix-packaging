//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
#include "MSIXWindows.hpp"
#include "AppxPackaging.hpp"
#include "MsixErrors.hpp"

#include "ApiTests.hpp"
#include "Verify.hpp"

#include <iostream>
#include <fstream>
#include <map>
#include <functional>
#include <vector>
#include <string>
#include <codecvt>
#include <locale>

#ifndef WIN32
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <unistd.h>
#endif

namespace MsixApiTest {

LPVOID STDMETHODCALLTYPE MyAllocate(SIZE_T cb)  { return std::malloc(cb); }
void   STDMETHODCALLTYPE MyFree(LPVOID pv)      { return std::free(pv);   }

std::string utf16_to_utf8(const std::wstring& utf16string)
{
    auto converted = std::wstring_convert<std::codecvt_utf8<wchar_t>>{}.to_bytes(utf16string.data());
    std::string result(converted.begin(), converted.end());
    return result;
}

std::wstring utf8_to_utf16(const std::string& utf8string)
{
    #ifdef WIN32
    auto converted = std::wstring_convert<std::codecvt_utf8_utf16<unsigned short>, unsigned short>{}.from_bytes(utf8string.data());
    #else
    auto converted = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(utf8string.data());
    #endif
    std::wstring result(converted.begin(), converted.end());
    return result;
}

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

#ifdef WIN32
    int mkdirp(std::string& utf8Path)
    {
        auto utf16Path = utf8_to_utf16(utf8Path);
        // Create subdirectories
        for (std::size_t i = 0; i < utf16Path.size(); i++)
        {
            if (utf16Path[i] == L'\\')
            {
                utf16Path[i] = L'\0';
                if (!CreateDirectory(utf16Path.c_str(), nullptr))
                {
                    int lastError = static_cast<int>(GetLastError());
                    if (lastError != ERROR_ALREADY_EXISTS)
                    {
                        return lastError;
                    }
                }
                utf16Path[i] = L'\\';
            }
        }
        // Create last directory
        if (!CreateDirectory(utf16Path.c_str(), nullptr))
        {
            int lastError = static_cast<int>(GetLastError());
            if (lastError != ERROR_ALREADY_EXISTS)
            {
                return lastError;
            }
        }
        return 0;
    }
#else
    // not all POSIX implementations provide an implementation of mkdirp
    int mkdirp(std::string& utf8Path)
    {
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

// Helper to parse input
class StreamOverrideHelper
{
public:
    StreamOverrideHelper(char* input, char* target)
    {
        if (input != nullptr)
        {
            in = std::ifstream(input);
            std::cin.rdbuf(in.rdbuf());
        }
        if (target != nullptr)
        {
            std::string t(target);
            mkdirp(t);
            #ifdef WIN32
            std::string file = t + "\\testApiResults.txt";
            #else
            std::string file = t + "/testApiResults.txt";
            #endif
            out = std::ofstream(file);
            std::cout.rdbuf(out.rdbuf());
        }
    }
private:
    std::ifstream in;
    std::ofstream out;
};

template<typename T>
T GetInput()
{
    T input;
    std::cin >> input;
    std::cin.ignore();
    return input;
}

template<>
std::string GetInput<std::string>()
{
    std::string input = "";
    while (input.empty()) // ignore black lines
    {
        std::getline(std::cin, input);
    }
    return input;
}

// Helper class to free string buffers obtained from the packaging APIs.
template<typename T>
class Text
{
public:
    T** operator&() { return &content; }
    ~Text() { Cleanup(); }
    T* Get() { return content; }
    std::string ToString();
protected:
    T* content = nullptr;
    void Cleanup() { if (content) { MyFree(content); content = nullptr; } }
};

template<>
std::string Text<char>::ToString() { return std::string(content); }

template<>
std::string Text<wchar_t>::ToString() { return utf16_to_utf8(content); }

// Defines a test
template<typename T>
struct Test
{
    using CBTest = std::function<void(T* obj)>;

    Test(const std::string& d, CBTest c) : description(d), callback(c) {}
    std::string description;
    CBTest callback;
};

// Helper maps for options
std::map<std::string, APPX_COMPRESSION_OPTION> g_compressionMap =
{
    {"compression_none", APPX_COMPRESSION_OPTION_NONE },
    {"compression_normal", APPX_COMPRESSION_OPTION_NORMAL },
    {"compression_maximum", APPX_COMPRESSION_OPTION_MAXIMUM },
    {"compression_fast", APPX_COMPRESSION_OPTION_FAST },
    {"compression_superfast", APPX_COMPRESSION_OPTION_SUPERFAST },
};

std::map<std::string, std::pair<APPX_FOOTPRINT_FILE_TYPE, std::string>> g_footprintFilesMap =
{
    {"manifest", std::make_pair(APPX_FOOTPRINT_FILE_TYPE_MANIFEST, "AppxManifest.xml") },
    {"blockmap", std::make_pair(APPX_FOOTPRINT_FILE_TYPE_BLOCKMAP, "AppxBlockMap.xml") },
    {"signature", std::make_pair(APPX_FOOTPRINT_FILE_TYPE_SIGNATURE, "AppxSignature.p7x") },
    {"codeintegrity", std::make_pair(APPX_FOOTPRINT_FILE_TYPE_CODEINTEGRITY, "AppxMetadata\\CodeIntegrity.cat") },
};

std::map<std::string, std::pair<APPX_BUNDLE_FOOTPRINT_FILE_TYPE, std::string>> g_bundleFootprintFilesMap =
{
    {"manifest", std::make_pair(APPX_BUNDLE_FOOTPRINT_FILE_TYPE_MANIFEST, "AppxMetadata\\AppxBundleManifest.xml") },
    {"blockmap", std::make_pair(APPX_BUNDLE_FOOTPRINT_FILE_TYPE_BLOCKMAP, "AppxBlockMap.xml") },
    {"signature", std::make_pair(APPX_BUNDLE_FOOTPRINT_FILE_TYPE_SIGNATURE, "AppxSignature.p7x") },
};

// Global vector to store the result of each test executed to display it.
std::vector<std::string> g_testRunned;
int g_testResult = 0;
std::string g_packageRootPath;

template<typename T>
void ParseAndRun(std::map<std::string, Test<T>>& options, std::string exit,  T* obj = nullptr)
{
    while(true)
    {
        auto input = GetInput<std::string>();
        if (input == exit) { break; } // We only get out when explicitly asked for
        auto option = options.find(input);
        if (option == options.end())
        {
            std::cout << "Invalid" << std::endl;
            std::cout << "Options:" << std::endl;
            for(auto const& test : options)
            {
                std::cout << "\t" << test.first << " - "<< test.second.description <<std::endl;
            }
        }
        else
        {
            bool passed = false;
            std::cout << "\nStarting: " << input << std::endl;
            try
            {
                option->second.callback(obj);
                passed = true;
            }
            catch(const std::exception& e)
            {
                std::cout << "ERROR: " << e.what() << std::endl;
            }
            catch(...)
            {
                std::cout << "ERROR: unexpected failure" << std::endl;
            }
            if (!passed)
            {
                Text<char> logs;
                auto logResult = GetLogTextUTF8(MyAllocate, &logs);
                if (logResult == 0)
                {
                    std::cout << "LOG:" << std::endl << logs.Get() << std::endl;
                }
                else
                {
                    std::cout << "UNABLE TO GET LOG WITH HR=" << std::hex << logResult << std::endl;
                }
                g_testResult = -1;
            }
            std::string result = "Test " + input + (passed ? " [PASSED]" : " [FAILED]" );
            std::cout << result << std::endl;
            g_testRunned.push_back(result);
        }
    }
    return;
}

void InitializePackageHelper(IAppxPackageReader** packageReader)
{
    *packageReader = nullptr;
    auto packageName = GetInput<std::string>();
    if (!g_packageRootPath.empty())
    {
        packageName = g_packageRootPath + packageName;
    }

    ComPtr<IAppxFactory> factory;
    ComPtr<IStream> inputStream;

    VERIFY_SUCCEEDED(CreateStreamOnFile(const_cast<char*>(packageName.c_str()), true, &inputStream));
    VERIFY_SUCCEEDED(CoCreateAppxFactoryWithHeap(MyAllocate, MyFree, MSIX_VALIDATION_OPTION_SKIPSIGNATURE, &factory));
    VERIFY_SUCCEEDED(factory->CreatePackageReader(inputStream.Get(), packageReader));
    VERIFY_NOT_NULL(*packageReader);
    return;
}

void InitializeBundleHelper(IAppxBundleReader** bundleReader)
{
    *bundleReader = nullptr;
    auto bundleName = GetInput<std::string>();
    if (!g_packageRootPath.empty())
    {
        bundleName = g_packageRootPath + bundleName;
    }

    ComPtr<IAppxBundleFactory> bundleFactory;
    VERIFY_SUCCEEDED(CoCreateAppxBundleFactoryWithHeap(
        MyAllocate,
        MyFree,
        MSIX_VALIDATION_OPTION::MSIX_VALIDATION_OPTION_SKIPSIGNATURE,
        static_cast<MSIX_APPLICABILITY_OPTIONS>(MSIX_APPLICABILITY_OPTIONS::MSIX_APPLICABILITY_OPTION_SKIPPLATFORM |
                                                MSIX_APPLICABILITY_OPTIONS::MSIX_APPLICABILITY_OPTION_SKIPLANGUAGE),
        &bundleFactory));

    ComPtr<IStream> inputStream;
    VERIFY_SUCCEEDED(CreateStreamOnFile(const_cast<char*>(bundleName.c_str()), true, &inputStream));
    VERIFY_SUCCEEDED(bundleFactory->CreateBundleReader(inputStream.Get(), bundleReader));
    VERIFY_NOT_NULL(*bundleReader);
    return;
}

void StartTestPackage(void*)
{
    std::cout << "Starting test: TestPackage" << std::endl;
    ComPtr<IAppxPackageReader> packageReader;
    InitializePackageHelper(&packageReader);

    std::map<std::string, Test<IAppxPackageReader>> packageTests =
    {
        { "Package.PayloadFiles", Test<IAppxPackageReader>("Validates all payload files from the package are correct",
            [](IAppxPackageReader* packageReader)
            {
                auto expectedNumOfFiles = GetInput<int>();
                std::vector<std::string> expectedPayloadFiles;
                int readFiles = 0;
                while(readFiles < expectedNumOfFiles)
                {
                    auto file = GetInput<std::string>();
                    if (!file.empty())
                    {
                        expectedPayloadFiles.push_back(file);
                        readFiles++;
                    }
                }

                ComPtr<IAppxFilesEnumerator> files;
                VERIFY_SUCCEEDED(packageReader->GetPayloadFiles(&files));
                BOOL hasCurrent = FALSE;
                VERIFY_SUCCEEDED(files->GetHasCurrent(&hasCurrent));
                int nFiles = 0;
                while (hasCurrent)
                {
                    ComPtr<IAppxFile> file;
                    VERIFY_SUCCEEDED(files->GetCurrent(&file));
                    Text<wchar_t> fileName;
                    VERIFY_SUCCEEDED(file->GetName(&fileName));
                    VERIFY_ARE_EQUAL(expectedPayloadFiles.at(nFiles), fileName.ToString());

                    ComPtr<IAppxFileUtf8> fileUtf8;
                    VERIFY_SUCCEEDED(file->QueryInterface(UuidOfImpl<IAppxFileUtf8>::iid, reinterpret_cast<void**>(&fileUtf8)));
                    Text<char> fileNameUtf8;
                    VERIFY_SUCCEEDED(fileUtf8->GetName(&fileNameUtf8));
                    VERIFY_ARE_EQUAL(expectedPayloadFiles.at(nFiles), fileNameUtf8.ToString());

                    // Compare that the file from GetPayloadFile is the same file
                    ComPtr<IAppxFile> file2;
                    VERIFY_SUCCEEDED(packageReader->GetPayloadFile(fileName.Get(), &file2));
                    VERIFY_ARE_SAME(file.Get(), file2.Get());

                    ComPtr<IAppxPackageReaderUtf8> packageReaderUtf8;
                    VERIFY_SUCCEEDED(packageReader->QueryInterface(UuidOfImpl<IAppxPackageReaderUtf8>::iid, reinterpret_cast<void**>(&packageReaderUtf8)));
                    ComPtr<IAppxFile> file3;
                    VERIFY_SUCCEEDED(packageReaderUtf8->GetPayloadFile(fileNameUtf8.Get(), &file3));
                    VERIFY_ARE_SAME(file2.Get(), file3.Get());

                    VERIFY_SUCCEEDED(files->MoveNext(&hasCurrent));
                    nFiles++;
                }
                VERIFY_ARE_EQUAL(expectedNumOfFiles, nFiles);

                return 0;
            })
        },
        { "Package.PayloadFile", Test<IAppxPackageReader>("Verifies a payload file information from the package",
            [](IAppxPackageReader* packageReader)
            {
                auto filename = GetInput<std::string>();
                auto file = utf8_to_utf16(filename);

                ComPtr<IAppxFile> appxFile;
                VERIFY_SUCCEEDED(packageReader->GetPayloadFile(file.c_str(), &appxFile));
                VERIFY_NOT_NULL(appxFile.Get());

                Text<wchar_t> appxFileName;
                VERIFY_SUCCEEDED(appxFile->GetName(&appxFileName));
                VERIFY_ARE_EQUAL(filename, appxFileName.ToString());

                auto compression = GetInput<std::string>();
                auto compressionOption = g_compressionMap.find(compression);
                VERIFY_IS_FALSE(compressionOption == g_compressionMap.end());
                APPX_COMPRESSION_OPTION fileCompression;
                VERIFY_SUCCEEDED(appxFile->GetCompressionOption(&fileCompression));
                VERIFY_ARE_EQUAL(static_cast<int>(compressionOption->second), static_cast<int>(fileCompression));

                auto size = GetInput<std::uint64_t>();
                UINT64 fileSize;
                VERIFY_SUCCEEDED(appxFile->GetSize(&fileSize));
                VERIFY_ARE_EQUAL(size, static_cast<std::uint64_t>(fileSize));
            })
        },
        { "Package.PayloadFile.DontExists", Test<IAppxPackageReader>("Validate a file is not in the package",
            [](IAppxPackageReader* packageReader)
            {
                auto fakefile = GetInput<std::string>();
                auto file = utf8_to_utf16(fakefile);

                ComPtr<IAppxFile> appxFile;
                VERIFY_HR(static_cast<HRESULT>(MSIX::Error::FileNotFound), packageReader->GetPayloadFile(file.c_str(), &appxFile));
                VERIFY_IS_NULL(appxFile.Get());
            }
        )},
        { "Package.FootprintFile", Test<IAppxPackageReader>("Validates a footprint file information",
            [](IAppxPackageReader* packageReader)
            {
                auto footprintFile = GetInput<std::string>();
                auto footprintOption = g_footprintFilesMap.find(footprintFile);
                VERIFY_IS_FALSE(footprintOption == g_footprintFilesMap.end());
                ComPtr<IAppxFile> appxFile;
                VERIFY_SUCCEEDED(packageReader->GetFootprintFile(footprintOption->second.first, &appxFile));

                Text<wchar_t> fileName;
                VERIFY_SUCCEEDED(appxFile->GetName(&fileName));
                VERIFY_ARE_EQUAL(footprintOption->second.second, fileName.ToString());

                auto compression = GetInput<std::string>();
                auto compressionOption = g_compressionMap.find(compression);
                VERIFY_IS_FALSE(compressionOption == g_compressionMap.end());
                APPX_COMPRESSION_OPTION fileCompression;
                VERIFY_SUCCEEDED(appxFile->GetCompressionOption(&fileCompression));
                VERIFY_ARE_EQUAL(static_cast<int>(compressionOption->second), static_cast<int>(fileCompression));

                auto size = GetInput<std::uint64_t>();
                UINT64 fileSize;
                VERIFY_SUCCEEDED(appxFile->GetSize(&fileSize));
                VERIFY_ARE_EQUAL(size, static_cast<std::uint64_t>(fileSize));

            }
        )},
    };
    ParseAndRun(packageTests, "Finish.TestPackage", packageReader.Get());
    return;
}

void StartTestPackageManifest(void*)
{
    std::cout << "Starting test: TestPackageManifest" << std::endl;
    ComPtr<IAppxPackageReader> packageReader;
    InitializePackageHelper(&packageReader);
    ComPtr<IAppxManifestReader> manifestReader;
    VERIFY_SUCCEEDED(packageReader->GetManifest(&manifestReader));
    VERIFY_NOT_NULL(manifestReader.Get());

    std::map<std::string, Test<IAppxManifestReader>> manifestTests =
    {
        { "Package.Manifest.Stream", Test<IAppxManifestReader>("Validates IAppxManifestReader::GetStream",
            [](IAppxManifestReader* manifestReader)
            {
                ComPtr<IStream> stream;
                VERIFY_SUCCEEDED(manifestReader->GetStream(&stream));
                VERIFY_NOT_NULL(stream.Get());
            }
        )},
        { "Package.Manifest.Applications", Test<IAppxManifestReader>("Validates application elements in the manifest",
            [](IAppxManifestReader* manifestReader)
            {
                auto expectedNumOfApps = GetInput<int>();
                ComPtr<IAppxManifestApplicationsEnumerator> enumerator;
                VERIFY_SUCCEEDED(manifestReader->GetApplications(&enumerator));
                BOOL hasCurrent = FALSE;
                VERIFY_SUCCEEDED(enumerator->GetHasCurrent(&hasCurrent));
                int nApps = 0;
                while(hasCurrent)
                {
                    ComPtr<IAppxManifestApplication> app;
                    VERIFY_SUCCEEDED(enumerator->GetCurrent(&app));

                    auto expectedAumid = GetInput<std::string>();
                    Text<wchar_t> aumid;
                    VERIFY_SUCCEEDED(app->GetAppUserModelId(&aumid));
                    VERIFY_ARE_EQUAL(expectedAumid, aumid.ToString());

                    Text<char> aumidUtf8;
                    ComPtr<IAppxManifestApplicationUtf8> appUtf8;
                    VERIFY_SUCCEEDED(app->QueryInterface(UuidOfImpl<IAppxManifestApplicationUtf8>::iid, reinterpret_cast<void**>(&appUtf8)));
                    VERIFY_SUCCEEDED(appUtf8->GetAppUserModelId(&aumidUtf8));
                    VERIFY_ARE_EQUAL(expectedAumid, aumidUtf8.ToString());

                    // Note: this isn't implemented. But adding check so when we do, we update this test too.
                    Text<wchar_t> value;
                    VERIFY_HR(static_cast<HRESULT>(MSIX::Error::NotImplemented), app->GetStringValue(L"dummy", &value));

                    VERIFY_SUCCEEDED(enumerator->MoveNext(&hasCurrent));
                    nApps++;
                }
                VERIFY_ARE_EQUAL(expectedNumOfApps, nApps);
            }
        )},
        { "Package.Manifest.Properties", Test<IAppxManifestReader>("Validates manifest properties",
            [](IAppxManifestReader* manifestReader)
            {
                auto numberOfTests = GetInput<int>();
                ComPtr<IAppxManifestProperties> properties;
                VERIFY_SUCCEEDED(manifestReader->GetProperties(&properties));

                ComPtr<IAppxManifestPropertiesUtf8> propertiesUtf8;
                VERIFY_SUCCEEDED(properties->QueryInterface(UuidOfImpl<IAppxManifestPropertiesUtf8>::iid, reinterpret_cast<void**>(&propertiesUtf8)));

                for(int i = 0; i < numberOfTests; i++)
                {
                    auto valueType = GetInput<std::string>();
                    auto valueUtf8 = GetInput<std::string>();
                    auto value = utf8_to_utf16(valueUtf8);
                    auto expected = GetInput<std::string>();
                    if (valueType == "BoolValue")
                    {
                        BOOL result = FALSE;
                        BOOL resultUtf8 = FALSE;
                        if (expected == "invalid")
                        {
                            VERIFY_HR(static_cast<HRESULT>(MSIX::Error::InvalidParameter), properties->GetBoolValue(value.c_str(), &result));
                        }
                        else
                        {
                            VERIFY_SUCCEEDED(properties->GetBoolValue(value.c_str(), &result));
                            VERIFY_SUCCEEDED(propertiesUtf8->GetBoolValue(valueUtf8.c_str(), &resultUtf8));
                            if (expected == "true")
                            {
                                VERIFY_IS_TRUE(result);
                                VERIFY_IS_TRUE(resultUtf8);
                            }
                            else if (expected == "false")
                            {
                                VERIFY_IS_FALSE(result);
                                VERIFY_IS_FALSE(resultUtf8);
                            }
                            else
                            {
                                throw VerifyException<std::string>(expected , std::string(" Invalid type: use true, false or invalid") ,__LINE__);
                            }
                        }
                    }
                    else if (valueType == "StringValue")
                    {
                        Text<wchar_t> result;
                        Text<char> resultUtf8;
                        if (expected == "invalid")
                        {
                            VERIFY_HR(static_cast<HRESULT>(MSIX::Error::InvalidParameter), properties->GetStringValue(value.c_str(), &result));
                        }
                        else if (expected == "null")
                        {
                            VERIFY_SUCCEEDED(properties->GetStringValue(value.c_str(), &result));
                            VERIFY_IS_NULL(result.Get());

                            VERIFY_SUCCEEDED(propertiesUtf8->GetStringValue(valueUtf8.c_str(), &resultUtf8));
                            VERIFY_IS_NULL(resultUtf8.Get());
                        }
                        else
                        {
                            VERIFY_SUCCEEDED(properties->GetStringValue(value.c_str(), &result));
                            VERIFY_ARE_EQUAL(expected, result.ToString());

                            VERIFY_SUCCEEDED(propertiesUtf8->GetStringValue(valueUtf8.c_str(), &resultUtf8));
                            VERIFY_ARE_EQUAL(expected, resultUtf8.ToString());
                        }
                    }
                    else
                    {
                        throw VerifyException<std::string>(valueType , std::string(" Invalid type: use BoolType or StringValue") ,__LINE__);
                    }
                }
            }

        )},
        { "Package.Manifest.Dependencies", Test<IAppxManifestReader>("Validates manifest dependencies",
            [](IAppxManifestReader* manifestReader)
            {
                auto expNumOfDependencies = GetInput<int>();
                ComPtr<IAppxManifestPackageDependenciesEnumerator> dependencies;
                VERIFY_SUCCEEDED(manifestReader->GetPackageDependencies(&dependencies));

                BOOL hasCurrent = FALSE;
                VERIFY_SUCCEEDED(dependencies->GetHasCurrent(&hasCurrent));
                int numDep = 0;
                while (hasCurrent)
                {
                    ComPtr<IAppxManifestPackageDependency> dependency;
                    VERIFY_SUCCEEDED(dependencies->GetCurrent(&dependency));

                    ComPtr<IAppxManifestPackageDependencyUtf8> dependencyUtf8;
                    VERIFY_SUCCEEDED(dependency->QueryInterface(UuidOfImpl<IAppxManifestPackageDependencyUtf8>::iid, reinterpret_cast<void**>(&dependencyUtf8)));

                    auto expectedName = GetInput<std::string>();
                    Text<wchar_t> name;
                    VERIFY_SUCCEEDED(dependency->GetName(&name));
                    VERIFY_ARE_EQUAL(expectedName, name.ToString());

                    Text<char> nameUtf8;
                    VERIFY_SUCCEEDED(dependencyUtf8->GetName(&nameUtf8));
                    VERIFY_ARE_EQUAL(expectedName, nameUtf8.ToString());

                    auto expectedMin = GetInput<UINT64>();
                    UINT64 min = 0;
                    VERIFY_SUCCEEDED(dependency->GetMinVersion(&min));
                    VERIFY_ARE_EQUAL(expectedMin, min);

                    auto expectedPublisher = GetInput<std::string>();
                    Text<wchar_t> publisher;
                    VERIFY_SUCCEEDED(dependency->GetPublisher(&publisher));
                    VERIFY_ARE_EQUAL(expectedPublisher, publisher.ToString());

                    Text<char> publisherUtf8;
                    VERIFY_SUCCEEDED(dependencyUtf8->GetPublisher(&publisherUtf8));
                    VERIFY_ARE_EQUAL(expectedPublisher, publisherUtf8.ToString());

                    VERIFY_SUCCEEDED(dependencies->MoveNext(&hasCurrent));
                    numDep++;
                }
                VERIFY_ARE_EQUAL(expNumOfDependencies, numDep);
            }
        )},
        { "Package.Manifest.Capabilities", Test<IAppxManifestReader>("Validates manifest capabilities",
            [](IAppxManifestReader* manifestReader)
            {
                auto expected = GetInput<int>();
                APPX_CAPABILITIES capabilities;
                VERIFY_SUCCEEDED(manifestReader->GetCapabilities(&capabilities));
                VERIFY_ARE_EQUAL(expected, static_cast<int>(capabilities));
            }
        )},
        { "Package.Manifest.Resources", Test<IAppxManifestReader>("Validates manifest resources",
            [](IAppxManifestReader* manifestReader)
            {
                auto expNumOfResources = GetInput<int>();
                ComPtr<IAppxManifestResourcesEnumerator> resources;
                VERIFY_SUCCEEDED(manifestReader->GetResources(&resources));
                ComPtr<IAppxManifestResourcesEnumeratorUtf8> resourcesUtf8;
                VERIFY_SUCCEEDED(resources->QueryInterface(UuidOfImpl<IAppxManifestResourcesEnumeratorUtf8>::iid, reinterpret_cast<void**>(&resourcesUtf8)));

                BOOL hasCurrent = FALSE;
                VERIFY_SUCCEEDED(resources->GetHasCurrent(&hasCurrent));
                int numOfDependencies = 0;
                while (hasCurrent)
                {
                    auto expectedResource = GetInput<std::string>();
                    Text<wchar_t> resource;
                    VERIFY_SUCCEEDED(resources->GetCurrent(&resource));
                    VERIFY_ARE_EQUAL(expectedResource, resource.ToString());

                    Text<char> resourceUtf8;
                    VERIFY_SUCCEEDED(resourcesUtf8->GetCurrent(&resourceUtf8));
                    VERIFY_ARE_EQUAL(expectedResource, resourceUtf8.ToString());

                    VERIFY_SUCCEEDED(resources->MoveNext(&hasCurrent));
                    numOfDependencies++;
                }
                VERIFY_ARE_EQUAL(expNumOfResources, numOfDependencies);
            }
        )},
        { "Package.Manifest.Tdf", Test<IAppxManifestReader>("Validates manifest target family devices",
            [](IAppxManifestReader* manifestReader)
            {
                auto expNumOfTdfs = GetInput<int>();
                ComPtr<IAppxManifestReader3> manifestReader3;
                VERIFY_SUCCEEDED(manifestReader->QueryInterface(UuidOfImpl<IAppxManifestReader3>::iid, reinterpret_cast<void**>(&manifestReader3)));
                ComPtr<IAppxManifestTargetDeviceFamiliesEnumerator> tdfEnum;
                VERIFY_SUCCEEDED(manifestReader3->GetTargetDeviceFamilies(&tdfEnum));
                BOOL hasCurrent = FALSE;
                VERIFY_SUCCEEDED(tdfEnum->GetHasCurrent(&hasCurrent));
                int numOfTdfs = 0;
                while (hasCurrent)
                {
                    ComPtr<IAppxManifestTargetDeviceFamily> tdf;
                    VERIFY_SUCCEEDED(tdfEnum->GetCurrent(&tdf));

                    auto expectedName = GetInput<std::string>();
                    Text<wchar_t> name;
                    VERIFY_SUCCEEDED(tdf->GetName(&name));
                    VERIFY_ARE_EQUAL(expectedName, name.ToString());

                    ComPtr<IAppxManifestTargetDeviceFamilyUtf8> tdfUtf8;
                    VERIFY_SUCCEEDED(tdf->QueryInterface(UuidOfImpl<IAppxManifestTargetDeviceFamilyUtf8>::iid, reinterpret_cast<void**>(&tdfUtf8)));
                    Text<char> nameUtf8;
                    VERIFY_SUCCEEDED(tdfUtf8->GetName(&nameUtf8));
                    VERIFY_ARE_EQUAL(expectedName, nameUtf8.ToString());

                    auto expectedMin = GetInput<UINT64>();
                    UINT64 min = 0;
                    VERIFY_SUCCEEDED(tdf->GetMinVersion(&min));
                    VERIFY_ARE_EQUAL(expectedMin, min);

                    auto expectedMax = GetInput<UINT64>();
                    UINT64 max = 0;
                    VERIFY_SUCCEEDED(tdf->GetMaxVersionTested(&max));
                    VERIFY_ARE_EQUAL(expectedMax, max);

                    VERIFY_SUCCEEDED(tdfEnum->MoveNext(&hasCurrent));
                    numOfTdfs++;
                }
                VERIFY_ARE_EQUAL(expNumOfTdfs, numOfTdfs);
            }
        )},
        { "Package.Manifest.MsixDocument", Test<IAppxManifestReader>("Validates IMsixDocumentElement, IMsixElement and IMsixElementEnumerator ",
            [](IAppxManifestReader* manifestReader)
            {
                auto expNumOfElements = GetInput<int>();
                ComPtr<IMsixDocumentElement> msixDocument;
                VERIFY_SUCCEEDED(manifestReader->QueryInterface(UuidOfImpl<IMsixDocumentElement>::iid, reinterpret_cast<void**>(&msixDocument)));
                ComPtr<IMsixElement> manifestElement;
                VERIFY_SUCCEEDED(msixDocument->GetDocumentElement(&manifestElement));

                auto xpathUtf8 = GetInput<std::string>();
                auto xpath = utf8_to_utf16(xpathUtf8);
                ComPtr<IMsixElementEnumerator> elementEnum;
                VERIFY_SUCCEEDED(manifestElement->GetElements(xpath.c_str(), &elementEnum));

                ComPtr<IMsixElementEnumerator> elementEnumFromUtf8;
                VERIFY_SUCCEEDED(manifestElement->GetElementsUtf8(xpathUtf8.c_str(), &elementEnumFromUtf8));

                BOOL hasCurrent = FALSE;
                BOOL hasCurrentFromUtf8 = FALSE;
                VERIFY_SUCCEEDED(elementEnum->GetHasCurrent(&hasCurrent));
                VERIFY_SUCCEEDED(elementEnumFromUtf8->GetHasCurrent(&hasCurrentFromUtf8));
                int numOfElements = 0;
                while(hasCurrent && hasCurrentFromUtf8)
                {
                    // NOTE: element and elementFromUtf8 are different objects, but should output the same results.
                    ComPtr<IMsixElement> element;
                    VERIFY_SUCCEEDED(elementEnum->GetCurrent(&element));

                    ComPtr<IMsixElement> elementFromUtf8;
                    VERIFY_SUCCEEDED(elementEnumFromUtf8->GetCurrent(&elementFromUtf8));

                    auto numOfAttributes = GetInput<int>();
                    for (int i = 0; i < numOfAttributes; i++)
                    {
                        auto attributeUtf8 = GetInput<std::string>();
                        Text<wchar_t> value;
                        Text<char> valueUtf8;
                        Text<wchar_t> valueFromUtf8;
                        Text<char> valueUtf8FromUtf8;
                        if (attributeUtf8 == "Fake")
                        {
                            VERIFY_SUCCEEDED(element->GetAttributeValue(L"Fake", &value));
                            VERIFY_IS_NULL(value.Get());

                            VERIFY_SUCCEEDED(element->GetAttributeValueUtf8("Fake", &valueUtf8));
                            VERIFY_IS_NULL(valueUtf8.Get());

                            VERIFY_SUCCEEDED(elementFromUtf8->GetAttributeValue(L"Fake", &valueFromUtf8));
                            VERIFY_IS_NULL(valueFromUtf8.Get());

                            VERIFY_SUCCEEDED(elementFromUtf8->GetAttributeValueUtf8("Fake", &valueUtf8FromUtf8));
                            VERIFY_IS_NULL(valueUtf8FromUtf8.Get());
                        }
                        else
                        {
                            auto attribute = utf8_to_utf16(attributeUtf8);
                            auto expectedValue = GetInput<std::string>();

                            VERIFY_SUCCEEDED(element->GetAttributeValue(attribute.c_str(), &value));
                            VERIFY_ARE_EQUAL(expectedValue, value.ToString());

                            VERIFY_SUCCEEDED(element->GetAttributeValueUtf8(attributeUtf8.c_str(), &valueUtf8));
                            VERIFY_ARE_EQUAL(expectedValue, valueUtf8.ToString());

                            VERIFY_SUCCEEDED(elementFromUtf8->GetAttributeValue(attribute.c_str(), &valueFromUtf8));
                            VERIFY_ARE_EQUAL(expectedValue, valueFromUtf8.ToString());

                            VERIFY_SUCCEEDED(elementFromUtf8->GetAttributeValueUtf8(attributeUtf8.c_str(), &valueUtf8FromUtf8));
                            VERIFY_ARE_EQUAL(expectedValue, valueUtf8FromUtf8.ToString());
                        }
                    }

                    VERIFY_SUCCEEDED(elementEnum->MoveNext(&hasCurrent));
                    VERIFY_SUCCEEDED(elementEnumFromUtf8->MoveNext(&hasCurrentFromUtf8));
                    numOfElements++;
                }
                VERIFY_ARE_EQUAL(expNumOfElements, numOfElements);
                VERIFY_IS_FALSE(hasCurrent || hasCurrentFromUtf8)
            }
        )},
        { "Package.Manifest.PackageId", Test<IAppxManifestReader>("Validates manifest package id",
            [](IAppxManifestReader* manifestReader)
            {
                ComPtr<IAppxManifestPackageId> packageId;
                VERIFY_SUCCEEDED(manifestReader->GetPackageId(&packageId));
                VERIFY_NOT_NULL(packageId.Get());

                ComPtr<IAppxManifestPackageIdUtf8> packageIdUtf8;
                VERIFY_SUCCEEDED(packageId->QueryInterface(UuidOfImpl<IAppxManifestPackageIdUtf8>::iid, reinterpret_cast<void**>(&packageIdUtf8)));

                auto expectedName = GetInput<std::string>();
                Text<wchar_t> name;
                VERIFY_SUCCEEDED(packageId->GetName(&name));
                VERIFY_ARE_EQUAL(expectedName, name.ToString())

                Text<char> nameUtf8;
                VERIFY_SUCCEEDED(packageIdUtf8->GetName(&nameUtf8));
                VERIFY_ARE_EQUAL(expectedName, nameUtf8.ToString())

                auto expectedArch = GetInput<int>();
                APPX_PACKAGE_ARCHITECTURE architecture;
                VERIFY_SUCCEEDED(packageId->GetArchitecture(&architecture));
                VERIFY_ARE_EQUAL(expectedArch, static_cast<int>(architecture));

                auto expectedPublisher = GetInput<std::string>();
                Text<wchar_t> publisher;
                VERIFY_SUCCEEDED(packageId->GetPublisher(&publisher));
                VERIFY_ARE_EQUAL(expectedPublisher, publisher.ToString());

                Text<char> publisherUtf8;
                VERIFY_SUCCEEDED(packageIdUtf8->GetPublisher(&publisherUtf8));
                VERIFY_ARE_EQUAL(expectedPublisher, publisherUtf8.ToString());

                BOOL isSame = FALSE;
                VERIFY_SUCCEEDED(packageId->ComparePublisher(publisher.Get(), &isSame));
                VERIFY_IS_TRUE(isSame);
                isSame = FALSE;
                VERIFY_SUCCEEDED(packageIdUtf8->ComparePublisher(publisherUtf8.Get(), &isSame));
                VERIFY_IS_TRUE(isSame);
                VERIFY_SUCCEEDED(packageId->ComparePublisher(L"OtherPublisher", &isSame));
                VERIFY_IS_FALSE(isSame);

                auto expectedVersion = GetInput<UINT64>();
                UINT64 packageVersion;
                VERIFY_SUCCEEDED(packageId->GetVersion(&packageVersion));
                VERIFY_ARE_EQUAL(expectedVersion, packageVersion);

                auto expectedResource = GetInput<std::string>();
                Text<wchar_t> resourceId;
                VERIFY_SUCCEEDED(packageId->GetResourceId(&resourceId));
                if (expectedResource == "no resource")
                {
                    VERIFY_IS_NULL(resourceId.Get());
                }
                else
                {
                    VERIFY_ARE_EQUAL(expectedResource, resourceId.ToString());

                    Text<char> resourceIdUtf8;
                    VERIFY_SUCCEEDED(packageIdUtf8->GetResourceId(&resourceIdUtf8));
                    VERIFY_ARE_EQUAL(expectedResource, resourceIdUtf8.ToString());
                }

                auto expectedFull = GetInput<std::string>();
                Text<wchar_t> packageFullName;
                VERIFY_SUCCEEDED(packageId->GetPackageFullName(&packageFullName));
                VERIFY_ARE_EQUAL(expectedFull, packageFullName.ToString());

                Text<char> packageFullNameUtf8;
                VERIFY_SUCCEEDED(packageIdUtf8->GetPackageFullName(&packageFullNameUtf8));
                VERIFY_ARE_EQUAL(expectedFull, packageFullNameUtf8.ToString());

                auto expectedFamily = GetInput<std::string>();
                Text<wchar_t> packageFamilyName;
                VERIFY_SUCCEEDED(packageId->GetPackageFamilyName(&packageFamilyName));
                VERIFY_ARE_EQUAL(expectedFamily, packageFamilyName.ToString());

                Text<char> packageFamilyNameUtf8;
                VERIFY_SUCCEEDED(packageIdUtf8->GetPackageFamilyName(&packageFamilyNameUtf8));
                VERIFY_ARE_EQUAL(expectedFamily, packageFamilyNameUtf8.ToString());
            }
        )},
    };
    ParseAndRun(manifestTests, "Finish.TestPackageManifest", manifestReader.Get());
    return;
}

void StartTestPackageBlockMap(void*)
{
    std::cout << "Starting test: TestPackageBlockMap" << std::endl;
    ComPtr<IAppxPackageReader> packageReader;
    InitializePackageHelper(&packageReader);
    ComPtr<IAppxBlockMapReader> blockMapReader;
    VERIFY_SUCCEEDED(packageReader->GetBlockMap(&blockMapReader));
    VERIFY_NOT_NULL(blockMapReader.Get());

    std::map<std::string, Test<IAppxBlockMapReader>> blockMapTests =
    {
        { "Package.BlockMap.Stream", Test<IAppxBlockMapReader>("Validates IAppxBlockMapReader::GetStream",
            [](IAppxBlockMapReader* blockMapReader)
            {
                ComPtr<IStream> stream;
                VERIFY_SUCCEEDED(blockMapReader->GetStream(&stream));
                VERIFY_NOT_NULL(stream.Get());
            }
        )},
        { "Package.BlockMap.ValidateFiles", Test<IAppxBlockMapReader>("Validates all files information in the blockmap",
            [](IAppxBlockMapReader* blockMapReader)
            {
                ComPtr<IAppxBlockMapFilesEnumerator> blockMapFiles;
                VERIFY_SUCCEEDED(blockMapReader->GetFiles(&blockMapFiles));

                auto expNumOfBlockMapFiles = GetInput<int>();
                BOOL hasCurrent = FALSE;
                VERIFY_SUCCEEDED(blockMapFiles->GetHasCurrent(&hasCurrent));
                int numOfBlockMapFiles = 0;
                while (hasCurrent)
                {
                    ComPtr<IAppxBlockMapFile> blockMapFile;
                    VERIFY_SUCCEEDED(blockMapFiles->GetCurrent(&blockMapFile));

                    auto expectedName = GetInput<std::string>();
                    Text<wchar_t> fileName;
                    VERIFY_SUCCEEDED(blockMapFile->GetName(&fileName))
                    VERIFY_ARE_EQUAL(expectedName, fileName.ToString());

                    ComPtr<IAppxBlockMapFileUtf8> blockMapFileUtf8;
                    VERIFY_SUCCEEDED(blockMapFile->QueryInterface(UuidOfImpl<IAppxBlockMapFileUtf8>::iid, reinterpret_cast<void**>(&blockMapFileUtf8)));
                    Text<char> fileNameUtf8;
                    VERIFY_SUCCEEDED(blockMapFileUtf8->GetName(&fileNameUtf8))
                    VERIFY_ARE_EQUAL(expectedName, fileNameUtf8.ToString());

                    ComPtr<IAppxBlockMapFile> blockMapFile2;
                    VERIFY_SUCCEEDED(blockMapReader->GetFile(fileName.Get(), &blockMapFile2));
                    VERIFY_ARE_SAME(blockMapFile.Get(), blockMapFile2.Get());

                    ComPtr<IAppxBlockMapFile> blockMapFile3;
                    ComPtr<IAppxBlockMapReaderUtf8> blockMapReaderUtf8;
                    VERIFY_SUCCEEDED(blockMapReader->QueryInterface(UuidOfImpl<IAppxBlockMapReaderUtf8>::iid, reinterpret_cast<void**>(&blockMapReaderUtf8)));
                    VERIFY_SUCCEEDED(blockMapReaderUtf8->GetFile(fileNameUtf8.Get(), &blockMapFile3));
                    VERIFY_ARE_SAME(blockMapFile.Get(), blockMapFile3.Get());

                    auto expectedLfh = GetInput<UINT32>();
                    UINT32 lfh;
                    VERIFY_SUCCEEDED(blockMapFile->GetLocalFileHeaderSize(&lfh));
                    VERIFY_ARE_EQUAL(expectedLfh, lfh);

                    auto expectedSize = GetInput<UINT64>();
                    UINT64 size;
                    VERIFY_SUCCEEDED(blockMapFile->GetUncompressedSize(&size));
                    VERIFY_ARE_EQUAL(expectedSize, size);

                    auto expNumOfBlocks = GetInput<int>();

                    // Helper function to parse blocks from string to a bytes vector
                    auto parseBlock = [](std::string block) -> std::vector<std::uint8_t>
                    {
                        if (block.size()%2 != 0) { throw VerifyException<std::string>(block, std::string(" Invalid block input"), __LINE__); }
                        std::vector<std::uint8_t> result;
                        for(std::size_t i = 0; i < block.size(); i+=2)
                        {
                            result.push_back(std::stoi(block.substr(i, 2), 0, 16));
                        }
                        return result;
                    };

                    ComPtr<IAppxBlockMapBlocksEnumerator> blockEnum;
                    VERIFY_SUCCEEDED(blockMapFile->GetBlocks(&blockEnum));
                    BOOL hasCurrentBlock = FALSE;
                    VERIFY_SUCCEEDED(blockEnum->GetHasCurrent(&hasCurrentBlock));
                    int numOfBlocks = 0;
                    while (hasCurrentBlock)
                    {
                        // get block hash and size
                        ComPtr<IAppxBlockMapBlock> block;
                        VERIFY_SUCCEEDED(blockEnum->GetCurrent(&block));

                        auto expectedBlock = GetInput<std::string>();
                        UINT32 bufferSize;
                        BYTE* buffer = nullptr;
                        VERIFY_SUCCEEDED(block->GetHash(&bufferSize, &buffer));
                        auto expectedBytes = parseBlock(expectedBlock);
                        VERIFY_ARE_EQUAL(static_cast<UINT32>(expectedBytes.size()), bufferSize);
                        for (UINT32 i = 0; i < bufferSize; i++)
                        {
                            VERIFY_ARE_EQUAL(expectedBytes[i], buffer[i]);
                        }

                        auto expectedSize = GetInput<std::uint32_t>();
                        UINT32 compsize;
                        VERIFY_SUCCEEDED(block->GetCompressedSize(&compsize));
                        VERIFY_ARE_EQUAL(expectedSize, compsize);

                        VERIFY_SUCCEEDED(blockEnum->MoveNext(&hasCurrentBlock));
                        numOfBlocks++;
                    }
                    VERIFY_ARE_EQUAL(expNumOfBlocks, numOfBlocks);

                    VERIFY_SUCCEEDED(blockMapFiles->MoveNext(&hasCurrent));
                    numOfBlockMapFiles++;
                }
                VERIFY_ARE_EQUAL(expNumOfBlockMapFiles, numOfBlockMapFiles);

            }
        )},
    };
    ParseAndRun(blockMapTests, "Finish.TestPackageBlockMap", blockMapReader.Get());
    return;
}

void StartTestBundle(void*)
{
    std::cout << "Starting test: TestBundle" << std::endl;
    ComPtr<IAppxBundleReader> bundleReader;
    InitializeBundleHelper(&bundleReader);

    std::map<std::string, Test<IAppxBundleReader>> bundleReaderTests =
    {
        { "Bundle.FootprintFile", Test<IAppxBundleReader>("Validates a footprint file information from a bundle",
            [](IAppxBundleReader* bundleReader)
            {
                auto footprintFile = GetInput<std::string>();
                auto footprintOption = g_bundleFootprintFilesMap.find(footprintFile);
                VERIFY_IS_FALSE(footprintOption == g_bundleFootprintFilesMap.end());
                ComPtr<IAppxFile> appxFile;
                VERIFY_SUCCEEDED(bundleReader->GetFootprintFile(footprintOption->second.first, &appxFile));

                Text<wchar_t> fileName;
                VERIFY_SUCCEEDED(appxFile->GetName(&fileName));
                VERIFY_ARE_EQUAL(footprintOption->second.second, fileName.ToString());

                auto compression = GetInput<std::string>();
                auto compressionOption = g_compressionMap.find(compression);
                VERIFY_IS_FALSE(compressionOption == g_compressionMap.end());
                APPX_COMPRESSION_OPTION fileCompression;
                VERIFY_SUCCEEDED(appxFile->GetCompressionOption(&fileCompression));
                VERIFY_ARE_EQUAL(static_cast<int>(compressionOption->second), static_cast<int>(fileCompression));

                auto size = GetInput<std::uint64_t>();
                UINT64 fileSize;
                VERIFY_SUCCEEDED(appxFile->GetSize(&fileSize));
                VERIFY_ARE_EQUAL(size, static_cast<std::uint64_t>(fileSize));
            }
        )},
        { "Bundle.PayloadPackages", Test<IAppxBundleReader>("Validates list of payload packages in the bundle",
            [](IAppxBundleReader* bundleReader)
            {
                auto expNumOfPackages = GetInput<int>();
                ComPtr<IAppxFilesEnumerator> packages;
                VERIFY_SUCCEEDED(bundleReader->GetPayloadPackages(&packages));

                BOOL hasCurrent = FALSE;
                VERIFY_SUCCEEDED(packages->GetHasCurrent(&hasCurrent));
                int numOfPackages = 0;
                while (hasCurrent)
                {
                    ComPtr<IAppxFile> package;
                    VERIFY_SUCCEEDED(packages->GetCurrent(&package));

                    auto expectedName = GetInput<std::string>();
                    Text<wchar_t> packageName;
                    VERIFY_SUCCEEDED(package->GetName(&packageName));
                    VERIFY_ARE_EQUAL(expectedName, packageName.ToString());

                    ComPtr<IAppxFile> package2;
                    VERIFY_SUCCEEDED(bundleReader->GetPayloadPackage(packageName.Get(), &package2));
                    VERIFY_ARE_SAME(package.Get(), package2.Get());

                    Text<char> packageNameUtf8;
                    ComPtr<IAppxFileUtf8> packageUtf8;
                    VERIFY_SUCCEEDED(package->QueryInterface(UuidOfImpl<IAppxFileUtf8>::iid, reinterpret_cast<void**>(&packageUtf8)));
                    VERIFY_SUCCEEDED(packageUtf8->GetName(&packageNameUtf8));
                    VERIFY_ARE_EQUAL(expectedName, packageNameUtf8.ToString());

                    ComPtr<IAppxFile> package3;
                    ComPtr<IAppxBundleReaderUtf8> bundleReaderUtf8;
                    VERIFY_SUCCEEDED(bundleReader->QueryInterface(UuidOfImpl<IAppxBundleReaderUtf8>::iid, reinterpret_cast<void**>(&bundleReaderUtf8)));
                    VERIFY_SUCCEEDED(bundleReaderUtf8->GetPayloadPackage(packageNameUtf8.Get(), &package3));
                    VERIFY_ARE_SAME(package.Get(), package3.Get());

                    VERIFY_SUCCEEDED(packages->MoveNext(&hasCurrent));
                    numOfPackages++;
                }
                VERIFY_ARE_EQUAL(expNumOfPackages, numOfPackages);
            }
        )},
    };
    ParseAndRun(bundleReaderTests, "Finish.TestBundle", bundleReader.Get());
    return;
}

void StartTestBundleManifest(void*)
{
    std::cout << "Starting test: TestBundleManifest" << std::endl;
    ComPtr<IAppxBundleReader> bundleReader;
    InitializeBundleHelper(&bundleReader);
    ComPtr<IAppxBundleManifestReader> bundleManifestReader;
    VERIFY_SUCCEEDED(bundleReader->GetManifest(&bundleManifestReader));

    std::map<std::string, Test<IAppxBundleManifestReader>> bundleManifestTests =
    {
        { "Bundle.Manifest.Stream", Test<IAppxBundleManifestReader>("Validates IAppxBundleManifestReader::GetStream",
            [](IAppxBundleManifestReader* bundleManifestReader)
            {
                ComPtr<IStream> stream;
                VERIFY_SUCCEEDED(bundleManifestReader->GetStream(&stream));
                VERIFY_NOT_NULL(stream.Get());
            }
        )},
        { "Bundle.Manifest.PackageId", Test<IAppxBundleManifestReader>("Validates bundle package id",
            [](IAppxBundleManifestReader* bundleManifestReader)
            {
                ComPtr<IAppxManifestPackageId> packageInfo;
                VERIFY_SUCCEEDED(bundleManifestReader->GetPackageId(&packageInfo));

                auto expectedFullName = GetInput<std::string>();
                Text<wchar_t> fullName;
                VERIFY_SUCCEEDED(packageInfo->GetPackageFullName(&fullName));
                VERIFY_ARE_EQUAL(expectedFullName, fullName.ToString());
            }
        )},
        { "Bundle.Manifest.PackageInfo", Test<IAppxBundleManifestReader>("Validates bundle package info items",
            [](IAppxBundleManifestReader* bundleManifestReader)
            {
                auto expNumOfItems = GetInput<int>();
                ComPtr<IAppxBundleManifestPackageInfoEnumerator> bundleManifestPackageInfoEnumerator;
                VERIFY_SUCCEEDED(bundleManifestReader->GetPackageInfoItems(&bundleManifestPackageInfoEnumerator));

                BOOL hasCurrent = FALSE;
                VERIFY_SUCCEEDED(bundleManifestPackageInfoEnumerator->GetHasCurrent(&hasCurrent));
                int numOfItems = 0;
                while (hasCurrent)
                {
                    ComPtr<IAppxBundleManifestPackageInfo> bundleManifestPackageInfo;
                    VERIFY_SUCCEEDED(bundleManifestPackageInfoEnumerator->GetCurrent(&bundleManifestPackageInfo));

                    auto expectedName = GetInput<std::string>();
                    Text<wchar_t> fileName;
                    VERIFY_SUCCEEDED(bundleManifestPackageInfo->GetFileName(&fileName));
                    VERIFY_ARE_EQUAL(expectedName, fileName.ToString());

                    Text<char> fileNameUtf8;
                    ComPtr<IAppxBundleManifestPackageInfoUtf8> bundleManifestPackageInfoUtf8;
                    VERIFY_SUCCEEDED(bundleManifestPackageInfo->QueryInterface(UuidOfImpl<IAppxBundleManifestPackageInfoUtf8>::iid, reinterpret_cast<void**>(&bundleManifestPackageInfoUtf8)));
                    VERIFY_SUCCEEDED(bundleManifestPackageInfoUtf8->GetFileName(&fileNameUtf8));
                    VERIFY_ARE_EQUAL(expectedName, fileNameUtf8.ToString());

                    auto expectedType = GetInput<std::string>();
                    APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE type;
                    VERIFY_SUCCEEDED(bundleManifestPackageInfo->GetPackageType(&type));
                    if (expectedType == "app")
                    {
                        VERIFY_ARE_EQUAL(static_cast<int>(APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_APPLICATION), static_cast<int>(type));
                    }
                    else if (expectedType == "resource")
                    {
                        VERIFY_ARE_EQUAL(static_cast<int>(APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE), static_cast<int>(type));
                    }
                    else
                    {
                        throw VerifyException<std::string>(expectedType , std::string(" Invalid type: use app or resource") ,__LINE__);
                    }

                    auto expectedOffset = GetInput<UINT64>();
                    UINT64 offset;
                    VERIFY_SUCCEEDED(bundleManifestPackageInfo->GetOffset(&offset));
                    VERIFY_ARE_EQUAL(expectedOffset, offset);

                    auto expectedSize = GetInput<UINT64>();
                    UINT64 size;
                    VERIFY_SUCCEEDED(bundleManifestPackageInfo->GetSize(&size));
                    VERIFY_ARE_EQUAL(expectedSize, size);

                    auto expNumOfLanguages = GetInput<int>();
                    ComPtr<IAppxManifestQualifiedResourcesEnumerator> manifestQualifiedResourcesEnumerator;
                    VERIFY_SUCCEEDED(bundleManifestPackageInfo->GetResources(&manifestQualifiedResourcesEnumerator));

                    BOOL hasCurrentResource = FALSE;
                    VERIFY_SUCCEEDED(manifestQualifiedResourcesEnumerator->GetHasCurrent(&hasCurrentResource));
                    int nLangs = 0;
                    while (hasCurrentResource)
                    {
                        ComPtr<IAppxManifestQualifiedResource> manifestQualifiedResources;
                        VERIFY_SUCCEEDED(manifestQualifiedResourcesEnumerator->GetCurrent(&manifestQualifiedResources));

                        auto expectedTag = GetInput<std::string>();
                        Text<wchar_t> language;
                        VERIFY_SUCCEEDED(manifestQualifiedResources->GetLanguage(&language));
                        VERIFY_ARE_EQUAL(expectedTag, language.ToString());

                        ComPtr<IAppxManifestQualifiedResourceUtf8> manifestQualifiedResourcesUtf8;
                        VERIFY_SUCCEEDED(manifestQualifiedResources->QueryInterface(UuidOfImpl<IAppxManifestQualifiedResourceUtf8>::iid, reinterpret_cast<void**>(&manifestQualifiedResourcesUtf8)));
                        Text<char> languageUtf8;
                        VERIFY_SUCCEEDED(manifestQualifiedResourcesUtf8->GetLanguage(&languageUtf8));
                        VERIFY_ARE_EQUAL(expectedTag, languageUtf8.ToString());

                        VERIFY_SUCCEEDED(manifestQualifiedResourcesEnumerator->MoveNext(&hasCurrentResource));
                        nLangs++;
                    }
                    VERIFY_ARE_EQUAL(expNumOfLanguages, nLangs);
                    VERIFY_SUCCEEDED(bundleManifestPackageInfoEnumerator->MoveNext(&hasCurrent));
                    numOfItems++;
                }
                VERIFY_ARE_EQUAL(expNumOfItems, numOfItems);
            }
        )},
    };
    ParseAndRun(bundleManifestTests, "Finish.TestBundleManifest", bundleManifestReader.Get());
    return;
}

int RunApiTestInternal(char* input, char* target, char* packageRootPath)
{
    // This is only used by the mobile tests
    if (packageRootPath != nullptr)
    {
        g_packageRootPath = std::string(packageRootPath);
    }

    auto streamHelper = StreamOverrideHelper(input, target);

    std::map<std::string, Test<void>> tests =
    {
        { "Start.TestPackage", Test<void>("Test IAppxPackageReader", StartTestPackage) },
        { "Start.TestPackageManifest", Test<void>("Test IAppxManifestReader", StartTestPackageManifest) },
        { "Start.TestPackageBlockMap", Test<void>("Test IAppxBlockMapReader", StartTestPackageBlockMap) },
        { "Start.TestBundle", Test<void>("Test IAppxBundleReader", StartTestBundle) },
        { "Start.TestBundleManifest", Test<void>("Test IAppxBundleManifestReader", StartTestBundleManifest) },
    };
    ParseAndRun(tests, "Finish");

    std::cout << "\nTests Results:" << std::endl;
    for (auto const& result : g_testRunned)
    {
        std::cout << "\t"  << result << std::endl;
    }

    return g_testResult;
}

} // MsixApiTest

#ifndef WIN32
__attribute__((visibility("default"))) 
#endif
int RunApiTest(char* input, char* target, char* packageRootPath = nullptr)
{
    return MsixApiTest::RunApiTestInternal(input, target, packageRootPath);
}
