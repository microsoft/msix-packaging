//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once
#include "Exceptions.hpp"

#include <memory>

#include <openssl/err.h>
#include <openssl/bio.h>
#include <openssl/objects.h>
#include <openssl/evp.h>
#include <openssl/x509v3.h>
#include <openssl/pkcs7.h>
#include <openssl/pkcs12.h>
#include <openssl/pem.h>
#include <openssl/crypto.h>
#include <openssl/rsa.h>
#include <openssl/asn1.h>

namespace MSIX
{
    struct unique_BIO_deleter {
        void operator()(BIO* b) const { if (b) BIO_free(b); };
    };

    struct unique_PKCS7_deleter {
        void operator()(PKCS7* p) const { if (p) PKCS7_free(p); };
    };

    struct unique_PKCS12_deleter {
        void operator()(PKCS12* p) const { if (p) PKCS12_free(p); };
    };

    struct unique_X509_deleter {
        void operator()(X509* x) const { if (x) X509_free(x); };
    };

    struct unique_X509_STORE_deleter {
        void operator()(X509_STORE* xs) const { if (xs) X509_STORE_free(xs); };
    };

    struct unique_X509_STORE_CTX_deleter {
        void operator()(X509_STORE_CTX* xsc) const { if (xsc) { X509_STORE_CTX_cleanup(xsc); X509_STORE_CTX_free(xsc); } };
    };

    struct unique_OPENSSL_string_deleter {
        void operator()(char* os) const { if (os) OPENSSL_free(os); };
    };

    struct unique_STACK_X509_deleter {
        void operator()(STACK_OF(X509)* sx) const { if (sx) sk_X509_free(sx); };
    };

    struct unique_EVP_PKEY_deleter {
        void operator()(EVP_PKEY* pkey) const { if (pkey) EVP_PKEY_free(pkey); }
    };

    struct unique_ASN1_STRING_deleter {
        void operator()(ASN1_STRING* pStr) const { if (pStr) ASN1_STRING_free(pStr); }
    };

    struct shared_BIO_deleter {
        void operator()(BIO* b) const { if (b) BIO_free(b); };
    };

    using unique_BIO = std::unique_ptr<BIO, unique_BIO_deleter>;
    using unique_PKCS7 = std::unique_ptr<PKCS7, unique_PKCS7_deleter>;
    using unique_PKCS12 = std::unique_ptr<PKCS12, unique_PKCS12_deleter>;
    using unique_X509 = std::unique_ptr<X509, unique_X509_deleter>;
    using unique_X509_STORE = std::unique_ptr<X509_STORE, unique_X509_STORE_deleter>;
    using unique_X509_STORE_CTX = std::unique_ptr<X509_STORE_CTX, unique_X509_STORE_CTX_deleter>;
    using unique_OPENSSL_string = std::unique_ptr<char, unique_OPENSSL_string_deleter>;
    using unique_STACK_X509 = std::unique_ptr<STACK_OF(X509), unique_STACK_X509_deleter>;
    using unique_EVP_PKEY = std::unique_ptr<EVP_PKEY, unique_EVP_PKEY_deleter>;
    using unique_ASN1_STRING = std::unique_ptr<ASN1_STRING, unique_ASN1_STRING_deleter>;

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

    // A common exception class to be used by all OpenSSL errors.
    // OpenSSL does not return detailed error codes, so we use a singular HResult.
    class OpenSSLException final : public Exception
    {
    public:
        OpenSSLException(std::string& message, DWORD error) : Exception(message, error)
        {
        }
    };
    
    MSIX_NOINLINE(void) RaiseOpenSSLException(const char* message, const int line, const char* const file, DWORD error = 0x80FA11ED);

    // Use only to verify the result of *_new functions from OpenSSL
    template <typename T>
    inline void CheckOpenSSLAlloc(const T& t, const int line, const char* const file)
    {
        if (!t)
        {
            RaiseOpenSSLException("OpenSSL allocation failed", line, file, static_cast<DWORD>(Error::OutOfMemory));
        }
    }

    // Use to check the result of functions that actually do something beyond allocation.
    // The overloads allow for other return types, such as pointers.
    inline void CheckOpenSSLErr(int err, const int line, const char* const file, const char* message = nullptr)
    {
        if (err <= 0)
        {
            RaiseOpenSSLException(message, line, file);
        }
    }

    template <typename T>
    inline void CheckOpenSSLErr(T* returnVal, const int line, const char* const file, const char* message = nullptr)
    {
        if (!returnVal)
        {
            RaiseOpenSSLException(message, line, file);
        }
    }
} // namespace MSIX

#define ThrowOpenSSLError(m)                MSIX::RaiseOpenSSLException(m, __LINE__, __FILE__)
#define ThrowOpenSSLErrIfAllocFailed(x)     MSIX::CheckOpenSSLAlloc(x, __LINE__, __FILE__)
#define ThrowOpenSSLErrIfFailed(x)          MSIX::CheckOpenSSLErr(x, __LINE__, __FILE__)
#define ThrowOpenSSLErrIfFailedMsg(x,m)     MSIX::CheckOpenSSLErr(x, __LINE__, __FILE__, m)
