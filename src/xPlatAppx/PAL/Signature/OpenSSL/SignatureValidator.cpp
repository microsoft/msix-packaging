#include "AppxSignature.hpp"
#include "Exceptions.hpp"
#include "FileStream.hpp"
#include "SignatureValidator.hpp"
#include "AppxCerts.hpp"

#include <regex>

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

    static void ConvertBase64Certificate(std::string base64Cert, std::vector<std::uint8_t>& )
    {        
        std::string result;
        try
        {
            std::regex r("^(?!-----BEGIN CERTIFICATE-----|-----END CERTIFICATE-----)([a-zA-Z0-9+/]+)$");
            std::smatch match;
            if (std::regex_search(base64Cert, match, r) && match.size() > 1) 
            {
                result = match.str(1);
            }
        } 
        catch (std::regex_error& e) 
        {
            // Syntax error in the regular expression
            std::cout << "syntax error";
        }

#ifdef DISABLED
        regex_match(input,integer);

        BIO *b64 = BIO_new(BIO_f_base64());
        BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
        BIO *mem = BIO_new(BIO_f_buffer());
        BIO_push(b64, mem);
        BIO_push(mem, file);

        // write data
        bool done = false;
        int res = 0;
        while (!done)
        {
            res = ;

            if(BIO_write(b64, input, leni) <= 0 && BIO_should_retry(b64)){
                    continue;
                }
                else // encoding failed
                {
                    /* Handle Error!!! */
                }
            }
            else // success!
                done = true;
        }

        BIO_flush(b64);
        BIO_pop(b64);
        BIO_free_all(b64);
        return 0;
#endif
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