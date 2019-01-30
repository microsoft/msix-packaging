//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#include "MSIXWindows.hpp"
#include "Exceptions.hpp"
#include "StreamBase.hpp"
#include "StorageObject.hpp"
#include "AppxSignature.hpp"
#include "AppxPackaging.hpp"
#include "HashStream.hpp"
#include "ComHelper.hpp"
#include "SignatureValidator.hpp"
#include "BlockMapStream.hpp"

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <set>

namespace MSIX {

void AppxSignatureObject::ValidateDigestHeader(DigestHeader* header, std::size_t numberOfHashes, std::size_t modHashes)
{
    ThrowErrorIf(Error::SignatureInvalid, (
        (header->name != DigestName::HEAD) &&
        (numberOfHashes != 4 && numberOfHashes != 5) &&
        (modHashes != 0)
    ), "bad signature data");
    
    Digest* digest = nullptr;
    std::uint8_t set[5] = { 0, 0, 0, 0, 0 };
    for (unsigned i = 0; i < numberOfHashes; i++)
    {        
        switch (header->hash[i].name)
        {
        case DigestName::AXPC:
            digest = &m_FileRecords;
            set[0] = 1;
            break;
        case DigestName::AXCD:
            digest = &m_CentralDirectory;
            set[1] = 1;
            break;            
        case DigestName::AXCT:
            digest = &m_ContentTypes;
            set[2] = 1;
            break;
        case DigestName::AXBM:
            digest = &m_AppxBlockMap;
            set[3] = 1;
            break;
        case DigestName::AXCI:
            digest = &m_CodeIntegrity;
            set[4] = 1;
            break;
        default:
            ThrowError(MSIX::Error::SignatureInvalid);
        }
        digest->assign(&header->hash[i].content[0], &header->hash[i].content[HASH_BYTES]);
    }
    std::uint8_t digestsFound = 0;
    for (auto i : set) { digestsFound+= i;}
    ThrowErrorIf(Error::SignatureInvalid, (digestsFound != 4 && digestsFound != 5), "Digest hashes missing entries");
}

AppxSignatureObject::AppxSignatureObject(IMsixFactory* factory, MSIX_VALIDATION_OPTION validationOptions, const ComPtr<IStream>& stream) : 
    m_stream(stream), 
    m_validationOptions(validationOptions)
{
    m_hasDigests = SignatureValidator::Validate(factory, validationOptions, stream, this, m_signatureOrigin, m_publisher);

    if (0 == (validationOptions & MSIX_VALIDATION_OPTION::MSIX_VALIDATION_OPTION_SKIPSIGNATURE))
    {   // reset the source stream back to the beginning after validating it.
        LARGE_INTEGER li{0};    
        ThrowHrIfFailed(stream->Seek(li, StreamBase::Reference::START, nullptr));
    }
}

ComPtr<IStream>  AppxSignatureObject::GetValidationStream(const std::string& part, const ComPtr<IStream>& stream)
{
    if (m_hasDigests)
    {
        if (part == std::string("AppxBlockMap.xml"))
        {   // This stream implementation will throw if the underlying stream does not match the digest
            return ComPtr<IStream>::Make<HashStream>(stream, this->GetAppxBlockMapDigest());
        }
        else if (part == std::string("[Content_Types].xml"))
        {   // This stream implementation will throw if the underlying stream does not match the digest'
            return ComPtr<IStream>::Make<HashStream>(stream, this->GetContentTypesDigest());
        }
        else if (part == std::string("AppxMetadata/CodeIntegrity.cat"))
        {   // This stream implementation will throw if the underlying stream does not match the digest
            return ComPtr<IStream>::Make<HashStream>(stream, this->GetCodeIntegrityDigest());
        }
        // TODO: unnamed stream for central directory?
    }
    return stream;
}

} // namespace MSIX
