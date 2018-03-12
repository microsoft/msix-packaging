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
#include "SHA256.hpp"

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

    bool SHA256::ComputeHash(std::uint8_t* buffer, std::uint32_t cbBuffer, std::vector<uint8_t>& hash)
    {
        NTSTATUS status = STATUS_SUCCESS;
        BCRYPT_HASH_HANDLE hashHandleT;
        BCRYPT_ALG_HANDLE algHandleT;
        DWORD hashLength = 0;
        DWORD resultLength = 0;

        // Open an algorithm handle
        // This code passes BCRYPT_HASH_REUSABLE_FLAG with BCryptAlgorithmProvider(...) to load a provider which supports reusable hash
        status = BCryptOpenAlgorithmProvider(
            &algHandleT,                // Alg Handle pointer
            BCRYPT_SHA256_ALGORITHM,    // Cryptographic Algorithm name (null terminated unicode string)
            nullptr,                    // Provider name; if null, the default provider is loaded
            0);                         // Flags; Loads a provider which supports reusable hash

        if (!NT_SUCCESS(status))
        {   throw MSIX::NtStatusException(status, "failed computing SHA256 hash");
        }

        unique_alg_handle algHandle(algHandleT);

        // Obtain the length of the hash
        status = BCryptGetProperty(
            algHandle.get(),            // Handle to a CNG object
            BCRYPT_HASH_LENGTH,         // Property name (null terminated unicode string)
            (PBYTE)&hashLength,         // Address of the output buffer which recieves the property value
            sizeof(hashLength),         // Size of the buffer in bytes
            &resultLength,              // Number of bytes that were copied into the buffer
            0);                         // Flags

        if (!NT_SUCCESS(status) || resultLength != sizeof(hashLength))
        {   throw MSIX::NtStatusException(status, "failed computing SHA256 hash");
        }

        // Size the hash buffer appropriately
        hash.resize(hashLength);

        // Create a hash handle
        status = BCryptCreateHash(
            algHandle.get(),            // Handle to an algorithm provider                 
            &hashHandleT,               // A pointer to a hash handle - can be a hash or hmac object
            nullptr,                    // Pointer to the buffer that recieves the hash/hmac object
            0,                          // Size of the buffer in bytes
            nullptr,                    // A pointer to a key to use for the hash or MAC
            0,                          // Size of the key in bytes
            0);                         // Flags

        if (!NT_SUCCESS(status))
        {   throw MSIX::NtStatusException(status, "failed computing SHA256 hash");
        }

        unique_hash_handle hashHandle(hashHandleT);

        // Hash the message(s)
        status = BCryptHashData(
            hashHandle.get(),           // Handle to the hash or MAC object
            (PBYTE)buffer,              // A pointer to a buffer that contains the data to hash
            cbBuffer,                   // Size of the buffer in bytes
            0);                         // Flags

        if (!NT_SUCCESS(status))
        {   throw MSIX::NtStatusException(status, "failed computing SHA256 hash");
        }

        // Obtain the hash of the message(s) into the hash buffer
        status = BCryptFinishHash(
            hashHandle.get(),           // Handle to the hash or MAC object
            hash.data(),                // A pointer to a buffer that receives the hash or MAC value
            hashLength,                 // Size of the buffer in bytes
            0);                         // Flags

        if (!NT_SUCCESS(status))
        {   throw MSIX::NtStatusException(status, "failed computing SHA256 hash");
        }

        return true;
    }
}
