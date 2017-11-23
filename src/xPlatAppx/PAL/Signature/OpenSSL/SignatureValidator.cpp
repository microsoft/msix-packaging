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
#include <openssl/x509.h>
#include <openssl/pkcs7.h>
#include <openssl/pem.h>

namespace xPlat
{
    struct unique_BIO_deleter {
        void operator()(BIO *b) const { BIO_free(b); };
    };
    
    struct unique_PKCS7_deleter {
        void operator()(PKCS7 *p) const { PKCS7_free(p); };
    };

    struct unique_X509_STORE_deleter {
        void operator()(X509_STORE *xs) const { X509_STORE_free(xs); };
    };

    struct unique_X509_STORE_CTX_deleter {
        void operator()(X509_STORE_CTX *xsc) const { X509_STORE_CTX_cleanup(xsc); };
    };

    typedef std::unique_ptr<BIO, unique_BIO_deleter> unique_BIO;
    typedef std::unique_ptr<PKCS7, unique_PKCS7_deleter> unique_PKCS7;
    typedef std::unique_ptr<X509_STORE, unique_X509_STORE_deleter> unique_X509_STORE;
    typedef std::unique_ptr<X509_STORE_CTX, unique_X509_STORE_CTX_deleter> unique_X509_STORE_CTX;
    

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

        std::map<std::string, std::string>::iterator it;
        for ( it = appxCerts.begin(); it != appxCerts.end(); it++ )
        {
            std::vector<std::uint8_t> cert;
            ConvertBase64Certificate(it->second, cert);

            std::cout << it->first  // string (key)
              << ':'
              << it->second   // string's value 
              << std::endl ;
        }

        // TODO: read digests
        unique_X509_STORE store(X509_STORE_new());
        STACK_OF(X509) *other = nullptr;
        STACK_OF(X509) *crls = nullptr;
        STACK_OF(X509) *certs = nullptr;
        unique_BIO in(BIO_new_file("/Users/admin/Documents/temp.p7s", "r"));
        unique_BIO indata;
        unique_BIO out;
        int flags = PKCS7_DETACHED;
        
        unique_PKCS7 p7(d2i_PKCS7_bio(in.get(), nullptr));
        
        //PKCS7_verify(p7, other, store, indata, out, flags);
        STACK_OF(X509) *signers = PKCS7_get0_signers(p7.get(), other, flags);

#ifdef DISABLED
        BIO* sig_BIO = BIO_new_mem_buf(sig, sig_length)
        PKCS7* sig_pkcs7 = d2i_PKCS7_bio(sig_BIO, NULL);

        BIO* data_BIO = BIO_new_mem_buf(data, data_length)
        BIO* data_pkcs7_BIO = PKCS7_dataInit(sig_pkcs7, data_BIO);

        // Goto this place in the BIO. Why? No idea!
        char unneeded[1024*4];
        while (BIO_read(dataPKCS7_BIO, unneeded, sizeof(buffer)) > 0);

        int result;
        X509_STORE *certificateStore = X509_STORE_new();
        X509_STORE_CTX certificateContext;
        STACK_OF(PKCS7_SIGNER_INFO) *signerStack = PKCS7_get_signer_info(sig_pkcs7);
        int numSignerInfo = sk_PKCS7_SIGNER_INFO_num(signerStack);
        for (int i=0; i<numSignerInfo; ++i) {
            PKCS7_SIGNER_INFO *signerInfo = sk_PKCS7_SIGNER_INFO_value(signerStack, i);
            result = PKCS7_dataVerify(certificateStore, &certificateContext, data_pkcs7_BIO, sig_pkcs7, signerInfo);
        }

        X509_STORE_CTX_cleanup(&certificateContext);
        BIO_free(sig_BIO);
        BIO_free(data_BIO);
        BIO_free(data_pkcs7_BIO);
        PKCS7_free(sig_pkcs7);
        X509_STORE_free(certificateStore);
#endif

        ThrowErrorIfNot(Error::AppxSignatureInvalid, (
            IsStoreOrigin(buffer.data(), buffer.size()) ||
            IsAuthenticodeOrigin(buffer.data(), buffer.size()) ||
            (option & APPX_VALIDATION_OPTION::APPX_VALIDATION_OPTION_ALLOWSIGNATUREORIGINUNKNOWN)
        ), "Signature origin check failed");
        return true;
    }

} // namespace xPlat