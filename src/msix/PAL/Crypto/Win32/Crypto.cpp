//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#include "ntstatus.h"
#define WIN32_NO_STATUS
#include <windows.h>
#include <bcrypt.h>
#include <winternl.h>
#include <winerror.h>
#include "Exceptions.hpp"
#include "Crypto.hpp"
#include "UnicodeConversion.hpp"

#include <memory>
#include <vector>

struct unique_hash_handle_deleter {
    void operator()(BCRYPT_HASH_HANDLE h) const {
        BCryptDestroyHash(h);
    };
};

struct unique_alg_handle_deleter {
    void operator()(BCRYPT_ALG_HANDLE h) const {
        BCryptCloseAlgorithmProvider(h, 0);
    };
};

typedef std::unique_ptr<void, unique_alg_handle_deleter> unique_alg_handle;
typedef std::unique_ptr<void, unique_hash_handle_deleter> unique_hash_handle;

namespace MSIX {

    class NtStatusException final : public Exception
    {
    public:
        NtStatusException(std::string& message, NTSTATUS error) : Exception(message, static_cast<std::uint32_t>(error)) {}
    };

    #define ThrowStatusIfFailed(a, m)                                                      \
    {   NTSTATUS _status = a;                                                              \
        if (!NT_SUCCESS(_status))                                                          \
        {   MSIX::RaiseException<MSIX::NtStatusException>(__LINE__, __FILE__, m, _status); \
        }                                                                                  \
    }
    
    SHA256::SHA256()
    {
        Reset();
    }

    SHA256::~SHA256()
    {
        if (m_hashContext != nullptr)
        {
            (void)BCryptDestroyHash(m_hashContext);
        }
    }

    void SHA256::Reset()
    {
        if (m_hashContext != nullptr)
        {
            (void)BCryptDestroyHash(m_hashContext);
            m_hashContext = nullptr;
        }

        BCRYPT_HASH_HANDLE hashHandleT;
        BCRYPT_ALG_HANDLE algHandleT;

        // Open an algorithm handle
        BCRYPT_ALG_HANDLE algHandleT{};
        ThrowStatusIfFailed(BCryptOpenAlgorithmProvider(
            &algHandleT,                 // Alg Handle pointer
            BCRYPT_SHA256_ALGORITHM,    // Cryptographic Algorithm name (null terminated unicode string)
            nullptr,                    // Provider name; if null, the default provider is loaded
            0),                         // Flags; Loads a provider which supports reusable hash
            "failed computing SHA256 hash");
        unique_alg_handle algHandle(algHandleT);

        // Create a hash handle
        ThrowStatusIfFailed(BCryptCreateHash(
            context->algHandle.get(),   // Handle to an algorithm provider
            &hashHandleT,               // A pointer to a hash handle - can be a hash or hmac object
            nullptr,                    // Pointer to the buffer that receives the hash/hmac object
            0,                          // Size of the buffer in bytes
            nullptr,                    // A pointer to a key to use for the hash or MAC
            0,                          // Size of the key in bytes
            0),                         // Flags
            "failed computing SHA256 hash");

        m_hashContext = hashHandleT;
    }
    
    void SHA256::HashData(const std::uint8_t* buffer, std::uint32_t cbBuffer)
    {
        ThrowErrorIf(Error::InvalidState, m_hashContext == nullptr, "HashData is called before hash context is initialized.");

        ThrowStatusIfFailed(BCryptHashData(
            m_hashContext,              // Handle to the hash or MAC object
            (PBYTE)buffer,              // A pointer to a buffer that contains the data to hash
            cbBuffer,                   // Size of the buffer in bytes
            0),                         // Flags
            "failed computing SHA256 hash");
    }

    void SHA256::FinalizeAndGetHashValue(std::vector<uint8_t>& hash)
    {
        ThrowErrorIf(Error::InvalidState, m_hashContext == nullptr, "HashData is called before hash context is initialized.");

        // Size the hash buffer appropriately
        hash.resize(SHA256_DIGEST_LENGTH);

        // Obtain the hash of the message(s) into the hash buffer
        ThrowStatusIfFailed(BCryptFinishHash(
            m_hashContext,              // Handle to the hash or MAC object
            hash.data(),                // A pointer to a buffer that receives the hash or MAC value
            static_cast<ULONG>(hash.size()),   // Size of the buffer in bytes
            0),                         // Flags
            "failed computing SHA256 hash");

        (void)BCryptDestroyHash(m_hashContext);
        m_hashContext = nullptr;
    }

    bool SHA256::ComputeHash(const std::uint8_t* buffer, std::uint32_t cbBuffer, std::vector<uint8_t>& hash)
    {
        SHA256 hashEngine;
        hashEngine.HashData(buffer, cbBuffer);
        hashEngine.FinalizeAndGetHashValue(hash);
        return true;
    }

    void SHA256::SHA256ContextDeleter::operator()(SHA256Context* context)
    {
        delete context;
    }

    std::string Base64::ComputeBase64(const std::vector<std::uint8_t>& buffer)
    {
        std::wstring result;
        DWORD encodingFlags = CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF;
        DWORD encodedHashSize = 0;
        ThrowHrIfFalse(CryptBinaryToStringW(buffer.data(), static_cast<DWORD>(buffer.size()), encodingFlags, nullptr, &encodedHashSize),
            "CryptBinaryToStringW failed");
        result.resize(encodedHashSize-1); // CryptBinaryToStringW returned size includes null termination
        ThrowHrIfFalse(CryptBinaryToStringW(buffer.data(), static_cast<DWORD>(buffer.size()), encodingFlags, const_cast<wchar_t*>(result.data()), &encodedHashSize),
            "CryptBinaryToStringW failed");
        return wstring_to_utf8(result);
    }
}
