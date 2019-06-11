//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once
#include <string>
#include <map>

namespace MsixTest {
    namespace Directory
    {
        bool CleanDirectory(const std::string& directory);
        bool CompareDirectory(const std::string& directory, const std::map<std::string, std::uint64_t>& files);

        std::string PathAsCurrentPlatform(const std::string& path);
    }
}
