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

    // Object backed by AppxSignature.p7x
    class AppxSignatureObject : public VerifierObject
    {
    public:        
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
        enum DigestName : std::uint32_t
        {
            HEAD = 0x41404058, // APPX
            AXPC = 0x41585043, // file records
            AXCD = 0x41584344, // central directory
            AXCT = 0x41584354, // [ContentTypes].xml (uncompressed)
            AXBM = 0x4158424D, // AppxBlockMap.xml (uncompressed)
            AXCI = 0x41584349, // AppxMetadata/CodeIntegrity.cat (uncompressed, optional)
        };

        std::map<DigestName, Digest> m_digests;
        SignatureOrigin              m_signatureOrigin = SignatureOrigin::Unsigned; // assume unsigned until proven otherwise.
        APPX_VALIDATION_OPTION       m_validationOptions;
        bool                         m_validatedSignature;
    };
} // namespace xPlat