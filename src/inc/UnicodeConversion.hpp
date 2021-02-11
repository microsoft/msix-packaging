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
        from: https://social.msdn.microsoft.com/Forums/en-US/8f40dcd8-c67f-4eba-9134-a19b9178e481/vs-2015-rc-linker-stdcodecvt-error?forum=vcgeneral
        Posted by Microsoft on 1/7/2016 at 8:12 PM
        On Windows, using std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> will produce:
            error LNK2001: unresolved external symbol "__declspec(dllimport) public: static class std::locale::id std::codecvt
            <char16_t,char,struct _Mbstatet>::id"(__imp_?id@?$codecvt@_SDU_Mbstatet@@@std@@2V0locale@2@A)
        This is a known bug since VS2015 that hasn't being fixed and I don't think is ever going to as <codecvt> is deprecated in C++17
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
    std::string wstring_to_utf8(const std::wstring& utf16string);
    std::string u16string_to_utf8(const std::u16string& utf16string);


} // namespace MSIX