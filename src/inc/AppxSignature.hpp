#pragma once
#include "ComHelper.hpp"
#include "AppxPackaging.hpp"
#include "VerifierObject.hpp"
#include "StreamBase.hpp"

#include <vector>
#include <map>

namespace xPlat {

    enum class SignatureOrigin
    {
        Windows,    // chains to the Windows RCA
        Store,      // chains to the Microsoft Store RCA
        LOB,        // chains to either Verisign, or Authenticode
        Unknown,    // signed, but unknown origin (e.g. self signed)
        Unsigned    // no signature.
    };

    // Object identifier for the Windows Store certificate. We look for this
    // identifier in the cert EKUs to determine if the cert originates from
    // Windows Store.
    const std::string WindowsStoreOid = "1.3.6.1.4.1.311.76.3.1";

    // APPX-specific header placed in the P7X file, before the actual signature
    const DWORD P7X_FILE_ID = 0x58434b50;

    // Object backed by AppxSignature.p7x
    class AppxSignatureObject : public VerifierObject
    {
    public:        
        enum DigestName : std::uint32_t
        {
            HEAD = 0x41404058, // APPX
            AXPC = 0x41585043, // file records
            AXCD = 0x41584344, // central directory
            AXCT = 0x41584354, // [ContentTypes].xml (uncompressed)
            AXBM = 0x4158424D, // AppxBlockMap.xml (uncompressed)
            AXCI = 0x41584349, // AppxMetadata/CodeIntegrity.cat (uncompressed, optional)
        };

        AppxSignatureObject(APPX_VALIDATION_OPTION validationOptions, IStream* stream);

        IStream* GetValidationStream(const std::string& part, IStream* stream) override;

        using Digest = std::vector<std::uint8_t>;

        SignatureOrigin GetSignatureOrigin() { return m_signatureOrigin; }
        Digest& GetFileRecordsDigest()       { return m_digests[DigestName::AXPC]; }
        Digest& GetCentralDirectoryDigest()  { return m_digests[DigestName::AXCD]; }
        Digest& GetContentTypesDigest()      { return m_digests[DigestName::AXCT]; }
        Digest& GetAppxBlockMapDigest()      { return m_digests[DigestName::AXBM]; }
        Digest& GetCodeIntegrityDigest()     { return m_digests[DigestName::AXCI]; }

    protected:
        std::map<DigestName, Digest> m_digests;
        SignatureOrigin              m_signatureOrigin = SignatureOrigin::Unsigned; // assume unsigned until proven otherwise.
        APPX_VALIDATION_OPTION       m_validationOptions;
    };
} // namespace xPlat