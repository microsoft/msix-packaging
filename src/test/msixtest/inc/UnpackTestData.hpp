//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once
#include <map>
#include <string>

namespace MsixTest {

    namespace Unpack {

        // Returns files that must be unpacked for StoreSigned_Desktop_x64_MoviesTV.appx
        const std::map<std::string, std::uint64_t>& GetExpectedFiles();

    }
}
