//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
//  Validates IAppxManifestReader interface and interfaces related to the AppxManifest.xml
#include "catch.hpp"
#include "msixtest_int.hpp"
#include "FileHelpers.hpp"
#include "macros.hpp"

#include <iostream>
#include <array>

// Validates IAppxManifestReader::GetStream
TEST_CASE("Api_AppxManifestReader_Stream", "[api]")
{
    std::string manifest = "Sample_AppxManifest.xml";
    MsixTest::ComPtr<IAppxManifestReader> manifestReader;
    MsixTest::InitializeManifestReader(manifest, &manifestReader);

    MsixTest::ComPtr<IStream> stream;
    REQUIRE_SUCCEEDED(manifestReader->GetStream(&stream));
    REQUIRE_NOT_NULL(stream.Get());
}

// Validates application elements in the manifest. IAppxManifestApplicationsEnumerator and IAppxManifestApplication
TEST_CASE("Api_AppxManifestReader_Applications", "[api]")
{
    std::string manifest = "Sample_AppxManifest.xml";
    MsixTest::ComPtr<IAppxManifestReader> manifestReader;
    MsixTest::InitializeManifestReader(manifest, &manifestReader);

    MsixTest::ComPtr<IAppxManifestApplicationsEnumerator> enumerator;
    REQUIRE_SUCCEEDED(manifestReader->GetApplications(&enumerator));
    BOOL hasCurrent = FALSE;
    REQUIRE_SUCCEEDED(enumerator->GetHasCurrent(&hasCurrent));
    int numOfApps = 0;
    while(hasCurrent)
    {
        MsixTest::ComPtr<IAppxManifestApplication> app;
        REQUIRE_SUCCEEDED(enumerator->GetCurrent(&app));

        std::string expectedAumid = "SampleAppManifest_8wekyb3d8bbwe!Mca.App";

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
    std::string manifest = "Sample_AppxManifest.xml";
    MsixTest::ComPtr<IAppxManifestReader> manifestReader;
    MsixTest::InitializeManifestReader(manifest, &manifestReader);

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
        { "DisplayName", "Sample app manifest" },
        { "PublisherDisplayName", "Microsoft Corporation" },
        { "Description", "This is a sample app manifest. It is not representative of what the manifest of a typical app would look like." },
        { "Logo", "logo.jpeg" }
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
    std::string manifest = "Sample_AppxManifest.xml";
    MsixTest::ComPtr<IAppxManifestReader> manifestReader;
    MsixTest::InitializeManifestReader(manifest, &manifestReader);

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
TEST_CASE("Api_AppxManifestReader_Capabilities", "[api]")
{
    std::string manifest = "Sample_AppxManifest.xml";
    MsixTest::ComPtr<IAppxManifestReader> manifestReader;
    MsixTest::InitializeManifestReader(manifest, &manifestReader);

    APPX_CAPABILITIES expected = static_cast<APPX_CAPABILITIES>(
        APPX_CAPABILITY_INTERNET_CLIENT |
        APPX_CAPABILITY_PRIVATE_NETWORK_CLIENT_SERVER |
        APPX_CAPABILITY_VIDEOS_LIBRARY |
        APPX_CAPABILITY_REMOVABLE_STORAGE);

    APPX_CAPABILITIES capabilities;
    REQUIRE_SUCCEEDED(manifestReader->GetCapabilities(&capabilities));
    REQUIRE(expected == capabilities);
}

// Validates manifest capabilities IAppxManifestReader::GetCapabilitiesByCapabilityClass
TEST_CASE("Api_AppxManifestReader_GetCapabilitiesByCapabilityClass", "[api]")
{
    std::string manifest = "Sample_AppxManifest.xml";
    MsixTest::ComPtr<IAppxManifestReader> manifestReader;
    MsixTest::InitializeManifestReader(manifest, &manifestReader);
    MsixTest::ComPtr<IAppxManifestReader3> manifestReader3;
    REQUIRE_SUCCEEDED(manifestReader->QueryInterface(UuidOfImpl<IAppxManifestReader3>::iid, reinterpret_cast<void**>(&manifestReader3)));

    std::vector<std::string> expectedValuesDefaultAndGeneral = 
    {
        "internetClient",
        "privateNetworkClientServer",
        "videosLibrary",
        "removableStorage",
    };

    MsixTest::ComPtr<IAppxManifestCapabilitiesEnumerator> capabilitiesDefault;
    REQUIRE_SUCCEEDED(manifestReader3->GetCapabilitiesByCapabilityClass(APPX_CAPABILITY_CLASS_GENERAL, &capabilitiesDefault));
    MsixTest::ComPtr<IAppxManifestCapabilitiesEnumeratorUtf8> capabilitiesDefaultUtf8;
    REQUIRE_SUCCEEDED(capabilitiesDefault->QueryInterface(UuidOfImpl<IAppxManifestCapabilitiesEnumeratorUtf8>::iid, reinterpret_cast<void**>(&capabilitiesDefaultUtf8)));

    BOOL hasCurrent = FALSE;
    REQUIRE_SUCCEEDED(capabilitiesDefault->GetHasCurrent(&hasCurrent));
    std::size_t numOfCaps = 0;
    while (hasCurrent)
    {
        MsixTest::Wrappers::Buffer<wchar_t> capability;
        REQUIRE_SUCCEEDED(capabilitiesDefault->GetCurrent(&capability));
        REQUIRE(expectedValuesDefaultAndGeneral[numOfCaps] == capability.ToString());

        MsixTest::Wrappers::Buffer<char> capabilityUtf8;
        REQUIRE_SUCCEEDED(capabilitiesDefaultUtf8->GetCurrent(&capabilityUtf8));
        REQUIRE(expectedValuesDefaultAndGeneral[numOfCaps] == capabilityUtf8.ToString());

        REQUIRE_SUCCEEDED(capabilitiesDefault->MoveNext(&hasCurrent));
        numOfCaps++;
    }
    REQUIRE(expectedValuesDefaultAndGeneral.size() == numOfCaps);

    std::vector<std::string> expectedValuesRestricted = 
    {
        "enterpriseDataPolicy",
        "previewStore",
    };

    MsixTest::ComPtr<IAppxManifestCapabilitiesEnumerator> capabilitiesRestricted;
    REQUIRE_SUCCEEDED(manifestReader3->GetCapabilitiesByCapabilityClass(APPX_CAPABILITY_CLASS_RESTRICTED, &capabilitiesRestricted));
    MsixTest::ComPtr<IAppxManifestCapabilitiesEnumeratorUtf8> capabilitiesRestrictedUtf8;
    REQUIRE_SUCCEEDED(capabilitiesRestricted->QueryInterface(UuidOfImpl<IAppxManifestCapabilitiesEnumeratorUtf8>::iid, reinterpret_cast<void**>(&capabilitiesRestrictedUtf8)));

    hasCurrent = FALSE;
    REQUIRE_SUCCEEDED(capabilitiesRestricted->GetHasCurrent(&hasCurrent));
    numOfCaps = 0;
    while (hasCurrent)
    {
        MsixTest::Wrappers::Buffer<wchar_t> capability;
        REQUIRE_SUCCEEDED(capabilitiesRestricted->GetCurrent(&capability));
        REQUIRE(expectedValuesRestricted[numOfCaps] == capability.ToString());

        MsixTest::Wrappers::Buffer<char> capabilityUtf8;
        REQUIRE_SUCCEEDED(capabilitiesRestrictedUtf8->GetCurrent(&capabilityUtf8));
        REQUIRE(expectedValuesRestricted[numOfCaps] == capabilityUtf8.ToString());

        REQUIRE_SUCCEEDED(capabilitiesRestricted->MoveNext(&hasCurrent));
        numOfCaps++;
    }
    REQUIRE(expectedValuesRestricted.size() == numOfCaps);

    std::vector<std::string> expectedValuesWindows = 
    {
        "shellExperience",
    };

    MsixTest::ComPtr<IAppxManifestCapabilitiesEnumerator> capabilitiesWindows;
    REQUIRE_SUCCEEDED(manifestReader3->GetCapabilitiesByCapabilityClass(APPX_CAPABILITY_CLASS_WINDOWS, &capabilitiesWindows));
    MsixTest::ComPtr<IAppxManifestCapabilitiesEnumeratorUtf8> capabilitiesWindowsUtf8;
    REQUIRE_SUCCEEDED(capabilitiesWindows->QueryInterface(UuidOfImpl<IAppxManifestCapabilitiesEnumeratorUtf8>::iid, reinterpret_cast<void**>(&capabilitiesWindowsUtf8)));

    hasCurrent = FALSE;
    REQUIRE_SUCCEEDED(capabilitiesWindows->GetHasCurrent(&hasCurrent));
    numOfCaps = 0;
    while (hasCurrent)
    {
        MsixTest::Wrappers::Buffer<wchar_t> capability;
        REQUIRE_SUCCEEDED(capabilitiesWindows->GetCurrent(&capability));
        REQUIRE(expectedValuesWindows[numOfCaps] == capability.ToString());

        MsixTest::Wrappers::Buffer<char> capabilityUtf8;
        REQUIRE_SUCCEEDED(capabilitiesWindowsUtf8->GetCurrent(&capabilityUtf8));
        REQUIRE(expectedValuesWindows[numOfCaps] == capabilityUtf8.ToString());

        REQUIRE_SUCCEEDED(capabilitiesWindows->MoveNext(&hasCurrent));
        numOfCaps++;
    }
    REQUIRE(expectedValuesWindows.size() == numOfCaps);

    std::vector<std::string> expectedValuesCustom = 
    {
        "fabrikam.CustomCap_5w7kyb3d8bbwe",
    };

    MsixTest::ComPtr<IAppxManifestCapabilitiesEnumerator> capabilitiesCustom;
    REQUIRE_SUCCEEDED(manifestReader3->GetCapabilitiesByCapabilityClass(APPX_CAPABILITY_CLASS_CUSTOM, &capabilitiesCustom));
    MsixTest::ComPtr<IAppxManifestCapabilitiesEnumeratorUtf8> capabilitiesCustomUtf8;
    REQUIRE_SUCCEEDED(capabilitiesCustom->QueryInterface(UuidOfImpl<IAppxManifestCapabilitiesEnumeratorUtf8>::iid, reinterpret_cast<void**>(&capabilitiesCustomUtf8)));

    hasCurrent = FALSE;
    REQUIRE_SUCCEEDED(capabilitiesCustom->GetHasCurrent(&hasCurrent));
    numOfCaps = 0;
    while (hasCurrent)
    {
        MsixTest::Wrappers::Buffer<wchar_t> capability;
        REQUIRE_SUCCEEDED(capabilitiesCustom->GetCurrent(&capability));
        REQUIRE(expectedValuesCustom[numOfCaps] == capability.ToString());

        MsixTest::Wrappers::Buffer<char> capabilityUtf8;
        REQUIRE_SUCCEEDED(capabilitiesCustomUtf8->GetCurrent(&capabilityUtf8));
        REQUIRE(expectedValuesCustom[numOfCaps] == capabilityUtf8.ToString());

        REQUIRE_SUCCEEDED(capabilitiesCustom->MoveNext(&hasCurrent));
        numOfCaps++;
    }
    REQUIRE(expectedValuesCustom.size() == numOfCaps);

    std::vector<std::string> expectedAll;
    expectedAll.insert(expectedAll.end(), expectedValuesDefaultAndGeneral.begin(), expectedValuesDefaultAndGeneral.end());
    expectedAll.insert(expectedAll.end(), expectedValuesWindows.begin(), expectedValuesWindows.end());
    expectedAll.insert(expectedAll.end(), expectedValuesRestricted.begin(), expectedValuesRestricted.end());
    expectedAll.insert(expectedAll.end(), expectedValuesCustom.begin(), expectedValuesCustom.end());

    MsixTest::ComPtr<IAppxManifestCapabilitiesEnumerator> capabilitiesAll;
    REQUIRE_SUCCEEDED(manifestReader3->GetCapabilitiesByCapabilityClass(APPX_CAPABILITY_CLASS_ALL, &capabilitiesAll));
    MsixTest::ComPtr<IAppxManifestCapabilitiesEnumeratorUtf8> capabilitiesAllUtf8;
    REQUIRE_SUCCEEDED(capabilitiesAll->QueryInterface(UuidOfImpl<IAppxManifestCapabilitiesEnumeratorUtf8>::iid, reinterpret_cast<void**>(&capabilitiesAllUtf8)));

    hasCurrent = FALSE;
    REQUIRE_SUCCEEDED(capabilitiesAll->GetHasCurrent(&hasCurrent));
    numOfCaps = 0;
    while (hasCurrent)
    {
        MsixTest::Wrappers::Buffer<wchar_t> capability;
        REQUIRE_SUCCEEDED(capabilitiesAll->GetCurrent(&capability));
        REQUIRE(expectedAll[numOfCaps] == capability.ToString());

        MsixTest::Wrappers::Buffer<char> capabilityUtf8;
        REQUIRE_SUCCEEDED(capabilitiesAllUtf8->GetCurrent(&capabilityUtf8));
        REQUIRE(expectedAll[numOfCaps] == capabilityUtf8.ToString());

        REQUIRE_SUCCEEDED(capabilitiesAll->MoveNext(&hasCurrent));
        numOfCaps++;
    }
    REQUIRE(expectedAll.size() == numOfCaps);

}

// Validates manifest resources. IAppxManifestResourcesEnumerator and IAppxManifestResourcesEnumeratorUtf8
TEST_CASE("Api_AppxManifestReader_Resources", "[api]")
{
    std::string manifest = "Sample_AppxManifest.xml";
    MsixTest::ComPtr<IAppxManifestReader> manifestReader;
    MsixTest::InitializeManifestReader(manifest, &manifestReader);

    std::array<std::string, 2> expectedResources =
    {
        "en-us",
        "es-mx",
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
    std::string manifest = "Sample_AppxManifest.xml";
    MsixTest::ComPtr<IAppxManifestReader> manifestReader;
    MsixTest::InitializeManifestReader(manifest, &manifestReader);

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
    std::string manifest = "Sample_AppxManifest.xml";
    MsixTest::ComPtr<IAppxManifestReader> manifestReader;
    MsixTest::InitializeManifestReader(manifest, &manifestReader);

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
    std::string manifest = "Sample_AppxManifest.xml";
    MsixTest::ComPtr<IAppxManifestReader> manifestReader;
    MsixTest::InitializeManifestReader(manifest, &manifestReader);

    MsixTest::ComPtr<IAppxManifestPackageId> packageId;
    REQUIRE_SUCCEEDED(manifestReader->GetPackageId(&packageId));
    REQUIRE_NOT_NULL(packageId.Get());

    MsixTest::ComPtr<IAppxManifestPackageIdUtf8> packageIdUtf8;
    REQUIRE_SUCCEEDED(packageId->QueryInterface(UuidOfImpl<IAppxManifestPackageIdUtf8>::iid, reinterpret_cast<void**>(&packageIdUtf8)));

    std::string expectedName = "SampleAppManifest";
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

    std::string expectedResourceId = "NorthAmerica";
    MsixTest::Wrappers::Buffer<wchar_t> resourceId;
    REQUIRE_SUCCEEDED(packageId->GetResourceId(&resourceId));
    REQUIRE(expectedResourceId == resourceId.ToString());

    std::string expectedFull = "SampleAppManifest_3.6.25071.0_x64_NorthAmerica_8wekyb3d8bbwe";
    MsixTest::Wrappers::Buffer<wchar_t> packageFullName;
    REQUIRE_SUCCEEDED(packageId->GetPackageFullName(&packageFullName));
    REQUIRE(expectedFull == packageFullName.ToString());

    MsixTest::Wrappers::Buffer<char> packageFullNameUtf8;
    REQUIRE_SUCCEEDED(packageIdUtf8->GetPackageFullName(&packageFullNameUtf8));
    REQUIRE(expectedFull == packageFullNameUtf8.ToString());

    std::string expectedFamily = "SampleAppManifest_8wekyb3d8bbwe";
    MsixTest::Wrappers::Buffer<wchar_t> packageFamilyName;
    REQUIRE_SUCCEEDED(packageId->GetPackageFamilyName(&packageFamilyName));
    REQUIRE(expectedFamily == packageFamilyName.ToString());

    MsixTest::Wrappers::Buffer<char> packageFamilyNameUtf8;
    REQUIRE_SUCCEEDED(packageIdUtf8->GetPackageFamilyName(&packageFamilyNameUtf8));
    REQUIRE(expectedFamily == packageFamilyNameUtf8.ToString());
}

// Validates optional packages. 
// IAppxManifestReader4, IAppxManifestOptionalPackageInfo, IAppxManifestOptionalPackageInfoUtf8, 
// IAppxManifestReader5, IAppxManifestMainPackageDependency, IAppxManifestMainPackageDependencyUtf8
TEST_CASE("Api_AppxManifestReader_OptionalPackage", "[api]")
{
    //// Validate non-optional package
    //std::string manifest = "Sample_AppxManifest.xml";
    //MsixTest::ComPtr<IAppxManifestReader> manifestReader;
    //MsixTest::InitializeManifestReader(manifest, &manifestReader);
    //MsixTest::ComPtr<IAppxManifestReader4> manifestReader4;
    //REQUIRE_SUCCEEDED(manifestReader->QueryInterface(UuidOfImpl<IAppxManifestReader4>::iid, reinterpret_cast<void**>(&manifestReader4)));

    //MsixTest::ComPtr<IAppxManifestOptionalPackageInfo> optionalPackageInfo;
    //REQUIRE_SUCCEEDED(manifestReader4->GetOptionalPackageInfo(&optionalPackageInfo));

    //BOOL isOptionalPackage;
    //REQUIRE_SUCCEEDED(optionalPackageInfo->GetIsOptionalPackage(&isOptionalPackage));
    //REQUIRE(isOptionalPackage == FALSE);

    //// A non-optional package does not have as associated main package name, so we expect
    //// the main package name to be NULL
    //MsixTest::Wrappers::Buffer<wchar_t> mainPackageName;
    //REQUIRE_SUCCEEDED(optionalPackageInfo->GetMainPackageName(&mainPackageName));
    //REQUIRE(mainPackageName.Get() == NULL);

    //Validate optional package
    //std::string manifestForOptionalPackage = "Sample_AppxManifest_WithMainPackageDependencies.xml";
    std::string manifestForOptionalPackage = "Sample_AppxManifest.xml";
    MsixTest::ComPtr<IAppxManifestReader> manifestReaderForOptionalPackage;
    MsixTest::InitializeManifestReader(manifestForOptionalPackage, &manifestReaderForOptionalPackage);
    MsixTest::ComPtr<IAppxManifestReader5> manifestReader5;
    REQUIRE_SUCCEEDED(manifestReaderForOptionalPackage->QueryInterface(UuidOfImpl<IAppxManifestReader5>::iid, reinterpret_cast<void**>(&manifestReader5)));

    MsixTest::ComPtr<IAppxManifestMainPackageDependenciesEnumerator> mainPackageDependencies;
    REQUIRE_SUCCEEDED(manifestReader5->GetMainPackageDependencies(&mainPackageDependencies));

    std::vector<std::string> expectedPublisherValues =
    {
        //"", // We expect the value to be NULL. Handle this special case separately  
        //"TestPublisher",
        "TestPublisher"
    };

    std::vector<std::string> expectedPackageFamilyNameValues =
    {
        //"SampleAppManifest_8wekyb3d8bbwe",
        //"Demo.MyMainApp_85z48rrqmdg38",
        "Demo.MyMainApp_85z48rrqmdg38"
    };

    BOOL hasCurrent = FALSE;
    REQUIRE_SUCCEEDED(mainPackageDependencies->GetHasCurrent(&hasCurrent));
    int numDep = 0;
    while (hasCurrent)
    {
        MsixTest::ComPtr<IAppxManifestMainPackageDependency> dependency;
        REQUIRE_SUCCEEDED(mainPackageDependencies->GetCurrent(&dependency));

        MsixTest::ComPtr<IAppxManifestMainPackageDependencyUtf8> dependencyUtf8;
        REQUIRE_SUCCEEDED(dependency->QueryInterface(UuidOfImpl<IAppxManifestMainPackageDependencyUtf8>::iid, reinterpret_cast<void**>(&dependencyUtf8)));

        std::string expectedName = "Demo.MyMainApp";
        MsixTest::Wrappers::Buffer<wchar_t> name;
        REQUIRE_SUCCEEDED(dependency->GetName(&name));
        REQUIRE(expectedName == name.ToString());

        MsixTest::Wrappers::Buffer<char> nameUtf8;
        REQUIRE_SUCCEEDED(dependencyUtf8->GetName(&nameUtf8));
        REQUIRE(expectedName == nameUtf8.ToString());

        //if (numDep == 0)
        //{
        //    MsixTest::Wrappers::Buffer<wchar_t> publisher;
        //    REQUIRE_SUCCEEDED(dependency->GetPublisher(&publisher));
        //    REQUIRE(publisher.Get() == NULL);

        //    MsixTest::Wrappers::Buffer<char> publisherUtf8;
        //    REQUIRE_SUCCEEDED(dependencyUtf8->GetPublisher(&publisherUtf8));
        //    REQUIRE(publisherUtf8.Get() == NULL);
        //}
        //else
        //{
        MsixTest::Wrappers::Buffer<wchar_t> publisher;
        REQUIRE_SUCCEEDED(dependency->GetPublisher(&publisher));
        REQUIRE(expectedPublisherValues[numDep] == publisher.ToString());

        MsixTest::Wrappers::Buffer<char> publisherUtf8;
        REQUIRE_SUCCEEDED(dependencyUtf8->GetPublisher(&publisherUtf8));
        REQUIRE(expectedPublisherValues[numDep] == publisherUtf8.ToString());
        //}

        MsixTest::Wrappers::Buffer<wchar_t> packageFamilyName;
        REQUIRE_SUCCEEDED(dependency->GetPackageFamilyName(&packageFamilyName));
        REQUIRE(expectedPackageFamilyNameValues[numDep] == packageFamilyName.ToString());

        MsixTest::Wrappers::Buffer<char> packageFamilyNameUtf8;
        REQUIRE_SUCCEEDED(dependencyUtf8->GetPackageFamilyName(&packageFamilyNameUtf8));
        REQUIRE(expectedPackageFamilyNameValues[numDep] == packageFamilyNameUtf8.ToString());

        REQUIRE_SUCCEEDED(mainPackageDependencies->MoveNext(&hasCurrent));
        numDep++;
    }
    //REQUIRE(2 == numDep);
    REQUIRE(1 == numDep);
}
