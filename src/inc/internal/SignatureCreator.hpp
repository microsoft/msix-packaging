//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once
#include "ComHelper.hpp"
#include "AppxPackaging.hpp"
#include "AppxSignature.hpp"

namespace MSIX {

    class SignatureCreator
    {
    public:
        // Creates a signature stream from the digests, signed by the certificate.
        static ComPtr<IStream> Sign(
            AppxSignatureObject* digests,
            MSIX_CERTIFICATE_FORMAT signingCertificateFormat,
            IStream* signingCertificate,
            const char* pass,
            IStream* privateKey);
    };
}
