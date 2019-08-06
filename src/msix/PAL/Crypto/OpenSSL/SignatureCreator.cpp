//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#include "AppxSignature.hpp"
#include "Exceptions.hpp"
#include "FileStream.hpp"
#include "SignatureCreator.hpp"
#include "MSIXResource.hpp"
#include "StreamHelper.hpp"

#include "SharedOpenSSL.hpp"
#include "OpenSSLWriting.hpp"

#include <algorithm>
#include <iterator>

namespace MSIX
{
    namespace
    {
        struct SigningInfo
        {
            SigningInfo(
                MSIX_CERTIFICATE_FORMAT signingCertificateFormat,
                IStream* signingCertificate,
                IStream* privateKey)
            {
                switch (signingCertificateFormat)
                {
                case MSIX_CERTIFICATE_FORMAT_PFX:
                {
                    auto certBytes = Helper::CreateBufferFromStream(signingCertificate);

                    unique_BIO certBIO{ BIO_new_mem_buf(reinterpret_cast<void*>(certBytes.data()), static_cast<int>(certBytes.size())) };
                    unique_PKCS12 cert{ d2i_PKCS12_bio(certBIO.get(), nullptr) };

                    ParsePKCS12(cert.get(), nullptr);
                }
                    break;

                default:
                    NOTSUPPORTED
                }
            }

            void ParsePKCS12(PKCS12* p12, const char* pass)
            {
                EVP_PKEY* pkey_ = nullptr;
                X509* cert_ = nullptr;
                STACK_OF(X509)* ca_ = chain.get();

                ThrowOpenSSLErrIfFailed(PKCS12_parse(p12, pass, &pkey_, &cert_, &ca_));

                // If ca existed, the certs will have been added to it and it is the same pointer.
                // If it is not set, a new stack will have been created and we need to set it out.
                if (!static_cast<bool>(chain))
                {
                    chain.reset(ca_);
                }
                certificate.reset(cert_);
                privateKey.reset(pkey_);
            }

            unique_X509 certificate;
            unique_STACK_X509 chain;
            unique_EVP_PKEY privateKey;
        };

        int PKCS7_indirect_data_content_new(PKCS7* p7, const CustomOpenSSLObjects& customObjects)
        {
            PKCS7* ret = NULL;

            if ((ret = PKCS7_new()) == NULL)
                goto err;

            ret->type = customObjects.Get(CustomOpenSSLObjectName::spcIndirectDataContext).GetObj();

            ret->d.other = ASN1_TYPE_new();
            if (!ret->d.other)
                goto err;

            if (!ASN1_TYPE_set_octetstring(ret->d.other, nullptr, 0))
                goto err;

            if (!PKCS7_set_content(p7, ret))
                goto err;

            return (1);
        err:
            if (ret != NULL)
                PKCS7_free(ret);
            return (0);
        }

        PKCS7* PKCS7_sign_indirect_data(X509* signcert, EVP_PKEY* pkey, STACK_OF(X509)* certs,
            BIO* data, int flags, const CustomOpenSSLObjects& customObjects)
        {
            PKCS7* p7;
            int i;

            if (!(p7 = PKCS7_new())) {
                PKCS7err(PKCS7_F_PKCS7_SIGN, ERR_R_MALLOC_FAILURE);
                return NULL;
            }

            if (!PKCS7_set_type(p7, NID_pkcs7_signed))
                goto err;

            // Standard PKCS7_sign only supports NID_pkcs7_data, but we want SPC_INDIRECT_DATA_OBJID
            if (!PKCS7_indirect_data_content_new(p7, customObjects))
                goto err;

            // Force SHA256 for now
            PKCS7_SIGNER_INFO* signerInfo = PKCS7_sign_add_signer(p7, signcert, pkey, EVP_sha256(), flags);
            if (!signerInfo) {
                PKCS7err(PKCS7_F_PKCS7_SIGN, PKCS7_R_PKCS7_ADD_SIGNER_ERROR);
                goto err;
            }

            // Add our authenticated attributes
            PKCS7_add_attrib_content_type(signerInfo, customObjects.Get(CustomOpenSSLObjectName::spcIndirectDataContext).GetObj());

            // TODO: Make a cleaner way to generate this sequence for the statement type.
            const uint8_t statementType[]{ 0x30, 0x0C, 0x06, 0x0A, 0x2B, 0x06, 0x01, 0x04, 0x01, 0x82, 0x37, 0x02, 0x01, 0x15 };

            // TODO: smart pointer
            ASN1_STRING* statementString = ASN1_STRING_type_new(V_ASN1_SEQUENCE);
            ASN1_STRING_set(statementString, const_cast<uint8_t*>(statementType), sizeof(statementType));

            PKCS7_add_signed_attribute(signerInfo, customObjects.Get(CustomOpenSSLObjectName::spcStatementType).GetNID(), V_ASN1_SEQUENCE, statementString);

            // TODO: Make a cleaner way to generate this too
            const uint8_t opusType[]{ 0x30, 0x00 };

            // TODO: smart pointer
            ASN1_STRING* opusString = ASN1_STRING_type_new(V_ASN1_SEQUENCE);
            ASN1_STRING_set(opusString, const_cast<uint8_t*>(opusType), sizeof(opusType));

            PKCS7_add_signed_attribute(signerInfo, customObjects.Get(CustomOpenSSLObjectName::spcSpOpusInfo).GetNID(), V_ASN1_SEQUENCE, opusString);

            // Always include the chain certs
            for (i = 0; i < sk_X509_num(certs); i++) {
                if (!PKCS7_add_certificate(p7, sk_X509_value(certs, i)))
                    goto err;
            }

            if (!PKCS7_final(p7, data, flags))
            {
                goto err;
            }

            return p7;

        err:
            PKCS7_free(p7);
            return NULL;
        }

        void AppendDigestName(std::vector<uint8_t>& target, DigestName name)
        {
            uint32_t nameVal = static_cast<uint32_t>(name);
            for (size_t i = 0; i < 4; ++i)
            {
                size_t bitShift = i * 8;
                uint32_t mask = 0xFF << bitShift;
                target.push_back(static_cast<uint8_t>((nameVal & mask) >> bitShift));
            }
        }

        void AppendDigest(std::vector<uint8_t>& target, const std::vector<uint8_t>& digest)
        {
            target.insert(target.end(), digest.begin(), digest.end());
        }

        std::vector<uint8_t> CreateDigestBlob(AppxSignatureObject* digests)
        {
            std::vector<uint8_t> result;

            AppendDigestName(result, DigestName::HEAD);
            AppendDigestName(result, DigestName::AXPC);
            AppendDigest(result, digests->GetFileRecordsDigest());
            AppendDigestName(result, DigestName::AXCD);
            AppendDigest(result, digests->GetCentralDirectoryDigest());
            AppendDigestName(result, DigestName::AXCT);
            AppendDigest(result, digests->GetContentTypesDigest());
            AppendDigestName(result, DigestName::AXBM);
            AppendDigest(result, digests->GetAppxBlockMapDigest());
            if (!digests->GetCodeIntegrityDigest().empty())
            {
                AppendDigestName(result, DigestName::AXCI);
                AppendDigest(result, digests->GetCodeIntegrityDigest());
            }

            return result;
        }

        std::vector<uint8_t> CreateDataToBeSigned(AppxSignatureObject* digests, const CustomOpenSSLObjects& customObjects)
        {
            std::vector<uint8_t> digestBlob = CreateDigestBlob(digests);

            std::vector<uint8_t> result;

            result
                << ( ASN1::Sequence{}
                    << ASN1::ObjectIdentifier{ customObjects.Get(CustomOpenSSLObjectName::spcSipInfoObjID).GetObj() }
                    << ( ASN1::Sequence{}
                        << ASN1::Integer{ APPX_SIP_DEFAULT_VERSION }
                        << ASN1::OctetString{ std::vector<uint8_t>{ APPX_SIP_GUID_BYTES } }
                        << ASN1::Integer{ 0 }
                        << ASN1::Integer{ 0 }
                        << ASN1::Integer{ 0 }
                        << ASN1::Integer{ 0 }
                        << ASN1::Integer{ 0 }
                    )
                )
                << ( ASN1::Sequence{}
                    << ( ASN1::Sequence{}
                        << ASN1::ObjectIdentifier{ OBJ_nid2obj(NID_sha256) }
                        << ASN1::Null{}
                    )
                << ASN1::OctetString{ digestBlob }
                );

            return result;
        }
    }

    // [X] 1. Get self signed PKCS7_sign working
    // [X] 2. Try to hack the OID of the contents to not be 'data'
    // [ ] 3. If that makes a sufficiently nice looking output, create indirect data blob in ASN
    // [ ] 4. Else?
    ComPtr<IStream> SignatureCreator::Sign(
        AppxSignatureObject* digests,
        MSIX_CERTIFICATE_FORMAT signingCertificateFormat,
        IStream* signingCertificate,
        IStream* privateKey)
    {
        OpenSSL_add_all_algorithms();
        CustomOpenSSLObjects customObjects{};

        // Read in the signing info based on format, etc.
        SigningInfo signingInfo{ signingCertificateFormat, signingCertificate, privateKey };

        // Create the blob to be signed
        std::vector<uint8_t> signedData = CreateDataToBeSigned(digests, customObjects);
        unique_BIO dataBIO{ BIO_new_mem_buf(signedData.data(), static_cast<int>(signedData.size())) };

        // Sign it
        unique_PKCS7 p7{ PKCS7_sign_indirect_data(signingInfo.certificate.get(), signingInfo.privateKey.get(), signingInfo.chain.get(), dataBIO.get(), PKCS7_BINARY | PKCS7_NOATTR, customObjects) };
        ThrowOpenSSLErrIfAllocFailed(p7);

        // Overwrite the signed contents with the complete one including the additional sequence at the beginning.
        // It is unclear why things work this way, but this is necessary.
        std::vector<uint8_t> completeBlob;
        completeBlob << ASN1::Sequence{ std::move(signedData) };

        // TODO: Smart poitners
        ASN1_STRING* sequenceString = ASN1_STRING_type_new(V_ASN1_SEQUENCE);
        ASN1_STRING_set(sequenceString, reinterpret_cast<void*>(completeBlob.data()), static_cast<int>(completeBlob.size()));

        ASN1_TYPE_set(p7->d.sign->contents->d.other, V_ASN1_SEQUENCE, sequenceString);

        unique_BIO outBIO{ BIO_new(BIO_s_mem()) };
        i2d_PKCS7_bio(outBIO.get(), p7.get());

        MSIX::ComPtr<IStream> outStream;
        ThrowHrIfFailed(CreateStreamOnFile(R"(D:\Temp\evernotesup\openssltest.p7s)", false, &outStream));

        char* out = nullptr;
        long cOut = BIO_get_mem_data(outBIO.get(), &out);

        outStream->Write(out, cOut, nullptr);

        {
            MSIX::ComPtr<IStream> p7xOutStream;
            ThrowHrIfFailed(CreateStreamOnFile(R"(D:\Temp\evernotesup\openssltest.p7x)", false, &p7xOutStream));

            uint32_t prefix = P7X_FILE_ID;
            p7xOutStream->Write(&prefix, sizeof(prefix), nullptr);
            p7xOutStream->Write(out, cOut, nullptr);
        }

        // For funsies
        MSIX::ComPtr<IStream> result;
        ThrowHrIfFailed(CreateStreamOnFile(R"(D:\Temp\evernotesup\openssltest.p7x)", true, &result));

        return result;
    }
} // namespace MSIX
