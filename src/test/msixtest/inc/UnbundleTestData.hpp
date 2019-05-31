//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once
#include "AppxPackaging.hpp"
#include <map>
#include <vector>
#include <string>

namespace MsixTest { 

    namespace Unbundle {

        struct ExpectedPackage
        {
            const std::string name;
            APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE type;
            const std::uint64_t offset;
            const std::uint64_t size;
            const std::vector<std::string> languages;
            const std::vector<UINT32> scales;

            ExpectedPackage(const char* n, APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE t, std::uint64_t o, std::uint64_t s, const std::vector<std::string>& l, const std::vector<UINT32>& sc) : 
                name(n), type(t), offset(o), size(s), languages(l), scales(sc) {}
        };

        // Expected files for full applicability of StoreSigned_Desktop_x86_x64_MoviesTV.appxbundle
        // in a system that only has English as its language
        const std::map<std::string, std::uint64_t>& GetExpectedFilesFullApplicable();

        // Expected files for language applicability of StoreSigned_Desktop_x86_x64_MoviesTV.appxbundle
        // in a system that only has English as its language that are NOT expected to be extracted.
        const std::map<std::string, std::uint64_t>& GetExpectedFilesNoLanguageApplicable();

        // Expected files for full applicability of StoreSigned_Desktop_x86_x64_MoviesTV.appxbundle
        // in a system that only has English as its language that are NOT expected to be extracted.
        const std::map<std::string, std::uint64_t>& GetExpectedFilesNoApplicable();

        // Expected files to be extracted when unbundling StoreSigned_Desktop_x86_x64_MoviesTV.appxbundle
        // with full applicability and the "pfn-flat" unpcack option. 
        const std::map<std::string, std::uint64_t>& GetExpectedBundleFilesPfnFlatFullApplicable();

        // Expected bundle manifest information from MainBundle.appxbundle
        const std::vector<ExpectedPackage>& GetExpectedPackages();
    }
}
