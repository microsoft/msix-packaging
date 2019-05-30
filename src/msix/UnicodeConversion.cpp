//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#include <memory>
#include <iostream>
#include <sstream>

#ifdef WIN32
#include <windows.h>
#include <stringapiset.h>
#endif

#include "UnicodeConversion.hpp"
#include "Exceptions.hpp"

namespace MSIX {

    StringType utf8_to_utf16(const std::string& utf8string)
    {
        return StringConvert{}.from_bytes(utf8string.data());
    }

    std::wstring utf8_to_wstring(const std::string& utf8string)
    {
        if (utf8string.empty()) { return {}; }
        #ifdef WIN32
        int size = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8string.data(), static_cast<int>(utf8string.size()), nullptr, 0);
        ThrowLastErrorIf(size == 0, "Error converting to wstring");
        std::wstring result(size, 0);
        MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8string.data(), static_cast<int>(utf8string.size()), &result[0], size);
        #else
        auto converted = utf8_to_utf16(utf8string);
        std::wstring result(converted.begin(), converted.end());
        #endif
        return result;
    }

    std::u16string utf8_to_u16string(const std::string& utf8string)
    {
        auto converted = utf8_to_utf16(utf8string);
        std::u16string result(converted.begin(), converted.end());
        return result;
    }

    std::string wstring_to_utf8(const std::wstring& utf16string)
    {
        if (utf16string.empty()) { return {}; }
        #ifdef WIN32
        int size = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, utf16string.data(), static_cast<int>(utf16string.size()), nullptr, 0, nullptr, nullptr);
        ThrowLastErrorIf(size == 0, "Error converting to string");
        std::string result(size, 0);
        WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, utf16string.data(), static_cast<int>(utf16string.size()), &result[0], size, nullptr, nullptr);
        #else
        auto result = std::wstring_convert<std::codecvt_utf8<wchar_t>>{}.to_bytes(utf16string.data());
        #endif
        return result;
    }

    std::string u16string_to_utf8(const std::u16string& utf16string)
    {
        #ifdef WIN32
        std::wstring intermediate(utf16string.begin(), utf16string.end());
        auto result = wstring_to_utf8(intermediate);
        #else
        auto result = StringConvert{}.to_bytes(utf16string.data());
        #endif
        return result;
    }

} // namespace MSIX