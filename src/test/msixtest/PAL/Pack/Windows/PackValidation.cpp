//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#include "catch.hpp"
#include "msixtest_int.hpp"
#include "PackTestData.hpp"
#include "FileHelpers.hpp"

#include "Windows.h"
#include <iostream>
namespace MsixTest {
    namespace Pack
    {
        // For windows, just use the MSIX SDK and Windows AppxPackaging APIs to validate the package.
        void ValidatePackageStream(const std::string& packageName)
        {
            // verify output package exists
            auto packageStream = MsixTest::StreamFile(packageName, true, true);

            // Verify new package can be unpacked via MSIX SDK
            auto outputDir = MsixTest::TestPath::GetInstance()->GetPath(MsixTest::TestPath::Directory::Output);
            REQUIRE_SUCCEEDED(UnpackPackageFromStream(MSIX_PACKUNPACK_OPTION::MSIX_PACKUNPACK_OPTION_NONE,
                                                      MSIX_VALIDATION_OPTION::MSIX_VALIDATION_OPTION_SKIPSIGNATURE,
                                                      packageStream.Get(),
                                                      const_cast<char*>(outputDir.c_str())));

            auto files = MsixTest::Pack::GetExpectedFiles();
            CHECK(MsixTest::Directory::CompareDirectory(outputDir, files));

            // Clean directory
            CHECK(MsixTest::Directory::CleanDirectory(outputDir));

            // Verify new package can be unpacked via Windows AppxPackaging Apis
            LARGE_INTEGER start = { 0 };
            REQUIRE_SUCCEEDED(packageStream->Seek(start, STREAM_SEEK_SET, nullptr));

            REQUIRE_SUCCEEDED(CoInitializeEx(NULL, COINIT_MULTITHREADED));
            {
                MsixTest::ComPtr<IAppxFactory> appxFactory;
                REQUIRE_SUCCEEDED(CoCreateInstance(__uuidof(AppxFactory), nullptr,
                    CLSCTX_INPROC_SERVER, __uuidof(IAppxFactory), reinterpret_cast<LPVOID*>(&appxFactory)));
            
                MsixTest::ComPtr<IAppxPackageReader> packageReader;
                REQUIRE_SUCCEEDED(appxFactory->CreatePackageReader(packageStream.Get(), &packageReader));
            }
            CoUninitialize();
        }
    } 
}
