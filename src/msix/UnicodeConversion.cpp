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

namespace MSIX {

    StringType utf8_to_utf16(const std::string& utf8string)
    {
        return StringConvert{}.from_bytes(utf8string.data());
    }

    std::wstring utf8_to_wstring(const std::string& utf8string)
    {
        #ifdef WIN32
        int size = MultiByteToWideChar(CP_UTF8, 0, utf8string.data(), utf8string.size(), nullptr, 0);
        std::wstring result(size, 0);
        MultiByteToWideChar(CP_UTF8, 0, utf8string.data(), utf8string.size(), &result[0], size);
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

    std::string utf16_to_utf8(const std::wstring& utf16string)
    {
        #ifdef WIN32
        int size = WideCharToMultiByte(CP_UTF8, 0, utf16string.data(), utf16string.size(), nullptr, 0, nullptr, nullptr);
        std::string result(size, 0);
        WideCharToMultiByte(CP_UTF8, 0, utf16string.data(), utf16string.size(), &result[0], size, nullptr, nullptr);
        #else
        auto converted = std::wstring_convert<std::codecvt_utf8<wchar_t>>{}.to_bytes(utf16string.data());
        std::string result(converted.begin(), converted.end());
        #endif
        return result;
    }
} // namespace MSIX