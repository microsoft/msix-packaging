//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
//  Validates IAppxBundleManifestReader interface and interfaces related to the AppxBundleManifest.xml
#include "catch.hpp"
#include "msixtest_int.hpp"
#include "FileHelpers.hpp"
#include "UnbundleTestData.hpp"
#include "macros.hpp"


// Validates IAppxBundleManifestReader::GetStream
TEST_CASE("Api_AppxBundleManifestReader_Stream", "[api]")
{
    std::string bundle = "StoreSigned_Desktop_x86_x64_MoviesTV.appxbundle";
    MsixTest::ComPtr<IAppxBundleReader> bundleReader;
    MsixTest::InitializeBundleReader(bundle, &bundleReader);
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
    MsixTest::InitializeBundleReader(bundle, &bundleReader);
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
    MsixTest::InitializeBundleReader(bundle, &bundleReader);
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
        int numOfScales = 0;
        while (hasCurrentResource)
        {
            MsixTest::ComPtr<IAppxManifestQualifiedResource> manifestQualifiedResources;
            REQUIRE_SUCCEEDED(manifestQualifiedResourcesEnumerator->GetCurrent(&manifestQualifiedResources));

            MsixTest::Wrappers::Buffer<wchar_t> language;
            REQUIRE_SUCCEEDED(manifestQualifiedResources->GetLanguage(&language));
            if (language.Get() != nullptr)
            {
                REQUIRE(expectedPackage.languages[numOfLangs] == language.ToString());

                MsixTest::ComPtr<IAppxManifestQualifiedResourceUtf8> manifestQualifiedResourcesUtf8;
                REQUIRE_SUCCEEDED(manifestQualifiedResources->QueryInterface(UuidOfImpl<IAppxManifestQualifiedResourceUtf8>::iid, reinterpret_cast<void**>(&manifestQualifiedResourcesUtf8)));
                MsixTest::Wrappers::Buffer<char> languageUtf8;
                REQUIRE_SUCCEEDED(manifestQualifiedResourcesUtf8->GetLanguage(&languageUtf8));
                REQUIRE(expectedPackage.languages[numOfLangs] == languageUtf8.ToString());

                numOfLangs++;
            }

            UINT32 scale = 0;
            REQUIRE_SUCCEEDED(manifestQualifiedResources->GetScale(&scale));
            if (scale != 0)
            {
                REQUIRE(expectedPackage.scales[numOfScales] == scale);
                numOfScales++;
            }
            
            REQUIRE_SUCCEEDED(manifestQualifiedResourcesEnumerator->MoveNext(&hasCurrentResource));
        }
        REQUIRE(expectedPackage.languages.size() == numOfLangs);
        REQUIRE_SUCCEEDED(bundleManifestPackageInfoEnumerator->MoveNext(&hasCurrent));
        numOfItems++;
    }
    REQUIRE(expectedPackages.size() == numOfItems);
}
