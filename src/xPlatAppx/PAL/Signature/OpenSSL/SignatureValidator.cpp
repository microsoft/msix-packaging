#include "AppxSignature.hpp"
#include "Exceptions.hpp"
#include "FileStream.hpp"
#include "SignatureValidator.hpp"
#include "AppxCerts.hpp"

#include <string>
#include <sstream>
#include <iostream>

#include <openssl/err.h>
#include <openssl/objects.h>
#include <openssl/evp.h>
#include <openssl/x509v3.h>
#include <openssl/pkcs7.h>
#include <openssl/pem.h>
#include <openssl/crypto.h>

namespace xPlat
{
    const char* SPC_INDIRECT_DATA_OBJID = {"1.3.6.1.4.1.311.2.1.4"};

    struct unique_BIO_deleter {
        void operator()(BIO *b) const { if (b) BIO_free(b); };
    };
    
    struct unique_PKCS7_deleter {
        void operator()(PKCS7 *p) const { if (p) PKCS7_free(p); };
    };

    struct unique_X509_deleter {
        void operator()(X509 *x) const { if (x) X509_free(x); };
    };
    
    struct unique_X509_STORE_deleter {
        void operator()(X509_STORE *xs) const { if (xs) X509_STORE_free(xs); };
    };

    struct unique_X509_STORE_CTX_deleter {
        void operator()(X509_STORE_CTX *xsc) const { if (xsc) {X509_STORE_CTX_cleanup(xsc); X509_STORE_CTX_free(xsc);} };
    };

    struct unique_X509_NAME_deleter {
        void operator()(X509_NAME *xn) const { if (xn) OPENSSL_free(xn); };
    };

    struct unique_STACK_X509_deleter {
        void operator()(STACK_OF(X509) *sx) const { if (sx) sk_X509_free(sx); };
    };

    struct shared_BIO_deleter {
        void operator()(BIO *b) const { if (b) BIO_free(b); };
    };

    typedef std::unique_ptr<BIO, unique_BIO_deleter> unique_BIO;
    typedef std::unique_ptr<PKCS7, unique_PKCS7_deleter> unique_PKCS7;
    typedef std::unique_ptr<X509, unique_X509_deleter> unique_X509;
    typedef std::unique_ptr<X509_STORE, unique_X509_STORE_deleter> unique_X509_STORE;
    typedef std::unique_ptr<X509_STORE_CTX, unique_X509_STORE_CTX_deleter> unique_X509_STORE_CTX;
    typedef std::unique_ptr<X509_NAME, unique_X509_NAME_deleter> unique_X509_NAME;
    typedef std::unique_ptr<STACK_OF(X509), unique_STACK_X509_deleter> unique_STACK_X509;

    typedef struct Asn1Sequence
    {
        std::uint8_t tag;
        std::uint8_t encoding;
        union 
        {   
            struct {
                std::uint8_t length;
                std::uint8_t content;
            } rle8;
            struct {
                std::uint8_t lengthHigh;
                std::uint8_t lengthLow;
                std::uint8_t content;
            } rle16;
            std::uint8_t content;
        };
    } Asn1Sequence;

    // Best effort to determine whether the signature file is associated with a store cert
    static bool IsStoreOrigin(std::uint8_t* signatureBuffer, std::uint32_t cbSignatureBuffer)
    {
        unique_BIO bmem(BIO_new_mem_buf(signatureBuffer, cbSignatureBuffer));
        unique_PKCS7 p7(d2i_PKCS7_bio(bmem.get(), nullptr));

        STACK_OF(X509) *certStack = p7.get()->d.sign->cert;
        for (int i = 0; i < sk_X509_num(certStack); i++)
        {
            X509* cert = sk_X509_value(certStack, i);
            STACK_OF(X509_EXTENSION) *exts = cert->cert_info->extensions;
            for (int i = 0; i < sk_X509_EXTENSION_num(exts); i++) 
            {
                X509_EXTENSION *ext = sk_X509_EXTENSION_value(exts, i);
                if (ext)
                {
                    if (X509_EXTENSION_get_object(ext))
                    {
                        unique_BIO extbio(BIO_new(BIO_s_mem()));
                        if (!X509V3_EXT_print(extbio.get(), ext, 0, 0)) 
                        {
                            M_ASN1_OCTET_STRING_print(extbio.get(), ext->value);
                        }
                        BUF_MEM *bptr = nullptr;
                        BIO_get_mem_ptr(extbio.get(), &bptr);
                        
                        if (bptr && bptr->data && 
                            std::string((char*)bptr->data).find(std::string(OID::WindowsStore)) != std::string::npos)
                        {
                            return true;
                        }
                    }
                }
            }
        }
        return false;
    }

    // Best effort to determine whether the signature file is associated with a store cert
    static bool IsAuthenticodeOrigin(std::uint8_t* signatureBuffer, std::uint32_t cbSignatureBuffer)
    {
        bool retValue = false;
        return retValue;
    }

    bool ReadDigestHashes(/*[in]*/ PKCS7* p7, 
        /*[inout]*/ std::map<xPlat::AppxSignatureObject::DigestName, xPlat::AppxSignatureObject::Digest>& digests,
        /*[inout]*/ unique_BIO& signatureDigest)
    {
        ThrowErrorIf(Error::AppxSignatureInvalid,
            !(p7 && 
            PKCS7_type_is_signed(p7) && 
            p7->d.sign && 
            p7->d.sign->contents && 
            p7->d.sign->contents->d.other &&
            p7->d.sign->contents->d.other->type == V_ASN1_SEQUENCE &&
            p7->d.sign->contents->d.other->value.asn1_string &&
            p7->d.sign->contents->d.other->value.asn1_string->data &&
            p7->d.sign->contents->d.other->value.asn1_string->length > sizeof(DigestHash)),
            "Signature origin check failed");

        Asn1Sequence *asn1Sequence = reinterpret_cast<Asn1Sequence*>(p7->d.sign->contents->d.other->value.asn1_string->data);
        std::uint8_t* spcIndirectDataContent = nullptr;
        std::uint16_t spcIndirectDataContentSize = 0;
       
        if ((asn1Sequence->encoding & 0x80) == 0)
        {
            spcIndirectDataContent = &asn1Sequence->content;
            spcIndirectDataContentSize = (asn1Sequence->encoding & 0x7F);          
        }
        else
        if ((asn1Sequence->encoding & 0x81) == 0x81) 
        {
            spcIndirectDataContent = &asn1Sequence->rle8.content;
            spcIndirectDataContentSize = static_cast<std::uint16_t>(asn1Sequence->rle8.length);
        }
        else
        if ((asn1Sequence->encoding & 0x82) == 0x82)
        {
            spcIndirectDataContent = &asn1Sequence->rle16.content;
            spcIndirectDataContentSize = (asn1Sequence->rle16.lengthHigh << 8) + (asn1Sequence->rle16.lengthLow);
        }

        ThrowErrorIf(Error::AppxSignatureInvalid,
            (spcIndirectDataContent == nullptr || spcIndirectDataContentSize == 0),
            "bad signature data");

        std::uint8_t* spcIndirectDataContentEnd = spcIndirectDataContent + spcIndirectDataContentSize - sizeof(std::uint32_t);

        // Create a BIO structure from the spcIndirectDataContent -- it will be needed when we verify the entire signature via PKCS7_verify.
        unique_BIO bioMem(BIO_new_mem_buf(spcIndirectDataContent, spcIndirectDataContentSize));
        signatureDigest.swap(bioMem);
        
        // Scan through the spcIndirectData for the APPX header
        bool found = false;
        while (spcIndirectDataContent < spcIndirectDataContentEnd && !found)
        {
            if (*(xPlat::AppxSignatureObject::DigestName *)spcIndirectDataContent == xPlat::AppxSignatureObject::DigestName::HEAD)
            {
                found = true;
                break;
            }
            spcIndirectDataContent++;
            spcIndirectDataContentSize--;
        }

        ThrowErrorIf(Error::AppxSignatureInvalid, (!found), "Could not find the digest hashes in the signature");

        // If we found the APPX header, validate the contents
        DigestHeader *header = reinterpret_cast<DigestHeader*>(spcIndirectDataContent);
        std::uint32_t numberOfHashes = 
            (spcIndirectDataContentSize - sizeof(xPlat::AppxSignatureObject::DigestName)) / sizeof(DigestHash);
        std::uint32_t modHashes = 
            (spcIndirectDataContentSize - sizeof(xPlat::AppxSignatureObject::DigestName)) % sizeof(DigestHash);
        ThrowErrorIf(Error::AppxSignatureInvalid, (
            (header->name != xPlat::AppxSignatureObject::DigestName::HEAD) &&
            (numberOfHashes != 4 && numberOfHashes != 5) &&
            (modHashes != 0)
        ), "bad signature data");

        for (unsigned i = 0; i < numberOfHashes; i++)
        {
            std::vector<std::uint8_t> hash(HASH_BYTES);
            switch (header->hash[i].name)
            {
                case xPlat::AppxSignatureObject::DigestName::AXPC:
                case xPlat::AppxSignatureObject::DigestName::AXCT:
                case xPlat::AppxSignatureObject::DigestName::AXBM:
                case xPlat::AppxSignatureObject::DigestName::AXCI:
                case xPlat::AppxSignatureObject::DigestName::AXCD:
                    hash.assign(&header->hash[i].content[0], &header->hash[i].content[HASH_BYTES]);
                    digests.emplace(header->hash[i].name, hash);
                    break;

                default:
                    throw xPlat::Exception(xPlat::Error::AppxSignatureInvalid);
            }
        }

        ThrowErrorIf(Error::AppxSignatureInvalid,
            (digests.size() != 4 && digests.size() != 5),
            "Digest hashes missing entries");

        return true;
	}
	
    // This callback will be invoked during certificate verification
    int VerifyCallback(int ok, X509_STORE_CTX *ctx)
    {
        // If we encounter an expired cert error (which is fine) or a critical extension (most MS
        // certs contain MS-specific extensions that OpenSSL doesn't know how to evaluate), 
        // just return success
        if (!ok && (ctx->error == X509_V_ERR_CERT_HAS_EXPIRED || 
                    ctx->error == X509_V_ERR_UNHANDLED_CRITICAL_EXTENSION))
        {
            ok = static_cast<int>(true);
        }
        return ok; 
    }

    bool SignatureValidator::Validate(
        /*in*/ APPX_VALIDATION_OPTION option, 
        /*in*/ IStream *stream, 
        /*inout*/ std::map<xPlat::AppxSignatureObject::DigestName, xPlat::AppxSignatureObject::Digest>& digests,
        /*inout*/ SignatureOrigin& origin)
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

        std::uint32_t p7sSize = end.u.LowPart - sizeof(fileID);
        std::vector<std::uint8_t> p7s(p7sSize);
        ULONG actualRead = 0;
        ThrowHrIfFailed(stream->Read(p7s.data(), p7s.size(), &actualRead));
        ThrowErrorIf(Error::AppxSignatureInvalid, (actualRead != p7s.size()), "read error");

        // Load the p7s into a BIO buffer
        unique_BIO bmem(BIO_new_mem_buf(p7s.data(), p7s.size()));
        // Initialize the PKCS7 object from the BIO buffer
        unique_PKCS7 p7(d2i_PKCS7_bio(bmem.get(), nullptr));

        // Tell OpenSSL to use all available algorithms when evaluating certs
        OpenSSL_add_all_algorithms();

        // Create a trusted cert store
        unique_X509_STORE store(X509_STORE_new());
        // Set a verify callback to evaluate errors
        X509_STORE_set_verify_cb(store.get(), &VerifyCallback);
        // We have to tell OpenSSL why we are using the store -- in this case, closest is ANY.
        X509_STORE_set_purpose(store.get(), X509_PURPOSE_ANY);
        
        // Loop through our trusted PEM certs, create X509 objects from them, and add to trusted store
        unique_STACK_X509 trustedChain(sk_X509_new_null());
        for ( std::string s : appxCerts )
        {
            // Load the cert into memory
            unique_BIO bcert(BIO_new_mem_buf(s.data(), s.size()));

            // Create a cert from the memory buffer
            unique_X509 cert(PEM_read_bio_X509(bcert.get(), nullptr, nullptr, nullptr));
            
            // Add the cert to the trusted store
            ThrowErrorIfNot(Error::AppxSignatureInvalid, 
                X509_STORE_add_cert(store.get(), cert.get()) == 1, 
                "Could not add cert to keychain");
            
            sk_X509_push(trustedChain.get(), cert.get());
        }

        unique_BIO signatureDigest(nullptr);
        ReadDigestHashes(p7.get(), digests, signatureDigest);
        
        // Loop through the untrusted certs and verify them
        STACK_OF(X509) *untrustedCerts = p7.get()->d.sign->cert;
        for (int i = 0; i < sk_X509_num(untrustedCerts); i++)
        {
            X509* cert = sk_X509_value(untrustedCerts, i);
            unique_X509_STORE_CTX context(X509_STORE_CTX_new());
            X509_STORE_CTX_init(context.get(), store.get(), nullptr, nullptr);

            X509_STORE_CTX_set_chain(context.get(), untrustedCerts);
            X509_STORE_CTX_trusted_stack(context.get(), trustedChain.get());
            X509_STORE_CTX_set_cert(context.get(), cert);

            X509_VERIFY_PARAM* param = X509_STORE_CTX_get0_param(context.get());
            X509_VERIFY_PARAM_set_flags(param, 
                X509_V_FLAG_CB_ISSUER_CHECK | X509_V_FLAG_TRUSTED_FIRST | X509_V_FLAG_IGNORE_CRITICAL);

            // This function prints the contents of a certificate
            //X509_print_fp(stdout, cert);

            ThrowErrorIfNot(Error::AppxSignatureInvalid, 
                X509_verify_cert(context.get()) == 1, 
                "Could not verify cert");
        }

        ThrowErrorIfNot(Error::AppxSignatureInvalid, 
            PKCS7_verify(p7.get(), trustedChain.get(), store.get(), signatureDigest.get(), nullptr/*out*/, PKCS7_NOCRL/*flags*/) == 1, 
            "Could not verify package signature");

        origin = xPlat::SignatureOrigin::Unknown;
        if (IsStoreOrigin(p7s.data(), p7s.size())) { origin = xPlat::SignatureOrigin::Store; }
        else if (IsAuthenticodeOrigin(p7s.data(), p7s.size())) { origin = xPlat::SignatureOrigin::LOB; }

        ThrowErrorIfNot(Error::AppxSignatureInvalid, (
            xPlat::SignatureOrigin::Store == origin ||
            xPlat::SignatureOrigin::LOB == origin ||
            (option & APPX_VALIDATION_OPTION::APPX_VALIDATION_OPTION_ALLOWSIGNATUREORIGINUNKNOWN)
        ), "Signature origin check failed");
        return true;
    }
} // namespace xPlat