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
    struct unique_BIO_deleter {
        void operator()(BIO *b) const { BIO_free(b); };
    };
    
    struct unique_PKCS7_deleter {
        void operator()(PKCS7 *p) const { PKCS7_free(p); };
    };

    struct unique_X509_deleter {
        void operator()(X509 *x) const { X509_free(x); };
    };
    
    struct unique_X509_STORE_deleter {
        void operator()(X509_STORE *xs) const { X509_STORE_free(xs); };
    };

    struct unique_X509_STORE_CTX_deleter {
        void operator()(X509_STORE_CTX *xsc) const { X509_STORE_CTX_cleanup(xsc); X509_STORE_CTX_free(xsc); };
    };

    struct unique_X509_NAME_deleter {
        void operator()(X509_NAME *xn) const { OPENSSL_free(xn); };
    };

    typedef std::unique_ptr<BIO, unique_BIO_deleter> unique_BIO;
    typedef std::unique_ptr<PKCS7, unique_PKCS7_deleter> unique_PKCS7;
    typedef std::unique_ptr<X509, unique_X509_deleter> unique_X509;
    typedef std::unique_ptr<X509_STORE, unique_X509_STORE_deleter> unique_X509_STORE;
    typedef std::unique_ptr<X509_STORE_CTX, unique_X509_STORE_CTX_deleter> unique_X509_STORE_CTX;
    typedef std::unique_ptr<X509_NAME, unique_X509_NAME_deleter> unique_X509_NAME;
    

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

    static bool ConvertBase64Certificate(std::string base64CertWithDelimiters, std::vector<std::uint8_t>& decodedCert)
    {   
        std::istringstream stringStream(base64CertWithDelimiters);
        std::string base64Cert;     
        std::string line;
        while (std::getline(stringStream, line)) 
        {
            if (line.find("-----BEGIN CERTIFICATE-----") == std::string::npos &&
                line.find("-----END CERTIFICATE-----") == std::string::npos) {
                base64Cert += line; }
        }

        // Load a BIO filter with the base64Cert
        unique_BIO bsrc(BIO_new_mem_buf(base64Cert.data(), base64Cert.size()));

        // Put a Base64 decoder on the front of it
        unique_BIO b64(BIO_push(BIO_new(BIO_f_base64()), bsrc.get()));
        // Ignore new lines
	    BIO_set_flags(b64.get(), BIO_FLAGS_BASE64_NO_NL);

        // Calculate how big the decode buffer needs to be
        int length = base64Cert.size();
        int padding = 0;
        char *base64CertT = (char*)base64Cert.data();
        if (base64CertT[length-1] == '=')
            padding++;
        if (base64CertT[length-2] == '=')
            padding++;
        
        // Resize the decoder buffer to the calculated length
        decodedCert.resize(((length * 3)/4) - padding);
        
        // Read the Base64 certificate thru the Base64 decoder into decodedCert 
        ThrowErrorIf(Error::AppxSignatureInvalid, 
            (BIO_read(b64.get(), (void*)decodedCert.data(), decodedCert.size()) != length),
            "Certificate is invalid");

        return true;
    }

    int VerifyCallback(int ok, X509_STORE_CTX *ctx)
    {
        //X509_V_ERR_CERT_HAS_EXPIRED
        std::cout << ok << std::endl;
        return 1; 
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

        std::uint32_t p7sSize = end.u.LowPart - sizeof(fileID);
        std::vector<std::uint8_t> p7s(p7sSize);
        ULONG actualRead = 0;
        ThrowHrIfFailed(stream->Read(p7s.data(), p7s.size(), &actualRead));
        ThrowErrorIf(Error::AppxSignatureInvalid, (actualRead != p7s.size()), "read error");

#ifdef LINKER_ERROR
        // This causes a linker error -- might need to add more libs
        //OpenSSL_add_all_algorithms();
#endif

#ifdef DISABLED
        // This is just experimental logic for loading all of the certs from a single file.
        // I saw a report that adding the certs individually to a store didn't work BUT this would.
        // I gave it a try, and found that, in fact, it didn't work at all. Not ready to completely
        // jettison the logic.
        unique_X509_STORE store(X509_STORE_new());
        X509_STORE_load_locations(store.get(), "/Users/admin/Documents/foo.pem", nullptr);
#endif

        // Add all of the trusted certficates individually to our X.509 store
        unique_X509_STORE store(X509_STORE_new());
        std::map<std::string, std::string>::iterator it;
        for ( it = appxCerts.begin(); it != appxCerts.end(); it++ )
        {
            // Load the cert into memory
            unique_BIO bcert(BIO_new_mem_buf(it->second.data(), it->second.size()));
            // Create a cert from the memory buffer
            unique_X509 cert(PEM_read_bio_X509(bcert.get(), nullptr, nullptr, nullptr));
            
            ThrowErrorIfNot(Error::AppxSignatureInvalid, 
                X509_STORE_add_cert(store.get(), cert.get()) == 1, 
                "Could not add cert to keychain");
        }

        // TODO: read digests
        unique_BIO bmem(BIO_new_mem_buf(p7s.data(), p7s.size()));
        unique_PKCS7 p7(d2i_PKCS7_bio(bmem.get(), nullptr));

#ifdef DISABLED        
        STACK_OF(PKCS7_SIGNER_INFO) *signerStack = PKCS7_get_signer_info(p7.get());
        int numSignerInfo = sk_PKCS7_SIGNER_INFO_num(signerStack);
        for (int i = 0; i < numSignerInfo; i++) 
        {
            PKCS7_SIGNER_INFO *signerInfo = sk_PKCS7_SIGNER_INFO_value(signerStack, i);

            X509_STORE_CTX* certContextT = nullptr;
            //unique_X509_STORE_CTX certContext;
            //result = PKCS7_dataVerify(certStore.get(), &certContext, nullptr, nullptr, nullptr);
        }
        //PKCS7_verify(p7, other, store, indata, out, flags);
        //STACK_OF(X509) *certStack = PKCS7_get0_signers(p7.get(), nullptr, 0);
#endif

        unique_BIO content(BIO_new(BIO_s_mem()));
        //X509_VERIFY_PARAM* param = X509_STORE_get0_param(store.get());
        //X509_VERIFY_PARAM_set_flags(param, X509_V_FLAG_CB_ISSUER_CHECK);
        X509_STORE_set_verify_cb(store.get(), &VerifyCallback);
        X509_STORE_set_purpose(store.get(), X509_PURPOSE_ANY);
        
        ThrowErrorIfNot(Error::AppxSignatureInvalid, 
            PKCS7_verify(p7.get(), nullptr, store.get(), nullptr/*indata*/, nullptr/*out*/, PKCS7_NOCRL/*flags*/) == 1, 
            "Could not verify package signature");

        STACK_OF(X509) *certStack = p7.get()->d.sign->cert;
        for (int i = 0; i < sk_X509_num(certStack); i++)
        {
            unique_X509 cert(sk_X509_value(certStack, i));
            unique_X509_STORE_CTX context(X509_STORE_CTX_new());
            X509_STORE_CTX_init(context.get(), store.get(), cert.get(), nullptr);

            X509_verify_cert(context.get());

            ThrowErrorIfNot(Error::AppxSignatureInvalid, 
                X509_verify_cert(context.get()) == 1, 
                "Could not verify cert");
        }

        ThrowErrorIfNot(Error::AppxSignatureInvalid, (
            IsStoreOrigin(p7s.data(), p7s.size()) ||
            IsAuthenticodeOrigin(p7s.data(), p7s.size()) ||
            (option & APPX_VALIDATION_OPTION::APPX_VALIDATION_OPTION_ALLOWSIGNATUREORIGINUNKNOWN)
        ), "Signature origin check failed");
        return true;
    }

} // namespace xPlat