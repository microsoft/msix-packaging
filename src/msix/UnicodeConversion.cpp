//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#include <memory>
#include <iostream>
#include <sstream>
#include "UnicodeConversion.hpp"

namespace MSIX {

    StringType utf8_to_utf16(const std::string& utf8string)
    {
        return StringConvert{}.from_bytes(utf8string.data());
    }

    std::wstring utf8_to_wstring(const std::string& utf8string)
    {
        auto converted = utf8_to_utf16(utf8string);
        std::wstring result(converted.begin(), converted.end());
        return result;
    }

    std::u16string utf8_to_u16string(const std::string& utf8string)
    {
        auto converted = utf8_to_utf16(utf8string);
        std::u16string result(converted.begin(), converted.end());
        return result;
    }

    std::string utf16_to_utf8(const std::wstring& utf16string)
    {
        auto converted = std::wstring_convert<std::codecvt_utf8<wchar_t>>{}.to_bytes(utf16string.data());
        std::string result(converted.begin(), converted.end());
        return result;
    }
} // namespace MSIX