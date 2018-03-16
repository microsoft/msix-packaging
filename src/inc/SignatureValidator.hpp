//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once
#include "AppxPackaging.hpp"
#include "AppxSignature.hpp"
#include "SHA256.hpp"

#include <vector>
#include <map>

namespace MSIX {

    typedef struct DigestHash
    {
        MSIX::AppxSignatureObject::DigestName name;
        std::uint8_t content[HASH_BYTES];
    } DigestHash;

    typedef struct DigestHeader
    {
        MSIX::AppxSignatureObject::DigestName name;
        DigestHash hash[1];
    } DigestHeader;

    class SignatureValidator
    {
    public:
        static bool Validate(
            IMSIXFactory* factory,
            MSIX_VALIDATION_OPTION option, 
            IStream *stream, 
            std::map<MSIX::AppxSignatureObject::DigestName, MSIX::AppxSignatureObject::Digest>& digests,
            SignatureOrigin& origin,
            std::string& publisher);
    };
}

