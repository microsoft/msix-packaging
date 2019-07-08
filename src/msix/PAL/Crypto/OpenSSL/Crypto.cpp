//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#include "Exceptions.hpp"
#include "Crypto.hpp"
#include "SharedOpenSSL.hpp"

#include "openssl/sha.h"
#include "openssl/evp.h"

namespace MSIX {

    struct SHA256Context : SHA256_CTX
    {
    };

    SHA256::SHA256() : context(new SHA256Context{})
    {
        ThrowOpenSSLErrIfFailed(SHA256_Init(context.get()));
    }

    void SHA256::Add(const uint8_t* buffer, size_t cbBuffer)
    {
        EnsureNotFinished();

        ThrowOpenSSLErrIfFailed(SHA256_Update(context.get(), buffer, cbBuffer));
    }

    void SHA256::Get(HashBuffer& hash)
    {
        EnsureNotFinished();

        hash.resize(SHA256_DIGEST_LENGTH);
        ThrowOpenSSLErrIfFailed(SHA256_Final(hash.data(), context.get()));

        context.reset();
    }

    bool SHA256::ComputeHash(std::uint8_t *buffer, std::uint32_t cbBuffer, HashBuffer& hash)
    {
        hash.resize(SHA256_DIGEST_LENGTH);
        ::SHA256(buffer, cbBuffer, hash.data());
        return true;
    }

    void SHA256::SHA256ContextDeleter::operator()(SHA256Context* context)
    {
        delete context;
    }

    std::string Base64::ComputeBase64(const std::vector<std::uint8_t>& buffer)
    {
        int expectedSize = static_cast<int>(((buffer.size() + 2) / 3) * 4); // +2 for a cheap round up if it needs padding
        std::vector<std::uint8_t> result(expectedSize + 1); // +1 for the null character
        int encodeResult = EVP_EncodeBlock(static_cast<unsigned char*>(result.data()), const_cast<unsigned char*>(buffer.data()), static_cast<int>(buffer.size()));
        ThrowErrorIf(Error::Unexpected, expectedSize != encodeResult, "Error computing base64");
        return std::string(result.begin(), result.end() - 1); // remove the null character
    }
}
