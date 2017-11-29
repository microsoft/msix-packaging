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

    namespace OID {
        // Object identifier for the Windows Store certificate. We look for this
        // identifier in the cert EKUs to determine if the cert originates from
        // Windows Store.
        const std::string WindowsStore    = "1.3.6.1.4.1.311.76.3.1";

        // https://support.microsoft.com/en-us/kb/287547
        const std::string IndirectData    = "1.3.6.1.4.1.311.2.1.4";
        const std::string StatementType   = "1.3.6.1.4.1.311.2.1.11";
        const std::string SpOpusInfo      = "1.3.6.1.4.1.311.2.1.12";
        const std::string SipInfo         = "1.3.6.1.4.1.311.2.1.30";
    } // namespace OID

    // APPX-specific header placed in the P7X file, before the actual signature
    const DWORD P7X_FILE_ID = 0x58434b50;

    // Object backed by AppxSignature.p7x
    class AppxSignatureObject : public ComClass<AppxSignatureObject, IVerifierObject>
    {
    public:        
        enum DigestName : std::uint32_t
        {
            HEAD = 0x58404041, // APPX
            AXPC = 0x43505841, // file records
            AXCD = 0x44435841, // central directory
            AXCT = 0x54435841, // [ContentTypes].xml (uncompressed)
            AXBM = 0x4D425841, // AppxBlockMap.xml (uncompressed)
            AXCI = 0x49435841, // AppxMetadata/CodeIntegrity.cat (uncompressed, optional)
        };

        AppxSignatureObject(APPX_VALIDATION_OPTION validationOptions, IStream* stream);

        // IVerifierObject
        bool HasStream()     override { return m_stream.Get() != nullptr; }
        IStream* GetStream() override { return m_stream.Get(); }        
        IStream* GetValidationStream(const std::string& part, IStream* stream) override;

        using Digest = std::vector<std::uint8_t>;

        SignatureOrigin GetSignatureOrigin() { return m_signatureOrigin; }
        Digest& GetFileRecordsDigest()       { return m_digests[DigestName::AXPC]; }
        Digest& GetCentralDirectoryDigest()  { return m_digests[DigestName::AXCD]; }
        Digest& GetContentTypesDigest()      { return m_digests[DigestName::AXCT]; }
        Digest& GetAppxBlockMapDigest()      { return m_digests[DigestName::AXBM]; }
        Digest& GetCodeIntegrityDigest()     { return m_digests[DigestName::AXCI]; }

    protected:
        bool                         m_hasDigests;
        std::map<DigestName, Digest> m_digests;
        SignatureOrigin              m_signatureOrigin = SignatureOrigin::Unsigned; // assume unsigned until proven otherwise.
        APPX_VALIDATION_OPTION       m_validationOptions;
        ComPtr<IStream>              m_stream;
    };
} // namespace xPlat