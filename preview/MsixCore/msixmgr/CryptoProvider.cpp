#include <Windows.h>
#include <WinCrypt.h>
//#include <ntstatus.h>

#include <BcryptLibrary.hpp>
#include <ByteBuffer.hpp>
#include <CryptoProvider.hpp>

#include "GeneralUtil.hpp"
#include <TraceLoggingProvider.h>
#include "MsixTraceLoggingProvider.hpp"
#include <assert.h>

namespace MsixCoreLib
{
    CryptoProvider::CryptoProvider()
    {
        this->providerHandle = NULL;
        this->hashHandle = NULL;
        this->digest.bytes = NULL;
        this->digest.length = 0;
    }

    CryptoProvider::~CryptoProvider()
    {
        Reset();
    }

    void CryptoProvider::Reset()
    {
        this->digest.bytes = NULL;
        this->digest.length = 0;

        if (NULL != this->hashHandle)
        {
            if (!SUCCEEDED(BcryptLibrary::BCryptDestroyHash(this->hashHandle)))
            {
                return;
            }
            this->hashHandle = NULL;
        }

        if (NULL != this->providerHandle)
        {
            if (!SUCCEEDED(BcryptLibrary::BCryptCloseAlgorithmProvider(this->providerHandle, 0)))
            {
                return;
            }
            this->providerHandle = NULL;
        }
    }

    HRESULT CryptoProvider::Create(
        _Outptr_ CryptoProvider** provider)
    {
        RETURN_IF_FAILED(BcryptLibrary::Load());

        std::unique_ptr<CryptoProvider> cp(new APPXCOMMON_NEW_TAG CryptoProvider());
        //RETURN_IF_NULL_ALLOC(cp);

        *provider = cp.release();
        return S_OK;
    }

    HRESULT CryptoProvider::OpenProvider()
    {
        assert(NULL == this->providerHandle);

        PCWSTR algorithmId = BCRYPT_SHA256_ALGORITHM;
        PCWSTR implementation = NULL;
        ULONG flags = 0;
        //NTSTATUS status = STATUS_SUCCESS;

        BcryptLibrary::BCryptOpenAlgorithmProvider(
            &this->providerHandle,
            algorithmId,
            implementation,
            flags);

        return S_OK;
    }

    HRESULT CryptoProvider::StartDigest()
    {
        BYTE* hashObject;
        ULONG hashObjectSize;
        ULONG resultSize;

        assert(reset == this->state);
        assert(NULL == this->hashHandle);

        RETURN_IF_FAILED(OpenProvider());

        BcryptLibrary::BCryptGetProperty(
            this->providerHandle,
            BCRYPT_OBJECT_LENGTH,
            (PUCHAR)&hashObjectSize,
            sizeof(hashObjectSize),
            &resultSize,
            0);

        if (sizeof(this->quickHashObjectBuffer) >= hashObjectSize)
        {
            hashObject = this->quickHashObjectBuffer;
        }
        else
        {
            RETURN_IF_FAILED(this->hashObjectBuffer.SetLength(hashObjectSize));
            hashObject = this->hashObjectBuffer.GetBufferPtr();
        }

        BcryptLibrary::BCryptCreateHash(
            this->providerHandle,
            &this->hashHandle,
            hashObject,
            hashObjectSize,
            NULL,
            NULL,
            0);

        return S_OK;
    }

    HRESULT CryptoProvider::DigestData(
        _In_ const COMMON_BYTES* data)
    {
        assert(inProgress == this->state);
        assert(NULL != this->hashHandle);

        //NTSTATUS status = STATUS_SUCCESS;

        BcryptLibrary::BCryptHashData(
            this->hashHandle,
            data->bytes,
            data->length,
            0);

        return S_OK;
    }

    HRESULT CryptoProvider::GetDigest(
        _Out_ COMMON_BYTES* digest)
    {
        BYTE* digestPtr;
        ULONG digestSize;

        assert(NULL != this->hashHandle);

        if (0 == this->digest.length)
        {
            NTSTATUS status;

            ULONG resultSize;

            assert(inProgress == this->state);

            status = BcryptLibrary::BCryptGetProperty(
                this->providerHandle,
                BCRYPT_HASH_LENGTH,
                (PUCHAR)&digestSize,
                sizeof(digestSize),
                &resultSize,
                0);

            _Analysis_assume_(status == STATUS_SUCCESS || FAILED(HRESULT_FROM_NT(status)));
            //IfFalseReturnError(status == STATUS_SUCCESS, HRESULT_FROM_NT(status));

            if (sizeof(this->quickDigestBuffer) >= digestSize)
            {
                digestPtr = this->quickDigestBuffer;
            }
            else
            {
                RETURN_IF_FAILED(this->digestBuffer.SetLength(digestSize));
                digestPtr = this->digestBuffer.GetBufferPtr();
            }

            status = BcryptLibrary::BCryptFinishHash(
                this->hashHandle,
                digestPtr,
                digestSize,
                0);

            _Analysis_assume_(status == STATUS_SUCCESS || FAILED(HRESULT_FROM_NT(status)));
            //IfFalseReturnError(status == STATUS_SUCCESS, HRESULT_FROM_NT(status));

            this->digest.bytes = digestPtr;
            this->digest.length = digestSize;
        }

        assert(done == this->state);

        *digest = this->digest;

        return S_OK;
    }
}
