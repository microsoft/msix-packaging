
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
#include <map>
#include <functional>


namespace xPlat {
  
    // This represents a LZW-compressed stream
    class InflateStream : public StreamBase
    {
    public:
        //static std::shared_ptr<StreamBase> MakeInflateStream(std::shared_ptr<StreamBase> stream, std::uint64_t compressedSize, std::uint64_t uncompressedSize);

        InflateStream(
            std::shared_ptr<StreamBase> stream,
            std::uint64_t uncompressedSize
        );

        ~InflateStream() override;

        void Write(std::size_t size, const std::uint8_t* bytes) override;
        std::size_t Read(std::size_t size, const std::uint8_t* bytes) override;
        void Seek(std::uint64_t offset, Reference where) override;
        int Ferror() override;
        bool Feof() override;
        std::uint64_t Ftell()  override;

    protected:
        void Cleanup();

        static const unsigned int BUFFERSIZE = 4096;
        enum class State : std::uint8_t
        {
            UNINITIALIZED = 0,
            READY_TO_READ,
            READY_TO_INFLATE,
            READY_TO_COPY,
            CLEANUP
        };

        State m_previous = State::UNINITIALIZED;
        State m_state = State::UNINITIALIZED;
        std::map<State, std::function<std::tuple<bool, State>(std::size_t cbReadBuffer, const std::uint8_t* readBuffer)>> m_stateMachine;

        std::uint64_t               m_seekPosition = 0;
        std::shared_ptr<StreamBase> m_stream;
        std::uint64_t               m_uncompressedSize;
        std::size_t                 m_bytesRead = 0;

        z_stream m_zstrm;
        int m_zret;
        std::uint8_t  m_compressedBuffer[InflateStream::BUFFERSIZE];
        std::uint8_t  m_inflateWindow[InflateStream::BUFFERSIZE];
        std::size_t   m_inflateWindowPosition = 0;

        std::uint64_t m_fileCurrentWindowPositionEnd = 0;
        std::uint64_t m_fileCurrentPosition = 0;
    };
}
