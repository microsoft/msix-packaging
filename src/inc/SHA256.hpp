//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once

#include <vector>

namespace MSIX {

    class SHA256
    {
    public:
        static bool ComputeHash(/*in*/ std::uint8_t *buffer, /*in*/ std::uint32_t cbBuffer, /*inout*/ std::vector<uint8_t>& hash);
    };
}