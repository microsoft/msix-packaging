#include <memory>
#include <iostream>
#include <string>
#include <sstream>
#include <locale>
#include <codecvt>

namespace xPlat {

    std::wstring utf8_to_utf16(const std::string& utf8string)
    {
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
        auto converted = std::wstring_convert<std::codecvt_utf8_utf16<unsigned short>, unsigned short>{}.from_bytes(utf8string.data());
        #else
        auto converted = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(utf8string.data()); 
        #endif
        std::wstring result(converted.begin(), converted.end());
        return result;
    }

    std::string utf16_to_utf8(const std::wstring& utf16string)
    {
        auto converted = std::wstring_convert<std::codecvt_utf8<wchar_t>>{}.to_bytes(utf16string.data());
        std::string result(converted.begin(), converted.end());
        return result;
    }

} // namespace xPlat