//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once
#ifdef WIN32
#include "zlib.h"
#else
#include <zlib.h>
#endif

#include "Exceptions.hpp"
#include "StreamBase.hpp"
#include "ComHelper.hpp"

// Windows.h defines max and min... 
#undef max
#undef min
#include <string>
#include <map>
#include <functional>
#include <vector>

namespace MSIX {

    // This represents a LZW-compressed stream
    class InflateStream final : public StreamBase
    {
    public:
        InflateStream(const ComPtr<IStream>& stream, std::uint64_t uncompressedSize);
        ~InflateStream();

        HRESULT STDMETHODCALLTYPE Seek(LARGE_INTEGER move, DWORD origin, ULARGE_INTEGER *newPosition) noexcept override;
        HRESULT STDMETHODCALLTYPE Read(void* buffer, ULONG countBytes, ULONG* bytesRead) noexcept override;
        HRESULT STDMETHODCALLTYPE Write(void const *buffer, ULONG countBytes, ULONG *bytesWritten) noexcept override
        {
            return static_cast<HRESULT>(Error::NotImplemented);
        }

        // IStreamInternal
        std::uint64_t GetSizeOnZip() override
        {   // The underlying ZipFileStream object knows, so go ask it.
            return m_stream.As<IStreamInternal>()->GetSizeOnZip();
        }

        bool IsCompressed() override
        {   // The underlying ZipFileStream object knows, so go ask it.
            return m_stream.As<IStreamInternal>()->IsCompressed();
        }

        std::string GetName() override
        {   // The underlying ZipFileStream object knows, so go ask it.
            return m_stream.As<IStreamInternal>()->GetName();
        }

        void Cleanup();
        
        enum class State : size_t
        {
            UNINITIALIZED = 0,
            READY_TO_READ,
            READY_TO_INFLATE,
            READY_TO_COPY,
            CLEANUP,
            MAX = CLEANUP + 1
        };

        State m_previous = State::UNINITIALIZED;
        State m_state    = State::UNINITIALIZED;

        ComPtr<IStream> m_stream;
        ULONGLONG       m_seekPosition = 0;
        ULONGLONG       m_uncompressedSize = 0;
        ULONG           m_bytesRead = 0;
        std::uint8_t*   m_startCurrentBuffer = nullptr;
        ULONG           m_inflateWindowPosition = 0;
        ULONGLONG       m_fileCurrentWindowPositionEnd = 0;
        ULONGLONG       m_fileCurrentPosition = 0;

        z_stream        m_zstrm;
        int             m_zret;

        std::unique_ptr<std::vector<std::uint8_t>> m_compressedBuffer;
        std::unique_ptr<std::vector<std::uint8_t>> m_inflateWindow;
    };
}
