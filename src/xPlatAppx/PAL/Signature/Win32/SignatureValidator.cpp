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
        void operator()(HLOCAL h) const {
            LocalFree(h);
        };
    };
    
    struct unique_cert_context_deleter {
        void operator()(PCCERT_CONTEXT p) const {
            CertFreeCertificateContext(p);
        };
    };

    struct unique_cert_chain_deleter {
        void operator()(PCCERT_CHAIN_CONTEXT p) const {
            CertFreeCertificateChain(p);
        };
    };
    struct unique_hash_handle_deleter {
        void operator()(BCRYPT_HASH_HANDLE h) const {
            BCryptDestroyHash(h);
        };
    };

    struct unique_alg_handle_deleter {
        void operator()(BCRYPT_ALG_HANDLE h) const {
            BCryptCloseAlgorithmProvider(h, 0);
        };
    };

    struct unique_cert_store_handle_deleter {
        void operator()(HCERTSTORE h) const {
            CertCloseStore(h, 0);
        };
    };

    struct unique_crypt_msg_handle_deleter {
        void operator()(HCRYPTMSG h) const {
            CryptMsgClose(h);
        };
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
        PCCERT_CHAIN_CONTEXT certChainContext;
        HCERTSTORE certStoreT;
        HCRYPTMSG signedMessageT;
        DWORD queryContentType = 0;
        DWORD queryFormatType = 0;
        BOOL result;

        CRYPT_DATA_BLOB signatureBlob = { 0 };
        signatureBlob.cbData = cbSignatureBuffer;
        signatureBlob.pbData = signatureBuffer;

        // Get the cert content
        result = CryptQueryObject(
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
            NULL);

        if (!result)
            throw xPlat::Exception(xPlat::Error::AppxSignatureInvalid);

        unique_cert_store_handle certStore(certStoreT);
        unique_crypt_msg_handle signedMessage(signedMessageT);

        // Get the signer size and information from the signed data message
        // The properties of the signer info will be used to uniquely identify the signing certificate in the certificate store
        CMSG_SIGNER_INFO* signerInfo = NULL;
        DWORD signerInfoSize = 0;
        result = CryptMsgGetParam(signedMessage.get(), CMSG_SIGNER_INFO_PARAM, 0, NULL, &signerInfoSize);
        if (!result)
            throw xPlat::Exception(xPlat::Error::AppxSignatureInvalid);

        // Check that the signer info size is within reasonable bounds; under the max length of a string for the issuer field
        if (signerInfoSize == 0 || signerInfoSize >= STRSAFE_MAX_CCH)
            throw xPlat::Exception(xPlat::Error::AppxSignatureInvalid);

        std::vector<byte> signerInfoBuffer(signerInfoSize);

        signerInfo = reinterpret_cast<CMSG_SIGNER_INFO*>(signerInfoBuffer.data());
        result = CryptMsgGetParam(signedMessage.get(), CMSG_SIGNER_INFO_PARAM, 0, signerInfo, &signerInfoSize);
        if (!result)
            throw xPlat::Exception(xPlat::Error::AppxSignatureInvalid);

        // Get the signing certificate from the certificate store based on the issuer and serial number of the signer info
        CERT_INFO certInfo;
        certInfo.Issuer = signerInfo->Issuer;
        certInfo.SerialNumber = signerInfo->SerialNumber;

        unique_cert_context signingCertContext(CertGetSubjectCertificateFromStore(
            certStore.get(),
            X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
            &certInfo));

        if (signingCertContext.get() == NULL)
            throw xPlat::Exception(xPlat::Error::AppxSignatureInvalid);

        CERT_CHAIN_PARA certChainParameters = { 0 };
        certChainParameters.cbSize = sizeof(CERT_CHAIN_PARA);
        certChainParameters.RequestedUsage.dwType = USAGE_MATCH_TYPE_AND;

        // Do not connect online for URL retrievals.
        // Note that this check does not respect the lifetime signing EKU on the signing certificate
        DWORD certChainFlags = CERT_CHAIN_CACHE_ONLY_URL_RETRIEVAL;

        // Get the signing certificate chain context
        result = CertGetCertificateChain(
            HCCE_LOCAL_MACHINE,
            signingCertContext.get(),
            NULL,   // Use the current system time for CRL validation
            certStore.get(),
            &certChainParameters,
            certChainFlags,
            NULL,   // Reserved parameter; must be NULL
            &certChainContext);

        if (!result)
            throw xPlat::Exception(xPlat::Error::AppxSignatureInvalid);

        return certChainContext;
    }

static bool GetEnhancedKeyUsage(
        PCCERT_CONTEXT pCertContext,
        std::vector<std::string>& values)
    {
        HRESULT hr = S_OK;
        DWORD cbExtensionUsage = 0;
        DWORD cbPropertyUsage = 0;
        DWORD i;
        std::vector<byte> extensionUsage(0);
        std::vector<byte> propertyUsage(0);
        
        bool result = CertGetEnhancedKeyUsage(
            pCertContext,
            CERT_FIND_EXT_ONLY_ENHKEY_USAGE_FLAG,
            NULL,
            &cbExtensionUsage);

        if (result)
        {
            extensionUsage.resize(cbExtensionUsage);
            
            result = CertGetEnhancedKeyUsage(
                pCertContext,
                CERT_FIND_EXT_ONLY_ENHKEY_USAGE_FLAG,
                reinterpret_cast<PCERT_ENHKEY_USAGE>(extensionUsage.data()),
                &cbExtensionUsage);
        }

        if (!result && GetLastError() != CRYPT_E_NOT_FOUND)
            throw xPlat::Exception(xPlat::Error::AppxSignatureInvalid);
                
        result = CertGetEnhancedKeyUsage(
            pCertContext,
            CERT_FIND_PROP_ONLY_ENHKEY_USAGE_FLAG,
            NULL,
            &cbPropertyUsage);
        
        if (result)
        {
            propertyUsage.resize(cbPropertyUsage);

            result = CertGetEnhancedKeyUsage(
                pCertContext,
                CERT_FIND_PROP_ONLY_ENHKEY_USAGE_FLAG,
                reinterpret_cast<PCERT_ENHKEY_USAGE>(propertyUsage.data()),
                &cbPropertyUsage);
        }

        if (!result && GetLastError() != CRYPT_E_NOT_FOUND)
            throw xPlat::Exception(xPlat::Error::AppxSignatureInvalid);

        result = false;

        //get OIDS from the extension or property
        if (extensionUsage.size() > 0)
        {
            PCERT_ENHKEY_USAGE pExtensionUsageT = reinterpret_cast<PCERT_ENHKEY_USAGE>(extensionUsage.data());
            for (i = 0; i < pExtensionUsageT->cUsageIdentifier; i++)
            {
                std::string sz(pExtensionUsageT->rgpszUsageIdentifier[i]);
                values.push_back(sz);
            }
            result = (pExtensionUsageT->cUsageIdentifier > 0);
        }
        else
        if (propertyUsage.size() > 0)
        {
            PCERT_ENHKEY_USAGE pPropertyUsageT = reinterpret_cast<PCERT_ENHKEY_USAGE>(propertyUsage.data());
            for (i = 0; i < pPropertyUsageT->cUsageIdentifier; i++)
            {
                std::string sz(pPropertyUsageT->rgpszUsageIdentifier[i]);
                values.push_back(sz);
            }
            result = (pPropertyUsageT->cUsageIdentifier > 0);
        }
        return result;
    }

    static BOOL IsMicrosoftTrustedChain(_In_ PCCERT_CHAIN_CONTEXT certChainContext)
    {
        // Validate that the certificate chain is rooted in one of the well-known MS root certs
        CERT_CHAIN_POLICY_PARA policyParameters = { 0 };
        policyParameters.cbSize = sizeof(CERT_CHAIN_POLICY_PARA);
        CERT_CHAIN_POLICY_STATUS policyStatus = { 0 };
        policyStatus.cbSize = sizeof(CERT_CHAIN_POLICY_STATUS);

        policyParameters.dwFlags = MICROSOFT_ROOT_CERT_CHAIN_POLICY_CHECK_APPLICATION_ROOT_FLAG;

        return CertVerifyCertificateChainPolicy(
            CERT_CHAIN_POLICY_MICROSOFT_ROOT,
            certChainContext,
            &policyParameters,
            &policyStatus);      
    }

    static BOOL IsAuthenticodeTrustedChain(_In_ PCCERT_CHAIN_CONTEXT certChainContext)
    {
        // Validate that the certificate chain is rooted in one of the well-known MS root certs
        CERT_CHAIN_POLICY_PARA policyParameters = { 0 };
        policyParameters.cbSize = sizeof(CERT_CHAIN_POLICY_PARA);
        CERT_CHAIN_POLICY_STATUS policyStatus = { 0 };
        policyStatus.cbSize = sizeof(CERT_CHAIN_POLICY_STATUS);

        //policyParameters.dwFlags = MICROSOFT_ROOT_CERT_CHAIN_POLICY_CHECK_APPLICATION_ROOT_FLAG;

        return CertVerifyCertificateChainPolicy(
            CERT_CHAIN_POLICY_AUTHENTICODE,
            certChainContext,
            &policyParameters,
            &policyStatus);
    }

    static BOOL IsCACert(_In_ PCCERT_CONTEXT pCertContext)
    {
        CERT_BASIC_CONSTRAINTS2_INFO *basicConstraintsT = NULL;
        PCERT_EXTENSION certExtension = NULL;
        DWORD cbDecoded = 0;
        bool retValue = FALSE;

        certExtension = CertFindExtension(
            szOID_BASIC_CONSTRAINTS2,
            pCertContext->pCertInfo->cExtension,
            pCertContext->pCertInfo->rgExtension);

        if (certExtension &&
            CryptDecodeObjectEx(
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
            retValue = basicConstraintsT->fCA ? TRUE : FALSE;
        }
        return retValue;
    }

    static BOOL IsCertificateSelfSigned(PCCERT_CONTEXT pContext,
        DWORD dwEncoding,
        DWORD dwFlags)
    {
        if (!(pContext) ||
            (dwFlags != 0))
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            return(FALSE);
        }

        if (!(CertCompareCertificateName(dwEncoding,
            &pContext->pCertInfo->Issuer,
            &pContext->pCertInfo->Subject)))
        {
            return(FALSE);
        }

        DWORD   dwFlag = CERT_STORE_SIGNATURE_FLAG;
        if (!(CertVerifySubjectCertificateContext(pContext, pContext, &dwFlag)) ||
            (dwFlag & CERT_STORE_SIGNATURE_FLAG))
        {
            return(FALSE);
        }

        return(TRUE);
    }

static PCCERT_CONTEXT GetCertContext(BYTE *signatureBuffer, ULONG cbSignatureBuffer)
    {
        HRESULT hr = S_OK;
        BOOL result;
        DWORD dwExpectedContentType = CERT_QUERY_CONTENT_FLAG_CERT |
            CERT_QUERY_CONTENT_FLAG_PKCS7_SIGNED |
            CERT_QUERY_CONTENT_FLAG_PKCS7_UNSIGNED;
        HCERTSTORE certStoreHandleT = NULL;
        DWORD dwContentType = 0;

        //get cert blob out
        PCCERT_CONTEXT pCertContext = NULL;
        CERT_BLOB blob;
        blob.pbData = signatureBuffer;
        blob.cbData = cbSignatureBuffer;

        //get cert context from strCertificate;
        result = CryptQueryObject(
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
            NULL);

        if (!result)
            throw xPlat::Exception(xPlat::Error::AppxSignatureInvalid);

        unique_cert_store_handle certStoreHandle(certStoreHandleT);

        if (dwContentType == CERT_QUERY_CONTENT_CERT)
        {
            //get the certificate context
            pCertContext = CertEnumCertificatesInStore(certStoreHandle.get(), NULL);
        }
        else //pkcs7
        {
            //get the end entity
            while (NULL != (pCertContext = CertEnumCertificatesInStore(certStoreHandle.get(), pCertContext)))
            {
                if (IsCertificateSelfSigned(pCertContext, pCertContext->dwCertEncodingType, 0) ||
                    IsCACert(pCertContext))
                {
                    continue;
                }
                else
                {
                    //end entity cert
                    break;
                }
            }
        }
        return pCertContext;
    }
    
    static BOOL DoesSignatureCertContainStoreEKU(
        _In_ byte* rawSignatureBuffer,
        _In_ ULONG dataSize)
    {
        unique_cert_context certificateContext(GetCertContext(rawSignatureBuffer, dataSize));
        
        std::vector<std::string> oids;
        bool result = GetEnhancedKeyUsage(certificateContext.get(), oids);

        if (result)
        {
            std::size_t count = oids.size();
            for (std::size_t i = 0; i < count; i++)
            {
                if (0 == oids.at(i).compare(OID::WindowsStore))
                {   return TRUE;
                }
            }
        }
        return FALSE;
    }

   // Best effort to determine whether the signature file is associated with a store cert
     static BOOL IsStoreOrigin(byte* signatureBuffer, ULONG cbSignatureBuffer)
    {
        BOOL retValue = FALSE;
        if (DoesSignatureCertContainStoreEKU(signatureBuffer, cbSignatureBuffer))
        {
            unique_cert_chain_context certChainContext(GetCertChainContext(signatureBuffer, cbSignatureBuffer));
            retValue = IsMicrosoftTrustedChain(certChainContext.get());
        }
        return retValue;
    }

    // Best effort to determine whether the signature file is associated with a store cert
    static BOOL IsAuthenticodeOrigin(byte* signatureBuffer, ULONG cbSignatureBuffer)
    {
        BOOL retValue = FALSE;
        {
            unique_cert_chain_context certChainContext(GetCertChainContext(signatureBuffer, cbSignatureBuffer));
            retValue = IsAuthenticodeTrustedChain(certChainContext.get());
        }
        return retValue;
    }

bool SignatureValidator::Validate(
        /*in*/ APPX_VALIDATION_OPTION option,
        /*in*/ IStream *stream,
        /*inout*/ std::map<xPlat::AppxSignatureObject::DigestName, xPlat::AppxSignatureObject::Digest>& digests,
        /*inout*/ SignatureOrigin& origin)
    {
        // If the caller wants to skip signature validation altogether, just bug out early. We will not read the digests
        if (option & APPX_VALIDATION_OPTION_SKIPSIGNATURE) { return false; }
        
        HRESULT hr;
        LARGE_INTEGER li = {0};
        ULARGE_INTEGER uli = {0};
        ThrowHrIfFailed(stream->Seek(li, StreamBase::Reference::END, &uli));
        ThrowErrorIf(Error::AppxSignatureInvalid, (uli.QuadPart <= sizeof(P7X_FILE_ID) || uli.QuadPart > (2 << 20)), "stream is too big");
        std::vector<std::uint8_t> p7x(uli.LowPart);

        ThrowHrIfFailed(stream->Seek(li, StreamBase::Reference::START, &uli));
        
        ULONG actualRead = 0;
        hr = stream->Read(p7x.data(), p7x.size(), &actualRead);
        if (FAILED(hr) || actualRead != p7x.size())
            throw xPlat::Exception(xPlat::Error::AppxSignatureInvalid);

        if (*(DWORD*)p7x.data() != P7X_FILE_ID)
            throw xPlat::Exception(xPlat::Error::AppxSignatureInvalid);

        BYTE *p7s = p7x.data() + sizeof(DWORD);
        std::uint32_t p7sSize = p7x.size() - sizeof(DWORD);

        // Decode the ASN.1 structure
        CRYPT_CONTENT_INFO* contentInfo = nullptr;
        DWORD contentInfoSize = 0;

        if (!CryptDecodeObjectEx(
            X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
            PKCS_CONTENT_INFO,
            p7s,
            p7sSize,
            CRYPT_DECODE_ALLOC_FLAG,
            nullptr,
            &contentInfo,
            &contentInfoSize))
        {
            throw xPlat::Exception(xPlat::Error::AppxSignatureInvalid); 
        }

        HCRYPTMSG cryptMsgT = CryptMsgOpenToDecode(
            X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
            0,
            CMSG_SIGNED,
            NULL,
            nullptr,
            nullptr);

        if (cryptMsgT == nullptr)
        {
            throw xPlat::Exception(xPlat::Error::AppxSignatureInvalid);
        }

        unique_crypt_msg_handle cryptMsg(cryptMsgT);

        // Get the crypographic message 
        if (!CryptMsgUpdate(
            cryptMsg.get(),
            contentInfo->Content.pbData,
            contentInfo->Content.cbData,
            TRUE))
        {
            throw xPlat::Exception(xPlat::Error::AppxSignatureInvalid);
        }

        // This first call to CryptMsgGetParam is expected to fail because we don't know
        // how big of a buffer that it needs to store the inner content 
        DWORD innerContentTypeSize = 0;
        
        ULONG readBytes = 0;
        if (!CryptMsgGetParam(
            cryptMsg.get(),
            CMSG_INNER_CONTENT_TYPE_PARAM,
            0,
            nullptr,
            &innerContentTypeSize) &&
            HRESULT_FROM_WIN32(GetLastError()) != HRESULT_FROM_WIN32(ERROR_MORE_DATA))
        {
            throw xPlat::Exception(xPlat::Error::AppxSignatureInvalid);
        }
        
        // Allocate a temporary buffer
        std::vector<std::uint8_t> innerContentType(innerContentTypeSize);

        if (!CryptMsgGetParam(
            cryptMsg.get(),
            CMSG_INNER_CONTENT_TYPE_PARAM,
            0,
            innerContentType.data(),
            &innerContentTypeSize))
        {
            throw xPlat::Exception(xPlat::Error::AppxSignatureInvalid);
        }
        
        size_t indirectDataObjIdLength = strlen(SPC_INDIRECT_DATA_OBJID);

        // Make sure the content type is expected
        if ((innerContentTypeSize != indirectDataObjIdLength + 1) ||
            (strncmp((char*)innerContentType.data(), SPC_INDIRECT_DATA_OBJID, indirectDataObjIdLength + 1) != 0))
        {
            throw xPlat::Exception(xPlat::Error::AppxSignatureInvalid);
        }

        DWORD innerContentSize = 0;

        if (!CryptMsgGetParam(
            cryptMsg.get(),
            CMSG_CONTENT_PARAM,
            0,
            nullptr,
            &innerContentSize) &&
            HRESULT_FROM_WIN32(GetLastError()) != HRESULT_FROM_WIN32(ERROR_MORE_DATA))
        {
            throw xPlat::Exception(xPlat::Error::AppxSignatureInvalid);
        }

        // Allocate a temporary buffer
        std::vector<std::uint8_t> innerContent(innerContentSize);

        if (!CryptMsgGetParam(
            cryptMsg.get(),
            CMSG_CONTENT_PARAM,
            0,
            innerContent.data(),
            &innerContentSize))
        {
            throw xPlat::Exception(xPlat::Error::AppxSignatureInvalid);
        }
        
        // Parse the ASN.1 to the the indirect data structure
        SPC_INDIRECT_DATA_CONTENT* indirectContent = NULL;
        DWORD indirectContentSize = 0;

        if (!CryptDecodeObjectEx(
            X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
            SPC_INDIRECT_DATA_CONTENT_STRUCT,
            innerContent.data(),
            innerContentSize,
            CRYPT_DECODE_ALLOC_FLAG,
            nullptr,
            &indirectContent,
            &indirectContentSize))
        {
            throw xPlat::Exception(xPlat::Error::AppxSignatureInvalid); 
        }

        DigestHeader *header = reinterpret_cast<DigestHeader*>(indirectContent->Digest.pbData);
        std::uint32_t numberOfHashes = (indirectContent->Digest.cbData - sizeof(DWORD)) / (sizeof(DWORD) + 32);
        std::uint32_t modHashes = (indirectContent->Digest.cbData - sizeof(DWORD)) % (sizeof(DWORD) + 32);

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

        // If the caller allows unknown origin certs, don't bother with the certificate check
        if (!(option & APPX_VALIDATION_OPTION_ALLOWUNKNOWNORIGIN))
        {
            // Build wintrust data to pass to WinVerifyTrust in order to validate signature
            GUID P7xSipGuid = { 0x5598cff1, 0x68db, 0x4340,{ 0xb5, 0x7f, 0x1c, 0xac, 0xf8, 0x8c, 0x9a, 0x51 } };
            GUID wintrustActionVerify = WINTRUST_ACTION_GENERIC_VERIFY_V2;
            WINTRUST_BLOB_INFO signatureBlobInfo = { 0 };
            WINTRUST_DATA trustData = { 0 };

            signatureBlobInfo.cbStruct = sizeof(WINTRUST_BLOB_INFO);
            signatureBlobInfo.gSubject = P7xSipGuid;
            signatureBlobInfo.cbMemObject = p7x.size();
            signatureBlobInfo.pbMemObject = p7x.data();

            trustData.cbStruct = sizeof(WINTRUST_DATA);
            trustData.dwUIChoice = WTD_UI_NONE;
            trustData.fdwRevocationChecks = WTD_REVOKE_NONE;
            trustData.dwUnionChoice = WTD_CHOICE_BLOB;
            trustData.dwStateAction = WTD_STATEACTION_VERIFY;
            trustData.dwProvFlags = WTD_CACHE_ONLY_URL_RETRIEVAL | WTD_REVOCATION_CHECK_NONE;
            trustData.pBlob = &signatureBlobInfo;

            // Verify whether we trust the certificate. If it fails, 
            if (WinVerifyTrust(static_cast<HWND>(INVALID_HANDLE_VALUE), &wintrustActionVerify, &trustData))
            {
                throw xPlat::Exception(xPlat::Error::AppxCertNotTrusted);
            }
        }

        if (IsStoreOrigin(p7s, p7sSize))
        {
            origin = xPlat::SignatureOrigin::Store;
            return true;
        }

        if (IsAuthenticodeOrigin(p7s, p7sSize))
        {
            origin = xPlat::SignatureOrigin::LOB;
            return true;
        }

        origin = xPlat::SignatureOrigin::Unknown;
        return true;
    }
}



