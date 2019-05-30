//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#include "catch.hpp"
#include "msixtest_int.hpp"
#include "FileHelpers.hpp"
#include "UnbundleTestData.hpp"

#include "macros.hpp"

void InitializeBundleHelper(const std::string& package, IAppxBundleReader** bundleReader)
{
    *bundleReader = nullptr;

    auto bundlePath = MsixTest::TestPath::GetInstance()->GetPath(MsixTest::TestPath::Directory::Unbundle) + "/" + package;
    bundlePath = MsixTest::Directory::PathAsCurrentPlatform(bundlePath);

    MsixTest::ComPtr<IAppxBundleFactory> bundleFactory;
    MsixTest::ComPtr<IStream> inputStream;

    REQUIRE_SUCCEEDED(CreateStreamOnFile(const_cast<char*>(bundlePath.c_str()), true, &inputStream));
    REQUIRE_SUCCEEDED(CoCreateAppxBundleFactoryWithHeap(
        MsixTest::Allocators::Allocate,
        MsixTest::Allocators::Free,
        MSIX_VALIDATION_OPTION::MSIX_VALIDATION_OPTION_SKIPSIGNATURE,
        static_cast<MSIX_APPLICABILITY_OPTIONS>(MSIX_APPLICABILITY_OPTIONS::MSIX_APPLICABILITY_OPTION_SKIPPLATFORM |
                                                MSIX_APPLICABILITY_OPTIONS::MSIX_APPLICABILITY_OPTION_SKIPLANGUAGE),
        &bundleFactory));

    REQUIRE_SUCCEEDED(bundleFactory->CreateBundleReader(inputStream.Get(), bundleReader));
    REQUIRE_NOT_NULL(*bundleReader);
    return;
}

// Validates a footprint files from a bundle
TEST_CASE("Api_AppxBundleReader_FootprintFiles", "[api]")
{
    std::string bundle = "StoreSigned_Desktop_x86_x64_MoviesTV.appxbundle";
    MsixTest::ComPtr<IAppxBundleReader> bundleReader;
    InitializeBundleHelper(bundle, &bundleReader);

    MsixTest::ComPtr<IAppxFile> appxBlockMap;
    REQUIRE_SUCCEEDED(bundleReader->GetFootprintFile(MsixTest::Constants::Bundle::AppxBlockMap.first, &appxBlockMap));
    MsixTest::Wrappers::Buffer<wchar_t> appxBlockMapName;
    REQUIRE_SUCCEEDED(appxBlockMap->GetName(&appxBlockMapName));
    REQUIRE(MsixTest::Constants::Bundle::AppxBlockMap.second == appxBlockMapName.ToString());

    MsixTest::ComPtr<IAppxFile> appxBundleManifest;
    REQUIRE_SUCCEEDED(bundleReader->GetFootprintFile(MsixTest::Constants::Bundle::AppxBundleManifest.first, &appxBundleManifest));
    MsixTest::Wrappers::Buffer<wchar_t> appxBundleManifestName;
    REQUIRE_SUCCEEDED(appxBundleManifest->GetName(&appxBundleManifestName));
    auto expected = MsixTest::Constants::Bundle::AppxBundleManifest.second;
    std::replace(expected.begin(), expected.end(), '/', '\\');
    REQUIRE(expected == appxBundleManifestName.ToString());

    MsixTest::ComPtr<IAppxFile> appxSignature;
    REQUIRE_SUCCEEDED(bundleReader->GetFootprintFile(MsixTest::Constants::Bundle::AppxSignature.first, &appxSignature));
    MsixTest::Wrappers::Buffer<wchar_t> appxSignatureName;
    REQUIRE_SUCCEEDED(appxSignature->GetName(&appxSignatureName));
    REQUIRE(MsixTest::Constants::Bundle::AppxSignature.second == appxSignatureName.ToString());
}

// Validates list of payload packages in the bundle
TEST_CASE("Api_AppxBundleReader_PayloadPackages", "[api]")
{
    std::string bundle = "StoreSigned_Desktop_x86_x64_MoviesTV.appxbundle";
    MsixTest::ComPtr<IAppxBundleReader> bundleReader;
    InitializeBundleHelper(bundle, &bundleReader);

    auto expectedPackages = MsixTest::Unbundle::GetExpectedPackages();

    MsixTest::ComPtr<IAppxBundleReaderUtf8> bundleReaderUtf8;
    REQUIRE_SUCCEEDED(bundleReader->QueryInterface(UuidOfImpl<IAppxBundleReaderUtf8>::iid, reinterpret_cast<void**>(&bundleReaderUtf8)));

    MsixTest::ComPtr<IAppxFilesEnumerator> packages;
    REQUIRE_SUCCEEDED(bundleReader->GetPayloadPackages(&packages));

    BOOL hasCurrent = FALSE;
    REQUIRE_SUCCEEDED(packages->GetHasCurrent(&hasCurrent));
    int numOfPackages = 0;
    while (hasCurrent)
    {
        MsixTest::ComPtr<IAppxFile> package;
        REQUIRE_SUCCEEDED(packages->GetCurrent(&package));

        auto expectedPackage = expectedPackages[numOfPackages];

        MsixTest::Wrappers::Buffer<wchar_t> packageName;
        REQUIRE_SUCCEEDED(package->GetName(&packageName));
        REQUIRE(expectedPackage.name == packageName.ToString());

        MsixTest::Wrappers::Buffer<char> packageNameUtf8;
        MsixTest::ComPtr<IAppxFileUtf8> packageUtf8;
        REQUIRE_SUCCEEDED(package->QueryInterface(UuidOfImpl<IAppxFileUtf8>::iid, reinterpret_cast<void**>(&packageUtf8)));
        REQUIRE_SUCCEEDED(packageUtf8->GetName(&packageNameUtf8));
        REQUIRE(expectedPackage.name == packageNameUtf8.ToString());

        MsixTest::ComPtr<IAppxFile> package2;
        REQUIRE_SUCCEEDED(bundleReader->GetPayloadPackage(packageName.Get(), &package2));
        REQUIRE_ARE_SAME(package.Get(), package2.Get());

        MsixTest::ComPtr<IAppxFile> package3;
        REQUIRE_SUCCEEDED(bundleReaderUtf8->GetPayloadPackage(packageNameUtf8.Get(), &package3));
        REQUIRE_ARE_SAME(package.Get(), package3.Get());

        REQUIRE_SUCCEEDED(packages->MoveNext(&hasCurrent));
        numOfPackages++;
    }
    REQUIRE(expectedPackages.size() == numOfPackages);
}

// Validates IAppxBundleManifestReader::GetStream
TEST_CASE("Api_AppxBundleManifestReader_Stream", "[api]")
{
    std::string bundle = "StoreSigned_Desktop_x86_x64_MoviesTV.appxbundle";
    MsixTest::ComPtr<IAppxBundleReader> bundleReader;
    InitializeBundleHelper(bundle, &bundleReader);
    MsixTest::ComPtr<IAppxBundleManifestReader> bundleManifestReader;
    REQUIRE_SUCCEEDED(bundleReader->GetManifest(&bundleManifestReader));

    MsixTest::ComPtr<IStream> stream;
    REQUIRE_SUCCEEDED(bundleManifestReader->GetStream(&stream));
    REQUIRE_NOT_NULL(stream.Get());
}

// Validates bundle package id
TEST_CASE("Api_AppxBundleManifestReader_PackageId", "[api]")
{
    std::string bundle = "StoreSigned_Desktop_x86_x64_MoviesTV.appxbundle";
    MsixTest::ComPtr<IAppxBundleReader> bundleReader;
    InitializeBundleHelper(bundle, &bundleReader);
    MsixTest::ComPtr<IAppxBundleManifestReader> bundleManifestReader;
    REQUIRE_SUCCEEDED(bundleReader->GetManifest(&bundleManifestReader));

    MsixTest::ComPtr<IAppxManifestPackageId> packageId;
    REQUIRE_SUCCEEDED(bundleManifestReader->GetPackageId(&packageId));

    MsixTest::ComPtr<IAppxManifestPackageIdUtf8> packageIdUtf8;
    REQUIRE_SUCCEEDED(packageId->QueryInterface(UuidOfImpl<IAppxManifestPackageIdUtf8>::iid, reinterpret_cast<void**>(&packageIdUtf8)));

    std::string expectedName = "Microsoft.ZuneVideo";
    MsixTest::Wrappers::Buffer<wchar_t> name;
    REQUIRE_SUCCEEDED(packageId->GetName(&name));
    REQUIRE(expectedName == name.ToString());

    MsixTest::Wrappers::Buffer<char> nameUtf8;
    REQUIRE_SUCCEEDED(packageIdUtf8->GetName(&nameUtf8));
    REQUIRE(expectedName == nameUtf8.ToString());

    APPX_PACKAGE_ARCHITECTURE expectedArch = APPX_PACKAGE_ARCHITECTURE_NEUTRAL;
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

    UINT64 expectedVersion = 568298005391671296; // 2019.6.25071.0
    UINT64 packageVersion;
    REQUIRE_SUCCEEDED(packageId->GetVersion(&packageVersion));
    REQUIRE(expectedVersion == packageVersion);

    std::string expectedResource = "~";
    MsixTest::Wrappers::Buffer<wchar_t> resourceId;
    REQUIRE_SUCCEEDED(packageId->GetResourceId(&resourceId));
    REQUIRE(expectedResource == resourceId.ToString());

    MsixTest::Wrappers::Buffer<char> resourceIdUtf8;
    REQUIRE_SUCCEEDED(packageIdUtf8->GetResourceId(&resourceIdUtf8));
    REQUIRE(expectedResource == resourceIdUtf8.ToString());

    std::string expectedFull = "Microsoft.ZuneVideo_2019.6.25071.0_neutral_~_8wekyb3d8bbwe";
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

// Validates bundle package info items
TEST_CASE("Api_AppxBundleManifestReader_PackageInfo", "[api]")
{
    std::string bundle = "StoreSigned_Desktop_x86_x64_MoviesTV.appxbundle";
    MsixTest::ComPtr<IAppxBundleReader> bundleReader;
    InitializeBundleHelper(bundle, &bundleReader);
    MsixTest::ComPtr<IAppxBundleManifestReader> bundleManifestReader;
    REQUIRE_SUCCEEDED(bundleReader->GetManifest(&bundleManifestReader));

    auto expectedPackages = MsixTest::Unbundle::GetExpectedPackages();

    MsixTest::ComPtr<IAppxBundleManifestPackageInfoEnumerator> bundleManifestPackageInfoEnumerator;
    REQUIRE_SUCCEEDED(bundleManifestReader->GetPackageInfoItems(&bundleManifestPackageInfoEnumerator));

    BOOL hasCurrent = FALSE;
    REQUIRE_SUCCEEDED(bundleManifestPackageInfoEnumerator->GetHasCurrent(&hasCurrent));
    int numOfItems = 0;
    while (hasCurrent)
    {
        auto expectedPackage = expectedPackages[numOfItems];

        MsixTest::ComPtr<IAppxBundleManifestPackageInfo> bundleManifestPackageInfo;
        REQUIRE_SUCCEEDED(bundleManifestPackageInfoEnumerator->GetCurrent(&bundleManifestPackageInfo));

        MsixTest::Wrappers::Buffer<wchar_t> fileName;
        REQUIRE_SUCCEEDED(bundleManifestPackageInfo->GetFileName(&fileName));
        REQUIRE(expectedPackage.name == fileName.ToString());

        MsixTest::Wrappers::Buffer<char> fileNameUtf8;
        MsixTest::ComPtr<IAppxBundleManifestPackageInfoUtf8> bundleManifestPackageInfoUtf8;
        REQUIRE_SUCCEEDED(bundleManifestPackageInfo->QueryInterface(UuidOfImpl<IAppxBundleManifestPackageInfoUtf8>::iid, reinterpret_cast<void**>(&bundleManifestPackageInfoUtf8)));
        REQUIRE_SUCCEEDED(bundleManifestPackageInfoUtf8->GetFileName(&fileNameUtf8));
        REQUIRE(expectedPackage.name == fileNameUtf8.ToString());

        APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE type;
        REQUIRE_SUCCEEDED(bundleManifestPackageInfo->GetPackageType(&type));
        REQUIRE(expectedPackage.type == type);

        UINT64 offset;
        REQUIRE_SUCCEEDED(bundleManifestPackageInfo->GetOffset(&offset));
        REQUIRE(expectedPackage.offset == offset);

        UINT64 size;
        REQUIRE_SUCCEEDED(bundleManifestPackageInfo->GetSize(&size));
        REQUIRE(expectedPackage.size == size);

        MsixTest::ComPtr<IAppxManifestQualifiedResourcesEnumerator> manifestQualifiedResourcesEnumerator;
        REQUIRE_SUCCEEDED(bundleManifestPackageInfo->GetResources(&manifestQualifiedResourcesEnumerator));

        BOOL hasCurrentResource = FALSE;
        REQUIRE_SUCCEEDED(manifestQualifiedResourcesEnumerator->GetHasCurrent(&hasCurrentResource));
        int numOfLangs = 0;
        while (hasCurrentResource)
        {
            MsixTest::ComPtr<IAppxManifestQualifiedResource> manifestQualifiedResources;
            REQUIRE_SUCCEEDED(manifestQualifiedResourcesEnumerator->GetCurrent(&manifestQualifiedResources));

            MsixTest::Wrappers::Buffer<wchar_t> language;
            REQUIRE_SUCCEEDED(manifestQualifiedResources->GetLanguage(&language));
            REQUIRE(expectedPackage.languages[numOfLangs] == language.ToString());

            MsixTest::ComPtr<IAppxManifestQualifiedResourceUtf8> manifestQualifiedResourcesUtf8;
            REQUIRE_SUCCEEDED(manifestQualifiedResources->QueryInterface(UuidOfImpl<IAppxManifestQualifiedResourceUtf8>::iid, reinterpret_cast<void**>(&manifestQualifiedResourcesUtf8)));
            MsixTest::Wrappers::Buffer<char> languageUtf8;
            REQUIRE_SUCCEEDED(manifestQualifiedResourcesUtf8->GetLanguage(&languageUtf8));
            REQUIRE(expectedPackage.languages[numOfLangs] == languageUtf8.ToString());

            REQUIRE_SUCCEEDED(manifestQualifiedResourcesEnumerator->MoveNext(&hasCurrentResource));
            numOfLangs++;
        }
        REQUIRE(expectedPackage.languages.size() == numOfLangs);
        REQUIRE_SUCCEEDED(bundleManifestPackageInfoEnumerator->MoveNext(&hasCurrent));
        numOfItems++;
    }
    REQUIRE(expectedPackages.size() == numOfItems);
}
