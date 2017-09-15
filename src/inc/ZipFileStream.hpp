
#include "Exceptions.hpp"
#include "StreamBase.hpp"

#include <string>

namespace xPlat {

    // This represents a raw stream over a file contained in a .zip file.
    class ZipFileStream : public StreamBase
    {
    public:
        enum CompressionType : std::uint16_t { Store = 0, Deflate = 8 };

        // TODO: define what streams to pass in on the .ctor
        ZipFileStream(std::string fileName, std::size_t compressedSize, std::size_t uncompressedSize,
            CompressionType compressionType, std::size_t headerOffset, std::uint32_t crc32) :
            fileName(fileName),
            compressedSize(compressedSize),
            uncompressedSize(uncompressedSize),
            compressionType(compressionType),
            headerOffset(headerOffset),
            crc32(crc32)
        {
        }

    protected:
        std::string fileName;
        std::size_t compressedSize;
        std::size_t uncompressedSize;
        CompressionType compressionType;

        std::size_t headerOffset;
        std::uint32_t crc32;

        std::string sanitizedName;

    };
}