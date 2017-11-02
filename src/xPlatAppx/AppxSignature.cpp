#include "Exceptions.hpp"
#include "StreamBase.hpp"
#include "StorageObject.hpp"
#include "AppxSignature.hpp"
#include "AppxPackaging.hpp"
#include "ComHelper.hpp"

#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace xPlat {

AppxSignatureObject::AppxSignatureObject(APPX_VALIDATION_OPTION validationOptions, IStream* stream) : 
    VerifierObject(stream), 
    m_validationOptions(validationOptions)
{
    // TODO: Implement
}

IStream* AppxSignatureObject::GetValidationStream(const std::string& part, IStream* stream)
{
    // TODO: Implement -- for now, just pass through.
    return stream;
}

} // namespace xPlat
