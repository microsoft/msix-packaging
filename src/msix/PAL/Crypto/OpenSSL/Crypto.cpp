//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#include "Exceptions.hpp"
#include "Crypto.hpp"

#include "openssl/sha.h"

namespace MSIX {
    bool SHA256::ComputeHash(std::uint8_t *buffer, std::uint32_t cbBuffer, std::vector<uint8_t>& hash)
    {
        hash.resize(SHA256_DIGEST_LENGTH);
        ::SHA256(buffer, cbBuffer, hash.data());
        return true;
    }
} // namespace MSIX {