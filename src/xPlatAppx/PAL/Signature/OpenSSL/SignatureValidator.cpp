#include "AppxSignature.hpp"
#include "Exceptions.hpp"
#include "FileStream.hpp"
#include "SignatureValidator.hpp"

#include <openssl/err.h>
#include <openssl/objects.h>
#include <openssl/evp.h>
#include <openssl/x509.h>
#include <openssl/pkcs7.h>
#include <openssl/pem.h>

namespace xPlat
{

    // Best effort to determine whether the signature file is associated with a store cert
    static bool IsStoreOrigin(std::uint8_t* signatureBuffer, std::uint32_t cbSignatureBuffer)
    {
        bool retValue = false;
        return retValue;
    }

    // Best effort to determine whether the signature file is associated with a store cert
    static bool IsAuthenticodeOrigin(std::uint8_t* signatureBuffer, std::uint32_t cbSignatureBuffer)
    {
        bool retValue = false;
        return retValue;
    }

    bool SignatureValidator::Validate(
        /*in*/ APPX_VALIDATION_OPTION option, 
        /*in*/ IStream *stream, 
        /*inout*/ std::map<xPlat::AppxSignatureObject::DigestName, xPlat::AppxSignatureObject::Digest>& digests,
        /*inout*/ SignatureOrigin&)
    {
        // If the caller wants to skip signature validation altogether, just bug out early. We will not read the digests
        if (option & APPX_VALIDATION_OPTION_SKIPSIGNATURE) { return false; }

        LARGE_INTEGER start = {0};
        ULARGE_INTEGER end = {0};
        ThrowHrIfFailed(stream->Seek(start, StreamBase::Reference::END, &end));
        ThrowErrorIf(Error::AppxSignatureInvalid, (end.QuadPart <= sizeof(P7X_FILE_ID) || end.QuadPart > (2 << 20)), "stream is too big");
        ThrowHrIfFailed(stream->Seek(start, StreamBase::Reference::START, nullptr));

        std::uint32_t fileID = 0;
        ThrowHrIfFailed(stream->Read(&fileID, sizeof(fileID), nullptr));
        ThrowErrorIf(Error::AppxSignatureInvalid, (fileID != P7X_FILE_ID), "unexpected p7x header");

        std::uint32_t streamSize = end.u.LowPart - sizeof(fileID);
        std::vector<std::uint8_t> buffer(streamSize);
        ULONG actualRead = 0;
        ThrowHrIfFailed(stream->Read(buffer.data(), streamSize, &actualRead));
        ThrowErrorIf(Error::AppxSignatureInvalid, (actualRead != streamSize), "read error");

        // TODO: read digests
        X509_STORE *store = nullptr;
        STACK_OF(X509) *other = nullptr;
        STACK_OF(X509) *crls = nullptr;
        STACK_OF(X509) *certs = nullptr;
        int flags = PKCS7_DETACHED;
        BIO* in = BIO_new_file("/Users/admin/Documents/temp.p7s", "r");
        BIO* indata = nullptr;
        BIO* out = nullptr;
        
        PKCS7* p7 = d2i_PKCS7_bio(in, nullptr);
        
        PKCS7_verify(p7, other, store, indata, out, flags);
        STACK_OF(X509) *signers = PKCS7_get0_signers(p7, other, flags);


        ThrowErrorIfNot(Error::AppxSignatureInvalid, (
            IsStoreOrigin(buffer.data(), buffer.size()) ||
            IsAuthenticodeOrigin(buffer.data(), buffer.size()) ||
            (option & APPX_VALIDATION_OPTION::APPX_VALIDATION_OPTION_ALLOWSIGNATUREORIGINUNKNOWN)
        ), "Signature origin check failed");
        return true;
    }

} // namespace xPlat