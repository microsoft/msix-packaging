//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once

#include <set>
#include <string>

namespace MSIX {

    class Applicability
    {
    public:
        static MSIX_PLATFORM GetPlatform();
        static std::set<std::string> GetLanguages();
    };
}