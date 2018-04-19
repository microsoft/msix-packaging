//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once

#include <string>
#include <locale>
#include <codecvt>

namespace MSIX {    
    /*
        from: https://connect.microsoft.com/VisualStudio/feedback/details/1403302/unresolved-external-when-using-codecvt-utf8
        Posted by Microsoft on 2/16/2016 at 11:49 AM
        <snip>
        A workaround is to replace 'char32_t' with 'unsigned int'. In VS2013, char32_t was a typedef of 'unsigned int'.
        In VS2015, char32_t is a distinct type of it's own. Switching your use of 'char32_t' to 'unsigned int' will get
        you the old behavior from earlier versions and won't trigger a missing export error.

        There is also a similar error to this one with 'char16_t' that can be worked around using 'unsigned short'.
        <snip>
    */
    #ifdef WIN32
    using StringType = std::basic_string<unsigned short>;
    using StringConvert = std::wstring_convert<std::codecvt_utf8_utf16<unsigned short>, unsigned short>;
    #else
    using StringType = std::u16string;
    using StringConvert = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>;
    #endif

    StringType utf8_to_utf16(const std::string& utf8string);

    // converts an input utf8 formatted string into a utf16 formatted string
    std::wstring utf8_to_wstring(const std::string& utf8string);
    std::u16string utf8_to_u16string(const std::string& utf8string);

    // converts an input utf16 formatted string into a utf8 formatted string
    std::string utf16_to_utf8(const std::wstring& utf16string);

} // namespace MSIX