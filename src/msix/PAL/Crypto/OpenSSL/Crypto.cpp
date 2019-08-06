//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#include "Exceptions.hpp"
#include "Crypto.hpp"

#include "openssl/sha.h"
#include "openssl/evp.h"

namespace MSIX {
    bool SHA256::ComputeHash(std::uint8_t *buffer, std::uint32_t cbBuffer, std::vector<uint8_t>& hash)
    {
        hash.resize(SHA256_DIGEST_LENGTH);
        ::SHA256(buffer, cbBuffer, hash.data());
        return true;
    }

    std::string Base64::ComputeBase64(const std::vector<std::uint8_t>& buffer)
    {
        int expectedSize = ((buffer.size() +2)/3)*4; // +2 for a cheap round up if it needs padding
        std::vector<std::uint8_t> result(expectedSize);
        int encodeResult = EVP_EncodeBlock(static_cast<unsigned char*>(result.data()), const_cast<unsigned char*>(buffer.data()), buffer.size());
        ThrowErrorIf(Error::Unexpected, expectedSize != encodeResult, "Error computing base64");
        return std::string(result.begin(), result.end());
    }
}
