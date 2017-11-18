#pragma once
#include "AppxPackaging.hpp"
#include "AppxSignature.hpp"
#include "SHA256.hpp"

#include <vector>
#include <map>

namespace xPlat {

    typedef struct DigestHash
    {
        xPlat::AppxSignatureObject::DigestName name;
        std::uint8_t content[HASH_BYTES];
    } DigestHash;

    typedef struct DigestHeader
    {
        xPlat::AppxSignatureObject::DigestName name;
        DigestHash hash[1];
    } DigestHeader;

    class SignatureValidator
    {
    public:
        static bool Validate(
            /*in*/ APPX_VALIDATION_OPTION option, 
            /*in*/ IStream *stream, 
            /*inout*/ std::map<xPlat::AppxSignatureObject::DigestName, xPlat::AppxSignatureObject::Digest>& digests,
            /*inout*/ SignatureOrigin& origin);
    };
}

