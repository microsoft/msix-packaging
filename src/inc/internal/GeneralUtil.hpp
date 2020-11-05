#pragma once

#include "MSIXWindows.hpp"
#include "UnicodeConversion.hpp"

namespace MSIX {

    //
    // Helper class to free string buffers obtained from the packaging APIs.
    //
    template<typename T>
    class Text
    {
    public:
        T** operator&() { return &content; }
        ~Text() { Cleanup(); }
        T* Get() { return content; }

        T* content = nullptr;
    protected:
        void Cleanup() { if (content) { std::free(content); content = nullptr; } }
    };

    std::uint64_t ConvertVersionStringToUint64(const std::string& versionString);

    std::string ConvertVersionToString(std::uint64_t version);

    bool PathIsExistingFolder(std::string path);

    bool PathIsExistingFile(std::string path);

}

