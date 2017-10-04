
#include "Exceptions.hpp"
#include "StreamBase.hpp"

#include <string>

namespace xPlat {

    // This represents a raw stream over a file contained in a .zip file.
    class ZipFileStream : public StreamBase
    {
    public:
        // TODO: define what streams to pass in on the .ctor
        ZipFileStream(
            std::string&& fileName,
            std::uint32_t offset, 
            std::uint32_t compressedSize,
            std::uint32_t uncompressedSize,
            bool isCompressed
            ) : 
            m_fileName(std::move(fileName)),
            m_offset(offset),
            m_compressedSize(compressedSize),
            m_uncompressedSize(uncompressedSize),
            m_isCompressed(isCompressed)
        {
        }

    protected:
        std::string m_fileName;

        // TODO: change to uint64_t when adding 4+GB support
        std::uint32_t m_offset;
        std::uint32_t m_compressedSize;
        std::uint32_t m_uncompressedSize;

        bool m_isCompressed = false;
        std::uint64_t m_relativePosition = 0;
    };
}