#include <windows.h>
#include <wincrypt.h>
#include <wintrust.h>
#include <bcrypt.h>
#include <winternl.h>
#include <strsafe.h>
#include <softpub.h>
#include "AppxSignature.hpp"
#include "FileStream.hpp"
#include "SignatureValidator.hpp"

namespace xPlat
{
    struct unique_local_alloc_deleter {
        void operator()(HLOCAL h) const { LocalFree(h); };
    };
    
    struct unique_cert_context_deleter {
        void operator()(PCCERT_CONTEXT p) const { CertFreeCertificateContext(p); };
    };

    struct unique_cert_chain_deleter {
        void operator()(PCCERT_CHAIN_CONTEXT p) const { CertFreeCertificateChain(p); };
    };
    struct unique_hash_handle_deleter {
        void operator()(BCRYPT_HASH_HANDLE h) const { BCryptDestroyHash(h); };
    };

    struct unique_alg_handle_deleter {
        void operator()(BCRYPT_ALG_HANDLE h) const { BCryptCloseAlgorithmProvider(h, 0); };
    };

    struct unique_cert_store_handle_deleter {
        void operator()(HCERTSTORE h) const { CertCloseStore(h, 0); };
    };

    struct unique_crypt_msg_handle_deleter {
        void operator()(HCRYPTMSG h) const { CryptMsgClose(h); };
    };
    
    typedef std::unique_ptr<void, unique_local_alloc_deleter> unique_local_alloc_handle;
    typedef std::unique_ptr<const CERT_CONTEXT, unique_cert_context_deleter> unique_cert_context;
    typedef std::unique_ptr<const CERT_CHAIN_CONTEXT, unique_cert_chain_deleter> unique_cert_chain_context;
    typedef std::unique_ptr<void, unique_hash_handle_deleter> unique_hash_handle;
    typedef std::unique_ptr<void, unique_alg_handle_deleter> unique_alg_handle;
    typedef std::unique_ptr<void, unique_cert_store_handle_deleter> unique_cert_store_handle;
    typedef std::unique_ptr<void, unique_crypt_msg_handle_deleter> unique_crypt_msg_handle;

 static PCCERT_CHAIN_CONTEXT GetCertChainContext(
        _In_ byte* signatureBuffer,
        _In_ ULONG cbSignatureBuffer)
    {
        // Get the cert content
        HCERTSTORE certStoreT;
        HCRYPTMSG signedMessageT;
        CRYPT_DATA_BLOB signatureBlob = { 0 };
        signatureBlob.cbData = cbSignatureBuffer;
        signatureBlob.pbData = signatureBuffer;
        ThrowErrorIfNot(Error::AppxSignatureInvalid, (
            CryptQueryObject(
                CERT_QUERY_OBJECT_BLOB,
                &signatureBlob,
                CERT_QUERY_CONTENT_FLAG_PKCS7_SIGNED,
                CERT_QUERY_FORMAT_FLAG_BINARY,
                0,      // Reserved parameter
                NULL,   // No encoding info needed
                NULL,
                NULL,
                &certStoreT,
                &signedMessageT,
                NULL)
            ), "CryptQueryObject failed.");
        unique_cert_store_handle certStore(certStoreT);
        unique_crypt_msg_handle signedMessage(signedMessageT);

        // Get the signer size and information from the signed data message
        // The properties of the signer info will be used to uniquely identify the signing certificate in the certificate store
        CMSG_SIGNER_INFO* signerInfo = NULL;
        DWORD signerInfoSize = 0;
        ThrowErrorIfNot(Error::AppxSignatureInvalid, (
            CryptMsgGetParam(signedMessage.get(), CMSG_SIGNER_INFO_PARAM, 0, NULL, &signerInfoSize)
            ), "CryptMsgGetParam failed");

        // Check that the signer info size is within reasonable bounds; under the max length of a string for the issuer field
        ThrowErrorIf(Error::AppxSignatureInvalid, 
            (signerInfoSize == 0 || signerInfoSize >= STRSAFE_MAX_CCH),
            "signer info size not within reasonable bounds");

        std::vector<byte> signerInfoBuffer(signerInfoSize);
        signerInfo = reinterpret_cast<CMSG_SIGNER_INFO*>(signerInfoBuffer.data());
        ThrowErrorIfNot(Error::AppxSignatureInvalid, (
            CryptMsgGetParam(signedMessage.get(), CMSG_SIGNER_INFO_PARAM, 0, signerInfo, &signerInfoSize)
            ), "CryptMsgGetParam failed");
        
        // Get the signing certificate from the certificate store based on the issuer and serial number of the signer info
        CERT_INFO certInfo;
        certInfo.Issuer = signerInfo->Issuer;
        certInfo.SerialNumber = signerInfo->SerialNumber;
        unique_cert_context signingCertContext(CertGetSubjectCertificateFromStore(
            certStore.get(),
            X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
            &certInfo));
        ThrowErrorIf(Error::AppxSignatureInvalid, (signingCertContext.get() == NULL), "failed to get signing cert context.");

        // Get the signing certificate chain context.  Do not connect online for URL 
        // retrievals.  Note that this check does not respect the lifetime signing 
        // EKU on the signing certificate.  
        CERT_CHAIN_PARA certChainParameters = { 0 };
        certChainParameters.cbSize = sizeof(CERT_CHAIN_PARA);
        certChainParameters.RequestedUsage.dwType = USAGE_MATCH_TYPE_AND;
        DWORD certChainFlags = CERT_CHAIN_CACHE_ONLY_URL_RETRIEVAL;
        PCCERT_CHAIN_CONTEXT certChainContext;
        ThrowErrorIfNot(Error::AppxSignatureInvalid, (
            CertGetCertificateChain(
                HCCE_LOCAL_MACHINE,
                signingCertContext.get(),
                NULL,   // Use the current system time for CRL validation
                certStore.get(),
                &certChainParameters,
                certChainFlags,
                NULL,   // Reserved parameter; must be NULL
                &certChainContext)
            ), "CertGetCertificateChain failed");

        return certChainContext;
    }

    static bool GetEnhancedKeyUsage(PCCERT_CONTEXT pCertContext, std::vector<std::string>& values)
    {                   
        //get OIDS from the extension or property

        DWORD cbExtensionUsage = 0;
        std::vector<byte> extensionUsage(0);
        CertGetEnhancedKeyUsage(pCertContext, CERT_FIND_EXT_ONLY_ENHKEY_USAGE_FLAG, NULL, &cbExtensionUsage);
        extensionUsage.resize(cbExtensionUsage);
        ThrowErrorIf(Error::AppxSignatureInvalid, (
            !CertGetEnhancedKeyUsage(
                pCertContext,
                CERT_FIND_EXT_ONLY_ENHKEY_USAGE_FLAG,
                reinterpret_cast<PCERT_ENHKEY_USAGE>(extensionUsage.data()),                
                &cbExtensionUsage) &&
            GetLastError() != CRYPT_E_NOT_FOUND
        ), "CertGetEnhacnedKeyUsage on extension usage failed.");

        if (extensionUsage.size() > 0)
        {
            PCERT_ENHKEY_USAGE pExtensionUsageT = reinterpret_cast<PCERT_ENHKEY_USAGE>(extensionUsage.data());
            for (DWORD i = 0; i < pExtensionUsageT->cUsageIdentifier; i++)
            {   values.push_back(std::move(std::string(pExtensionUsageT->rgpszUsageIdentifier[i])));
            }
            return (pExtensionUsageT->cUsageIdentifier > 0);
        }

        DWORD cbPropertyUsage = 0;
        std::vector<byte> propertyUsage(0);
        CertGetEnhancedKeyUsage(pCertContext, CERT_FIND_PROP_ONLY_ENHKEY_USAGE_FLAG, NULL, &cbPropertyUsage);
        propertyUsage.resize(cbPropertyUsage);
        ThrowErrorIf(Error::AppxSignatureInvalid, (            
            !CertGetEnhancedKeyUsage(
                pCertContext,
                CERT_FIND_PROP_ONLY_ENHKEY_USAGE_FLAG,
                reinterpret_cast<PCERT_ENHKEY_USAGE>(propertyUsage.data()),
                &cbPropertyUsage) &&
            GetLastError() != CRYPT_E_NOT_FOUND
        ), "CertGetEnhancedKeyUsage on property usage failed.");

        if (propertyUsage.size() > 0)
        {
            PCERT_ENHKEY_USAGE pPropertyUsageT = reinterpret_cast<PCERT_ENHKEY_USAGE>(propertyUsage.data());
            for (DWORD i = 0; i < pPropertyUsageT->cUsageIdentifier; i++)
            {   values.push_back(std::move(std::string(pPropertyUsageT->rgpszUsageIdentifier[i])));
            }
            return (pPropertyUsageT->cUsageIdentifier > 0);
        }

        return false;
    }

    static bool IsMicrosoftTrustedChain(_In_ PCCERT_CHAIN_CONTEXT certChainContext)
    {
        // Validate that the certificate chain is rooted in one of the well-known MS root certs
        CERT_CHAIN_POLICY_PARA policyParameters = { 0 };
        policyParameters.cbSize = sizeof(CERT_CHAIN_POLICY_PARA);
        CERT_CHAIN_POLICY_STATUS policyStatus = { 0 };
        policyStatus.cbSize = sizeof(CERT_CHAIN_POLICY_STATUS);
        policyParameters.dwFlags = MICROSOFT_ROOT_CERT_CHAIN_POLICY_CHECK_APPLICATION_ROOT_FLAG;

        ThrowErrorIfNot(Error::AppxSignatureInvalid, 
            CertVerifyCertificateChainPolicy(
                CERT_CHAIN_POLICY_MICROSOFT_ROOT,
                certChainContext,
                &policyParameters,
                &policyStatus),
            "CertVerifyCertificateChainPolicy failed");    
        
        return ERROR_SUCCESS == policyStatus.dwError;
    }

    static bool IsAuthenticodeTrustedChain(_In_ PCCERT_CHAIN_CONTEXT certChainContext)
    {
        CERT_CHAIN_POLICY_PARA policyParameters = { 0 };
        policyParameters.cbSize = sizeof(CERT_CHAIN_POLICY_PARA);
        CERT_CHAIN_POLICY_STATUS policyStatus = { 0 };
        policyStatus.cbSize = sizeof(CERT_CHAIN_POLICY_STATUS);

        //policyParameters.dwFlags = MICROSOFT_ROOT_CERT_CHAIN_POLICY_CHECK_APPLICATION_ROOT_FLAG;
        ThrowErrorIfNot(Error::AppxSignatureInvalid, 
            CertVerifyCertificateChainPolicy(
                CERT_CHAIN_POLICY_AUTHENTICODE,
                certChainContext,
                &policyParameters,
                &policyStatus),
            "CertVerifyCertificateChainPolicy failed");

        bool isAuthenticode = (ERROR_SUCCESS == policyStatus.dwError);

        policyParameters = { 0 };
        policyParameters.cbSize = sizeof(CERT_CHAIN_POLICY_PARA);            
        policyStatus = {0};
        policyStatus.cbSize = sizeof(CERT_CHAIN_POLICY_STATUS);
        ThrowErrorIfNot(Error::AppxSignatureInvalid, 
            CertVerifyCertificateChainPolicy(
                CERT_CHAIN_POLICY_BASE,
                certChainContext,
                &policyParameters,
                &policyStatus),
            "CertVerifyCertificateChainPolicy failed");
        
        bool chainsToTrustedRoot = (ERROR_SUCCESS == policyStatus.dwError);
        return isAuthenticode && chainsToTrustedRoot;
    }

    static bool IsCACert(_In_ PCCERT_CONTEXT pCertContext)
    {
        PCERT_EXTENSION certExtension = CertFindExtension(
            szOID_BASIC_CONSTRAINTS2,
            pCertContext->pCertInfo->cExtension,
            pCertContext->pCertInfo->rgExtension);

        CERT_BASIC_CONSTRAINTS2_INFO *basicConstraintsT = NULL;
        DWORD cbDecoded = 0;            
        if (certExtension && CryptDecodeObjectEx(
                X509_ASN_ENCODING,
                X509_BASIC_CONSTRAINTS2,
                certExtension->Value.pbData,
                certExtension->Value.cbData,
                CRYPT_DECODE_ALLOC_FLAG,
                NULL/*pDecodePara*/,
                (LPVOID*)&basicConstraintsT,
                &cbDecoded))
        {
            unique_local_alloc_handle basicConstraints(basicConstraintsT);
            return basicConstraintsT->fCA ? true : false;
        }
        return false;
    }

    static bool IsCertificateSelfSigned(PCCERT_CONTEXT pContext, DWORD dwEncoding, DWORD dwFlags)
    {
        ThrowErrorIf(Error::InvalidParameter, (!(pContext) || (dwFlags != 0)), "Invalid args");
        if (!(CertCompareCertificateName(dwEncoding,
            &pContext->pCertInfo->Issuer,
            &pContext->pCertInfo->Subject)))
        {   return(false);
        }

        DWORD dwFlag = CERT_STORE_SIGNATURE_FLAG;
        if (!(CertVerifySubjectCertificateContext(pContext, pContext, &dwFlag)) ||
            (dwFlag & CERT_STORE_SIGNATURE_FLAG))
        {   return(false);
        }
        return(true);
    }

    static PCCERT_CONTEXT GetCertContext(BYTE *signatureBuffer, ULONG cbSignatureBuffer)
    {
        //get cert context from strCertificate;
        DWORD dwExpectedContentType = CERT_QUERY_CONTENT_FLAG_CERT |
            CERT_QUERY_CONTENT_FLAG_PKCS7_SIGNED |
            CERT_QUERY_CONTENT_FLAG_PKCS7_UNSIGNED;

        HCERTSTORE certStoreHandleT = NULL;
        DWORD dwContentType = 0;
        CERT_BLOB blob;
        blob.pbData = signatureBuffer;
        blob.cbData = cbSignatureBuffer;
        ThrowErrorIfNot(Error::AppxSignatureInvalid, (
            CryptQueryObject(
                CERT_QUERY_OBJECT_BLOB,
                &blob,
                dwExpectedContentType,
                CERT_QUERY_FORMAT_FLAG_ALL,
                0,
                NULL,
                &dwContentType,
                NULL,
                &certStoreHandleT,
                NULL,
                NULL)
            ),"CryptQueryObject failed");
        unique_cert_store_handle certStoreHandle(certStoreHandleT);

        PCCERT_CONTEXT pCertContext = NULL;
        if (dwContentType == CERT_QUERY_CONTENT_CERT)
        {   //get the certificate context
            pCertContext = CertEnumCertificatesInStore(certStoreHandle.get(), NULL);
        }
        else 
        {   //pkcs7 -- get the end entity
            while (NULL != (pCertContext = CertEnumCertificatesInStore(certStoreHandle.get(), pCertContext)))
            {   if (IsCertificateSelfSigned(pCertContext, pCertContext->dwCertEncodingType, 0) || IsCACert(pCertContext))
                { continue;
                }
                else
                {   //end entity cert
                    break;
                }
            }
        }
        return pCertContext;
    }
    
    static bool DoesSignatureCertContainStoreEKU(_In_ byte* rawSignatureBuffer, _In_ ULONG dataSize)
    {
        unique_cert_context certificateContext(GetCertContext(rawSignatureBuffer, dataSize));        
        std::vector<std::string> oids;
        if (GetEnhancedKeyUsage(certificateContext.get(), oids))
        {   std::size_t count = oids.size();
            for (std::size_t i = 0; i < count; i++)
            {   if (0 == oids.at(i).compare(OID::WindowsStore))
                {   return true;
                }
            }
        }
        return false;
    }

    // Best effort to determine whether the signature file is associated with a store cert
    static bool IsStoreOrigin(byte* signatureBuffer, ULONG cbSignatureBuffer)
    {
        if (DoesSignatureCertContainStoreEKU(signatureBuffer, cbSignatureBuffer))
        {   unique_cert_chain_context certChainContext(GetCertChainContext(signatureBuffer, cbSignatureBuffer));
            return IsMicrosoftTrustedChain(certChainContext.get());
        }
        return false;
    }

    // Best effort to determine whether the signature file is associated with a store cert
    static bool IsAuthenticodeOrigin(byte* signatureBuffer, ULONG cbSignatureBuffer)
    {   unique_cert_chain_context certChainContext(GetCertChainContext(signatureBuffer, cbSignatureBuffer));
        return IsAuthenticodeTrustedChain(certChainContext.get());
    }

    static bool GetPublisherName(/*in*/byte* signatureBuffer, /*in*/ ULONG cbSignatureBuffer, /*inout*/ std::string& publisher)
    {
        unique_cert_context certificateContext(GetCertContext(signatureBuffer, cbSignatureBuffer));
        
        int requiredLength = CertNameToStrA(
            X509_ASN_ENCODING,
            &certificateContext.get()->pCertInfo->Subject,
            CERT_X500_NAME_STR,
            nullptr,
            0);

        std::vector<char> publisherT;
        publisherT.reserve(requiredLength + 1);
        
        if (CertNameToStrA(
            X509_ASN_ENCODING,
            &certificateContext.get()->pCertInfo->Subject,
            CERT_X500_NAME_STR,
            publisherT.data(),
            requiredLength) > 0)
        {
            publisher = std::string(publisherT.data());
            return true;
        }
        return false;
    }


    bool SignatureValidator::Validate(
        /*in*/ APPX_VALIDATION_OPTION option,
        /*in*/ IStream *stream,
        /*inout*/ std::map<xPlat::AppxSignatureObject::DigestName, xPlat::AppxSignatureObject::Digest>& digests,
        /*inout*/ SignatureOrigin& origin,
        /*inout*/ std::string& publisher)
    {
        // If the caller wants to skip signature validation altogether, just bug out early. We will not read the digests
        if (option & APPX_VALIDATION_OPTION_SKIPSIGNATURE) { return false; }
        
        ThrowErrorIfNot(Error::AppxMissingSignatureP7X, (stream), "AppxSignature.p7x missing");
        
        LARGE_INTEGER li = {0};
        ULARGE_INTEGER uli = {0};
        ThrowHrIfFailed(stream->Seek(li, StreamBase::Reference::END, &uli));
        ThrowErrorIf(Error::AppxSignatureInvalid, (uli.QuadPart <= sizeof(P7X_FILE_ID) || uli.QuadPart > (2 << 20)), "stream is too big");

        std::vector<std::uint8_t> p7x(uli.LowPart);
        ThrowHrIfFailed(stream->Seek(li, StreamBase::Reference::START, &uli));

        ULONG actualRead = 0;
        ThrowHrIfFailed(stream->Read(p7x.data(), p7x.size(), &actualRead));
        ThrowErrorIf(Error::AppxSignatureInvalid,
            ((actualRead != p7x.size() || (*(DWORD*)p7x.data() != P7X_FILE_ID))),
            "Failed to read p7x or p7x header mismatch");

        BYTE *p7s = p7x.data() + sizeof(DWORD);
        std::uint32_t p7sSize = p7x.size() - sizeof(DWORD);

        // Decode the ASN.1 structure
        CRYPT_CONTENT_INFO* contentInfo = nullptr;
        DWORD contentInfoSize = 0;
        ThrowErrorIfNot(Error::AppxSignatureInvalid, (
            CryptDecodeObjectEx(
                X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                PKCS_CONTENT_INFO,
                p7s,
                p7sSize,
                CRYPT_DECODE_ALLOC_FLAG,
                nullptr,
                &contentInfo,
                &contentInfoSize)
            ),"CryptDecodeObjectEx failed.");

        HCRYPTMSG cryptMsgT = CryptMsgOpenToDecode(
            X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
            0,
            CMSG_SIGNED,
            NULL,
            nullptr,
            nullptr);
        ThrowErrorIf(Error::AppxSignatureInvalid, (cryptMsgT == nullptr), "CryptMsgOpenToDecode failed");
        unique_crypt_msg_handle cryptMsg(cryptMsgT);

        // Get the crypographic message 
        ThrowErrorIfNot(Error::AppxSignatureInvalid, (
            CryptMsgUpdate(
                cryptMsg.get(),
                contentInfo->Content.pbData,
                contentInfo->Content.cbData,
                TRUE)
            ), "CryptMsgUpdate failed");

        // This first call to CryptMsgGetParam is expected to fail because we don't know
        // how big of a buffer that it needs to store the inner content 
        DWORD innerContentTypeSize = 0;        
        ULONG readBytes = 0;
        ThrowErrorIf(Error::AppxSignatureInvalid, (
            !CryptMsgGetParam(
                cryptMsg.get(),
                CMSG_INNER_CONTENT_TYPE_PARAM,
                0,
                nullptr,
                &innerContentTypeSize) &&
            HRESULT_FROM_WIN32(GetLastError()) != HRESULT_FROM_WIN32(ERROR_MORE_DATA)
        ), "CryptMsgGetParam failed");
        
        // Allocate a temporary buffer
        std::vector<std::uint8_t> innerContentType(innerContentTypeSize);
        ThrowErrorIfNot(Error::AppxSignatureInvalid, (
            CryptMsgGetParam(
                cryptMsg.get(),
                CMSG_INNER_CONTENT_TYPE_PARAM,
                0,
                innerContentType.data(),
                &innerContentTypeSize)
            ),"CryptMsgGetParam failed");

        size_t indirectDataObjIdLength = strlen(SPC_INDIRECT_DATA_OBJID);
        ThrowErrorIf(Error::AppxSignatureInvalid, (
            (innerContentTypeSize != indirectDataObjIdLength + 1) ||
            (strncmp((char*)innerContentType.data(), SPC_INDIRECT_DATA_OBJID, indirectDataObjIdLength + 1) != 0)
        ), "unexpected content type");

        DWORD innerContentSize = 0;
        ThrowErrorIfNot(Error::AppxSignatureInvalid, ( 
            CryptMsgGetParam(
                cryptMsg.get(),
                CMSG_CONTENT_PARAM,
                0,
                nullptr,
                &innerContentSize) &&
                HRESULT_FROM_WIN32(GetLastError()) != HRESULT_FROM_WIN32(ERROR_MORE_DATA)
            ), "CryptMsgGetParam failed");

        // Allocate a temporary buffer
        std::vector<std::uint8_t> innerContent(innerContentSize);
        ThrowErrorIfNot(Error::AppxSignatureInvalid, (
            CryptMsgGetParam(
                cryptMsg.get(),
                CMSG_CONTENT_PARAM,
                0,
                innerContent.data(),
                &innerContentSize)
        ), "CryptMsgGetParam failed");
        
        // Parse the ASN.1 to the the indirect data structure
        SPC_INDIRECT_DATA_CONTENT* indirectContent = NULL;
        DWORD indirectContentSize = 0;
        ThrowErrorIfNot(Error::AppxSignatureInvalid, (
            CryptDecodeObjectEx(
                X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                SPC_INDIRECT_DATA_CONTENT_STRUCT,
                innerContent.data(),
                innerContentSize,
                CRYPT_DECODE_ALLOC_FLAG,
                nullptr,
                &indirectContent,
                &indirectContentSize)
        ), "CryptDecodeObjectEx failed");

        DigestHeader *header = reinterpret_cast<DigestHeader*>(indirectContent->Digest.pbData);
        std::uint32_t numberOfHashes = (indirectContent->Digest.cbData - sizeof(DWORD)) / sizeof(DigestHash);
        std::uint32_t modHashes = (indirectContent->Digest.cbData - sizeof(DWORD)) % sizeof(DigestHash);
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

        origin = xPlat::SignatureOrigin::Unknown;
        if (IsStoreOrigin(p7s, p7sSize)) { origin = xPlat::SignatureOrigin::Store; }
        else if (IsAuthenticodeOrigin(p7s, p7sSize)) { origin = xPlat::SignatureOrigin::LOB; }

        bool SignatureOriginUnknownAllowed = (option & APPX_VALIDATION_OPTION_ALLOWSIGNATUREORIGINUNKNOWN) == APPX_VALIDATION_OPTION_ALLOWSIGNATUREORIGINUNKNOWN;
        ThrowErrorIf(Error::AppxCertNotTrusted, 
            ((xPlat::SignatureOrigin::Unknown == origin) && !SignatureOriginUnknownAllowed),
            "Unknown signature origin");

        ThrowErrorIfNot(Error::AppxSignatureInvalid,
            GetPublisherName(p7s, p7sSize, publisher) == true,
            "Could not retrieve publisher name");
                
        return true;
    }
} // namespace xPlat