//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once

#include <algorithm>
#include <string>

namespace MSIX {
    namespace Helper {

        inline std::string tolower(std::string s)
        {
            std::transform(s.begin(), s.end(), s.begin(), ::tolower);
            return s;
        }

        inline std::wstring towlower(std::wstring s)
        {
            std::transform(s.begin(), s.end(), s.begin(), ::towlower);
            return s;
        }

        inline std::string toBackLash(std::string s)
        {
            std::replace(s.begin(), s.end(), '/', '\\');
            return s;
        }
    }
}
