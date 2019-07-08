//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#include "Signing.hpp"
#include "Exceptions.hpp"
#include "FileStream.hpp"
#include "AppxPackageObject.hpp"
#include "StorageObject.hpp"
#include "ContentTypeWriter.hpp"
#include "AppxPackageWriter.hpp"
#include "StreamHelper.hpp"
#include "SHA256HashStream.hpp"

#include <algorithm>
#include <string>

namespace MSIX
{

// Given a file name, determine the format of the certificate.
MSIX_CERTIFICATE_FORMAT DetermineCertificateFormat(LPCSTR file)
{
    std::string fileStr{ file };

    // Since we only have one supported format currently, just go directly for it.
    if (fileStr.length() > 4)
    {
        std::string ext = fileStr.substr(fileStr.length() - 4);
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

        if (ext == ".pfx")
        {
            return MSIX_CERTIFICATE_FORMAT::MSIX_CERTIFICATE_FORMAT_PFX;
        }
    }

    return MSIX_CERTIFICATE_FORMAT::MSIX_CERTIFICATE_FORMAT_UNKNOWN;
}

// Given a format, is a separate private key file required?
bool DoesCertificateFormatRequirePrivateKey(MSIX_CERTIFICATE_FORMAT format)
{
    switch (format)
    {
    case MSIX_CERTIFICATE_FORMAT::MSIX_CERTIFICATE_FORMAT_PFX:
        return false;
    }

    UNEXPECTED
}

// Signs a package in-place with the given certificate.
void SignPackage(
    IAppxPackageReader* package,
    MSIX_CERTIFICATE_FORMAT signingCertificateFormat,
    IStream* signingCertificate,
    IStream* privateKey)
{
    // TODO: Dissimenate or delete comment
    // Steps:
    //  Create digest accumulator (AppxSipCreateIndirectData)
    //      Hashes are for [in order]:
    //          Package Content [hash of everything but the central directory, except signature file]
    //          Central Directory [Containing everything but the signature file]
    //          Content Types [final file stream, must include signature and catalog types]
    //          Block Map [full stream]
    //          CI Catalog [full stream] [optional]
    //  Extract Content Types for editing
    //  Remove content types, any existing catalog and signature
    //  Spin through all files, passing them along to accumulator
    //  Create catalog if needed (also adds its own digest) (CreateSignedPEFilesCatalog)
    //  [MERGE WITH CONCURRENT PACKAGE WRITE PATH HERE]
    //  Collect last few digests (non-file based ones)
    //  Create signature stream
    //  Append content types, catalog, and signature
    //  Write zip central directory back

    std::unique_ptr<SignatureAccumulator> signatureAccumulator = std::make_unique<SignatureAccumulator>();

    // Get the publisher from the manifest for verifying later
    // TODO: Figure out that flow from both paths...

    // Send all of the files to the accumulator, skipping the files modified by signing
    auto packageAsIPackage = ComPtr<IPackage>::From(package);
    auto packageAsIStorageObject = ComPtr<IStorageObject>::From(package);
    auto underlyingStorage = packageAsIPackage->GetUnderlyingStorageObject();
    auto underlyingZipObject = underlyingStorage.As<IZipObject>();
    auto filenames = underlyingStorage->GetFileNames(FileNameOptions::All);

    for (const auto& filename : filenames)
    {
        // These are the files created by signing; they are not to be included from the original package.
        if (std::find(signingModifiedFiles.cbegin(), signingModifiedFiles.cend(), filename) != signingModifiedFiles.cend())
        {
            continue;
        }

        auto fileAccumulator = signatureAccumulator->GetFileAccumulator(filename);

        if (fileAccumulator->WantsRaw())
        {
            auto validatedStream = packageAsIStorageObject->GetFile(filename);
            fileAccumulator->AccumulateRaw(validatedStream.Get());
        }

        if (fileAccumulator->WantsZip())
        {
            auto entireZipStream = underlyingZipObject->GetEntireZipFileStream(filename);
            fileAccumulator->AccumulateZip(entireZipStream.Get());
        }
    }

    // Extract content types for editing, then create a new writer that will be ready to append.
    auto contentTypesStream = underlyingStorage->GetFile(CONTENT_TYPES_XML);
    ContentTypeWriter contentTypeWriter{ contentTypesStream.Get() };

    // Create a package writer from the reader, giving it our objects. With this, the new package writer will
    // be at the same point as it would be if we were doing signing while creating the package. Then we just
    // continue that process with Close.
    auto packageWriter = ComPtr<IPackageWriter>::Make<AppxPackageWriter>(packageAsIPackage.Get(), std::move(signatureAccumulator), std::move(contentTypeWriter));

    packageWriter->Close(signingCertificateFormat, signingCertificate, privateKey);
}

// SignatureAccumulator

std::unique_ptr<SignatureAccumulator::FileAccumulator> SignatureAccumulator::GetFileAccumulator(std::string partName)
{
    return std::unique_ptr<SignatureAccumulator::FileAccumulator>{ new FileAccumulator{ *this, std::move(partName), createCICatalog } };
}

ComPtr<IStream> SignatureAccumulator::GetCodeIntegrityStream(
    MSIX_CERTIFICATE_FORMAT signingCertificateFormat,
    IStream* signingCertificate,
    IStream* privateKey)
{
    if (!createCICatalog)
    {
        return {};
    }

    NOTIMPLEMENTED
}

ComPtr<AppxSignatureObject> SignatureAccumulator::GetSignatureObject(IZipWriter* zipWriter)
{
    ComPtr<AppxSignatureObject> result = GetSignatureObject();

    // Blockmap and Content Types hashes get set directly by the FileAccumulator closing out.
    // The CI catalog hash similarly gets set when the code integrity stream is created.
    // This leaves only the full package hash and the central directory hash.

    // Simply copy the zip hash over
    result->GetFileRecordsDigest() = GetZipHasher().Get();

    // Create the central directory as it currently exists and hash it
    ComPtr<SHA256HashStream> cdHash = ComPtr<SHA256HashStream>::Make<SHA256HashStream>();
    zipWriter->WriteCentralDirectoryToStream(cdHash.Get());
    result->GetCentralDirectoryDigest() = cdHash->GetHash();

    return result;
}

// FileAccumulator

SignatureAccumulator::FileAccumulator::FileAccumulator(SignatureAccumulator& accumulator, std::string partName, bool createCICatalog) :
    signatureAccumulator(accumulator), name(std::move(partName))
{
    // We always need the blockmap and content types raw data, and we only need to see the other
    // files if we are creating the CI catalog.
    if (name == APPXBLOCKMAP_XML)
    {
        isBlockmap = true;
    }
    else if (name == CONTENT_TYPES_XML)
    {
        isContentTypes = true;
    }
    else if (name == APPXSIGNATURE_P7X)
    {
        // Ignore the signature file when it comes in
        wantsRaw = false;
        wantsZip = false;
    }
    else if (!createCICatalog)
    {
        wantsRaw = false;
    }
}

SignatureAccumulator::FileAccumulator::~FileAccumulator()
{
    if (isBlockmap)
    {
        signatureAccumulator.GetSignatureObject()->GetAppxBlockMapDigest() = GetRawHasher().Get();
    }
    else if (isContentTypes)
    {
        signatureAccumulator.GetSignatureObject()->GetContentTypesDigest() = GetRawHasher().Get();
    }
    else
    {
        // TODO: Implement CI catalog
    }
}

bool SignatureAccumulator::FileAccumulator::AccumulateRaw(IStream* stream)
{
    if (wantsRaw)
    {
        // These just need their entire contents hashed
        if (isBlockmap || isContentTypes)
        {
            auto& hasher = GetRawHasher();

            for (const auto& bytes : Helper::StreamProcessor{ stream, 1 << 20 })
            {
                hasher.Add(bytes);
            }
        }
        else
        {
            // Only other reason to want raw is to inspect this for a PE header
            // TODO: But CI catalog not yet implemented
            NOTIMPLEMENTED
        }
    }

    return wantsRaw;
}

bool SignatureAccumulator::FileAccumulator::AccumulateRaw(const std::vector<std::uint8_t>& data)
{
    if (wantsRaw)
    {
        // These just need their entire contents hashed
        if (isBlockmap || isContentTypes)
        {
            GetRawHasher().Add(data);
        }
        else
        {
            // Only other reason to want raw is to inspect this for a PE header
            // TODO: But CI catalog not yet implemented
            NOTIMPLEMENTED
        }
    }

    return wantsRaw;
}

bool SignatureAccumulator::FileAccumulator::AccumulateZip(IStream* stream)
{
    if (wantsZip)
    {
        auto& hasher = GetZipHasher();

        for (const auto& bytes : Helper::StreamProcessor{ stream, 1 << 20 })
        {
            hasher.Add(bytes);
        }
    }

    return wantsZip;
}

}
