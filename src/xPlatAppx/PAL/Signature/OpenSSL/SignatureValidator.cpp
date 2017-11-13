#include "AppxSignature.hpp"
#include "FileStream.hpp"
#include "SignatureValidator.hpp"

namespace xPlat
{
    bool SignatureValidator::Validate(
        /*in*/ APPX_VALIDATION_OPTION option, 
        /*in*/ IStream *stream, 
        /*inout*/ std::map<xPlat::AppxSignatureObject::DigestName, xPlat::AppxSignatureObject::Digest>& digests)
    {
        // TODO: Implement here...
        throw Exception(Error::NotImplemented);
    }

} // namespace xPlat