#define NOMINMAX /* windows.h, or more correctly windef.h, defines min as a macro... */
#include "Exceptions.hpp"
#include "ZipFileStream.hpp"
#include "InflateStream.hpp"
#include "StreamBase.hpp"

#include <cassert>
#include <algorithm>
#include <cstring>

namespace xPlat {
    InflateStream::InflateStream(
        IStream* stream, std::uint64_t uncompressedSize
    ) : m_stream(stream),
        m_state(State::UNINITIALIZED),
        m_uncompressedSize(uncompressedSize)
    {
        m_zstrm = {0};
        m_stateMachine =
        {
            { State::CLEANUP, [&](void*, ULONG )
                {
                    Cleanup();
                    return std::make_pair(false, State::UNINITIALIZED);
                }
            }, // State::CLEANUP
            { State::UNINITIALIZED , [&](void*, ULONG)
                {
                    ThrowHrIfFailed(m_stream->Seek({0}, StreamBase::START, nullptr));
                    m_zstrm = { 0 };
                    m_fileCurrentPosition = 0;
                    m_fileCurrentWindowPositionEnd = 0;

                    int ret = inflateInit2(&m_zstrm, -MAX_WBITS);
                    ThrowErrorIfNot(Error::InflateInitialize, (ret == Z_OK), "inflateInit2 failed");
                    return std::make_pair(true, State::READY_TO_READ);
                }
            }, // State::UNINITIALIZED
            { State::READY_TO_READ , [&](void*, ULONG)
                {
                    ThrowErrorIfNot(Error::InflateRead,(m_zstrm.avail_in == 0), "uninflated bytes overwritten");
                    ULONG available = 0;
                    ThrowHrIfFailed(m_stream->Read(m_compressedBuffer, InflateStream::BUFFERSIZE, &available));
                    ThrowErrorIf(Error::FileRead, (available == 0), "Getting nothing back is unexpected here.");
                    m_zstrm.avail_in = static_cast<uInt>(available);
                    m_zstrm.next_in = m_compressedBuffer;
                    return std::make_pair(true, State::READY_TO_INFLATE);
                }
            }, // State::READY_TO_READ
            { State::READY_TO_INFLATE, [&](void*, ULONG)
                {
                    m_inflateWindowPosition = 0;
                    m_zstrm.avail_out = InflateStream::BUFFERSIZE;
                    m_zstrm.next_out = m_inflateWindow;
                    m_zret = inflate(&m_zstrm, Z_NO_FLUSH);
                    switch (m_zret)
                    {
                    case Z_NEED_DICT:
                    case Z_DATA_ERROR:
                    case Z_MEM_ERROR:
                        Cleanup();
                        ThrowErrorIfNot(Error::InflateCorruptData, false, "inflate failed unexpectedly.");
                    case Z_STREAM_END:
                    default:
                        m_fileCurrentWindowPositionEnd += (InflateStream::BUFFERSIZE - m_zstrm.avail_out);
                        return std::make_pair(true, State::READY_TO_COPY);
                    }
                }
            }, // State::READY_TO_INFLATE
            { State::READY_TO_COPY , [&](void* buffer, ULONG countBytes)
                {
                    // Check if we're actually at the end of stream.
                    if (m_fileCurrentPosition >= m_uncompressedSize)
                    {
                        ThrowErrorIfNot(Error::InflateCorruptData, ((m_zret == Z_STREAM_END) && (m_zstrm.avail_in == 0)), "unexpected extra data");
                        return std::make_pair(true, State::CLEANUP);
                    }

                    // If the end of the current window position is less than the seek position, keep inflating
                    if (m_fileCurrentWindowPositionEnd < m_seekPosition)
                    {
                        m_fileCurrentPosition += m_zstrm.avail_out;
                        return std::make_pair(true, (m_zstrm.avail_in == 0) ? State::READY_TO_READ : State::READY_TO_INFLATE);
                    }

                    // now that we're within the window between current file position and seek position
                    // calculate the number of bytes to skip ahead within this window
                    ULONG bytesToSkipInWindow = (ULONG)(m_seekPosition - m_fileCurrentPosition);
                    m_inflateWindowPosition += bytesToSkipInWindow;

                    // Calculate the difference between the beginning of the window and the seek position.
                    // if there's nothing left in the window to copy, then we need to fetch another window.
                    ULONG bytesRemainingInWindow = (InflateStream::BUFFERSIZE - m_zstrm.avail_out) - m_inflateWindowPosition;
                    if (bytesRemainingInWindow == 0)
                    {
                        return std::make_pair(true, (m_zstrm.avail_in == 0) ? State::READY_TO_READ : State::READY_TO_INFLATE);
                    }

                    ULONG bytesToCopy = std::min(countBytes, bytesRemainingInWindow);
                    if (bytesToCopy > 0)
                    {
                        memcpy(buffer, &(m_inflateWindow[m_inflateWindowPosition]), bytesToCopy);
                        m_bytesRead             += bytesToCopy;
                        m_seekPosition          += bytesToCopy;
                        m_inflateWindowPosition += bytesToCopy;
                        m_fileCurrentPosition   += bytesToCopy;
                    }

                    return std::make_pair(countBytes != 0, State::READY_TO_COPY);
                }
            } // State::READY_TO_COPY
        };
    }

    InflateStream::~InflateStream()
    {
        Cleanup();
    }

    HRESULT InflateStream::Read(void* buffer, ULONG countBytes, ULONG* bytesRead)
    {
        return ResultOf([&]{
            m_bytesRead = 0;
            m_startCurrentBuffer = reinterpret_cast<std::uint8_t*>(buffer);
            if (m_seekPosition < m_uncompressedSize)
            {
                bool stayInLoop = true;
                while (stayInLoop && (m_bytesRead < countBytes))
                {
                    const auto& stateHandler = m_stateMachine[m_state];
                    auto&& result = stateHandler(m_startCurrentBuffer + m_bytesRead, countBytes - m_bytesRead);
                    stayInLoop = std::get<0>(result);
                    m_previous = m_state;
                    m_state = std::get<1>(result);
                }
            }
            m_startCurrentBuffer = nullptr;
            if (bytesRead) { *bytesRead = m_bytesRead; }
        });
    }

    HRESULT InflateStream::Seek(LARGE_INTEGER move, DWORD origin, ULARGE_INTEGER *newPosition)
    {
        return ResultOf([&]{
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
        });
    }

    void InflateStream::Cleanup()
    {
        if (m_state != State::UNINITIALIZED)
        {
            inflateEnd(&m_zstrm);
            m_state = State::UNINITIALIZED;
        }
    }
} /* xPlat */

