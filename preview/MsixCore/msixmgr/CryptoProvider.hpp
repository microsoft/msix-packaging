#pragma once

#include <ByteBuffer.hpp>

#define APPXCOMMON_NEW_TAG (std::nothrow)

namespace MsixCoreLib
{
    // CryptoProvider objects are not thread-safe, hence should not be called from multiple threads simultaneously.
    // Usage:
    //   (StartDigest DigestData* GetDigest* Reset)*
    class CryptoProvider
    {
    private:

        BCRYPT_ALG_HANDLE providerHandle;
        BCRYPT_HASH_HANDLE hashHandle;
        ByteBuffer hashObjectBuffer;
        ByteBuffer digestBuffer;
        BYTE quickHashObjectBuffer[700];  // Tests shows that HMAC with 256-bit or 512-bit keys requires 600+ bytes of hash object space.
        BYTE quickDigestBuffer[64];   // accommodates up to 64-byte hashes
        COMMON_BYTES digest;

        HRESULT OpenProvider();

        CryptoProvider();

    public:
        ~CryptoProvider();

        void Reset();

        HRESULT StartDigest();

        HRESULT DigestData(
            _In_ const COMMON_BYTES* data);

        HRESULT GetDigest(
            _Out_ COMMON_BYTES* digest);

        static HRESULT Create(
            _Outptr_ CryptoProvider** provider);
    };
}