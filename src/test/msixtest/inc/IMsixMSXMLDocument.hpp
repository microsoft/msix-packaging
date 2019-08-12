//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once
#include "AppxPackaging.hpp"

namespace MsixTest {
    namespace XML
    {
        void ValidateIMsixMSXMLDocument(const ComPtr<IAppxManifestReader>& manifestReader);
    }
}
