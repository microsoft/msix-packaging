#pragma once

#include "MSIXWindows.hpp"
#include "UnicodeConversion.hpp"

namespace MSIX {

    std::uint64_t ConvertVersionStringToUint64(const std::string& versionString);

    std::string ConvertVersionToString(std::uint64_t version);
}

