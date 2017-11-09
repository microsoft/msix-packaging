#pragma once

#include <string>

namespace xPlat {

    // converts an input utf8 formatted string into a utf16 formatted string
    std::wstring utf8_to_utf16(const std::string& utf8string);

    // converts an input utf16 formatted string into a utf8 formatted string
    std::string utf16_to_utf8(const std::wstring& utf16string);

} // namespace xPlat