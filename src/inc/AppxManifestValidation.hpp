//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once

#include "IXml.hpp"

namespace MSIX {

    // Verification done on an AppxManifest that cannot be done in the schema.
    struct AppxManifestValidation
    {
        // Returns true if the given identifier is valid.
        static bool IsIdentifierValid(const std::string& identifier);

        // Validates the entire manifest; after the manifest has been validated by schema.
        static void ValidateManifest(IXmlDom* manifest);
    };
}
