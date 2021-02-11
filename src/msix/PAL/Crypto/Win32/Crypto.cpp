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
#include <deque>

//////////////////////////////////////////////////////////////////////////////////////////////
//                                       ScopeGuard                                         //
//////////////////////////////////////////////////////////////////////////////////////////////
// inspired by
// https://channel9.msdn.com/Shows/Going+Deep/C-and-Beyond-2012-Andrei-Alexandrescu-Systematic-Error-Handling-in-C
class ScopeGuard
{
    public:
    enum class Policy
    {
        Always,
        NoException,
        Exception
    };

    ScopeGuard(ScopeGuard&&) = default;
    explicit ScopeGuard(Policy p = Policy::Always) : _policy(p) {}

    template <class Lambda> ScopeGuard(Lambda&& func, Policy p = Policy::Always) : _policy(p)
    {
        this->operator+=<Lambda>(std::forward<Lambda>(func));
    }

    template <class Lambda> ScopeGuard& operator+=(Lambda&& func)
    try
    {
        _handlers.emplace_front(std::forward<Lambda>(func));
        return *this;
    }
    catch (...)
    {
        if (_policy != Policy::NoException) { func(); }
        throw;
    }

    ~ScopeGuard()
    {
        if (_policy == Policy::Always || (std::uncaught_exception() == (_policy == Policy::Exception)))
        {
            for (const auto& f : _handlers) { f(); /* better not throw! */ }
        }
    }

    void Dismiss() noexcept { _handlers.clear(); }

    private:
    ScopeGuard(const ScopeGuard&) = delete;
    void operator=(const ScopeGuard&) = delete;

    std::deque<std::function<void()>> _handlers;
    Policy                            _policy = Policy::Always;
};

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

    bool SHA256::ComputeHash(std::uint8_t* buffer, std::uint32_t cbBuffer, std::vector<uint8_t>& hash)
    {
        BCRYPT_HASH_HANDLE hashHandleT;
        BCRYPT_ALG_HANDLE algHandleT;
        DWORD hashLength = 0;
        DWORD resultLength = 0;

        // Open an algorithm handle
        // This code passes BCRYPT_HASH_REUSABLE_FLAG with BCryptAlgorithmProvider(...) to load a provider which supports reusable hash
        ThrowStatusIfFailed(BCryptOpenAlgorithmProvider(
            &algHandleT,                // Alg Handle pointer
            BCRYPT_SHA256_ALGORITHM,    // Cryptographic Algorithm name (null terminated unicode string)
            nullptr,                    // Provider name; if null, the default provider is loaded
            0),                         // Flags; Loads a provider which supports reusable hash
        "failed computing SHA256 hash");

        // Obtain the length of the hash
        ThrowStatusIfFailed(BCryptGetProperty(
            algHandleT,            // Handle to a CNG object
            BCRYPT_HASH_LENGTH,    // Property name (null terminated unicode string)
            (PBYTE)&hashLength,    // Address of the output buffer which receives the property value
            sizeof(hashLength),    // Size of the buffer in bytes
            &resultLength,         // Number of bytes that were copied into the buffer
            0),                    // Flags
        "failed computing SHA256 hash");
        ThrowErrorIf(Error::Unexpected, (resultLength != sizeof(hashLength)), "failed computing SHA256 hash");
        ScopeGuard guard([&algHandleT](){
            BCryptCloseAlgorithmProvider(algHandleT, 0);
        });

        // Size the hash buffer appropriately
        hash.resize(hashLength);

        // Create a hash handle
        ThrowStatusIfFailed(BCryptCreateHash(
            algHandleT,                 // Handle to an algorithm provider                 
            &hashHandleT,               // A pointer to a hash handle - can be a hash or hmac object
            nullptr,                    // Pointer to the buffer that receives the hash/hmac object
            0,                          // Size of the buffer in bytes
            nullptr,                    // A pointer to a key to use for the hash or MAC
            0,                          // Size of the key in bytes
            0),                         // Flags
        "failed computing SHA256 hash");

        // Hash the message(s)
        ThrowStatusIfFailed(BCryptHashData(
            hashHandleT,                // Handle to the hash or MAC object
            (PBYTE)buffer,              // A pointer to a buffer that contains the data to hash
            cbBuffer,                   // Size of the buffer in bytes
            0),                         // Flags
        "failed computing SHA256 hash");
        guard+= [&hashHandleT](){ BCryptDestroyHash(hashHandleT); };

        // Obtain the hash of the message(s) into the hash buffer
        ThrowStatusIfFailed(BCryptFinishHash(
            hashHandleT,                // Handle to the hash or MAC object
            hash.data(),                // A pointer to a buffer that receives the hash or MAC value
            hashLength,                 // Size of the buffer in bytes
            0),                         // Flags
        "failed computing SHA256 hash");

        return true;
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
