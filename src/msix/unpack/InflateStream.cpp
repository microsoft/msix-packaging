//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#include "Exceptions.hpp"
#include "ZipFileStream.hpp"
#include "InflateStream.hpp"
#include "StreamBase.hpp"

#include <cassert>
#include <algorithm>
#include <cstring>
#include <array>
#include <utility>

namespace MSIX {

    // Buffer size used for compressed buffer and inflate window.
    // See zlib's updatewindow comment.
    static const size_t BufferSize = 32*1024;

    struct InflateHandler
    {
        typedef std::pair<bool, InflateStream::State>(*lambda)(InflateStream* self, void* buffer, ULONG countBytes);
        InflateHandler(lambda f): Handler(f) {}
        lambda Handler;
    };

    std::array<InflateHandler, static_cast<size_t>(InflateStream::State::MAX)> stateMachine =
    {            
        // State::UNINITIALIZED
        InflateHandler([](InflateStream* self, void*, ULONG)
        {
            ThrowHrIfFailed(self->m_stream->Seek({0}, StreamBase::START, nullptr));
            self->m_fileCurrentPosition = 0;
            self->m_fileCurrentWindowPositionEnd = 0;

            self->m_compressionStatus = self->m_compressionObject->Initialize(CompressionOperation::Inflate);
            ThrowErrorIfNot(Error::InflateInitialize, (self->m_compressionStatus == CompressionStatus::Ok), "compression_stream_init failed");
            return std::make_pair(true, InflateStream::State::READY_TO_READ);
        }), // State::UNINITIALIZED

        // State::READY_TO_READ
        InflateHandler([](InflateStream* self, void*, ULONG)
        {
            ThrowErrorIfNot(Error::InflateRead,(self->m_compressionObject->GetAvailableSourceSize() == 0), "uninflated bytes overwritten");
            ULONG available = 0;
            self->m_compressedBuffer.swap(std::make_unique<std::vector<std::uint8_t>>(BufferSize));
            ThrowHrIfFailed(self->m_stream->Read(self->m_compressedBuffer->data(), static_cast<ULONG>(self->m_compressedBuffer->size()), &available));
            ThrowErrorIf(Error::FileRead, (available == 0), "Getting nothing back is unexpected here.");
            self->m_compressionObject->SetInput(self->m_compressedBuffer->data(), static_cast<size_t>(available));
            return std::make_pair(true, InflateStream::State::READY_TO_INFLATE);
        }), // State::READY_TO_READ

        // State::READY_TO_INFLATE
        InflateHandler([](InflateStream* self, void*, ULONG)
        {
            self->m_inflateWindow.swap(std::make_unique<std::vector<std::uint8_t>>(BufferSize));
            self->m_inflateWindowPosition = 0;
            self->m_compressionObject->SetOutput(self->m_inflateWindow->data(), self->m_inflateWindow->size());
            self->m_compressionStatus = self->m_compressionObject->Inflate();
            switch (self->m_compressionStatus)
            {
            case CompressionStatus::Error:
                self->Cleanup();
                ThrowErrorIfNot(Error::InflateCorruptData, false, "inflate failed unexpectedly.");
                break;
            case CompressionStatus::Ok:
            case CompressionStatus::End:
            default:
                self->m_fileCurrentWindowPositionEnd += (BufferSize - self->m_compressionObject->GetAvailableDestinationSize());
                return std::make_pair(true, InflateStream::State::READY_TO_COPY);
            }
        }), // State::READY_TO_INFLATE

        // State::READY_TO_COPY
        InflateHandler([](InflateStream* self, void* buffer, ULONG countBytes)
        {
            // Check if we're actually at the end of stream.
            if (self->m_fileCurrentPosition >= self->m_uncompressedSize)
            {
                ThrowErrorIfNot(Error::InflateCorruptData, ((self->m_compressionStatus  == CompressionStatus::End) && (self->m_compressionObject->GetAvailableSourceSize() == 0)), "unexpected extra data");
                return std::make_pair(true, InflateStream::State::CLEANUP);
            }

            // If the end of the current window position is less than the seek position, keep inflating
            if (self->m_fileCurrentWindowPositionEnd < self->m_seekPosition)
            {
                self->m_fileCurrentPosition = self->m_fileCurrentWindowPositionEnd;
                return std::make_pair(true, (self->m_compressionObject->GetAvailableDestinationSize() == 0) ? InflateStream::State::READY_TO_INFLATE : InflateStream::State::READY_TO_READ);
            }

            // now that we're within the window between current file position and seek position
            // calculate the number of bytes to skip ahead within this window
            ULONG bytesToSkipInWindow = (ULONG)(self->m_seekPosition - self->m_fileCurrentPosition);
            self->m_inflateWindowPosition += bytesToSkipInWindow;

            // Calculate the difference between the beginning of the window and the seek position.
            // if there's nothing left in the window to copy, then we need to fetch another window.
            ULONG bytesRemainingInWindow = static_cast<ULONG>((BufferSize - self->m_compressionObject->GetAvailableDestinationSize()) - self->m_inflateWindowPosition);
            if (bytesRemainingInWindow == 0)
            {
                return std::make_pair(true, (self->m_compressionObject->GetAvailableDestinationSize() == 0) ? InflateStream::State::READY_TO_INFLATE : InflateStream::State::READY_TO_READ);
            }

            ULONG bytesToCopy = std::min(countBytes, bytesRemainingInWindow);
            if (bytesToCopy > 0)
            {
                memcpy(buffer, &(self->m_inflateWindow->at(self->m_inflateWindowPosition)), bytesToCopy);
                self->m_bytesRead             += bytesToCopy;
                self->m_seekPosition          += bytesToCopy;
                self->m_inflateWindowPosition += bytesToCopy;
                self->m_fileCurrentPosition   += bytesToCopy;
            }

            if (self->m_fileCurrentPosition == self->m_uncompressedSize)
            {
                self->Cleanup();
                return std::make_pair(false, InflateStream::State::UNINITIALIZED);
            }
            
            return std::make_pair(countBytes != 0, InflateStream::State::READY_TO_COPY);
            
        }), // State::READY_TO_COPY

        // State::CLEANUP    
        InflateHandler([](InflateStream* self, void*, ULONG)
        {
            self->Cleanup();
            return std::make_pair(false, InflateStream::State::UNINITIALIZED);
        }) // State::CLEANUP            
    };

    InflateStream::InflateStream(
        const ComPtr<IStream>& stream, std::uint64_t uncompressedSize
    ) : m_stream(stream),
        m_state(State::UNINITIALIZED),
        m_uncompressedSize(uncompressedSize)
    {
        m_compressionObject = CreateCompressionObject();
    }

    InflateStream::~InflateStream()
    {
        m_compressedBuffer = nullptr;
        m_inflateWindow = nullptr;
        Cleanup();
    }

    HRESULT InflateStream::Read(void* buffer, ULONG countBytes, ULONG* bytesRead) noexcept try
    {
        m_bytesRead = 0;
        m_startCurrentBuffer = reinterpret_cast<std::uint8_t*>(buffer);
        if (m_seekPosition < m_uncompressedSize)
        {
            bool stayInLoop = true;
            while (stayInLoop && (m_bytesRead < countBytes))
            {
                auto&& result = stateMachine[static_cast<size_t>(m_state)].Handler(this, m_startCurrentBuffer + m_bytesRead, countBytes - m_bytesRead);
                stayInLoop = std::get<0>(result);
                m_previous = m_state;
                m_state = std::get<1>(result);
            }
        }
        m_startCurrentBuffer = nullptr;
        if (bytesRead) { *bytesRead = m_bytesRead; }
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    HRESULT InflateStream::Seek(LARGE_INTEGER move, DWORD origin, ULARGE_INTEGER *newPosition) noexcept try
    {
        LARGE_INTEGER seekPosition = { 0 };
        switch (origin)
        {
        case Reference::CURRENT:
            seekPosition.QuadPart = m_seekPosition + move.QuadPart;
            break;
        case Reference::START:
            seekPosition.QuadPart = move.QuadPart;
            break;
        case Reference::END:
            seekPosition.QuadPart = m_uncompressedSize + move.QuadPart;
            break;
        }

        // Can't seek beyond the end of the uncompressed stream
        seekPosition.QuadPart = std::min(seekPosition.QuadPart, static_cast<LONGLONG>(m_uncompressedSize));

        if (seekPosition.QuadPart != m_seekPosition)
        {
            m_seekPosition = seekPosition.QuadPart;
            // If the caller is trying to seek back to an earlier
            // point in the inflated stream, we will need to reset
            // zlib and start inflating from the beginning of the
            // stream; otherwise, seeking forward is fine: We will
            // catch up to the seek pointer during the ::Read operation.
            if (m_seekPosition < m_fileCurrentPosition)
            {
                m_fileCurrentPosition = 0;
                Cleanup();
            }
        }
        if (newPosition) { newPosition->QuadPart = m_seekPosition; }
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    void InflateStream::Cleanup()
    {
        if (m_state != State::UNINITIALIZED)
        {
            m_compressionObject->Cleanup();
            m_state = State::UNINITIALIZED;
        }
    }
} /* msix */

