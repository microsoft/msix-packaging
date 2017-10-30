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

AppxSignatureObject::AppxSignatureObject(APPX_VALIDATION_OPTION validationOptions, std::shared_ptr<StreamBase> stream) : 
    VerifierObject(std::move(stream)), 
    m_validationOptions(validationOptions)
{
    // TODO: Implement
}

std::shared_ptr<StreamBase> AppxSignatureObject::GetValidationStream(const std::string& part, std::shared_ptr<StreamBase> stream)
{
    // TODO: Implement -- for now, just pass through.
    return stream;
}

} // namespace xPlat
