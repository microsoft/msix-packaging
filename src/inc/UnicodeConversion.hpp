//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once

#include <string>

namespace MSIX {

    // converts an input utf8 formatted string into a utf16 formatted string
    #ifdef WIN32
    std::basic_string<unsigned short>
    #else
    std::u16string
    #endif
    utf8_to_utf16(const std::string& utf8string);
    std::wstring utf8_to_wstring(const std::string& utf8string);
    std::u16string utf8_to_u16string(const std::string& utf8string);

    // converts an input utf16 formatted string into a utf8 formatted string
    std::string utf16_to_utf8(const std::wstring& utf16string);

} // namespace MSIX