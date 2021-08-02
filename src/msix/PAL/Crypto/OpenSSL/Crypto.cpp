//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#include "Exceptions.hpp"
#include "Crypto.hpp"

#include "openssl/sha.h"
#include "openssl/evp.h"

namespace MSIX {
    SHA256::SHA256()
    {
        m_hashContext = new SHA256_CTX;
        Reset();
    }

    SHA256::~SHA256()
    {
        if (m_hashContext != nullptr)
        {
            // Linux, aosp (Android) and iOS compilers do not allow delete a void pointer, hence the casting.
            delete (SHA256_CTX*)m_hashContext;
        }
    }

    void SHA256::Reset()
    {
        ThrowErrorIfNot(Error::Unexpected, ::SHA256_Init((SHA256_CTX*)m_hashContext), "SHA256_Init failed");
    }

    void SHA256::HashData(const std::uint8_t* buffer, std::uint32_t cbBuffer)
    {
        ThrowErrorIfNot(Error::Unexpected, ::SHA256_Update((SHA256_CTX*)m_hashContext, buffer, cbBuffer), "SHA256_Update failed");
    }

    void SHA256::FinalizeAndGetHashValue(std::vector<uint8_t>& hash)
    {
        hash.resize(SHA256_DIGEST_LENGTH);
        ThrowErrorIfNot(Error::Unexpected, ::SHA256_Final(hash.data(), (SHA256_CTX*)m_hashContext), "SHA256_Final failed");
    }

    bool SHA256::ComputeHash(const std::uint8_t *buffer, std::uint32_t cbBuffer, std::vector<uint8_t>& hash)
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
