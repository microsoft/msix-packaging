//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#include "AppxSignature.hpp"
#include "Exceptions.hpp"
#include "SignatureCreator.hpp"
#include "MSIXResource.hpp"
#include "StreamHelper.hpp"
#include "MemoryStream.hpp"

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

                    ParsePKCS12(cert.get(), nullptr, "Unable to open PFX file");
                }
                    break;

                default:
                    NOTSUPPORTED
                }
            }

            void ParsePKCS12(PKCS12* p12, const char* pass, const char* failureMessage = nullptr)
            {
                EVP_PKEY* pkey_ = nullptr;
                X509* cert_ = nullptr;
                STACK_OF(X509)* ca_ = chain.get();

                ThrowOpenSSLErrIfFailedMsg(PKCS12_parse(p12, pass, &pkey_, &cert_, &ca_), failureMessage);

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

        // Create the indirect data object within the given message.
        // Copied and modified from the internal OpenSSL function that creates the content based on the type of the outer message.
        // Required to attach our custom type (spcIndirectDataContext) to it.
        void PKCS7_indirect_data_content_new(PKCS7* p7, const CustomOpenSSLObjects& customObjects)
        {
            unique_PKCS7 content{ PKCS7_new() };
            ThrowOpenSSLErrIfAllocFailed(content);

            content->type = customObjects.Get(CustomOpenSSLObjectName::spcIndirectDataContext).GetObj();

            content->d.other = ASN1_TYPE_new();
            ThrowOpenSSLErrIfAllocFailed(content->d.other);

            ThrowOpenSSLErrIfFailed(ASN1_TYPE_set_octetstring(content->d.other, nullptr, 0));
            ThrowOpenSSLErrIfFailed(PKCS7_set_content(p7, content.get()));

            // The parent PKCS7 now owns this
            content.release();
        }

        // Create a signed PKCS7 message from the given data.
        // Copied and modified from PKCS7_sign.
        // Required because we need to attach our message contents and various attributes.
        unique_PKCS7 PKCS7_sign_indirect_data(X509* signcert, EVP_PKEY* pkey, STACK_OF(X509)* certs,
            BIO* data, int flags, const CustomOpenSSLObjects& customObjects)
        {
            unique_PKCS7 p7{ PKCS7_new() };
            ThrowOpenSSLErrIfAllocFailed(p7);

            ThrowOpenSSLErrIfFailed(PKCS7_set_type(p7.get(), NID_pkcs7_signed));

            // Standard PKCS7_sign only supports NID_pkcs7_data, but we want SPC_INDIRECT_DATA_OBJID
            PKCS7_indirect_data_content_new(p7.get(), customObjects);

            // Force SHA256 for now
            PKCS7_SIGNER_INFO* signerInfo = PKCS7_sign_add_signer(p7.get(), signcert, pkey, EVP_sha256(), flags);
            ThrowOpenSSLErrIfFailed(signerInfo);

            // Add our authenticated attributes
            ThrowOpenSSLErrIfFailed(PKCS7_add_attrib_content_type(signerInfo, customObjects.Get(CustomOpenSSLObjectName::spcIndirectDataContext).GetObj()));

            // Add individual code signing statement type to the authenticated attributes.
            std::vector<uint8_t> statementTypeSequence;
            statementTypeSequence << ( ASN1::Sequence{} << ASN1::ObjectIdentifier{ customObjects.Get(CustomOpenSSLObjectName::individualCodeSigning).GetObj() } );

            unique_ASN1_STRING statementString{ ASN1_STRING_type_new(V_ASN1_SEQUENCE) };
            ThrowOpenSSLErrIfAllocFailed(statementString);

            // ASN1_STRING_set copies the given data
            ThrowOpenSSLErrIfFailed(ASN1_STRING_set(statementString.get(), static_cast<void*>(statementTypeSequence.data()), static_cast<int>(statementTypeSequence.size())));

            ThrowOpenSSLErrIfFailed(PKCS7_add_signed_attribute(signerInfo, customObjects.Get(CustomOpenSSLObjectName::spcStatementType).GetNID(), V_ASN1_SEQUENCE, statementString.get()));
            statementString.release();

            // Add empty opusType
            std::vector<uint8_t> opusTypeSequence;
            opusTypeSequence << ASN1::Sequence{};

            unique_ASN1_STRING opusString{ ASN1_STRING_type_new(V_ASN1_SEQUENCE) };
            ThrowOpenSSLErrIfAllocFailed(opusString);

            ThrowOpenSSLErrIfFailed(ASN1_STRING_set(opusString.get(), static_cast<void*>(opusTypeSequence.data()), static_cast<int>(opusTypeSequence.size())));

            ThrowOpenSSLErrIfFailed(PKCS7_add_signed_attribute(signerInfo, customObjects.Get(CustomOpenSSLObjectName::spcSpOpusInfo).GetNID(), V_ASN1_SEQUENCE, opusString.get()));
            opusString.release();

            // Always include the chain certs
            for (int i = 0; i < sk_X509_num(certs); i++) {
                ThrowOpenSSLErrIfFailed(PKCS7_add_certificate(p7.get(), sk_X509_value(certs, i)));
            }

            ThrowOpenSSLErrIfFailed(PKCS7_final(p7.get(), data, flags));

            return p7;
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

        // Create the custom digest blob that contains the hashes to be signed.
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

        // Encapsulate the digest blob within the ASN1 wrapper.
        // All of this data is signed.
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

    // Given a set of digest hashes from a package and the signing info, create a p7x signature stream.
    ComPtr<IStream> SignatureCreator::Sign(
        AppxSignatureObject* digests,
        MSIX_CERTIFICATE_FORMAT signingCertificateFormat,
        IStream* signingCertificate,
        IStream* privateKey)
    {
        // TODO: likely needs init thread safety as in SignatureValidator::Validate
        OpenSSL_add_all_algorithms();
        CustomOpenSSLObjects customObjects{};

        // Read in the signing info based on format, etc.
        SigningInfo signingInfo{ signingCertificateFormat, signingCertificate, privateKey };

        // Create the blob to be signed
        std::vector<uint8_t> signedData = CreateDataToBeSigned(digests, customObjects);
        unique_BIO dataBIO{ BIO_new_mem_buf(signedData.data(), static_cast<int>(signedData.size())) };
        ThrowOpenSSLErrIfAllocFailed(dataBIO);

        // Sign it
        unique_PKCS7 p7{ PKCS7_sign_indirect_data(signingInfo.certificate.get(), signingInfo.privateKey.get(), signingInfo.chain.get(), dataBIO.get(), PKCS7_BINARY | PKCS7_NOATTR, customObjects) };

        // Overwrite the signed contents with the complete one including the additional sequence at the beginning.
        // It is unclear why things work this way, but this is necessary.
        std::vector<uint8_t> completeBlob;
        completeBlob << ASN1::Sequence{ std::move(signedData) };

        unique_ASN1_STRING sequenceString{ ASN1_STRING_type_new(V_ASN1_SEQUENCE) };
        ThrowOpenSSLErrIfAllocFailed(sequenceString);
        ThrowOpenSSLErrIfFailed(ASN1_STRING_set(sequenceString.get(), static_cast<void*>(completeBlob.data()), static_cast<int>(completeBlob.size())));

        ASN1_TYPE_set(p7->d.sign->contents->d.other, V_ASN1_SEQUENCE, sequenceString.get());
        sequenceString.release();

        // Serialize the PKCS7
        unique_BIO outBIO{ BIO_new(BIO_s_mem()) };
        ThrowOpenSSLErrIfAllocFailed(outBIO);
        ThrowOpenSSLErrIfFailed(i2d_PKCS7_bio(outBIO.get(), p7.get()));

        // Write the signature out, including the extra bytes that tag it as a package signature.
        ComPtr<IStream> p7xOutStream = ComPtr<IStream>::Make<MemoryStream>();

        char* out = nullptr;
        long cOut = BIO_get_mem_data(outBIO.get(), &out);

        uint32_t prefix = P7X_FILE_ID;
        p7xOutStream->Write(&prefix, sizeof(prefix), nullptr);
        p7xOutStream->Write(out, cOut, nullptr);

        return p7xOutStream;
    }
} // namespace MSIX
