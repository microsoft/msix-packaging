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

    class SignatureValidator
    {
    public:
        static bool Validate(
            IMsixFactory* factory,
            MSIX_VALIDATION_OPTION option, 
            const ComPtr<IStream>& stream,
            AppxSignatureObject* signatureObject,
            SignatureOrigin& origin,
            std::string& publisher);
    };
}

