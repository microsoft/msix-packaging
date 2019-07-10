//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
//  Validates IAppxBundleReader interface
#include "catch.hpp"
#include "msixtest_int.hpp"
#include "FileHelpers.hpp"
#include "UnbundleTestData.hpp"
#include "macros.hpp"

// Validates a footprint files from a bundle
TEST_CASE("Api_AppxBundleReader_FootprintFiles", "[api]")
{
    std::string bundle = "StoreSigned_Desktop_x86_x64_MoviesTV.appxbundle";
    MsixTest::ComPtr<IAppxBundleReader> bundleReader;
    MsixTest::InitializeBundleReader(bundle, &bundleReader);

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
    MsixTest::InitializeBundleReader(bundle, &bundleReader);

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
