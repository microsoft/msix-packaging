//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once

#include <vector>
#include <string>

#include "AppxPackaging.hpp"

namespace MSIX {

    class Applicability
    {
    public:
        static MSIX_PLATFORMS GetPlatform();
        static std::vector<std::string> GetLanguages();
    };
}