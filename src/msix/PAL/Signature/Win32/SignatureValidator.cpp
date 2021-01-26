//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
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

namespace MSIX
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

    //+-------------------------------------------------------------------------
    //  SHA256 Key Identifier (only PublicKey) of the Microsoft application roots
    //--------------------------------------------------------------------------
    const BYTE MicrosoftApplicationRootList[HASH_BYTES] = {
        // The following is the SHA256 of PublicKey for the Microsoft Application
        // Root:
        //      CN=Microsoft Root Certificate Authority 2011
        //      O=Microsoft Corporation
        //      L=Redmond
        //      S=Washington
        //      C=US
        //
        //  NotBefore:: Tue Mar 22 15:05:28 2011
        //  NotAfter:: Sat Mar 22 15:13:04 2036
        0x4A, 0xBB, 0x05, 0x94, 0xD3, 0x03, 0xEF, 0x70, 0x77, 0x13,
        0x88, 0x34, 0xAB, 0x31, 0x5E, 0x94, 0x1E, 0x96, 0x30, 0x93,
        0xE0, 0x5B, 0x4B, 0x14, 0xAF, 0x5D, 0xCB, 0x52, 0x77, 0x12,
        0xC0, 0x0A
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
        ThrowErrorIfNot(Error::SignatureInvalid, (
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
        ThrowErrorIfNot(Error::SignatureInvalid, (
            CryptMsgGetParam(signedMessage.get(), CMSG_SIGNER_INFO_PARAM, 0, NULL, &signerInfoSize)
            ), "CryptMsgGetParam failed");

        // Check that the signer info size is within reasonable bounds; under the max length of a string for the issuer field
        ThrowErrorIf(Error::SignatureInvalid, 
            (signerInfoSize == 0 || signerInfoSize >= STRSAFE_MAX_CCH),
            "signer info size not within reasonable bounds");

        std::vector<byte> signerInfoBuffer(signerInfoSize);
        signerInfo = reinterpret_cast<CMSG_SIGNER_INFO*>(signerInfoBuffer.data());
        ThrowErrorIfNot(Error::SignatureInvalid, (
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
        ThrowErrorIf(Error::SignatureInvalid, (signingCertContext.get() == NULL), "failed to get signing cert context.");

        // Get the signing certificate chain context.  Do not connect online for URL 
        // retrievals.  Note that this check does not respect the lifetime signing 
        // EKU on the signing certificate.  
        CERT_CHAIN_PARA certChainParameters = { 0 };
        certChainParameters.cbSize = sizeof(CERT_CHAIN_PARA);
        certChainParameters.RequestedUsage.dwType = USAGE_MATCH_TYPE_AND;
        DWORD certChainFlags = CERT_CHAIN_CACHE_ONLY_URL_RETRIEVAL;
        PCCERT_CHAIN_CONTEXT certChainContext;
        ThrowErrorIfNot(Error::SignatureInvalid, (
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
		if (pCertContext == NULL) 
		{
			return false;
		}
        DWORD cbExtensionUsage = 0;
        std::vector<byte> extensionUsage(0);
        CertGetEnhancedKeyUsage(pCertContext, CERT_FIND_EXT_ONLY_ENHKEY_USAGE_FLAG, NULL, &cbExtensionUsage);
        if (cbExtensionUsage > 0)
        {
            extensionUsage.resize(cbExtensionUsage);
            ThrowErrorIf(Error::SignatureInvalid, (
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
                {
                    values.push_back(std::move(std::string(pExtensionUsageT->rgpszUsageIdentifier[i])));
                }
                return (pExtensionUsageT->cUsageIdentifier > 0);
            }
        }

        DWORD cbPropertyUsage = 0;
        std::vector<byte> propertyUsage(0);
        CertGetEnhancedKeyUsage(pCertContext, CERT_FIND_PROP_ONLY_ENHKEY_USAGE_FLAG, NULL, &cbPropertyUsage);
        if (cbPropertyUsage > 0)
        {
            propertyUsage.resize(cbPropertyUsage);
            ThrowErrorIf(Error::SignatureInvalid, (
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
                {
                    values.push_back(std::move(std::string(pPropertyUsageT->rgpszUsageIdentifier[i])));
                }
                return (pPropertyUsageT->cUsageIdentifier > 0);
            }
        }

        return false;
    }

    // Check that the top level certificate contains the public key for the Microsoft root on Win7.
    static bool IsMicrosoftTrustedChainForLegacySystems(PCCERT_CHAIN_CONTEXT certChainContext)
    {
        PCERT_SIMPLE_CHAIN chain = certChainContext->rgpChain[0];
        DWORD chainElement = chain->cElement;;
        PCCERT_CONTEXT cert = chain->rgpElement[chainElement - 1]->pCertContext;
        BYTE keyId[HASH_BYTES];
        DWORD keyIdLength = HASH_BYTES;

        ThrowErrorIfNot(Error::SignatureInvalid,
            (CryptHashCertificate2(
                BCRYPT_SHA256_ALGORITHM,
                0,                  // dwFlags
                nullptr,            // pvReserved
                cert->pCertInfo->SubjectPublicKeyInfo.PublicKey.pbData,
                cert->pCertInfo->SubjectPublicKeyInfo.PublicKey.cbData,
                keyId,
                &keyIdLength) || HASH_BYTES != keyIdLength),
            "CryptHashCertificate2 failed");
        return (0 == memcmp(MicrosoftApplicationRootList, keyId, HASH_BYTES));
    }

    static bool IsMicrosoftTrustedChain(_In_ PCCERT_CHAIN_CONTEXT certChainContext)
    {
        // Validate that the certificate chain is rooted in one of the well-known MS root certs
        CERT_CHAIN_POLICY_PARA policyParameters = { 0 };
        policyParameters.cbSize = sizeof(CERT_CHAIN_POLICY_PARA);
        CERT_CHAIN_POLICY_STATUS policyStatus = { 0 };
        policyStatus.cbSize = sizeof(CERT_CHAIN_POLICY_STATUS);
        policyParameters.dwFlags = MICROSOFT_ROOT_CERT_CHAIN_POLICY_CHECK_APPLICATION_ROOT_FLAG;

        ThrowErrorIfNot(Error::SignatureInvalid,
            CertVerifyCertificateChainPolicy(
                CERT_CHAIN_POLICY_MICROSOFT_ROOT,
                certChainContext,
                &policyParameters,
                &policyStatus),
            "CertVerifyCertificateChainPolicy failed");

        bool chainsToTrustedRoot = (policyStatus.dwError == ERROR_SUCCESS);
        if (!chainsToTrustedRoot && policyStatus.dwError == CERT_E_UNTRUSTEDROOT)    
        {   // CertVerifyCertificateChainPolicy fails with CERT_E_UNTRUSTEDROOT on Win7.
            chainsToTrustedRoot = IsMicrosoftTrustedChainForLegacySystems(certChainContext);
        }
        return chainsToTrustedRoot;
    }

    static bool IsAuthenticodeTrustedChain(_In_ PCCERT_CHAIN_CONTEXT certChainContext)
    {
        CERT_CHAIN_POLICY_PARA policyParameters = { 0 };
        policyParameters.cbSize = sizeof(CERT_CHAIN_POLICY_PARA);
        CERT_CHAIN_POLICY_STATUS policyStatus = { 0 };
        policyStatus.cbSize = sizeof(CERT_CHAIN_POLICY_STATUS);

        //policyParameters.dwFlags = MICROSOFT_ROOT_CERT_CHAIN_POLICY_CHECK_APPLICATION_ROOT_FLAG;
        ThrowErrorIfNot(Error::SignatureInvalid, 
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
        ThrowErrorIfNot(Error::SignatureInvalid, 
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

    static PCCERT_CONTEXT GetCertContext(BYTE *signatureBuffer, ULONG cbSignatureBuffer, bool allowSelfSignedCert)
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
        ThrowErrorIfNot(Error::SignatureInvalid, (
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

        if (dwContentType == CERT_QUERY_CONTENT_CERT)
        {   //get the certificate context
            return CertEnumCertificatesInStore(certStoreHandle.get(), NULL);
        }
        else 
        {   //pkcs7 -- get the end entity
            PCCERT_CONTEXT pCertContext = NULL;
            while (NULL != (pCertContext = CertEnumCertificatesInStore(certStoreHandle.get(), pCertContext)))
            {    
                if (IsCertificateSelfSigned(pCertContext, pCertContext->dwCertEncodingType, 0))
                {
                    if (allowSelfSignedCert)
                        return pCertContext;
                    continue;
                }
                if (IsCACert(pCertContext))
                {
                    continue;
                }
                else
                {   //end entity cert
                    return pCertContext;
                }
            }
            return NULL;
        }
    }
    
    static bool DoesSignatureCertContainStoreEKU(_In_ byte* rawSignatureBuffer, _In_ ULONG dataSize)
    {
        unique_cert_context certificateContext(GetCertContext(rawSignatureBuffer, dataSize, false));
		if (certificateContext.get() == NULL)
		{
			return false;
		}

        std::vector<std::string> oids;
        if (GetEnhancedKeyUsage(certificateContext.get(), oids)) {
            std::size_t count = oids.size();
            for (std::size_t i = 0; i < count; i++) {
                if (0 == oids.at(i).compare(OID::WindowsStore())) {
                    return true;
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

    static bool GetPublisherDisplayName(/*in*/byte* signatureBuffer, /*in*/ ULONG cbSignatureBuffer, /*inout*/ std::string& publisher)
    {
        unique_cert_context certificateContext(GetCertContext(signatureBuffer, cbSignatureBuffer, true));
		if (certificateContext.get() == NULL)
		{
			return false;
		}
        int requiredLength = CertNameToStrW(
            X509_ASN_ENCODING,
            &certificateContext.get()->pCertInfo->Subject,
            CERT_X500_NAME_STR | CERT_NAME_STR_REVERSE_FLAG,
            nullptr,
            0);

        std::vector<wchar_t> publisherT;
        publisherT.reserve(requiredLength + 1);
        
        if (CertNameToStrW(
            X509_ASN_ENCODING,
            &certificateContext.get()->pCertInfo->Subject,
            CERT_X500_NAME_STR | CERT_NAME_STR_REVERSE_FLAG,
            publisherT.data(),
            requiredLength) > 0)
        {
            auto converted = std::wstring_convert<std::codecvt_utf8<wchar_t>>{}.to_bytes(publisherT.data());
            std::string result(converted.begin(), converted.end());
            publisher = result;
            return true;
        }
        return false;
    }


    bool SignatureValidator::Validate(
        IMsixFactory* factory,
        MSIX_VALIDATION_OPTION option,
        const ComPtr<IStream>& stream,
        AppxSignatureObject* signatureObject,
        SignatureOrigin& origin,
        std::string& publisher)
    {
        // If the caller wants to skip signature validation altogether, just bug out early; we will not read the digests
        if (option & MSIX_VALIDATION_OPTION_SKIPSIGNATURE) { return false; }
        
        ThrowErrorIf(Error::MissingAppxSignatureP7X, (nullptr == stream.Get()), "AppxSignature.p7x missing");
        
        LARGE_INTEGER li = {0};
        ULARGE_INTEGER uli = {0};
        ThrowHrIfFailed(stream->Seek(li, StreamBase::Reference::END, &uli));
        ThrowErrorIf(Error::SignatureInvalid, (uli.QuadPart <= sizeof(P7X_FILE_ID) || uli.QuadPart > (2 << 20)), "stream is too big");

        std::vector<std::uint8_t> p7x(uli.LowPart);
        ThrowHrIfFailed(stream->Seek(li, StreamBase::Reference::START, &uli));

        ULONG actualRead = 0;
        ThrowHrIfFailed(stream->Read(p7x.data(), static_cast<ULONG>(p7x.size()), &actualRead));
        ThrowErrorIf(Error::SignatureInvalid,
            ((actualRead != p7x.size() || (*(DWORD*)p7x.data() != P7X_FILE_ID))),
            "Failed to read p7x or p7x header mismatch");

        BYTE *p7s = p7x.data() + sizeof(DWORD);
        std::uint32_t p7sSize = static_cast<uint32_t>(p7x.size() - sizeof(DWORD));

        // Decode the ASN.1 structure
        CRYPT_CONTENT_INFO* contentInfo = nullptr;
        DWORD contentInfoSize = 0;
        ThrowErrorIfNot(Error::SignatureInvalid, (
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
        ThrowErrorIf(Error::SignatureInvalid, (cryptMsgT == nullptr), "CryptMsgOpenToDecode failed");
        unique_crypt_msg_handle cryptMsg(cryptMsgT);

        // Get the crypographic message 
        ThrowErrorIfNot(Error::SignatureInvalid, (
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
        ThrowErrorIf(Error::SignatureInvalid, (
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
        ThrowErrorIfNot(Error::SignatureInvalid, (
            CryptMsgGetParam(
                cryptMsg.get(),
                CMSG_INNER_CONTENT_TYPE_PARAM,
                0,
                innerContentType.data(),
                &innerContentTypeSize)
            ),"CryptMsgGetParam failed");

        size_t indirectDataObjIdLength = strlen(SPC_INDIRECT_DATA_OBJID);
        ThrowErrorIf(Error::SignatureInvalid, (
            (innerContentTypeSize != indirectDataObjIdLength + 1) ||
            (strncmp((char*)innerContentType.data(), SPC_INDIRECT_DATA_OBJID, indirectDataObjIdLength + 1) != 0)
        ), "unexpected content type");

        DWORD innerContentSize = 0;
        ThrowErrorIfNot(Error::SignatureInvalid, ( 
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
        ThrowErrorIfNot(Error::SignatureInvalid, (
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
        ThrowErrorIfNot(Error::SignatureInvalid, (
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

        signatureObject->ValidateDigestHeader(
            reinterpret_cast<DigestHeader*>(indirectContent->Digest.pbData),
            (indirectContent->Digest.cbData - sizeof(DWORD)) / sizeof(DigestHash),
            (indirectContent->Digest.cbData - sizeof(DWORD)) % sizeof(DigestHash)
        );

        origin = MSIX::SignatureOrigin::Unknown;
        if (IsStoreOrigin(p7s, p7sSize)) { origin = MSIX::SignatureOrigin::Store; }
        else if (IsAuthenticodeOrigin(p7s, p7sSize)) { origin = MSIX::SignatureOrigin::LOB; }

        bool signatureOriginUnknownAllowed = (option & MSIX_VALIDATION_OPTION_ALLOWSIGNATUREORIGINUNKNOWN) == MSIX_VALIDATION_OPTION_ALLOWSIGNATUREORIGINUNKNOWN;
        
        ThrowErrorIf(Error::CertNotTrusted, 
            ((MSIX::SignatureOrigin::Unknown == origin) && !signatureOriginUnknownAllowed),
            "Unknown signature origin");

        ThrowErrorIfNot(Error::SignatureInvalid,
            GetPublisherDisplayName(p7s, p7sSize, publisher) == true,
            "Could not retrieve publisher name");
                
        return true;
    }
} // namespace MSIX