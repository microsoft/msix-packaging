#include "GeneralUtil.hpp"

namespace MSIX{
    
    std::uint64_t ConvertVersionStringToUint64(const std::string& versionString)
    {
        std::uint64_t version = 0;
        size_t position = 0;
        auto nextPeriod = versionString.find('.', position);
        version = (std::uint64_t)std::stoi(versionString.substr(position, nextPeriod)) << 0x30;

        position = nextPeriod + 1;
        nextPeriod = versionString.find('.', position);
        version += (std::uint64_t)std::stoi(versionString.substr(position, nextPeriod)) << 0x20;

        position = nextPeriod + 1;
        nextPeriod = versionString.find('.', position);
        version += (std::uint64_t)std::stoi(versionString.substr(position, nextPeriod)) << 0x10;

        position = nextPeriod + 1;
        nextPeriod = versionString.find('.', position);
        version += (std::uint64_t)std::stoi(versionString.substr(position, nextPeriod));

        return version;
    }

    std::string ConvertVersionToString(std::uint64_t version)
    {
        return std::to_string((version >> 0x30) & 0xFFFF) + "."
            + std::to_string((version >> 0x20) & 0xFFFF) + "."
            + std::to_string((version >> 0x10) & 0xFFFF) + "."
            + std::to_string((version) & 0xFFFF);
    }
}
