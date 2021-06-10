//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once

#include <vector>

#ifndef SHA256_DIGEST_LENGTH
#define SHA256_DIGEST_LENGTH    32
#endif

namespace MSIX {

    class SHA256
    {
    public:
        static bool ComputeHash(const std::uint8_t *buffer, std::uint32_t cbBuffer, std::vector<uint8_t>& hash);

        /// <summary>
        /// Construct and initialize the hash engine so it can be used to compute hash of input data.
        /// </summary>
        SHA256();
        ~SHA256();

        /// <summary>
        /// Reset the internal state of the hash engine so it can be used again to hash data.
        /// </summary>
        void Reset();

        /// <summary>
        /// Hash data. Can be called repeatedly to hash a stream of data. Call FinalizeAndGetHashValue to finalize the hash engine
        /// and get the hash value of all the input data.
        /// </summary>
        /// <param name="buffer">Buffer containing data</param>
        /// <param name="cbBuffer">Size of the data in bytes</param>
        void HashData(const std::uint8_t* buffer, std::uint32_t cbBuffer);

        /// <summary>
        /// Hash data. Can be called repeatedly to hash a stream of data. Call FinalizeAndGetHashValue to finalize the hash engine
        /// and get the hash value of all the input data.
        /// </summary>
        /// <param name="buffer">Buffer containing data</param>
        inline void HashData(const std::vector<std::uint8_t>& buffer) {
            HashData(buffer.data(), static_cast<uint32_t>(buffer.size()));
        }

        /// <summary>
        /// Finalize the hash engine and get the computed hash value of all the input data from HashData calls.
        /// After this call, the hash engine cannot be used again until Reset is called to reset its state.
        /// </summary>
        /// <param name="hash">Output bufer to receive the computed hash value.</param>
        void FinalizeAndGetHashValue(std::vector<uint8_t>& hash);

    private:
        void* m_hashContext = nullptr;
    };

    class Base64
    {
    public:
        static std::string ComputeBase64(const std::vector<std::uint8_t>& buffer);
    };
}
