
#pragma once
#ifdef WIN32
#define ZLIB_WINAPI
#endif

#include "Exceptions.hpp"
#include "StreamBase.hpp"
#include "StreamBase.hpp"
#ifdef PLATFORM_APPLE
#include <zlib.h>
#else
#include "..\..\lib\zlib\zlib.h"
#endif

#undef max
#include <string>

namespace xPlat {
  
    // This represents a LZW-compressed stream
    class InflateStream : public StreamBase
    {
    public:
        //static std::shared_ptr<StreamBase> MakeInflateStream(std::shared_ptr<StreamBase> stream, std::uint64_t compressedSize, std::uint64_t uncompressedSize);

        InflateStream(
            std::shared_ptr<StreamBase> stream,
            std::uint64_t compressedSize,
            std::uint64_t uncompressedSize
        );

        void Write(std::size_t size, const std::uint8_t* bytes) override;
        std::size_t Read(std::size_t size, const std::uint8_t* bytes) override;
        void Seek(std::uint64_t offset, Reference where) override;
        int Ferror() override;
        bool Feof() override;
        std::uint64_t Ftell()  override;

    protected:
        static const unsigned int BUFFERSIZE = 4096;

        std::uint64_t m_seekPosition = 0;
        std::shared_ptr<StreamBase> m_stream;
        std::uint64_t m_compressedSize;
        std::uint64_t m_uncompressedSize;

        std::uint8_t m_state;
        z_stream m_zstrm;
        int m_zret;
        std::uint8_t m_compressedBuffer[InflateStream::BUFFERSIZE];
        std::uint8_t  m_uncompressedBuffer[InflateStream::BUFFERSIZE];
        std::uint64_t m_offsetOfUncompressedWindow;
    };
}
