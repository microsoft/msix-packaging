//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#include "catch.hpp"
#include "msixtest_int.hpp"
#include "FileHelpers.hpp"
#include "macros.hpp"

#include <iostream>

TEST_CASE("Unbundle_NotSupported", "[unbundle]")
{
    HRESULT expected = static_cast<HRESULT>(MSIX::Error::NotSupported);
    auto bundle = "StoreSigned_Desktop_x86_x64_MoviesTV.appxbundle";
    auto bundlePath = MsixTest::TestPath::GetInstance()->GetPath(MsixTest::TestPath::Directory::Unbundle) + "/" + bundle;
    bundlePath = MsixTest::Directory::PathAsCurrentPlatform(bundlePath);
    auto outputDir = MsixTest::TestPath::GetInstance()->GetPath(MsixTest::TestPath::Directory::Output);

    std::cout << "Testing: " << std::endl;
    std::cout << "\tBundle: " << bundle << std::endl; 

    HRESULT actual = UnpackBundle(MSIX_PACKUNPACK_OPTION_NONE,
                                  MSIX_VALIDATION_OPTION_FULL,
                                  MSIX_APPLICABILITY_OPTION_FULL,
                                  const_cast<char*>(bundlePath.c_str()),
                                  const_cast<char*>(outputDir.c_str()));

    CHECK(expected == actual);
    MsixTest::Log::PrintMsixLog(expected, actual);
}