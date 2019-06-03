//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#include "catch.hpp"
#include "msixtest_int.hpp"
#include "FileHelpers.hpp"
#include "UnpackTestData.hpp"
#include "BlockMapTestData.hpp"
#include "macros.hpp"

#include <iostream>
#include <array>

void InitializePackageReader(const std::string& package, IAppxPackageReader** packageReader)
{
    *packageReader = nullptr;

    auto packagePath = MsixTest::TestPath::GetInstance()->GetPath(MsixTest::TestPath::Directory::Unpack) + "/" + package;
    packagePath = MsixTest::Directory::PathAsCurrentPlatform(packagePath);

    MsixTest::ComPtr<IAppxFactory> factory;
    MsixTest::ComPtr<IStream> inputStream;

    REQUIRE_SUCCEEDED(CreateStreamOnFile(const_cast<char*>(packagePath.c_str()), true, &inputStream));
    REQUIRE_SUCCEEDED(CoCreateAppxFactoryWithHeap(MsixTest::Allocators::Allocate, MsixTest::Allocators::Free, MSIX_VALIDATION_OPTION_SKIPSIGNATURE, &factory));
    REQUIRE_SUCCEEDED(factory->CreatePackageReader(inputStream.Get(), packageReader));
    REQUIRE_NOT_NULL(*packageReader);
    return;
}

// Validates all payload files from the package are correct
TEST_CASE("Api_AppxPackageReader_PayloadFiles", "[api]")
{
    std::string package = "StoreSigned_Desktop_x64_MoviesTV.appx";
    MsixTest::ComPtr<IAppxPackageReader> packageReader;
    InitializePackageReader(package, &packageReader);

    // Expected files in the package
    auto expectedFiles = MsixTest::Unpack::GetExpectedFiles();

    MsixTest::ComPtr<IAppxFilesEnumerator> files;
    REQUIRE_SUCCEEDED(packageReader->GetPayloadFiles(&files));
    BOOL hasCurrent = FALSE;
    REQUIRE_SUCCEEDED(files->GetHasCurrent(&hasCurrent));
    while (hasCurrent)
    {
        MsixTest::ComPtr<IAppxFile> file;
        REQUIRE_SUCCEEDED(files->GetCurrent(&file));

        MsixTest::Wrappers::Buffer<wchar_t> fileName;
        REQUIRE_SUCCEEDED(file->GetName(&fileName));
        auto name = fileName.ToString();
        std::replace(name.begin(), name.end(), '\\', '/'); // expected files uses / separator
        auto find = expectedFiles.find(name);
        REQUIRE(find != expectedFiles.end());
        expectedFiles.erase(find);

        MsixTest::ComPtr<IAppxFileUtf8> fileUtf8;
        REQUIRE_SUCCEEDED(file->QueryInterface(UuidOfImpl<IAppxFileUtf8>::iid, reinterpret_cast<void**>(&fileUtf8)));
        MsixTest::Wrappers::Buffer<char> fileNameUtf8;
        REQUIRE_SUCCEEDED(fileUtf8->GetName(&fileNameUtf8));
        REQUIRE(fileName.ToString() == fileNameUtf8.ToString());

        // Compare that the file from GetPayloadFile is the same file
        MsixTest::ComPtr<IAppxFile> file2;
        REQUIRE_SUCCEEDED(packageReader->GetPayloadFile(fileName.Get(), &file2));
        REQUIRE_ARE_SAME(file.Get(), file2.Get());

        MsixTest::ComPtr<IAppxPackageReaderUtf8> packageReaderUtf8;
        REQUIRE_SUCCEEDED(packageReader->QueryInterface(UuidOfImpl<IAppxPackageReaderUtf8>::iid, reinterpret_cast<void**>(&packageReaderUtf8)));
        MsixTest::ComPtr<IAppxFile> file3;
        REQUIRE_SUCCEEDED(packageReaderUtf8->GetPayloadFile(fileNameUtf8.Get(), &file3));
        REQUIRE_ARE_SAME(file2.Get(), file3.Get());

        REQUIRE_SUCCEEDED(files->MoveNext(&hasCurrent));
    }
    // The only files left should be footprint files
    expectedFiles.erase(MsixTest::Constants::Package::AppxBlockMap.second);
    expectedFiles.erase(MsixTest::Constants::Package::AppxManifest.second);
    expectedFiles.erase(MsixTest::Constants::Package::AppxSignature.second);
    expectedFiles.erase(MsixTest::Constants::Package::CodeIntegrity.second);
    REQUIRE(expectedFiles.empty());
}

// Verifies a payload file information from the package
TEST_CASE("Api_AppxPackageReader_PayloadFile", "[api]")
{
    std::string package = "StoreSigned_Desktop_x64_MoviesTV.appx";
    MsixTest::ComPtr<IAppxPackageReader> packageReader;
    InitializePackageReader(package, &packageReader);

    MsixTest::ComPtr<IAppxFile> appxFile;

    // Even on non windows, GetPayloadFile expects a '\'
    std::string fileName = "Assets\\video_offline_demo_page2.jpg";
    auto fileNameW = MsixTest::String::utf8_to_utf16(fileName);

    REQUIRE_SUCCEEDED(packageReader->GetPayloadFile(fileNameW.c_str(), &appxFile));
    REQUIRE_NOT_NULL(appxFile.Get());

    MsixTest::Wrappers::Buffer<wchar_t> appxFileName;
    REQUIRE_SUCCEEDED(appxFile->GetName(&appxFileName));
    REQUIRE(fileName == appxFileName.ToString());

    APPX_COMPRESSION_OPTION fileCompression;
    REQUIRE_SUCCEEDED(appxFile->GetCompressionOption(&fileCompression));
    REQUIRE(APPX_COMPRESSION_OPTION_NONE == fileCompression);

    UINT64 fileSize;
    REQUIRE_SUCCEEDED(appxFile->GetSize(&fileSize));
    REQUIRE(78720 == static_cast<std::uint64_t>(fileSize));
}

// Validate a file is not in the package.
TEST_CASE("Api_AppxPackageReader_PayloadFile_DoesNotExist", "[api]")
{
    std::string package = "StoreSigned_Desktop_x64_MoviesTV.appx";
    MsixTest::ComPtr<IAppxPackageReader> packageReader;
    InitializePackageReader(package, &packageReader);

    MsixTest::ComPtr<IAppxFile> appxFile;
    REQUIRE_HR(static_cast<HRESULT>(MSIX::Error::FileNotFound),
        packageReader->GetPayloadFile(L"thisIsAFakeFile.txt", &appxFile));
}

// Validates a footprint files
TEST_CASE("Api_AppxPackageReader_FootprintFile", "[api]")
{
    std::string package = "StoreSigned_Desktop_x64_MoviesTV.appx";
    MsixTest::ComPtr<IAppxPackageReader> packageReader;
    InitializePackageReader(package, &packageReader);

    MsixTest::ComPtr<IAppxFile> appxBlockMap;
    REQUIRE_SUCCEEDED(packageReader->GetFootprintFile(MsixTest::Constants::Package::AppxBlockMap.first, &appxBlockMap));
    MsixTest::Wrappers::Buffer<wchar_t> appxBlockMapName;
    REQUIRE_SUCCEEDED(appxBlockMap->GetName(&appxBlockMapName));
    REQUIRE(MsixTest::Constants::Package::AppxBlockMap.second == appxBlockMapName.ToString());

    MsixTest::ComPtr<IAppxFile> appxManifest;
    REQUIRE_SUCCEEDED(packageReader->GetFootprintFile(MsixTest::Constants::Package::AppxManifest.first, &appxManifest));
    MsixTest::Wrappers::Buffer<wchar_t> appxManifestName;
    REQUIRE_SUCCEEDED(appxManifest->GetName(&appxManifestName));
    REQUIRE(MsixTest::Constants::Package::AppxManifest.second == appxManifestName.ToString());

    MsixTest::ComPtr<IAppxFile> appxSignature;
    REQUIRE_SUCCEEDED(packageReader->GetFootprintFile(MsixTest::Constants::Package::AppxSignature.first, &appxSignature));
    MsixTest::Wrappers::Buffer<wchar_t> appxSignatureName;
    REQUIRE_SUCCEEDED(appxSignature->GetName(&appxSignatureName));
    REQUIRE(MsixTest::Constants::Package::AppxSignature.second == appxSignatureName.ToString());

    MsixTest::ComPtr<IAppxFile> appxCodeIntegrity;
    REQUIRE_SUCCEEDED(packageReader->GetFootprintFile(MsixTest::Constants::Package::CodeIntegrity.first, &appxCodeIntegrity));
    MsixTest::Wrappers::Buffer<wchar_t> appxCodeIntegrityName;
    REQUIRE_SUCCEEDED(appxCodeIntegrity->GetName(&appxCodeIntegrityName));
    auto codeIntegrityName = MsixTest::Constants::Package::CodeIntegrity.second;
    std::replace(codeIntegrityName.begin(), codeIntegrityName.end(), '/', '\\');
    REQUIRE(codeIntegrityName == appxCodeIntegrityName.ToString());
}

// Validates IAppxManifestReader::GetStream
TEST_CASE("Api_AppxManifestReader_Stream", "[api]")
{
    std::string package = "StoreSigned_Desktop_x64_MoviesTV.appx";
    MsixTest::ComPtr<IAppxPackageReader> packageReader;
    InitializePackageReader(package, &packageReader);
    MsixTest::ComPtr<IAppxManifestReader> manifestReader;
    REQUIRE_SUCCEEDED(packageReader->GetManifest(&manifestReader));
    REQUIRE_NOT_NULL(manifestReader.Get());

    MsixTest::ComPtr<IStream> stream;
    REQUIRE_SUCCEEDED(manifestReader->GetStream(&stream));
    REQUIRE_NOT_NULL(stream.Get());
}

// Validates application elements in the manifest. IAppxManifestApplicationsEnumerator and IAppxManifestApplication
TEST_CASE("Api_AppxManifestReader_Applications", "[api]")
{
    std::string package = "StoreSigned_Desktop_x64_MoviesTV.appx";
    MsixTest::ComPtr<IAppxPackageReader> packageReader;
    InitializePackageReader(package, &packageReader);
    MsixTest::ComPtr<IAppxManifestReader> manifestReader;
    REQUIRE_SUCCEEDED(packageReader->GetManifest(&manifestReader));
    REQUIRE_NOT_NULL(manifestReader.Get());

    MsixTest::ComPtr<IAppxManifestApplicationsEnumerator> enumerator;
    REQUIRE_SUCCEEDED(manifestReader->GetApplications(&enumerator));
    BOOL hasCurrent = FALSE;
    REQUIRE_SUCCEEDED(enumerator->GetHasCurrent(&hasCurrent));
    int numOfApps = 0;
    while(hasCurrent)
    {
        MsixTest::ComPtr<IAppxManifestApplication> app;
        REQUIRE_SUCCEEDED(enumerator->GetCurrent(&app));

        std::string expectedAumid = "Microsoft.ZuneVideo_8wekyb3d8bbwe!Microsoft.ZuneVideo";

        MsixTest::Wrappers::Buffer<wchar_t> aumid;
        REQUIRE_SUCCEEDED(app->GetAppUserModelId(&aumid));
        REQUIRE(expectedAumid == aumid.ToString());

        MsixTest::Wrappers::Buffer<char> aumidUtf8;
        MsixTest::ComPtr<IAppxManifestApplicationUtf8> appUtf8;
        REQUIRE_SUCCEEDED(app->QueryInterface(UuidOfImpl<IAppxManifestApplicationUtf8>::iid, reinterpret_cast<void**>(&appUtf8)));
        REQUIRE_SUCCEEDED(appUtf8->GetAppUserModelId(&aumidUtf8));
        REQUIRE(expectedAumid == aumidUtf8.ToString());

        // Note: this isn't implemented. But adding check so when we do, we update this test too.
        MsixTest::Wrappers::Buffer<wchar_t> value;
        REQUIRE_HR(static_cast<HRESULT>(MSIX::Error::NotImplemented), app->GetStringValue(L"fakeValue", &value));

        REQUIRE_SUCCEEDED(enumerator->MoveNext(&hasCurrent));
        numOfApps++;
    }
    REQUIRE(1 == numOfApps);
}

// Validates manifest properties. IAppxManifestPropertie and IAppxManifestPropertiesUtf8
TEST_CASE("Api_AppxManifestReader_Properties", "[api]")
{
    std::string package = "StoreSigned_Desktop_x64_MoviesTV.appx";
    MsixTest::ComPtr<IAppxPackageReader> packageReader;
    InitializePackageReader(package, &packageReader);
    MsixTest::ComPtr<IAppxManifestReader> manifestReader;
    REQUIRE_SUCCEEDED(packageReader->GetManifest(&manifestReader));
    REQUIRE_NOT_NULL(manifestReader.Get());

    MsixTest::ComPtr<IAppxManifestProperties> properties;
    MsixTest::ComPtr<IAppxManifestPropertiesUtf8> propertiesUtf8;
    REQUIRE_SUCCEEDED(manifestReader->GetProperties(&properties));
    REQUIRE_SUCCEEDED(properties->QueryInterface(UuidOfImpl<IAppxManifestPropertiesUtf8>::iid, reinterpret_cast<void**>(&propertiesUtf8)));

    std::array<std::pair<std::string, BOOL>, 3> testBoolValues =
    { {
        { "Framework", FALSE },
        { "AllowExecution", TRUE },
        { "ResourcePackage", FALSE }
    } };

    for (const auto& test : testBoolValues)
    {
        BOOL result = FALSE;
        auto nameW = MsixTest::String::utf8_to_utf16(test.first);
        REQUIRE_SUCCEEDED(properties->GetBoolValue(nameW.c_str(), &result));
        REQUIRE(test.second == result);

        BOOL resultUtf8 = FALSE;
        REQUIRE_SUCCEEDED(propertiesUtf8->GetBoolValue(test.first.c_str(), &resultUtf8));
        REQUIRE(test.second == resultUtf8);
    }

    std::array<std::pair<std::string, std::string>, 4> testStringValues
    { {
        { "DisplayName", "ms-resource:IDS_MANIFEST_VIDEO_APP_NAME" },
        { "PublisherDisplayName", "Microsoft Corporation" },
        { "Description", "ms-resource:IDS_MANIFEST_VIDEO_APP_DESCRIPTION" },
        { "Logo", "Assets\\Movie-TVStoreLogo.png" }
    } };

    for (const auto& test : testStringValues)
    {
        MsixTest::Wrappers::Buffer<wchar_t> value;
        auto nameW = MsixTest::String::utf8_to_utf16(test.first);
        REQUIRE_SUCCEEDED(properties->GetStringValue(nameW.c_str(), &value));
        REQUIRE(test.second == value.ToString());

        MsixTest::Wrappers::Buffer<char> valueUtf8;
        REQUIRE_SUCCEEDED(propertiesUtf8->GetStringValue(test.first.c_str(), &valueUtf8));
        REQUIRE(test.second == valueUtf8.ToString());
    }

    // Test with invalid values
    BOOL result = FALSE;
    REQUIRE_HR(static_cast<HRESULT>(MSIX::Error::InvalidParameter),
        properties->GetBoolValue(L"InvalidValue", &result));
    REQUIRE_HR(static_cast<HRESULT>(MSIX::Error::InvalidParameter),
        propertiesUtf8->GetBoolValue("InvalidValue", &result));

    MsixTest::Wrappers::Buffer<wchar_t> value;
    REQUIRE_HR(static_cast<HRESULT>(MSIX::Error::InvalidParameter),
        properties->GetStringValue(L"InvalidValue", &value));

    MsixTest::Wrappers::Buffer<char> valueUtf8;
    REQUIRE_HR(static_cast<HRESULT>(MSIX::Error::InvalidParameter),
        propertiesUtf8->GetStringValue("InvalidValue", &valueUtf8));
}

// Validates package dependencies. IAppxManifestPackageDependency and IAppxManifestPackageDependencyUtf8
TEST_CASE("Api_AppxManifestReader_PackageDependencies", "[api]")
{
    std::string package = "StoreSigned_Desktop_x64_MoviesTV.appx";
    MsixTest::ComPtr<IAppxPackageReader> packageReader;
    InitializePackageReader(package, &packageReader);
    MsixTest::ComPtr<IAppxManifestReader> manifestReader;
    REQUIRE_SUCCEEDED(packageReader->GetManifest(&manifestReader));
    REQUIRE_NOT_NULL(manifestReader.Get());

    MsixTest::ComPtr<IAppxManifestPackageDependenciesEnumerator> dependencies;
    REQUIRE_SUCCEEDED(manifestReader->GetPackageDependencies(&dependencies));

    BOOL hasCurrent = FALSE;
    REQUIRE_SUCCEEDED(dependencies->GetHasCurrent(&hasCurrent));
    int numDep = 0;
    while (hasCurrent)
    {
        MsixTest::ComPtr<IAppxManifestPackageDependency> dependency;
        REQUIRE_SUCCEEDED(dependencies->GetCurrent(&dependency));

        MsixTest::ComPtr<IAppxManifestPackageDependencyUtf8> dependencyUtf8;
        REQUIRE_SUCCEEDED(dependency->QueryInterface(UuidOfImpl<IAppxManifestPackageDependencyUtf8>::iid, reinterpret_cast<void**>(&dependencyUtf8)));

        std::string expectedName = "Microsoft.VCLibs.140.00";
        MsixTest::Wrappers::Buffer<wchar_t> name;
        REQUIRE_SUCCEEDED(dependency->GetName(&name));
        REQUIRE(expectedName == name.ToString());

        MsixTest::Wrappers::Buffer<char> nameUtf8;
        REQUIRE_SUCCEEDED(dependencyUtf8->GetName(&nameUtf8));
        REQUIRE(expectedName == nameUtf8.ToString());

        UINT64 expectedMin = 3940649673949184; // 14.0.0.0
        UINT64 min = 0;
        REQUIRE_SUCCEEDED(dependency->GetMinVersion(&min));
        REQUIRE(expectedMin == min);

        std::string expectedPublisher = "CN=Microsoft Corporation, O=Microsoft Corporation, L=Redmond, S=Washington, C=US";
        MsixTest::Wrappers::Buffer<wchar_t> publisher;
        REQUIRE_SUCCEEDED(dependency->GetPublisher(&publisher));
        REQUIRE(expectedPublisher == publisher.ToString());

        MsixTest::Wrappers::Buffer<char> publisherUtf8;
        REQUIRE_SUCCEEDED(dependencyUtf8->GetPublisher(&publisherUtf8));
        REQUIRE(expectedPublisher == publisherUtf8.ToString());

        REQUIRE_SUCCEEDED(dependencies->MoveNext(&hasCurrent));
        numDep++;
    }
    REQUIRE(1 == numDep);
}

// Validates manifest capabilities IAppxManifestReader::GetCapabilities
// NOTE: There's currently a bug in non-windows devices were we only return
// the elements that doesn't the same namespace as the xmlns of the manifest.
// TODO: fix...
TEST_CASE("Api_AppxManifestReader_Capabilities", "[api][!hide]")
{
    std::string package = "StoreSigned_Desktop_x64_MoviesTV.appx";
    MsixTest::ComPtr<IAppxPackageReader> packageReader;
    InitializePackageReader(package, &packageReader);
    MsixTest::ComPtr<IAppxManifestReader> manifestReader;
    REQUIRE_SUCCEEDED(packageReader->GetManifest(&manifestReader));
    REQUIRE_NOT_NULL(manifestReader.Get());

    APPX_CAPABILITIES expected = static_cast<APPX_CAPABILITIES>(
        APPX_CAPABILITY_INTERNET_CLIENT |
        APPX_CAPABILITY_PRIVATE_NETWORK_CLIENT_SERVER |
        APPX_CAPABILITY_VIDEOS_LIBRARY |
        APPX_CAPABILITY_REMOVABLE_STORAGE);

    APPX_CAPABILITIES capabilities;
    REQUIRE_SUCCEEDED(manifestReader->GetCapabilities(&capabilities));
    REQUIRE(expected == capabilities);
}

// Validates manifest resources. IAppxManifestResourcesEnumerator and IAppxManifestResourcesEnumeratorUtf8
TEST_CASE("Api_AppxManifestReader_Resources", "[api]")
{
    std::string package = "StoreSigned_Desktop_x64_MoviesTV.appx";
    MsixTest::ComPtr<IAppxPackageReader> packageReader;
    InitializePackageReader(package, &packageReader);
    MsixTest::ComPtr<IAppxManifestReader> manifestReader;
    REQUIRE_SUCCEEDED(packageReader->GetManifest(&manifestReader));
    REQUIRE_NOT_NULL(manifestReader.Get());

    std::array<std::string, 3> expectedResources =
    {
        "en",
        "en-US",
        "en-GB"
    };

    MsixTest::ComPtr<IAppxManifestResourcesEnumerator> resources;
    REQUIRE_SUCCEEDED(manifestReader->GetResources(&resources));
    MsixTest::ComPtr<IAppxManifestResourcesEnumeratorUtf8> resourcesUtf8;
    REQUIRE_SUCCEEDED(resources->QueryInterface(UuidOfImpl<IAppxManifestResourcesEnumeratorUtf8>::iid, reinterpret_cast<void**>(&resourcesUtf8)));

    BOOL hasCurrent = FALSE;
    REQUIRE_SUCCEEDED(resources->GetHasCurrent(&hasCurrent));
    std::size_t numOfDependencies = 0;
    while (hasCurrent)
    {
        MsixTest::Wrappers::Buffer<wchar_t> resource;
        REQUIRE_SUCCEEDED(resources->GetCurrent(&resource));
        REQUIRE(expectedResources[numOfDependencies] == resource.ToString());

        MsixTest::Wrappers::Buffer<char> resourceUtf8;
        REQUIRE_SUCCEEDED(resourcesUtf8->GetCurrent(&resourceUtf8));
        REQUIRE(expectedResources[numOfDependencies] == resourceUtf8.ToString());

        REQUIRE_SUCCEEDED(resources->MoveNext(&hasCurrent));
        numOfDependencies++;
    }
    REQUIRE(expectedResources.size() == numOfDependencies);
}

// Validates manifest target family devices. IAppxManifestTargetDeviceFamily and IAppxManifestTargetDeviceFamilyUtf8
TEST_CASE("Api_AppxManifestReader_Tdf", "[api]")
{
    std::string package = "StoreSigned_Desktop_x64_MoviesTV.appx";
    MsixTest::ComPtr<IAppxPackageReader> packageReader;
    InitializePackageReader(package, &packageReader);
    MsixTest::ComPtr<IAppxManifestReader> manifestReader;
    REQUIRE_SUCCEEDED(packageReader->GetManifest(&manifestReader));
    REQUIRE_NOT_NULL(manifestReader.Get());

    MsixTest::ComPtr<IAppxManifestReader3> manifestReader3;
    REQUIRE_SUCCEEDED(manifestReader->QueryInterface(UuidOfImpl<IAppxManifestReader3>::iid, reinterpret_cast<void**>(&manifestReader3)));
    MsixTest::ComPtr<IAppxManifestTargetDeviceFamiliesEnumerator> tdfEnum;
    REQUIRE_SUCCEEDED(manifestReader3->GetTargetDeviceFamilies(&tdfEnum));
    BOOL hasCurrent = FALSE;
    REQUIRE_SUCCEEDED(tdfEnum->GetHasCurrent(&hasCurrent));
    int numOfTdfs = 0;
    while (hasCurrent)
    {
        MsixTest::ComPtr<IAppxManifestTargetDeviceFamily> tdf;
        REQUIRE_SUCCEEDED(tdfEnum->GetCurrent(&tdf));

        std::string expectedName = "Windows.Desktop";
        MsixTest::Wrappers::Buffer<wchar_t> name;
        REQUIRE_SUCCEEDED(tdf->GetName(&name));
        REQUIRE(expectedName == name.ToString());

        MsixTest::ComPtr<IAppxManifestTargetDeviceFamilyUtf8> tdfUtf8;
        REQUIRE_SUCCEEDED(tdf->QueryInterface(UuidOfImpl<IAppxManifestTargetDeviceFamilyUtf8>::iid, reinterpret_cast<void**>(&tdfUtf8)));
        MsixTest::Wrappers::Buffer<char> nameUtf8;
        REQUIRE_SUCCEEDED(tdfUtf8->GetName(&nameUtf8));
        REQUIRE(expectedName == nameUtf8.ToString());

        UINT64 expectedMin = 2814750438195200; // 10.0.10586.0
        UINT64 min = 0;
        REQUIRE_SUCCEEDED(tdf->GetMinVersion(&min));
        REQUIRE(expectedMin == min);

        UINT64 expectedMax = 2814750704271360; // 10.0.16172.0
        UINT64 max = 0;
        REQUIRE_SUCCEEDED(tdf->GetMaxVersionTested(&max));
        REQUIRE(expectedMax == max);

        REQUIRE_SUCCEEDED(tdfEnum->MoveNext(&hasCurrent));
        numOfTdfs++;
    }
    REQUIRE(1 == numOfTdfs);
}

// "Validates IMsixDocumentElement, IMsixElement and IMsixElementEnumerator"
TEST_CASE("Api_AppxManifestReader_MsixDocument", "[api]")
{
    std::string package = "StoreSigned_Desktop_x64_MoviesTV.appx";
    MsixTest::ComPtr<IAppxPackageReader> packageReader;
    InitializePackageReader(package, &packageReader);
    MsixTest::ComPtr<IAppxManifestReader> manifestReader;
    REQUIRE_SUCCEEDED(packageReader->GetManifest(&manifestReader));
    REQUIRE_NOT_NULL(manifestReader.Get());

    MsixTest::ComPtr<IMsixDocumentElement> msixDocument;
    REQUIRE_SUCCEEDED(manifestReader->QueryInterface(UuidOfImpl<IMsixDocumentElement>::iid, reinterpret_cast<void**>(&msixDocument)));
    MsixTest::ComPtr<IMsixElement> manifestElement;
    REQUIRE_SUCCEEDED(msixDocument->GetDocumentElement(&manifestElement));

    #ifdef MSIX_MSXML6
    std::string xpath = "/*[local-name()='Package']/*[local-name()='Extensions']/*[local-name()='Extension']";
    #else
    std::string xpath = "/Package/Extensions/Extension";
    #endif
    auto xpathW = MsixTest::String::utf8_to_utf16(xpath);

    std::array<std::string, 3> expectedCategories = 
    {
        "windows.activatableClass.inProcessServer",
        "windows.activatableClass.inProcessServer",
        "windows.activatableClass.proxyStub"
    };

    std::string attributeName = "Category";
    auto attributeNameW = MsixTest::String::utf8_to_utf16(attributeName);

    MsixTest::ComPtr<IMsixElementEnumerator> elementEnum;
    REQUIRE_SUCCEEDED(manifestElement->GetElements(xpathW.c_str(), &elementEnum));
    BOOL hasCurrent = FALSE;
    size_t numOfElements = 0;
    REQUIRE_SUCCEEDED(elementEnum->GetHasCurrent(&hasCurrent));
    while(hasCurrent)
    {
        MsixTest::ComPtr<IMsixElement> element;
        REQUIRE_SUCCEEDED(elementEnum->GetCurrent(&element));

        MsixTest::Wrappers::Buffer<wchar_t> value;
        REQUIRE_SUCCEEDED(element->GetAttributeValue(attributeNameW.c_str(), &value));
        REQUIRE(expectedCategories[numOfElements] == value.ToString());

        MsixTest::Wrappers::Buffer<char> valueUtf8;
        REQUIRE_SUCCEEDED(element->GetAttributeValueUtf8(attributeName.c_str(), &valueUtf8));
        REQUIRE(expectedCategories[numOfElements] == valueUtf8.ToString());

        REQUIRE_SUCCEEDED(elementEnum->MoveNext(&hasCurrent));
        numOfElements++;
    }
    REQUIRE(expectedCategories.size() == numOfElements);

    MsixTest::ComPtr<IMsixElementEnumerator> elementEnumFromUtf8;
    REQUIRE_SUCCEEDED(manifestElement->GetElementsUtf8(xpath.c_str(), &elementEnumFromUtf8));
    hasCurrent = FALSE;
    numOfElements = 0;
    REQUIRE_SUCCEEDED(elementEnumFromUtf8->GetHasCurrent(&hasCurrent));
    while(hasCurrent)
    {
        MsixTest::ComPtr<IMsixElement> element;
        REQUIRE_SUCCEEDED(elementEnumFromUtf8->GetCurrent(&element));

        MsixTest::Wrappers::Buffer<wchar_t> value;
        REQUIRE_SUCCEEDED(element->GetAttributeValue(attributeNameW.c_str(), &value));
        REQUIRE(expectedCategories[numOfElements] == value.ToString());

        MsixTest::Wrappers::Buffer<char> valueUtf8;
        REQUIRE_SUCCEEDED(element->GetAttributeValueUtf8(attributeName.c_str(), &valueUtf8));
        REQUIRE(expectedCategories[numOfElements] == valueUtf8.ToString());

        REQUIRE_SUCCEEDED(elementEnumFromUtf8->MoveNext(&hasCurrent));
        numOfElements++;
    }
    REQUIRE(expectedCategories.size() == numOfElements);
}

TEST_CASE("Api_AppxManifestReader_PackageId", "[api]")
{
    std::string package = "StoreSigned_Desktop_x64_MoviesTV.appx";
    MsixTest::ComPtr<IAppxPackageReader> packageReader;
    InitializePackageReader(package, &packageReader);
    MsixTest::ComPtr<IAppxManifestReader> manifestReader;
    REQUIRE_SUCCEEDED(packageReader->GetManifest(&manifestReader));
    REQUIRE_NOT_NULL(manifestReader.Get());

    MsixTest::ComPtr<IAppxManifestPackageId> packageId;
    REQUIRE_SUCCEEDED(manifestReader->GetPackageId(&packageId));
    REQUIRE_NOT_NULL(packageId.Get());

    MsixTest::ComPtr<IAppxManifestPackageIdUtf8> packageIdUtf8;
    REQUIRE_SUCCEEDED(packageId->QueryInterface(UuidOfImpl<IAppxManifestPackageIdUtf8>::iid, reinterpret_cast<void**>(&packageIdUtf8)));

    std::string expectedName = "Microsoft.ZuneVideo";
    MsixTest::Wrappers::Buffer<wchar_t> name;
    REQUIRE_SUCCEEDED(packageId->GetName(&name));
    REQUIRE(expectedName == name.ToString());

    MsixTest::Wrappers::Buffer<char> nameUtf8;
    REQUIRE_SUCCEEDED(packageIdUtf8->GetName(&nameUtf8));
    REQUIRE(expectedName == nameUtf8.ToString());

    APPX_PACKAGE_ARCHITECTURE expectedArch = APPX_PACKAGE_ARCHITECTURE_X64;
    APPX_PACKAGE_ARCHITECTURE architecture;
    REQUIRE_SUCCEEDED(packageId->GetArchitecture(&architecture));
    REQUIRE(expectedArch == architecture);

    std::string expectedPublisher = "CN=Microsoft Corporation, O=Microsoft Corporation, L=Redmond, S=Washington, C=US";
    MsixTest::Wrappers::Buffer<wchar_t> publisher;
    REQUIRE_SUCCEEDED(packageId->GetPublisher(&publisher));
    REQUIRE(expectedPublisher == publisher.ToString());

    MsixTest::Wrappers::Buffer<char> publisherUtf8;
    REQUIRE_SUCCEEDED(packageIdUtf8->GetPublisher(&publisherUtf8));
    REQUIRE(expectedPublisher == publisherUtf8.ToString());

    BOOL isSame = FALSE;
    REQUIRE_SUCCEEDED(packageId->ComparePublisher(publisher.Get(), &isSame));
    REQUIRE(isSame == TRUE);
    isSame = FALSE;
    REQUIRE_SUCCEEDED(packageIdUtf8->ComparePublisher(publisherUtf8.Get(), &isSame));
    REQUIRE(isSame == TRUE);
    REQUIRE_SUCCEEDED(packageId->ComparePublisher(L"OtherPublisher", &isSame));
    REQUIRE(isSame == FALSE);

    UINT64 expectedVersion = 844452342988800; // 3.6.25071.0
    UINT64 packageVersion;
    REQUIRE_SUCCEEDED(packageId->GetVersion(&packageVersion));
    REQUIRE(expectedVersion == packageVersion);

    MsixTest::Wrappers::Buffer<wchar_t> resourceId;
    REQUIRE_SUCCEEDED(packageId->GetResourceId(&resourceId));
    REQUIRE(resourceId.Get() == nullptr);

    std::string expectedFull = "Microsoft.ZuneVideo_3.6.25071.0_x64__8wekyb3d8bbwe";
    MsixTest::Wrappers::Buffer<wchar_t> packageFullName;
    REQUIRE_SUCCEEDED(packageId->GetPackageFullName(&packageFullName));
    REQUIRE(expectedFull == packageFullName.ToString());

    MsixTest::Wrappers::Buffer<char> packageFullNameUtf8;
    REQUIRE_SUCCEEDED(packageIdUtf8->GetPackageFullName(&packageFullNameUtf8));
    REQUIRE(expectedFull == packageFullNameUtf8.ToString());

    std::string expectedFamily = "Microsoft.ZuneVideo_8wekyb3d8bbwe";
    MsixTest::Wrappers::Buffer<wchar_t> packageFamilyName;
    REQUIRE_SUCCEEDED(packageId->GetPackageFamilyName(&packageFamilyName));
    REQUIRE(expectedFamily == packageFamilyName.ToString());

    MsixTest::Wrappers::Buffer<char> packageFamilyNameUtf8;
    REQUIRE_SUCCEEDED(packageIdUtf8->GetPackageFamilyName(&packageFamilyNameUtf8));
    REQUIRE(expectedFamily == packageFamilyNameUtf8.ToString());
}

// Validates IAppxBlockMapReader::GetStream
TEST_CASE("Api_AppxBlockMapReader_Stream", "[api]")
{
    std::string package = "TestAppxPackage_Win32.appx";
    MsixTest::ComPtr<IAppxPackageReader> packageReader;
    InitializePackageReader(package, &packageReader);
    MsixTest::ComPtr<IAppxBlockMapReader> blockMapReader;
    REQUIRE_SUCCEEDED(packageReader->GetBlockMap(&blockMapReader));
    REQUIRE_NOT_NULL(blockMapReader.Get());

    MsixTest::ComPtr<IStream> stream;
    REQUIRE_SUCCEEDED(blockMapReader->GetStream(&stream));
    REQUIRE_NOT_NULL(stream.Get());
}

// Validates all files information in the blockmap, IAppxBlockMapReaderUtf8, 
// IAppxBlockMapFilesEnumerator, IAppxBlockMapFile and IAppxBlockMapBlock
TEST_CASE("Api_AppxBlockMapReader_Files", "[api]")
{
    std::string package = "TestAppxPackage_Win32.appx";
    MsixTest::ComPtr<IAppxPackageReader> packageReader;
    InitializePackageReader(package, &packageReader);
    MsixTest::ComPtr<IAppxBlockMapReader> blockMapReader;
    REQUIRE_SUCCEEDED(packageReader->GetBlockMap(&blockMapReader));
    REQUIRE_NOT_NULL(blockMapReader.Get());

    auto expectedBlockMapFiles = MsixTest::BlockMap::GetExpectedBlockMapFiles();

    MsixTest::ComPtr<IAppxBlockMapReaderUtf8> blockMapReaderUtf8;
    REQUIRE_SUCCEEDED(blockMapReader->QueryInterface(UuidOfImpl<IAppxBlockMapReaderUtf8>::iid, reinterpret_cast<void**>(&blockMapReaderUtf8)));

    MsixTest::ComPtr<IAppxBlockMapFilesEnumerator> blockMapFiles;
    REQUIRE_SUCCEEDED(blockMapReader->GetFiles(&blockMapFiles));

    BOOL hasCurrent = FALSE;
    REQUIRE_SUCCEEDED(blockMapFiles->GetHasCurrent(&hasCurrent));
    size_t numOfBlockMapFiles = 0;
    while(hasCurrent)
    {
        MsixTest::ComPtr<IAppxBlockMapFile> blockMapFile;
        REQUIRE_SUCCEEDED(blockMapFiles->GetCurrent(&blockMapFile));

        auto expectedFile = expectedBlockMapFiles.at(numOfBlockMapFiles);

        auto compareBlock = [](const std::vector<std::uint8_t>& expected, BYTE* buffer) -> bool
        {
            for(int i = 0; i < expected.size(); ++i)
            {
                if (expected[i] != static_cast<std::uint8_t>(buffer[i]))
                {
                    return false;
                }
            }
            return true;
        };

        MsixTest::Wrappers::Buffer<wchar_t> fileName;
        REQUIRE_SUCCEEDED(blockMapFile->GetName(&fileName));
        REQUIRE(expectedFile.name == fileName.ToString());

        MsixTest::ComPtr<IAppxBlockMapFileUtf8> blockMapFileUtf8;
        REQUIRE_SUCCEEDED(blockMapFile->QueryInterface(UuidOfImpl<IAppxBlockMapFileUtf8>::iid, reinterpret_cast<void**>(&blockMapFileUtf8)));
        MsixTest::Wrappers::Buffer<char> fileNameUtf8;
        REQUIRE_SUCCEEDED(blockMapFileUtf8->GetName(&fileNameUtf8));
        REQUIRE(expectedFile.name == fileNameUtf8.ToString());

        MsixTest::ComPtr<IAppxBlockMapFile> blockMapFile2;
        REQUIRE_SUCCEEDED(blockMapReader->GetFile(fileName.Get(), &blockMapFile2));
        REQUIRE_ARE_SAME(blockMapFile.Get(), blockMapFile2.Get());

        MsixTest::ComPtr<IAppxBlockMapFile> blockMapFile3;
        REQUIRE_SUCCEEDED(blockMapReaderUtf8->GetFile(fileNameUtf8.Get(), &blockMapFile3));
        REQUIRE_ARE_SAME(blockMapFile.Get(), blockMapFile3.Get());

        UINT32 lfh = 0;
        REQUIRE_SUCCEEDED(blockMapFile->GetLocalFileHeaderSize(&lfh));
        REQUIRE(expectedFile.lfh == static_cast<std::uint32_t>(lfh));

        UINT64 size = 0;
        REQUIRE_SUCCEEDED(blockMapFile->GetUncompressedSize(&size));
        REQUIRE(expectedFile.size == size);

        // Compare blocks
        MsixTest::ComPtr<IAppxBlockMapBlocksEnumerator> blockEnum;
        REQUIRE_SUCCEEDED(blockMapFile->GetBlocks(&blockEnum));
        BOOL hasCurrentBlock = FALSE;
        REQUIRE_SUCCEEDED(blockEnum->GetHasCurrent(&hasCurrentBlock));
        size_t numOfBlocks = 0;
        while (hasCurrentBlock)
        {
            MsixTest::ComPtr<IAppxBlockMapBlock> block;
            REQUIRE_SUCCEEDED(blockEnum->GetCurrent(&block));

            auto expectedBlock = expectedFile.blocks[numOfBlocks];

            UINT32 bufferSize = 0;
            MsixTest::Wrappers::Buffer<BYTE> buffer;
            REQUIRE_SUCCEEDED(block->GetHash(&bufferSize, &buffer));
            REQUIRE(expectedBlock.hash.size() == bufferSize);
            REQUIRE(compareBlock(expectedBlock.hash, buffer.Get()));

            UINT32 compSize = 0;
            REQUIRE_SUCCEEDED(block->GetCompressedSize(&compSize));
            REQUIRE(expectedBlock.compressedSize == static_cast<std::uint32_t>(compSize));

            REQUIRE_SUCCEEDED(blockEnum->MoveNext(&hasCurrentBlock));
            numOfBlocks++;
        }
        REQUIRE(expectedFile.blocks.size() == numOfBlocks);

        REQUIRE_SUCCEEDED(blockMapFiles->MoveNext(&hasCurrent));
        numOfBlockMapFiles++;
    }
    REQUIRE(expectedBlockMapFiles.size() == numOfBlockMapFiles);
}
