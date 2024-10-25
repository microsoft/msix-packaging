//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once
#include "AppxPackaging.hpp"
#include "ComHelper.hpp"
#include "AppxSignature.hpp"
#include "Crypto.hpp"
#include "ZipObjectWriter.hpp"

#include <memory>
#include <vector>
#include <string>
#include <array>

namespace MSIX
{

static const std::array<const char*, 3> signingModifiedFiles =
{
    CONTENT_TYPES_XML,
    CODEINTEGRITY_CAT,
    APPXSIGNATURE_P7X,
};

// Given a file name, determine the format of the certificate.
MSIX_CERTIFICATE_FORMAT DetermineCertificateFormat(LPCSTR file);

// Given a format, is a separate private key file required?
bool DoesCertificateFormatRequirePrivateKey(MSIX_CERTIFICATE_FORMAT format);

// Signs a package in-place with the given certificate.
void SignPackage(
    IAppxPackageReader* package,
    MSIX_CERTIFICATE_FORMAT signingCertificateFormat,
    IStream* signingCertificate,
    const char* pass,
    IStream* privateKey);

// Allows signature data to be accumulated, either for creation or validation of a signature.
// The design is intended to enable signing both during and after packaging, as well as 
// to be used during signature validation.
struct SignatureAccumulator
{
    // TODO: Take in hash algorithm and options for what to accumulate
    SignatureAccumulator() = default;

    SignatureAccumulator(const SignatureAccumulator&) = delete;
    SignatureAccumulator& operator=(const SignatureAccumulator&) = delete;

    SignatureAccumulator(SignatureAccumulator&&) = default;
    SignatureAccumulator& operator=(SignatureAccumulator&&) = default;

    // RAII class to encompass a single file's accumulated data.
    struct FileAccumulator
    {
        friend SignatureAccumulator;

        ~FileAccumulator();

        FileAccumulator(const FileAccumulator&) = delete;
        FileAccumulator& operator=(const FileAccumulator&) = delete;

        FileAccumulator(FileAccumulator&&) = delete;
        FileAccumulator& operator=(FileAccumulator&&) = delete;

        inline bool WantsRaw() const { return wantsRaw; }

        bool AccumulateRaw(IStream* stream);
        bool AccumulateRaw(const std::vector<std::uint8_t>& data);

        inline bool WantsZip() const { return wantsZip; }

        bool AccumulateZip(IStream* stream);

    private:
        FileAccumulator(SignatureAccumulator& accumulator, std::string partName, bool createCICatalog);

        SignatureAccumulator& signatureAccumulator;
        std::string name;
        bool wantsRaw = true;
        bool wantsZip = true;
        bool isBlockmap = false;
        bool isContentTypes = false;

        SHA256& GetRawHasher()
        {
            if (!rawHasher)
            {
                rawHasher = std::make_unique<SHA256>();
            }
            return *rawHasher;
        }

        SHA256& GetZipHasher()
        {
            return signatureAccumulator.GetZipHasher();
        }

        std::unique_ptr<SHA256> rawHasher;
    };

    friend FileAccumulator;

    // Gets a file accumulator for the given filename. The caller can then accumulate data
    // into it as it comes in.
    std::unique_ptr<FileAccumulator> GetFileAccumulator(std::string partName);

    ComPtr<IStream> GetCodeIntegrityStream(
        MSIX_CERTIFICATE_FORMAT signingCertificateFormat,
        IStream* signingCertificate,
        IStream* privateKey);

    ComPtr<AppxSignatureObject> GetSignatureObject(IZipWriter* zipWriter);

private:
    // Not currently thread safe; but the hash operation for zips needs to be in order in any
    // case.  So if multi-threaded packing/signing is ever implemented, some external code
    // will keep us safe anyway.
    SHA256& GetZipHasher()
    {
        if (!zipHasher)
        {
            zipHasher = std::make_unique<SHA256>();
        }
        return *zipHasher;
    }

    AppxSignatureObject* GetSignatureObject()
    {
        if (!digests)
        {
            digests = ComPtr<AppxSignatureObject>::Make<AppxSignatureObject>();
        }
        return digests.Get();
    }

    // TODO: Implement CI catalog
    bool createCICatalog = false;
    std::unique_ptr<SHA256> zipHasher;
    ComPtr<AppxSignatureObject> digests;
};

}
