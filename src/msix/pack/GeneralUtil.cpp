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

    bool PathIsExistingFolder(std::string path)
    {
        //Get fulloutputpath from path if needed
        DWORD attributes = GetFileAttributes(utf8_to_wstring(path).c_str());
        return ((attributes != INVALID_FILE_ATTRIBUTES) && ((attributes & FILE_ATTRIBUTE_DIRECTORY) != 0));
    }

    bool PathIsExistingFile(std::string path)
    {
        //Get fulloutputpath from path if needed
        DWORD attributes = GetFileAttributes(utf8_to_wstring(path).c_str());
        return ((attributes != INVALID_FILE_ATTRIBUTES) && ((attributes & FILE_ATTRIBUTE_DIRECTORY) == 0));
    }

}
