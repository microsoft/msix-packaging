//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once

#include <algorithm>
#include <string>

namespace MSIX {
    namespace Helper {

        std::string tolower(std::string s)
        {
            std::transform(s.begin(), s.end(), s.begin(), ::tolower);
            return s;
        }

    }
}
