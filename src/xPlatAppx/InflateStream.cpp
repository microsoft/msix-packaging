#include "Exceptions.hpp"
#include "ZipFileStream.hpp"
#include "InflateStream.hpp"
#include "StreamBase.hpp"

#include <tuple>

#include <limits>
#include <algorithm>
#include <cassert>

namespace xPlat {

    class InflateException : public ExceptionBase
    {
    public:
        InflateException() : ExceptionBase(ExceptionBase::SubFacility::INFLATE) { assert(false); }
    };

    #define Assert(a) {if (!(a)) throw InflateException();}

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
            { State::CLEANUP, [&](std::size_t, const std::uint8_t*)
                {
                    Cleanup();
                    return std::make_pair(false, State::UNINITIALIZED);
                }
            }, // State::CLEANUP
            { State::UNINITIALIZED , [&](std::size_t, const std::uint8_t*)
                {
                    m_zstrm = { 0 };
                    m_inflateBufferSeekPosition = 0;

                    int ret = inflateInit2(&m_zstrm, -MAX_WBITS);
                    if (ret != Z_OK) { throw InflateException(); }
                    return std::make_pair(true, State::READY_TO_READ);
                }
            }, // State::UNINITIALIZED
            { State::READY_TO_READ , [&](std::size_t, const std::uint8_t*)
                {
                    m_zstrm.avail_in = m_stream->Read(InflateStream::BUFFERSIZE, m_compressedBuffer);
                    m_zstrm.next_in = m_compressedBuffer;
                    return std::make_pair(true, State::READY_TO_INFLATE);
                }
            }, // State::READY_TO_READ
            { State::READY_TO_INFLATE, [&](std::size_t, const std::uint8_t*)
                {
                    m_zstrm.avail_out = InflateStream::BUFFERSIZE;
                    m_zstrm.next_out = m_inflateBuffer;
                    m_zret = inflate(&m_zstrm, Z_NO_FLUSH);
                    switch (m_zret)
                    {
                    case Z_NEED_DICT:
                    case Z_DATA_ERROR:
                    case Z_MEM_ERROR:
                        Cleanup();
                        throw xPlat::InflateException();
                    case Z_STREAM_END:
                    default:
                        return std::make_pair(true, State::READY_TO_COPY);
                    }
                }
            }, // State::READY_TO_INFLATE
            { State::READY_TO_COPY , [&](std::size_t cbReadBuffer, const std::uint8_t* readBuffer)
                {
                    std::size_t bytesAvailable = InflateStream::BUFFERSIZE - m_zstrm.avail_out;
                    Assert(bytesAvailable >= 0);
                    if (bytesAvailable == 0)
                    {   // actually at end of stream.
                        Assert(m_zret == Z_STREAM_END);
                        return std::make_pair(true, State::CLEANUP);
                    }

                    // If end of the inflated buffer position is less than the seek position,
                    // we need to keep inflating until we reach the seek position
                    if ((m_inflateBufferSeekPosition + bytesAvailable) < m_seekPosition)
                    {
                        m_inflateBufferSeekPosition += bytesAvailable;
                        return std::make_pair(true, (m_zstrm.avail_out == 0) ? State::READY_TO_INFLATE : State::READY_TO_READ);
                    }

                    // Calculate the difference between the beginning of the window and the seek position.
                    std::size_t skipBytes = (m_seekPosition - m_inflateBufferSeekPosition);
                    std::size_t bytesToCopy = min(cbReadBuffer, bytesAvailable - skipBytes);
                    Assert(bytesToCopy > 0);
                    memcpy((void*)readBuffer, &m_inflateBuffer[skipBytes], bytesToCopy);
                    readBuffer += bytesToCopy;
                    cbReadBuffer -= bytesToCopy;
                    m_bytesRead += bytesToCopy;
                    m_seekPosition += bytesToCopy;

                    // If there's still stuff remaining in the inflate buffer, it means the caller
                    // didn't request all of it
                    if ((skipBytes + bytesToCopy) < bytesAvailable)
                    {
                        return std::make_pair(false, State::READY_TO_COPY);
                    }

                    // We have drained the current inflate buffer. Update the window seek position.
                    m_inflateBufferSeekPosition = m_seekPosition;

                    // The caller got everything that they need for now. Return it to them.
                    if (cbReadBuffer == 0)
                    {
                        return std::make_pair(false, State::READY_TO_COPY);
                    }

                    // If avail_out == 0, then there's still stuff that can be inflated; otherwise,
                    // zlib is starved for new I/O, so we need to read the next chunk
                    return std::make_pair(true, (m_zstrm.avail_out == 0) ? State::READY_TO_INFLATE : State::READY_TO_READ);
                }
            } // State::READY_TO_COPY
        };
    }

    InflateStream::~InflateStream()
    {
        Cleanup();
    }

    void InflateStream::Write(std::size_t size, const std::uint8_t* bytes)
    {
        throw NotImplementedException();
    }

    std::size_t InflateStream::Read(std::size_t cbReadBuffer, const std::uint8_t* readBuffer)
    {
        m_bytesRead = 0;
        if (m_seekPosition < m_uncompressedSize)
        {
            bool stayInLoop = true;
            do
            {
                const auto& stateHandler = m_stateMachine[m_state];
                auto&& result = stateHandler(cbReadBuffer, readBuffer);
                stayInLoop = std::get<0>(result);
                m_previous = m_state;
                m_state = std::get<1>(result);
            } while (stayInLoop);
        }

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
            seekPosition = m_seekPosition = offset;
            break;
        case Reference::END:
            seekPosition = m_uncompressedSize + offset;
            break;
        }
        
        // Can't seek beyond the end of the uncompressed stream
        seekPosition = min(m_seekPosition, m_uncompressedSize);

        if (seekPosition != m_seekPosition)
        {
            m_seekPosition = seekPosition;
            // If the caller is trying to seek back to an earlier
            // point in the inflated stream, we will need to reset
            // zlib and start inflating from the beginning of the
            // stream; otherwise, seeking forward is fine: We will 
            // catch up to the seek pointer during the ::Read operation.
            if (m_seekPosition < m_inflateBufferSeekPosition)
            {
                m_inflateBufferSeekPosition = 0;
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

