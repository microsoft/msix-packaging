#define NOMINMAX /* windows.h, or more correctly windef.h, defines min as a macro... */
#include "Exceptions.hpp"
#include "ZipFileStream.hpp"
#include "InflateStream.hpp"
#include "StreamBase.hpp"

#include <cassert>
#include <algorithm>

namespace xPlat {
    InflateStream::InflateStream(
        std::shared_ptr<StreamBase> stream,
        std::uint64_t uncompressedSize
    ) : m_stream(stream),
        m_state(State::UNINITIALIZED),
        m_uncompressedSize(uncompressedSize)
    {
        m_zstrm = {0};
        m_stateMachine =
        {
            { State::CLEANUP, [&](const std::uint8_t*, const std::uint8_t*)
                {
                    Cleanup();
                    return std::make_pair(false, State::UNINITIALIZED);
                }
            }, // State::CLEANUP
            { State::UNINITIALIZED , [&](const std::uint8_t*, const std::uint8_t*)
                {
                    m_zstrm = { 0 };
                    m_fileCurrentPosition = 0;
                    m_fileCurrentWindowPositionEnd = 0;

                    int ret = inflateInit2(&m_zstrm, -MAX_WBITS);
                    ThrowErrorIfNot(Error::InflateInitialize, (ret == Z_OK), "inflateInit2 failed");
                    return std::make_pair(true, State::READY_TO_READ);
                }
            }, // State::UNINITIALIZED
            { State::READY_TO_READ , [&](const std::uint8_t*, const std::uint8_t*)
                {
                    ThrowErrorIfNot(Error::InflateRead,(m_zstrm.avail_in == 0), "uninflated bytes overwritten");
                    m_zstrm.avail_in = m_stream->Read(m_compressedBuffer, m_compressedBuffer + InflateStream::BUFFERSIZE);
                    m_zstrm.next_in = m_compressedBuffer;
                    return std::make_pair(true, State::READY_TO_INFLATE);
                }
            }, // State::READY_TO_READ
            { State::READY_TO_INFLATE, [&](const std::uint8_t*, const std::uint8_t*)
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
            { State::READY_TO_COPY , [&](const std::uint8_t* start, const std::uint8_t* end)
                {
                    // Check if we're actually at the end of stream.
                    if (0 == (m_uncompressedSize - m_fileCurrentPosition))
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
                    std::size_t bytesToSkipInWindow = m_seekPosition - m_fileCurrentPosition;
                    m_inflateWindowPosition += bytesToSkipInWindow;

                    // Calculate the difference between the beginning of the window and the seek position.
                    // if there's nothing left in the window to copy, then we need to fetch another window.
                    std::size_t bytesRemainingInWindow = (InflateStream::BUFFERSIZE - m_zstrm.avail_out) - m_inflateWindowPosition;
                    if (bytesRemainingInWindow == 0)
                    {
                        return std::make_pair(true, (m_zstrm.avail_in == 0) ? State::READY_TO_READ : State::READY_TO_INFLATE);
                    }

                    std::size_t bytesToCopy = std::min(static_cast<size_t>(end - start), bytesRemainingInWindow);
                    if (bytesToCopy > 0)
                    {
                        memcpy(
                            static_cast<void*>(const_cast<std::uint8_t*>(start)),
                            &(m_inflateWindow[m_inflateWindowPosition]),
                            bytesToCopy);
                        m_bytesRead             += bytesToCopy;
                        m_seekPosition          += bytesToCopy;
                        m_inflateWindowPosition += bytesToCopy;
                        m_fileCurrentPosition   += bytesToCopy;
                    }

                    return std::make_pair(m_bytesRead != (end - m_startCurrentBuffer), State::READY_TO_COPY);
                }
            } // State::READY_TO_COPY
        };
    }

    InflateStream::~InflateStream()
    {
        Cleanup();
    }

    void InflateStream::Write(const std::uint8_t* , const std::uint8_t*)
    {
        throw Exception(Error::NotImplemented);
    }

    std::size_t InflateStream::Read(const std::uint8_t* start, const std::uint8_t* end)
    {
        m_bytesRead = 0;
        m_startCurrentBuffer = start;
        if (m_seekPosition < m_uncompressedSize)
        {
            bool stayInLoop = true;
            while (stayInLoop && ((start + m_bytesRead) != end))
            {
                const auto& stateHandler = m_stateMachine[m_state];
                auto&& result = stateHandler(start + m_bytesRead, end);
                stayInLoop = std::get<0>(result);
                m_previous = m_state;
                m_state = std::get<1>(result);
            }
        }
        m_startCurrentBuffer = nullptr;
        return m_bytesRead;
    }

    void InflateStream::Seek(std::uint64_t offset, Reference where)
    {
        std::uint64_t seekPosition = 0;
        switch (where)
        {
        case Reference::CURRENT:
            seekPosition = m_seekPosition + offset;
            break;
        case Reference::START:
            seekPosition = offset;
            break;
        case Reference::END:
            seekPosition = m_uncompressedSize + offset;
            break;
        }
        
        // Can't seek beyond the end of the uncompressed stream
        seekPosition = std::min(seekPosition, m_uncompressedSize);

        if (seekPosition != m_seekPosition)
        {
            m_seekPosition = seekPosition;
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
    }

    int InflateStream::Ferror()
    {
        return 0;
    }

    bool InflateStream::Feof()
    {
        return m_seekPosition >= m_uncompressedSize;
    }

    std::uint64_t InflateStream::Ftell()
    {
        return m_seekPosition;
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

