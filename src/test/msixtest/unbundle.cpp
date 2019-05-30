//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#include "catch.hpp"
#include "msixtest_int.hpp"
#include "UnbundleTestData.hpp"
#include "FileHelpers.hpp"

#include <iostream>

void RunUnbundleTest(HRESULT expected, const std::string& bundle, MSIX_VALIDATION_OPTION validation,
    MSIX_PACKUNPACK_OPTION packUnpack, MSIX_APPLICABILITY_OPTIONS applicability,
    MsixTest::TestPath::Directory dir = MsixTest::TestPath::Directory::Unbundle, bool clean = true)
{
    std::cout << "Testing: " << std::endl;
    std::cout << "\tBundle: " << bundle << std::endl; 

    auto testData = MsixTest::TestPath::GetInstance();

    auto bundlePath = testData->GetPath(dir) + "/" + std::string(bundle);
    bundlePath = MsixTest::Directory::PathAsCurrentPlatform(bundlePath);

    auto outputDir = testData->GetPath(MsixTest::TestPath::Directory::Output);

    HRESULT actual = UnpackBundle(packUnpack,
                                  validation,
                                  applicability,
                                  const_cast<char*>(bundlePath.c_str()),
                                  const_cast<char*>(outputDir.c_str()));

    CHECK(expected == actual);
    MsixTest::Log::PrintMsixLog(expected, actual);

    // clean directory if succeeded and requested
    if ((actual == S_OK) && clean)
    {
        CHECK(MsixTest::Directory::CleanDirectory(outputDir));
    }
}

TEST_CASE("Unbundle_StoreSigned_Desktop_x86_x64_MoviesTV", "[unbundle]")
{
    HRESULT expected                         = S_OK;
    std::string bundle                       = "StoreSigned_Desktop_x86_x64_MoviesTV.appxbundle";
    MSIX_VALIDATION_OPTION validation        = MSIX_VALIDATION_OPTION_FULL;
    MSIX_PACKUNPACK_OPTION packUnpack        = MSIX_PACKUNPACK_OPTION_NONE;
    MSIX_APPLICABILITY_OPTIONS applicability = MSIX_APPLICABILITY_OPTION_FULL;

    RunUnbundleTest(expected, bundle, validation, packUnpack, applicability, MsixTest::TestPath::Directory::Unbundle, false);

    auto outputDir = MsixTest::TestPath::GetInstance()->GetPath(MsixTest::TestPath::Directory::Output);

    // Verify all the files extracted on disk are correct
    auto files = MsixTest::Unbundle::GetExpectedFilesFullApplicable();
    CHECK(MsixTest::Directory::CompareDirectory(outputDir, files));

    // Verify non expected files aren't present
    auto filesNotApplicable = MsixTest::Unbundle::GetExpectedFilesNoApplicable();
    CHECK(!MsixTest::Directory::CompareDirectory(outputDir, filesNotApplicable));

    // Clean directory
    CHECK(MsixTest::Directory::CleanDirectory(outputDir));
}

TEST_CASE("Unbundle_StoreSigned_Desktop_x86_x64_MoviesTV_pfn", "[unbundle]")
{
    HRESULT expected                         = S_OK;
    std::string bundle                       = "StoreSigned_Desktop_x86_x64_MoviesTV.appxbundle";
    MSIX_VALIDATION_OPTION validation        = MSIX_VALIDATION_OPTION_FULL;
    MSIX_PACKUNPACK_OPTION packUnpack        = MSIX_PACKUNPACK_OPTION_CREATEPACKAGESUBFOLDER;
    MSIX_APPLICABILITY_OPTIONS applicability = MSIX_APPLICABILITY_OPTION_FULL;

    RunUnbundleTest(expected, bundle, validation, packUnpack, applicability, MsixTest::TestPath::Directory::Unbundle, false);

    // The expected folder structure should be <output>/Microsoft.ZuneVideo_2019.6.25071.0_neutral_~_8wekyb3d8bbwe/<files>
    // Append it to the already existing expected files map
    std::string pfn = "Microsoft.ZuneVideo_2019.6.25071.0_neutral_~_8wekyb3d8bbwe/";
    auto files = MsixTest::Unbundle::GetExpectedFilesFullApplicable();
    std::map<std::string, uint64_t> filesWithPfn;
    for (const auto& file : files)
    {
        filesWithPfn.emplace(pfn + file.first, file.second);
    }

    auto outputDir = MsixTest::TestPath::GetInstance()->GetPath(MsixTest::TestPath::Directory::Output);
    CHECK(MsixTest::Directory::CompareDirectory(outputDir, filesWithPfn));

    // Clean directory
    CHECK(MsixTest::Directory::CleanDirectory(outputDir));
}

TEST_CASE("Unbundle_StoreSigned_Desktop_x86_x64_MoviesTV_lang_applicability_off", "[unbundle]")
{
    HRESULT expected                         = S_OK;
    std::string bundle                       = "StoreSigned_Desktop_x86_x64_MoviesTV.appxbundle";
    MSIX_VALIDATION_OPTION validation        = MSIX_VALIDATION_OPTION_FULL;
    MSIX_PACKUNPACK_OPTION packUnpack        = MSIX_PACKUNPACK_OPTION_NONE;
    MSIX_APPLICABILITY_OPTIONS applicability = MSIX_APPLICABILITY_OPTION_SKIPLANGUAGE;

    RunUnbundleTest(expected, bundle, validation, packUnpack, applicability, MsixTest::TestPath::Directory::Unbundle, false);

    auto outputDir = MsixTest::TestPath::GetInstance()->GetPath(MsixTest::TestPath::Directory::Output);

    // Verify all the files extracted on disk are correct
    // We ran without language applicability, so expect all files to be there.
    auto files = MsixTest::Unbundle::GetExpectedFilesFullApplicable();
    auto filesNotApplicable = MsixTest::Unbundle::GetExpectedFilesNoApplicable();

    std::map<std::string, std::uint64_t> allFiles;
    allFiles.insert(files.begin(), files.end());
    allFiles.insert(filesNotApplicable.begin(), filesNotApplicable.end());
    CHECK(MsixTest::Directory::CompareDirectory(outputDir, allFiles));

    // Clean directory
    CHECK(MsixTest::Directory::CleanDirectory(outputDir));
}

TEST_CASE("Unbundle_BlockMapContainsPayloadPackage", "[unbundle]")
{
    HRESULT expected                         = static_cast<HRESULT>(MSIX::Error::BlockMapSemanticError);
    std::string bundle                       = "BlockMapContainsPayloadPackage.appxbundle";
    MSIX_VALIDATION_OPTION validation        = MSIX_VALIDATION_OPTION_SKIPSIGNATURE;
    MSIX_PACKUNPACK_OPTION packUnpack        = MSIX_PACKUNPACK_OPTION_NONE;
    MSIX_APPLICABILITY_OPTIONS applicability = MSIX_APPLICABILITY_OPTION_FULL;

    RunUnbundleTest(expected, bundle, validation, packUnpack, applicability);
}

TEST_CASE("Unbundle_BlockMapIsMissing", "[unbundle]")
{
    HRESULT expected                         = static_cast<HRESULT>(MSIX::Error::MissingAppxBlockMapXML);
    std::string bundle                       = "BlockMapIsMissing.appxbundle";
    MSIX_VALIDATION_OPTION validation        = MSIX_VALIDATION_OPTION_SKIPSIGNATURE;
    MSIX_PACKUNPACK_OPTION packUnpack        = MSIX_PACKUNPACK_OPTION_NONE;
    MSIX_APPLICABILITY_OPTIONS applicability = MSIX_APPLICABILITY_OPTION_FULL;

    RunUnbundleTest(expected, bundle, validation, packUnpack, applicability);
}

TEST_CASE("Unbundle_BlockMapViolatesSchema", "[unbundle]")
{
    HRESULT expected                         = static_cast<HRESULT>(MSIX::Error::XmlError);
    std::string bundle                       = "BlockMapViolatesSchema.appxbundle";
    MSIX_VALIDATION_OPTION validation        = MSIX_VALIDATION_OPTION_SKIPSIGNATURE;
    MSIX_PACKUNPACK_OPTION packUnpack        = MSIX_PACKUNPACK_OPTION_NONE;
    MSIX_APPLICABILITY_OPTIONS applicability = MSIX_APPLICABILITY_OPTION_FULL;

    RunUnbundleTest(expected, bundle, validation, packUnpack, applicability);
}

TEST_CASE("Unbundle_ContainsNoPayload", "[unbundle]")
{
    HRESULT expected                         = static_cast<HRESULT>(MSIX::Error::XmlError);
    std::string bundle                       = "ContainsNoPayload.appxbundle";
    MSIX_VALIDATION_OPTION validation        = MSIX_VALIDATION_OPTION_SKIPSIGNATURE;
    MSIX_PACKUNPACK_OPTION packUnpack        = MSIX_PACKUNPACK_OPTION_NONE;
    MSIX_APPLICABILITY_OPTIONS applicability = MSIX_APPLICABILITY_OPTION_FULL;

    RunUnbundleTest(expected, bundle, validation, packUnpack, applicability);
}

TEST_CASE("Unbundle_ContainsOnlyResourcePackages", "[unbundle]")
{
    HRESULT expected                         = static_cast<HRESULT>(MSIX::Error::AppxManifestSemanticError);
    std::string bundle                       = "ContainsOnlyResourcePackages.appxbundle";
    MSIX_VALIDATION_OPTION validation        = MSIX_VALIDATION_OPTION_SKIPSIGNATURE;
    MSIX_PACKUNPACK_OPTION packUnpack        = MSIX_PACKUNPACK_OPTION_NONE;
    MSIX_APPLICABILITY_OPTIONS applicability = MSIX_APPLICABILITY_OPTION_FULL;

    RunUnbundleTest(expected, bundle, validation, packUnpack, applicability);
}

TEST_CASE("Unbundle_MainBundle", "[unbundle]")
{
    HRESULT expected                         = S_OK;
    std::string bundle                       = "MainBundle.appxbundle";
    MSIX_VALIDATION_OPTION validation        = MSIX_VALIDATION_OPTION_SKIPSIGNATURE;
    MSIX_PACKUNPACK_OPTION packUnpack        = MSIX_PACKUNPACK_OPTION_NONE;
    MSIX_APPLICABILITY_OPTIONS applicability = MSIX_APPLICABILITY_OPTION_FULL;

    RunUnbundleTest(expected, bundle, validation, packUnpack, applicability);
}

TEST_CASE("Unbundle_ManifestIsMissing", "[unbundle]")
{
    HRESULT expected                         = static_cast<HRESULT>(MSIX::Error::MissingAppxManifestXML);
    std::string bundle                       = "ManifestIsMissing.appxbundle";
    MSIX_VALIDATION_OPTION validation        = MSIX_VALIDATION_OPTION_SKIPSIGNATURE;
    MSIX_PACKUNPACK_OPTION packUnpack        = MSIX_PACKUNPACK_OPTION_NONE;
    MSIX_APPLICABILITY_OPTIONS applicability = MSIX_APPLICABILITY_OPTION_FULL;

    RunUnbundleTest(expected, bundle, validation, packUnpack, applicability);
}

TEST_CASE("Unbundle_ManifestPackageHasIncorrectSize", "[unbundle]")
{
    HRESULT expected                         = static_cast<HRESULT>(MSIX::Error::AppxManifestSemanticError);
    std::string bundle                       = "ManifestPackageHasIncorrectSize.appxbundle";
    MSIX_VALIDATION_OPTION validation        = MSIX_VALIDATION_OPTION_SKIPSIGNATURE;
    MSIX_PACKUNPACK_OPTION packUnpack        = MSIX_PACKUNPACK_OPTION_NONE;
    MSIX_APPLICABILITY_OPTIONS applicability = MSIX_APPLICABILITY_OPTION_FULL;

    RunUnbundleTest(expected, bundle, validation, packUnpack, applicability);
}

TEST_CASE("Unbundle_ManifestViolatesSchema", "[unbundle]")
{
    HRESULT expected                         = static_cast<HRESULT>(MSIX::Error::XmlError);
    std::string bundle                       = "ManifestViolatesSchema.appxbundle";
    MSIX_VALIDATION_OPTION validation        = MSIX_VALIDATION_OPTION_SKIPSIGNATURE;
    MSIX_PACKUNPACK_OPTION packUnpack        = MSIX_PACKUNPACK_OPTION_NONE;
    MSIX_APPLICABILITY_OPTIONS applicability = MSIX_APPLICABILITY_OPTION_FULL;

    RunUnbundleTest(expected, bundle, validation, packUnpack, applicability);
}

TEST_CASE("Unbundle_PayloadPackageHasNonAppxExtension", "[unbundle]")
{
    HRESULT expected                         = static_cast<HRESULT>(MSIX::Error::AppxManifestSemanticError);
    std::string bundle                       = "PayloadPackageHasNonAppxExtension.appxbundle";
    MSIX_VALIDATION_OPTION validation        = MSIX_VALIDATION_OPTION_SKIPSIGNATURE;
    MSIX_PACKUNPACK_OPTION packUnpack        = MSIX_PACKUNPACK_OPTION_NONE;
    MSIX_APPLICABILITY_OPTIONS applicability = MSIX_APPLICABILITY_OPTION_FULL;

    RunUnbundleTest(expected, bundle, validation, packUnpack, applicability);
}

TEST_CASE("Unbundle_PayloadPackageIsCompressed", "[unbundle]")
{
    HRESULT expected                         = static_cast<HRESULT>(MSIX::Error::AppxManifestSemanticError);
    std::string bundle                       = "PayloadPackageIsCompressed.appxbundle";
    MSIX_VALIDATION_OPTION validation        = MSIX_VALIDATION_OPTION_SKIPSIGNATURE;
    MSIX_PACKUNPACK_OPTION packUnpack        = MSIX_PACKUNPACK_OPTION_NONE;
    MSIX_APPLICABILITY_OPTIONS applicability = MSIX_APPLICABILITY_OPTION_FULL;

    RunUnbundleTest(expected, bundle, validation, packUnpack, applicability);
}

TEST_CASE("Unbundle_PayloadPackageIsEmpty", "[unbundle]")
{
    HRESULT expected                         = static_cast<HRESULT>(MSIX::Error::FileRead);
    std::string bundle                       = "PayloadPackageIsEmpty.appxbundle";
    MSIX_VALIDATION_OPTION validation        = MSIX_VALIDATION_OPTION_SKIPSIGNATURE;
    MSIX_PACKUNPACK_OPTION packUnpack        = MSIX_PACKUNPACK_OPTION_NONE;
    MSIX_APPLICABILITY_OPTIONS applicability = MSIX_APPLICABILITY_OPTION_FULL;

    RunUnbundleTest(expected, bundle, validation, packUnpack, applicability);
}

TEST_CASE("Unbundle_PayloadPackageIsNotAppxPackage", "[unbundle]")
{
    HRESULT expected                         = E_INVALIDARG;
    std::string bundle                       = "PayloadPackageIsNotAppxPackage.appxbundle";
    MSIX_VALIDATION_OPTION validation        = MSIX_VALIDATION_OPTION_SKIPSIGNATURE;
    MSIX_PACKUNPACK_OPTION packUnpack        = MSIX_PACKUNPACK_OPTION_NONE;
    MSIX_APPLICABILITY_OPTIONS applicability = MSIX_APPLICABILITY_OPTION_FULL;

    RunUnbundleTest(expected, bundle, validation, packUnpack, applicability);
}

TEST_CASE("Unbundle_SignedUntrustedCert-CERT_E_CHAINING", "[unbundle]")
{
    HRESULT expected                         = static_cast<HRESULT>(MSIX::Error::CertNotTrusted);
    std::string bundle                       = "SignedUntrustedCert-CERT_E_CHAINING.appxbundle";
    MSIX_VALIDATION_OPTION validation        = MSIX_VALIDATION_OPTION_FULL;
    MSIX_PACKUNPACK_OPTION packUnpack        = MSIX_PACKUNPACK_OPTION_NONE;
    MSIX_APPLICABILITY_OPTIONS applicability = MSIX_APPLICABILITY_OPTION_FULL;

    RunUnbundleTest(expected, bundle, validation, packUnpack, applicability);
}

TEST_CASE("Unbundle_BundleWithIntlPackage", "[unbundle]")
{
    HRESULT expected                         = S_OK;
    std::string bundle                       = "BundleWithIntlPackage.appxbundle";
    MSIX_VALIDATION_OPTION validation        = MSIX_VALIDATION_OPTION_SKIPSIGNATURE;
    MSIX_PACKUNPACK_OPTION packUnpack        = MSIX_PACKUNPACK_OPTION_NONE;
    MSIX_APPLICABILITY_OPTIONS applicability = MSIX_APPLICABILITY_OPTION_FULL;

    RunUnbundleTest(expected, bundle, validation, packUnpack, applicability);
}

TEST_CASE("Unbundle_FlatBundleWithAsset", "[unbundle][flat]")
{
    HRESULT expected                         = S_OK;
    std::string bundle                       = "FlatBundleWithAsset.appxbundle";
    MSIX_VALIDATION_OPTION validation        = MSIX_VALIDATION_OPTION_SKIPSIGNATURE;
    MSIX_PACKUNPACK_OPTION packUnpack        = MSIX_PACKUNPACK_OPTION_NONE;
    MSIX_APPLICABILITY_OPTIONS applicability = MSIX_APPLICABILITY_OPTION_FULL;

    RunUnbundleTest(expected, bundle, validation, packUnpack, applicability, MsixTest::TestPath::Directory::Flat);
}

TEST_CASE("Unbundle_FlatBundleWithAsset_MissingPackage", "[unbundle][flat]")
{
    HRESULT expected                         = static_cast<HRESULT>(MSIX::Error::FileOpen);
    std::string bundle                       = "FlatBundleWithAsset.appxbundle";
    MSIX_VALIDATION_OPTION validation        = MSIX_VALIDATION_OPTION_SKIPSIGNATURE;
    MSIX_PACKUNPACK_OPTION packUnpack        = MSIX_PACKUNPACK_OPTION_NONE;
    MSIX_APPLICABILITY_OPTIONS applicability = MSIX_APPLICABILITY_OPTION_FULL;

    RunUnbundleTest(expected, bundle, validation, packUnpack, applicability, MsixTest::TestPath::Directory::BadFlat);
}
