
#include "ZipFileStream.hpp"
#include "InflateStream.hpp"
#include "StreamBase.hpp"

#include <limits>
#include <algorithm>

#define STATE_UNINITIALIZED               0
#define STATE_COULD_NOT_INITIALIZE        1
#define STATE_READY_TO_READ               2
#define STATE_IO_ERROR_DURING_READ        3
#define STATE_READY_TO_INFLATE            4
#define STATE_ZLIB_ERROR_DURING_INFLATE   5
#define STATE_READY_TO_COPY               6
#define STATE_CLEANUP                     9
#define STATE_EXIT                       10

#ifdef WIN32
#define Assert(a) {if (!(a)) MessageBoxA(GetTopWindow(nullptr), #a, "Assert", MB_OK);}
#else
#define Assert(a) 
#endif

namespace xPlat {

    InflateStream::InflateStream(
        std::shared_ptr<StreamBase> stream,
        std::uint64_t compressedSize,
        std::uint64_t uncompressedSize
    ) : m_stream(stream)
    {
        m_state = STATE_UNINITIALIZED;
        m_compressedSize = compressedSize;
        m_uncompressedSize = uncompressedSize;
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
        std::size_t bytesRead = 0;

        if (m_seekPosition < m_uncompressedSize)
        {
            bool stayInLoop = true;
            while (stayInLoop)
            {
                switch (m_state)
                {
                    case STATE_UNINITIALIZED:
                    {
                        m_zstrm.zalloc = Z_NULL;
                        m_zstrm.zfree = Z_NULL;
                        m_zstrm.opaque = Z_NULL;
                        m_zstrm.avail_in = 0;
                        m_zstrm.next_in = Z_NULL;
                        int ret = inflateInit2(&m_zstrm, -MAX_WBITS);
                        m_state = (ret == Z_OK) ? STATE_READY_TO_READ : STATE_COULD_NOT_INITIALIZE;
                        m_inflateBufferSeekPosition = 0;
                    }
                    break;

                    case STATE_COULD_NOT_INITIALIZE:
                    {
                        m_state = STATE_UNINITIALIZED;
                        throw xPlat::IOException();
                    }
                    break;

                    case STATE_READY_TO_READ:
                    {
                        try
                        {
                            m_zstrm.avail_in = m_stream->Read(InflateStream::BUFFERSIZE, m_compressedBuffer);
                            m_zstrm.next_in = m_compressedBuffer;
                            m_state = STATE_READY_TO_INFLATE;
                        }
                        catch (xPlat::ExceptionBase&)
                        {
                            m_state = STATE_IO_ERROR_DURING_READ;
                        }
                    }
                    break;

                    case STATE_IO_ERROR_DURING_READ:
                    case STATE_ZLIB_ERROR_DURING_INFLATE:
                    {
                        Cleanup();
                        throw xPlat::IOException();
                    }
                    break;

                    case STATE_READY_TO_INFLATE:
                    {
                        m_zstrm.avail_out = InflateStream::BUFFERSIZE;
                        m_zstrm.next_out = m_inflateBuffer;
                        m_zret = inflate(&m_zstrm, Z_NO_FLUSH);
                        switch (m_zret)
                        {
                            case Z_NEED_DICT:
                            case Z_DATA_ERROR:
                            case Z_MEM_ERROR:
                                m_state = STATE_ZLIB_ERROR_DURING_INFLATE;
                                break;
                            case Z_STREAM_END:
                            default:
                                m_state = STATE_READY_TO_COPY;
                                break;
                        }
                    }
                    break;


                    case STATE_READY_TO_COPY:
                    {
                        std::size_t bytesAvailable = InflateStream::BUFFERSIZE - m_zstrm.avail_out;
                        if (bytesAvailable > 0)
                        {
                            // If end of the inflated buffer position is less than the seek position,
                            // we need to keep inflating until we reach the seek position
                            if ((m_inflateBufferSeekPosition + bytesAvailable) < m_seekPosition)
                            {
                                m_inflateBufferSeekPosition += bytesAvailable;
                                m_state = (m_zstrm.avail_out == 0) ? STATE_READY_TO_INFLATE : STATE_READY_TO_READ;
                            }
                            else
                            {
                                std::size_t skipBytes = (m_seekPosition - m_inflateBufferSeekPosition);
                                std::size_t bytesToCopy = min(cbReadBuffer, bytesAvailable - skipBytes);
                                Assert(bytesToCopy > 0);
                                memcpy((void*)readBuffer, &m_inflateBuffer[skipBytes], bytesToCopy);
                                readBuffer += bytesToCopy;
                                cbReadBuffer -= bytesToCopy;
                                bytesRead += bytesToCopy;
                                m_seekPosition += bytesToCopy;

                                // If there's still stuff remaining in the inflate buffer, it means the caller
                                // didn't request all of it
                                if ((skipBytes + bytesToCopy) < bytesAvailable)
                                {
                                    stayInLoop = false;
                                }
                                else
                                {
                                    // We have drained the current inflate buffer. Update the window seek position.
                                    m_inflateBufferSeekPosition = m_seekPosition;
                                    // If avail_out == 0, then there's still stuff that can be inflated; otherwise,
                                    // zlib is starved for new I/O, so we need to read the next chunk
                                    m_state = (m_zstrm.avail_out == 0) ? STATE_READY_TO_INFLATE : STATE_READY_TO_READ;
                                    // The caller got everything that they need for now. Return it to them.
                                    if (cbReadBuffer == 0)
                                    {
                                        stayInLoop = false;
                                    }
                                }
                            }
                        }
                        else
                        {
                            Assert(m_zret == Z_STREAM_END);
                            m_state = (m_zret == Z_STREAM_END) ? STATE_CLEANUP : STATE_READY_TO_READ;
                        }
                    }
                    break;

                    case STATE_CLEANUP:
                    {
                        Cleanup();
                        stayInLoop = false;
                    }
                    break;

                    default:
                    {
                        Assert(false);
                    }
                    break;

               } //switch
            } //while
        } //if    

        return bytesRead;      
    }

    void InflateStream::Seek(std::uint64_t offset, Reference where)
    {
        switch (where)
        {
        case Reference::CURRENT:
            m_seekPosition = m_seekPosition + offset;
            break;
        case Reference::START:
            m_seekPosition = offset;
            break;
        case Reference::END:
            m_seekPosition = m_uncompressedSize + offset;
            break;
        }
        //we can't seek beyond the end of the uncompressed stream
        m_seekPosition = min(m_seekPosition, m_uncompressedSize);
        m_inflateBufferSeekPosition = 0;
        Cleanup();
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
        if (m_state != STATE_UNINITIALIZED)
        {
            inflateEnd(&m_zstrm);
            m_state = STATE_UNINITIALIZED;
        }
    }

} /* xPlat */

