//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
// End-to-end pack tests
#include "catch.hpp"
#include "msixtest_int.hpp"
#include "macros.hpp"
#include "FileHelpers.hpp"
#include "PackTestData.hpp"

#include <iostream>

static std::string outputPackage = "package.msix";

void RunPackTest(HRESULT expected, const std::string& directory)
{
    std::cout << "\tPacking test directory: " << directory << std::endl; 

    auto testData = MsixTest::TestPath::GetInstance();

    auto directoryPath = testData->GetPath(MsixTest::TestPath::Directory::Pack) + "/" + directory;
    directoryPath = MsixTest::Directory::PathAsCurrentPlatform(directoryPath);

    auto outputDir = testData->GetPath(MsixTest::TestPath::Directory::Output);

    HRESULT actual = PackPackage(MSIX_PACKUNPACK_OPTION::MSIX_PACKUNPACK_OPTION_NONE,
                                 MSIX_VALIDATION_OPTION::MSIX_VALIDATION_OPTION_SKIPSIGNATURE,
                                 const_cast<char*>(directoryPath.c_str()),
                                 const_cast<char*>(outputPackage.c_str()));

    CHECK(expected == actual);
    MsixTest::Log::PrintMsixLog(expected, actual);
}

TEST_CASE("Pack_Good", "[pack]")
{
    HRESULT expected      = S_OK;
    std::string directory = "input";

    RunPackTest(expected, directory);

    // verify output package exists
    auto outputStream = MsixTest::StreamFile(outputPackage, true, true);

    // Verify new package can be unpacked
    auto outputDir = MsixTest::TestPath::GetInstance()->GetPath(MsixTest::TestPath::Directory::Output);
    REQUIRE_SUCCEEDED(UnpackPackageFromStream(MSIX_PACKUNPACK_OPTION::MSIX_PACKUNPACK_OPTION_NONE,
                                              MSIX_VALIDATION_OPTION::MSIX_VALIDATION_OPTION_SKIPSIGNATURE,
                                              outputStream.Get(),
                                              const_cast<char*>(outputDir.c_str())));

    auto files = MsixTest::Pack::GetExpectedFiles();
    CHECK(MsixTest::Directory::CompareDirectory(outputDir, files));

    // Clean directory
    CHECK(MsixTest::Directory::CleanDirectory(outputDir));
}

// Fail if there's no AppxManifest.xml
TEST_CASE("Pack_AppxManifestNotPresent", "[pack]")
{
    HRESULT expected      = static_cast<HRESULT>(MSIX::Error::FileOpen);
    std::string directory = "input_nomanifest";

    RunPackTest(expected, directory);
}

// Fails if AppxManifest.xml is not in the top level of the directory
TEST_CASE("Pack_AppxManifestWrongLocation", "[pack]")
{
    HRESULT expected      = static_cast<HRESULT>(MSIX::Error::FileOpen);
    std::string directory = "input_misplace_manifest";

    RunPackTest(expected, directory);
}

// Fail if input directory does not exist
TEST_CASE("Pack_DirectoryNotPresent", "[pack]")
{
    HRESULT expected      = static_cast<HRESULT>(MSIX::Error::FileOpen);
    std::string directory = "fake_directory";

    RunPackTest(expected, directory);
}

// Validates output file is deleted when failed.
TEST_CASE("Pack_DeleteFileOnFailure", "[pack]")
{
    HRESULT expected      = static_cast<HRESULT>(MSIX::Error::InvalidParameter);
    std::string directory = "input_wrongfile";

    RunPackTest(expected, directory);

    // input_wrong file contains a file with an invalid name, but the output stream
    // was already created. Verify is gone.
    MsixTest::ComPtr<IStream> stream;
    REQUIRE_FAILED(CreateStreamOnFile(const_cast<char*>(outputPackage.c_str()), true, &stream));
}

// Fails if input parameters are null
TEST_CASE("Pack_InvalidParameters", "[pack]")
{
    REQUIRE_HR(static_cast<HRESULT>(MSIX::Error::InvalidParameter),
        PackPackage(MSIX_PACKUNPACK_OPTION::MSIX_PACKUNPACK_OPTION_NONE,
                    MSIX_VALIDATION_OPTION::MSIX_VALIDATION_OPTION_SKIPSIGNATURE,
                    nullptr,
                    const_cast<char*>(outputPackage.c_str())));
        
    REQUIRE_HR(static_cast<HRESULT>(MSIX::Error::InvalidParameter),
        PackPackage(MSIX_PACKUNPACK_OPTION::MSIX_PACKUNPACK_OPTION_NONE,
                    MSIX_VALIDATION_OPTION::MSIX_VALIDATION_OPTION_SKIPSIGNATURE,
                    const_cast<char*>(outputPackage.c_str()),
                    nullptr));
}
