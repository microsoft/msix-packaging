//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#include "catch.hpp"
#include "msixtest_int.hpp"
#include "PackTestData.hpp"
#include "FileHelpers.hpp"

namespace MsixTest {
    namespace Pack
    {
        // For non-windows, just use the MSIX SDK to validate the package.
        void ValidatePackageStream(const std::string& packageName)
        {
            // verify output package exists
            auto outputStream = MsixTest::StreamFile(packageName, true, true);

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
    } 
}
