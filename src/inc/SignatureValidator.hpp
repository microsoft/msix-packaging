#pragma once
#include "AppxPackaging.hpp"
#include "AppxSignature.hpp"

#include <vector>
#include <map>

namespace xPlat {

    class SignatureValidator
    {
    public:
        static bool Validate(
            /*in*/ APPX_VALIDATION_OPTION option, 
            /*in*/ IStream *stream, 
            /*inout*/ std::map<xPlat::AppxSignatureObject::DigestName, xPlat::AppxSignatureObject::Digest>& digests);
    };
}