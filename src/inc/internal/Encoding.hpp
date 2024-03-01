//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
//
#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace MSIX { namespace Encoding {

    std::string DecodeFileName(const std::string& fileName);
    std::string EncodeFileName(const std::string& fileName);

    std::string Base32Encoding(const std::vector<std::uint8_t>& bytes);
    std::vector<std::uint8_t> GetBase64DecodedValue(const std::string& value);

} /*Encoding */ } /* MSIX */
