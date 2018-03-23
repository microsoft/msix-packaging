//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once
#include "ComHelper.hpp"
#include "AppxPackaging.hpp"
#include "VerifierObject.hpp"
#include "StreamBase.hpp"
#include "AppxFactory.hpp"

namespace MSIX {

    enum class SignatureOrigin
    {
        Windows,    // chains to the Windows RCA
        Store,      // chains to the Microsoft Store RCA
        LOB,        // chains to either Verisign, or Authenticode
        Unknown,    // signed, but unknown origin (e.g. self signed)
        Unsigned    // no signature.
    };

    class OID 
    {
    public:
        // Object identifier for the Windows Store certificate. We look for this
        // identifier in the cert EKUs to determine if the cert originates from
        // Windows Store.
        static const char* WindowsStore()   { return "1.3.6.1.4.1.311.76.3.1"; }
    };

    // APPX-specific header placed in the P7X file, before the actual signature
    const DWORD P7X_FILE_ID = 0x58434b50;

    enum class DigestName : std::uint32_t
    {
        HEAD = 0x58505041, // APPX
        AXPC = 0x43505841, // file records
        AXCD = 0x44435841, // central directory
        AXCT = 0x54435841, // [ContentTypes].xml (uncompressed)
        AXBM = 0x4D425841, // AppxBlockMap.xml (uncompressed)
        AXCI = 0x49435841, // AppxMetadata/CodeIntegrity.cat (uncompressed, optional)
    };    

    const unsigned HASH_BYTES = 32;

    struct DigestHash
    {
        DigestName name;
        std::uint8_t content[HASH_BYTES];
    };

    struct DigestHeader
    {
        DigestName name;
        DigestHash hash[1];
    };    

    // Object backed by AppxSignature.p7x
    class AppxSignatureObject : public ComClass<AppxSignatureObject, IVerifierObject>
    {
    public:

        AppxSignatureObject(IMSIXFactory* factory, MSIX_VALIDATION_OPTION validationOptions,const ComPtr<IStream>& stream);

        // IVerifierObject
        const std::string& GetPublisher() override  { return m_publisher; }
        bool HasStream() override                   { return !!m_stream; }
        ComPtr<IStream> GetStream() override        { return m_stream; }
        ComPtr<IStream> GetValidationStream(const std::string& part, const ComPtr<IStream>& stream) override;
        const std::string GetPackageFullName() override { NOTSUPPORTED; }

        void ValidateDigestHeader(DigestHeader* header, std::size_t numberOfHashes, std::size_t modHashes);

        SignatureOrigin GetSignatureOrigin() { return m_signatureOrigin; }

        using Digest = std::vector<std::uint8_t>;        
        Digest& GetFileRecordsDigest()       { return m_FileRecords; }
        Digest& GetCentralDirectoryDigest()  { return m_CentralDirectory; }
        Digest& GetContentTypesDigest()      { return m_ContentTypes; }
        Digest& GetAppxBlockMapDigest()      { return m_AppxBlockMap; }
        Digest& GetCodeIntegrityDigest()     { return m_CodeIntegrity; }

    protected:
        bool                         m_hasDigests;
        Digest                       m_FileRecords;
        Digest                       m_CentralDirectory;
        Digest                       m_ContentTypes;
        Digest                       m_AppxBlockMap;
        Digest                       m_CodeIntegrity;
        SignatureOrigin              m_signatureOrigin = SignatureOrigin::Unsigned; // assume unsigned until proven otherwise.
        MSIX_VALIDATION_OPTION       m_validationOptions;
        ComPtr<IStream>              m_stream;
        std::string                  m_publisher;
    };
} // namespace MSIX