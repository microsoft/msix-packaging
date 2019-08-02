//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#include "catch.hpp"
#include "msixtest_int.hpp"
#include "UnpackTestData.hpp"
#include "FileHelpers.hpp"

#include <iostream>

void RunUnpackTest(HRESULT expected, const std::string& package, MSIX_VALIDATION_OPTION validation,
    MSIX_PACKUNPACK_OPTION packUnpack, bool clean = true, bool absolutePaths = false)
{
    std::cout << "Testing: " << std::endl;
    std::cout << "\tPackage:" << package << std::endl; 

    auto testData = MsixTest::TestPath::GetInstance();

    auto packagePath = testData->GetPath(MsixTest::TestPath::Directory::Unpack) + "/" + package;
    packagePath = MsixTest::Directory::PathAsCurrentPlatform(packagePath);

    auto outputDir = testData->GetPath(MsixTest::TestPath::Directory::Output);
    outputDir = MsixTest::Directory::PathAsCurrentPlatform(outputDir);

    if (absolutePaths)
    {
        packagePath = MsixTest::Directory::PathAsAbsolute(packagePath);
        outputDir = MsixTest::Directory::PathAsAbsolute(outputDir);
    }

    HRESULT actual = UnpackPackage(packUnpack,
                                   validation,
                                   const_cast<char*>(packagePath.c_str()),
                                   const_cast<char*>(outputDir.c_str()));

    CHECK(expected == actual);
    MsixTest::Log::PrintMsixLog(expected, actual);

    // clean directory if succeeded and requested
    if ((actual == S_OK) && clean)
    {
        CHECK(MsixTest::Directory::CleanDirectory(outputDir));
    }
}

// End-to-end unpacking tests
TEST_CASE("Unpack_StoreSigned_Desktop_x64_MoviesTV", "[unpack]")
{
    HRESULT expected                  = S_OK;
    std::string package               = "StoreSigned_Desktop_x64_MoviesTV.appx";
    MSIX_VALIDATION_OPTION validation = MSIX_VALIDATION_OPTION_FULL;
    MSIX_PACKUNPACK_OPTION packUnpack = MSIX_PACKUNPACK_OPTION_NONE;

    RunUnpackTest(expected, package, validation, packUnpack, false);

    // Verify all the files extracted on disk are correct
    auto files = MsixTest::Unpack::GetExpectedFiles();
    auto outputDir = MsixTest::TestPath::GetInstance()->GetPath(MsixTest::TestPath::Directory::Output);
    CHECK(MsixTest::Directory::CompareDirectory(outputDir, files));

    // Clean directory
    CHECK(MsixTest::Directory::CleanDirectory(outputDir));
}

TEST_CASE("Unpack_StoreSigned_Desktop_x64_MoviesTV_pfn", "[unpack]")
{
    HRESULT expected                  = S_OK;
    std::string package               = "StoreSigned_Desktop_x64_MoviesTV.appx";
    MSIX_VALIDATION_OPTION validation = MSIX_VALIDATION_OPTION_FULL;
    MSIX_PACKUNPACK_OPTION packUnpack = MSIX_PACKUNPACK_OPTION_CREATEPACKAGESUBFOLDER;

    RunUnpackTest(expected, package, validation, packUnpack, false);

    // Verify all the files extracted on disk are correct
    auto files = MsixTest::Unpack::GetExpectedFiles();
    // The expected folder structure should be <output>/Microsoft.ZuneVideo_3.6.25071.0_x64__8wekyb3d8bbwe/<files>
    // Append it to the already existing expected files map
    std::string pfn = "Microsoft.ZuneVideo_3.6.25071.0_x64__8wekyb3d8bbwe/";
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

// For packages we expect identical behavior between MSIX_PACKUNPACK_OPTION_UNPACKWITHFLATSTRUCTURE and MSIX_PACKUNPACK_OPTION_CREATEPACKAGESUBFOLDER
TEST_CASE("Unpack_StoreSigned_Desktop_x64_MoviesTV_pfn-flat", "[unpack]")
{
    HRESULT expected = S_OK;
    std::string package = "StoreSigned_Desktop_x64_MoviesTV.appx";
    MSIX_VALIDATION_OPTION validation = MSIX_VALIDATION_OPTION_FULL;
    MSIX_PACKUNPACK_OPTION packUnpack = MSIX_PACKUNPACK_OPTION_UNPACKWITHFLATSTRUCTURE;

    RunUnpackTest(expected, package, validation, packUnpack, false);

    // Verify all the files extracted on disk are correct
    auto files = MsixTest::Unpack::GetExpectedFiles();
    // The expected folder structure should be <output>/Microsoft.ZuneVideo_3.6.25071.0_x64__8wekyb3d8bbwe/<files>
    // Append it to the already existing expected files map
    std::string pfn = "Microsoft.ZuneVideo_3.6.25071.0_x64__8wekyb3d8bbwe/";
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

TEST_CASE("Unpack_Empty", "[unpack]")
{
    HRESULT expected                  = static_cast<HRESULT>(MSIX::Error::FileSeek);
    std::string package               = "Empty.appx";
    MSIX_VALIDATION_OPTION validation = MSIX_VALIDATION_OPTION_ALLOWSIGNATUREORIGINUNKNOWN;
    MSIX_PACKUNPACK_OPTION packUnpack = MSIX_PACKUNPACK_OPTION_NONE;

    RunUnpackTest(expected, package, validation, packUnpack);
}

TEST_CASE("Unpack_HelloWorld", "[unpack]")
{
    HRESULT expected                  = S_OK;
    std::string package               = "HelloWorld.appx";
    MSIX_VALIDATION_OPTION validation = MSIX_VALIDATION_OPTION_SKIPSIGNATURE;
    MSIX_PACKUNPACK_OPTION packUnpack = MSIX_PACKUNPACK_OPTION_NONE;

    RunUnpackTest(expected, package, validation, packUnpack);
}

TEST_CASE("Unpack_NotepadPlusPlus", "[unpack]")
{
    HRESULT expected                  = S_OK;
    std::string package               = "NotepadPlusPlus.appx";
    MSIX_VALIDATION_OPTION validation = MSIX_VALIDATION_OPTION_SKIPSIGNATURE;
    MSIX_PACKUNPACK_OPTION packUnpack = MSIX_PACKUNPACK_OPTION_NONE;

    RunUnpackTest(expected, package, validation, packUnpack);
}

TEST_CASE("Unpack_IntlPackage", "[unpack]")
{
    HRESULT expected                  = S_OK;
    std::string package               = "IntlPackage.appx";
    MSIX_VALIDATION_OPTION validation = MSIX_VALIDATION_OPTION_SKIPSIGNATURE;
    MSIX_PACKUNPACK_OPTION packUnpack = MSIX_PACKUNPACK_OPTION_NONE;

    RunUnpackTest(expected, package, validation, packUnpack);
}

TEST_CASE("Unpack_SignatureNotLastPart-ERROR_BAD_FORMAT", "[unpack]")
{
    HRESULT expected                  = static_cast<HRESULT>(MSIX::Error::CertNotTrusted);
    std::string package               = "SignatureNotLastPart-ERROR_BAD_FORMAT.appx";
    MSIX_VALIDATION_OPTION validation = MSIX_VALIDATION_OPTION_FULL;
    MSIX_PACKUNPACK_OPTION packUnpack = MSIX_PACKUNPACK_OPTION_NONE;

    RunUnpackTest(expected, package, validation, packUnpack);
}

TEST_CASE("Unpack_SignedTamperedBlockMap-TRUST_E_BAD_DIGEST", "[unpack]")
{
    HRESULT expected                  = static_cast<HRESULT>(MSIX::Error::CertNotTrusted);
    std::string package               = "SignedTamperedBlockMap-TRUST_E_BAD_DIGEST.appx";
    MSIX_VALIDATION_OPTION validation = MSIX_VALIDATION_OPTION_FULL;
    MSIX_PACKUNPACK_OPTION packUnpack = MSIX_PACKUNPACK_OPTION_NONE;

    RunUnpackTest(expected, package, validation, packUnpack);
}

// NOTE: This test is temporarily disable because it seems that there's a compiler bug
// that causes a SIGSEV in Android NDK r18b. The segmentation fault occurs
// even before the test runs any msix related code/test and only for release binaries
// This problem doesn't repro for in r17c or r19c. 
// As of 5/29/19, the current azure hosted images have r18b. Given that the newer ndk version
// works we are just going to wait for them to update it.
// TODO: enable this test after https://github.com/microsoft/azure-pipelines-image-generation/issues/980
// is resolved.
TEST_CASE("Unpack_SignedTamperedBlockMap-TRUST_E_BAD_DIGEST_sv", "[unpack][!hide]")
{
    HRESULT expected                  = static_cast<HRESULT>(MSIX::Error::SignatureInvalid);
    std::string package               = "SignedTamperedBlockMap-TRUST_E_BAD_DIGEST.appx";
    MSIX_VALIDATION_OPTION validation = MSIX_VALIDATION_OPTION_ALLOWSIGNATUREORIGINUNKNOWN;
    MSIX_PACKUNPACK_OPTION packUnpack = MSIX_PACKUNPACK_OPTION_NONE;

    RunUnpackTest(expected, package, validation, packUnpack);
}

TEST_CASE("Unpack_SignedTamperedCD-TRUST_E_BAD_DIGEST", "[unpack]")
{
    HRESULT expected                  = static_cast<HRESULT>(MSIX::Error::CertNotTrusted);
    std::string package               = "SignedTamperedCD-TRUST_E_BAD_DIGEST.appx";
    MSIX_VALIDATION_OPTION validation = MSIX_VALIDATION_OPTION_FULL;
    MSIX_PACKUNPACK_OPTION packUnpack = MSIX_PACKUNPACK_OPTION_NONE;

    RunUnpackTest(expected, package, validation, packUnpack);
}

TEST_CASE("Unpack_SignedUntrustedCert", "[unpack]")
{
    HRESULT expected                  = static_cast<HRESULT>(MSIX::Error::CertNotTrusted);
    std::string package               = "SignedUntrustedCert-CERT_E_CHAINING.appx";
    MSIX_VALIDATION_OPTION validation = MSIX_VALIDATION_OPTION_FULL;
    MSIX_PACKUNPACK_OPTION packUnpack = MSIX_PACKUNPACK_OPTION_NONE;

    RunUnpackTest(expected, package, validation, packUnpack);
}

TEST_CASE("Unpack_TestAppxPackage_Win32", "[unpack]")
{
    HRESULT expected                  = S_OK;
    std::string package               = "TestAppxPackage_Win32.appx";
    MSIX_VALIDATION_OPTION validation = MSIX_VALIDATION_OPTION_SKIPSIGNATURE;
    MSIX_PACKUNPACK_OPTION packUnpack = MSIX_PACKUNPACK_OPTION_NONE;

    RunUnpackTest(expected, package, validation, packUnpack);
}

TEST_CASE("Unpack_TestAppxPackage_x64", "[unpack]")
{
    HRESULT expected                  = S_OK;
    std::string package               = "TestAppxPackage_x64.appx";
    MSIX_VALIDATION_OPTION validation = MSIX_VALIDATION_OPTION_SKIPSIGNATURE;
    MSIX_PACKUNPACK_OPTION packUnpack = MSIX_PACKUNPACK_OPTION_NONE;

    RunUnpackTest(expected, package, validation, packUnpack);
}

TEST_CASE("Unpack_UnsignedZip64WithCI-APPX_E_MISSING_REQUIRED_FILE", "[unpack]")
{
    HRESULT expected                  = static_cast<HRESULT>(MSIX::Error::MissingAppxSignatureP7X);
    std::string package               = "UnsignedZip64WithCI-APPX_E_MISSING_REQUIRED_FILE.appx";
    MSIX_VALIDATION_OPTION validation = MSIX_VALIDATION_OPTION_FULL;
    MSIX_PACKUNPACK_OPTION packUnpack = MSIX_PACKUNPACK_OPTION_NONE;

    RunUnpackTest(expected, package, validation, packUnpack);
}

TEST_CASE("Unpack_FileDoesNotExist", "[unpack]")
{
    HRESULT expected                  = static_cast<HRESULT>(MSIX::Error::FileOpen);
    std::string package               = "FileDoesNotExist.appx";
    MSIX_VALIDATION_OPTION validation = MSIX_VALIDATION_OPTION_SKIPSIGNATURE;
    MSIX_PACKUNPACK_OPTION packUnpack = MSIX_PACKUNPACK_OPTION_NONE;

    RunUnpackTest(expected, package, validation, packUnpack);
}

TEST_CASE("Unpack_BlockMap_Missing_Manifest_in_blockmap", "[unpack]")
{
    HRESULT expected                  = static_cast<HRESULT>(MSIX::Error::BlockMapSemanticError);
    std::string package               = "BlockMap/Missing_Manifest_in_blockmap.appx";
    MSIX_VALIDATION_OPTION validation = MSIX_VALIDATION_OPTION_SKIPSIGNATURE;
    MSIX_PACKUNPACK_OPTION packUnpack = MSIX_PACKUNPACK_OPTION_NONE;

    RunUnpackTest(expected, package, validation, packUnpack);
}

TEST_CASE("Unpack_BlockMap_ContentTypes_in_blockmap", "[unpack]")
{
    HRESULT expected                  = static_cast<HRESULT>(MSIX::Error::BlockMapSemanticError);
    std::string package               = "BlockMap/ContentTypes_in_blockmap.appx";
    MSIX_VALIDATION_OPTION validation = MSIX_VALIDATION_OPTION_SKIPSIGNATURE;
    MSIX_PACKUNPACK_OPTION packUnpack = MSIX_PACKUNPACK_OPTION_NONE;

    RunUnpackTest(expected, package, validation, packUnpack);
}

TEST_CASE("Unpack_BlockMap_Invalid_Bad_Block", "[unpack]")
{
    HRESULT expected                  = static_cast<HRESULT>(MSIX::Error::BlockMapSemanticError);
    std::string package               = "BlockMap/Invalid_Bad_Block.msix";
    MSIX_VALIDATION_OPTION validation = MSIX_VALIDATION_OPTION_SKIPSIGNATURE;
    MSIX_PACKUNPACK_OPTION packUnpack = MSIX_PACKUNPACK_OPTION_NONE;

    RunUnpackTest(expected, package, validation, packUnpack);
}

TEST_CASE("Unpack_BlockMap_Size_wrong_uncompressed", "[unpack]")
{
    HRESULT expected                  = static_cast<HRESULT>(MSIX::Error::BlockMapSemanticError);
    std::string package               = "BlockMap/Size_wrong_uncompressed.msix";
    MSIX_VALIDATION_OPTION validation = MSIX_VALIDATION_OPTION_SKIPSIGNATURE;
    MSIX_PACKUNPACK_OPTION packUnpack = MSIX_PACKUNPACK_OPTION_NONE;

    RunUnpackTest(expected, package, validation, packUnpack);
}

TEST_CASE("Unpack_BlockMap_Extra_file_in_blockmap", "[unpack]")
{
    HRESULT expected                  = static_cast<HRESULT>(MSIX::Error::FileNotFound);
    std::string package               = "BlockMap/Extra_file_in_blockmap.msix";
    MSIX_VALIDATION_OPTION validation = MSIX_VALIDATION_OPTION_SKIPSIGNATURE;
    MSIX_PACKUNPACK_OPTION packUnpack = MSIX_PACKUNPACK_OPTION_NONE;

    RunUnpackTest(expected, package, validation, packUnpack);
}

TEST_CASE("Unpack_BlockMap_File_missing_from_blockmap", "[unpack]")
{
    HRESULT expected                  = static_cast<HRESULT>(MSIX::Error::BlockMapSemanticError);
    std::string package               = "BlockMap/File_missing_from_blockmap.msix";
    MSIX_VALIDATION_OPTION validation = MSIX_VALIDATION_OPTION_SKIPSIGNATURE;
    MSIX_PACKUNPACK_OPTION packUnpack = MSIX_PACKUNPACK_OPTION_NONE;

    RunUnpackTest(expected, package, validation, packUnpack);
}

TEST_CASE("Unpack_BlockMap_No_blockmap.appx", "[unpack]")
{
    HRESULT expected                  = static_cast<HRESULT>(MSIX::Error::MissingAppxBlockMapXML);
    std::string package               = "BlockMap/No_blockmap.appx";
    MSIX_VALIDATION_OPTION validation = MSIX_VALIDATION_OPTION_SKIPSIGNATURE;
    MSIX_PACKUNPACK_OPTION packUnpack = MSIX_PACKUNPACK_OPTION_NONE;

    RunUnpackTest(expected, package, validation, packUnpack);
}

TEST_CASE("Unpack_BlockMap_Bad_Namespace_Blockmap", "[unpack]")
{
    HRESULT expected                  = static_cast<HRESULT>(MSIX::Error::XmlFatal);
    std::string package               = "BlockMap/Bad_Namespace_Blockmap.appx";
    MSIX_VALIDATION_OPTION validation = MSIX_VALIDATION_OPTION_SKIPSIGNATURE;
    MSIX_PACKUNPACK_OPTION packUnpack = MSIX_PACKUNPACK_OPTION_NONE;

    RunUnpackTest(expected, package, validation, packUnpack);
}

TEST_CASE("Unpack_BlockMap_Duplicate_file_in_blockmap", "[unpack]")
{
    HRESULT expected                  = static_cast<HRESULT>(MSIX::Error::BlockMapSemanticError);
    std::string package               = "BlockMap/Duplicate_file_in_blockmap.appx";
    MSIX_VALIDATION_OPTION validation = MSIX_VALIDATION_OPTION_SKIPSIGNATURE;
    MSIX_PACKUNPACK_OPTION packUnpack = MSIX_PACKUNPACK_OPTION_NONE;

    RunUnpackTest(expected, package, validation, packUnpack);
}

TEST_CASE("Unpack_IntlCharsInPath", "[unpack]")
{
    HRESULT expected                  = S_OK;
    std::string package               = "महसुस/StoreSigned_Desktop_x64_MoviesTV.appx";
    MSIX_VALIDATION_OPTION validation = MSIX_VALIDATION_OPTION_ALLOWSIGNATUREORIGINUNKNOWN;
    MSIX_PACKUNPACK_OPTION packUnpack = MSIX_PACKUNPACK_OPTION_NONE;

    RunUnpackTest(expected, package, validation, packUnpack);
}

TEST_CASE("Unpack_To_Absolute_Path", "[unpack]")
{
    HRESULT expected                  = S_OK;
    std::string package               = "StoreSigned_Desktop_x64_MoviesTV.appx";
    MSIX_VALIDATION_OPTION validation = MSIX_VALIDATION_OPTION_FULL;
    MSIX_PACKUNPACK_OPTION packUnpack = MSIX_PACKUNPACK_OPTION_NONE;

    RunUnpackTest(expected, package, validation, packUnpack, false, true);

    // Verify all the files extracted on disk are correct
    auto files = MsixTest::Unpack::GetExpectedFiles();
    auto outputDir = MsixTest::TestPath::GetInstance()->GetPath(MsixTest::TestPath::Directory::Output);
    CHECK(MsixTest::Directory::CompareDirectory(outputDir, files));

    // Clean directory
    CHECK(MsixTest::Directory::CleanDirectory(outputDir));
}
