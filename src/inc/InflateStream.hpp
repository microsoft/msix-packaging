
#pragma once
#ifdef WIN32
#include "zlib.h"
#else
#include <zlib.h>
#endif

#include "Exceptions.hpp"
#include "StreamBase.hpp"
#include "ComHelper.hpp"

//TODO: windows.h defines max and min... 
#undef max
#undef min
#include <string>
#include <map>
#include <functional>

namespace xPlat {

    // This represents a LZW-compressed stream
    class InflateStream : public StreamBase
    {
    public:
        InflateStream(IStream* stream, std::uint64_t uncompressedSize);

        HRESULT Seek(LARGE_INTEGER move, DWORD origin, ULARGE_INTEGER *newPosition) override;
        HRESULT Read(void* buffer, ULONG countBytes, ULONG* bytesRead) override;
        HRESULT Write(void const *buffer, ULONG countBytes, ULONG *bytesWritten) override
        {
            return static_cast<HRESULT>(Error::NotImplemented);
        }

        HRESULT STDMETHODCALLTYPE GetSize(UINT64* size) override
        {
            if (size) { *size = m_uncompressedSize; }
            return static_cast<HRESULT>(Error::OK);
        }

        HRESULT STDMETHODCALLTYPE GetCompressionOption(APPX_COMPRESSION_OPTION* compressionOption) override
        {   // The underlying ZipFileStream object knows, so go ask it.
            return ResultOf([&]{
                auto appxFile = m_stream.As<IAppxFile>();
                return appxFile->GetCompressionOption(compressionOption);
            });
        }

    protected:
        void Cleanup();

        static const ULONG BUFFERSIZE = 4096;
        enum class State : std::uint8_t
        {
            UNINITIALIZED = 0,
            READY_TO_READ,
            READY_TO_INFLATE,
            READY_TO_COPY,
            CLEANUP
        };

        State m_previous = State::UNINITIALIZED;
        State m_state    = State::UNINITIALIZED;
        std::map<State, std::function<std::tuple<bool, State>(void* buffer, ULONG countBytes)>> m_stateMachine;

        ComPtr<IStream> m_stream;
        std::uint64_t   m_seekPosition = 0;
        std::uint64_t   m_uncompressedSize;
        std::size_t     m_bytesRead = 0;
        std::uint8_t*   m_startCurrentBuffer = nullptr;
        std::uint8_t    m_compressedBuffer[InflateStream::BUFFERSIZE];
        std::uint8_t    m_inflateWindow[InflateStream::BUFFERSIZE];
        std::size_t     m_inflateWindowPosition = 0;
        std::uint64_t   m_fileCurrentWindowPositionEnd = 0;
        std::uint64_t   m_fileCurrentPosition = 0;
        z_stream        m_zstrm;
        int             m_zret;
    };
}
