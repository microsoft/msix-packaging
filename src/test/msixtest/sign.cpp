//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
// End-to-end sign tests
#include "catch.hpp"
#include "msixtest_int.hpp"
#include "macros.hpp"
#include "FileHelpers.hpp"
#include "PackTestData.hpp"
#include "PackValidation.hpp"

#include <iostream>

static std::string outputPackage = "package-signed.msix";

void RunSignTest(
        HRESULT expected,
        const std::string& directory,
        const std::string& pfx,
        const LPCSTR pass = nullptr
    )
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

    CHECK(S_OK == actual);

    auto signPath = testData->GetPath(MsixTest::TestPath::Directory::Sign) + "/" + pfx;

    std::cout << "\tSigning generated package: " << outputPackage << std::endl;

    actual = SignPackage(
            MSIX_SIGNING_OPTIONS::MSIX_SIGNING_OPTIONS_NONE,
            outputPackage.c_str(),
            MSIX_CERTIFICATE_FORMAT::MSIX_CERTIFICATE_FORMAT_PFX,
            signPath.c_str(),
            pass,
            nullptr);

    CHECK(expected == actual);
            
    MsixTest::Log::PrintMsixLog(expected, actual);
}

TEST_CASE("Sign_Good_NoPass", "[sign]")
{
    HRESULT expected      = S_OK;
    std::string directory = "input";

    RunSignTest(expected, directory, "test-self-signed-pfx-no-pass.pfx");

    // Verify output package
    MsixTest::Pack::ValidatePackageStream(outputPackage, true);
}

TEST_CASE("Sign_Good_SimplePass", "[sign]")
{
    HRESULT expected      = S_OK;
    std::string directory = "input";

    RunSignTest(expected, directory, "test-self-signed-pfx-simple-pass.pfx", "testPass");

    // Verify output package
    MsixTest::Pack::ValidatePackageStream(outputPackage, true);
}
