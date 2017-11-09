#include "Exceptions.hpp"
#include "SHA256.hpp"
#include <windows.h>
#include <bcrypt.h>

#include <memory>
#include <vector>

namespace xPlat {

    static bool SHA256::ComputeHash(/*in*/ std::uint8_t* buffer, /*in*/ std::uint32_t cbBuffer, /*inout*/ std::vector<uint8_t>& hash)
    {
        NTSTATUS    status;

        BCRYPT_HASH_HANDLE hashHandleT;
        BCRYPT_ALG_HANDLE algHandleT;

        DWORD   hashLength = 0;
        DWORD   resultLength = 0;

        // Open an algorithm handle
        // This code passes BCRYPT_HASH_REUSABLE_FLAG with BCryptAlgorithmProvider(...) to load a provider which supports reusable hash
        status = BCryptOpenAlgorithmProvider(
            &algHandleT,                // Alg Handle pointer
            BCRYPT_SHA256_ALGORITHM,    // Cryptographic Algorithm name (null terminated unicode string)
            NULL,                       // Provider name; if null, the default provider is loaded
            BCRYPT_HASH_REUSABLE_FLAG); // Flags; Loads a provider which supports reusable hash

        if (!NT_SUCCESS(status))
            throw xPlat::Exception(xPlat::Error::AppxSignatureInvalid); //TODO

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
            throw xPlat::Exception(xPlat::Error::AppxSignatureInvalid); //TODO

        // Size the hash buffer appropriately
        hash.resize(hashLength);

        // Create a hash handle
        status = BCryptCreateHash(
            algHandle.get(),            // Handle to an algorithm provider                 
            &hashHandleT,               // A pointer to a hash handle - can be a hash or hmac object
            NULL,                       // Pointer to the buffer that recieves the hash/hmac object
            0,                          // Size of the buffer in bytes
            NULL,                       // A pointer to a key to use for the hash or MAC
            0,                          // Size of the key in bytes
            0);                         // Flags

        if (!NT_SUCCESS(status))
            throw xPlat::Exception(xPlat::Error::AppxSignatureInvalid); //TODO

        unique_hash_handle hashHandle(hashHandleT);

        // Hash the message(s)
        status = BCryptHashData(
            hashHandle.get(),           // Handle to the hash or MAC object
            (PBYTE)buffer,              // A pointer to a buffer that contains the data to hash
            cbBuffer,               // Size of the buffer in bytes
            0);                         // Flags

        if (!NT_SUCCESS(status))
            throw xPlat::Exception(xPlat::Error::AppxSignatureInvalid); //TODO

        // Obtain the hash of the message(s) into the hash buffer
        status = BCryptFinishHash(
            hashHandle.get(),           // Handle to the hash or MAC object
            hash.data(),                // A pointer to a buffer that receives the hash or MAC value
            hashLength,                 // Size of the buffer in bytes
            0);                         // Flags

        if (!NT_SUCCESS(status))
            throw xPlat::Exception(xPlat::Error::AppxSignatureInvalid); //TODO

        return true;
    }
}
