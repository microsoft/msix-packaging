//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#include "SignatureCreator.hpp"

namespace MSIX
{
    ComPtr<IStream> Sign(
        AppxSignatureObject* digests,
        MSIX_CERTIFICATE_FORMAT signingCertificateFormat,
        IStream* signingCertificate,
        IStream* privateKey)
    {
        NOTIMPLEMENTED
    }
} // namespace MSIX
