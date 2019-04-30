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
        static bool ComputeHash(std::uint8_t *buffer, std::uint32_t cbBuffer, std::vector<uint8_t>& hash);
    };
}