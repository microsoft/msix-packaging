
#include "ZipFileStream.hpp"
#include "InflateStream.hpp"
#include "StreamBase.hpp"

#include <limits>
#include <algorithm>

#define STATE_UNINITIALIZED               0
#define STATE_COULD_NOT_INITIALIZE        1
#define STATE_READY_TO_READ               2
#define STATE_IO_ERROR_DURING_READ        3
#define STATE_COMPRESSED_DATA_READ        4
#define STATE_ZLIB_ERROR_DURING_INFLATE   5
#define STATE_UNCOMPRESSED_DATA_AVAILABLE 6
#define STATE_END_OF_COMPRESSED_STREAM    7
#define STATE_UNCOMPRESSED_DATA_WRITTEN   8
#define STATE_NO_MORE_DATA                9

namespace xPlat {

    InflateStream::InflateStream(
        std::shared_ptr<StreamBase> stream,
        std::uint64_t compressedSize,
        std::uint64_t uncompressedSize
    ) : m_stream(stream)
    {
        m_state = STATE_UNINITIALIZED;
    }

    void InflateStream::Write(std::size_t size, const std::uint8_t* bytes)
    {
        throw NotImplementedException();
    }

    std::size_t InflateStream::Read(std::size_t size, const std::uint8_t* bytes)
    {
        std::size_t bytesRead = 0;

        while (m_state != STATE_NO_MORE_DATA)
        {
            switch (m_state)
            {
                case STATE_UNINITIALIZED:
                {
                    m_offsetOfUncompressedWindow = 0;
                    memset(&m_zstrm, 0, sizeof(m_zstrm));
                    m_zstrm.zalloc = Z_NULL;
                    m_zstrm.zfree = Z_NULL;
                    m_zstrm.opaque = Z_NULL;
                    m_zstrm.avail_in = 0;
                    m_zstrm.next_in = Z_NULL;
                    int ret = inflateInit(&m_zstrm);
                    m_state = (ret == Z_OK) ? STATE_READY_TO_READ : STATE_COULD_NOT_INITIALIZE;
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
                        m_state = (m_zstrm.avail_in == 0) ? STATE_NO_MORE_DATA : STATE_COMPRESSED_DATA_READ;                       
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
                    inflateEnd(&m_zstrm);
                    m_state = STATE_UNINITIALIZED;
                    throw xPlat::IOException();
                }
                break;
                
                case STATE_COMPRESSED_DATA_READ:
                {
                    m_zstrm.avail_out = InflateStream::BUFFERSIZE;
                    m_zstrm.next_out = m_uncompressedBuffer;
                    m_zret = inflate(&m_zstrm, Z_NO_FLUSH);
                    switch (m_zret)
                    {
                        case Z_NEED_DICT:
                        case Z_DATA_ERROR:
                        case Z_MEM_ERROR:
                            m_state = STATE_ZLIB_ERROR_DURING_INFLATE;
                            break;
                        case Z_STREAM_END:
                            //TODO
                            break;
                        default:
                            std::size_t bytesAvailable = InflateStream::BUFFERSIZE - m_zstrm.avail_out;
                            m_state = (bytesAvailable > 0) ? STATE_UNCOMPRESSED_DATA_AVAILABLE : STATE_COMPRESSED_DATA_READ;
                            break;
                    }
                }
                break;

                case STATE_UNCOMPRESSED_DATA_AVAILABLE:
                {
                    std::size_t bytesAvailable = InflateStream::BUFFERSIZE - m_zstrm.avail_out;
                    std::uint64_t offsetOfEndUncompressedWindow = (m_offsetOfUncompressedWindow + bytesAvailable);
                    if (offsetOfEndUncompressedWindow < m_seekPosition)
                    {
                        m_offsetOfUncompressedWindow += bytesAvailable;
                        m_state = STATE_COMPRESSED_DATA_READ;
                    }
                    else
                    if (m_offsetOfUncompressedWindow < m_seekPosition)
                    {
                        std::size_t bytesToCopy = min((size - bytesRead), bytesAvailable);
                    }
                }
                break;




            }
        }
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

} /* xPlat */

#define CHUNK 4096
#define assert(a) {;}

int inf(FILE *source, FILE *dest)
{
    int ret;
    unsigned have;
    z_stream strm;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];
    
    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit(&strm);
    if (ret != Z_OK)
        return ret;
    
    /* decompress until deflate stream ends or end of file */
    do 
    {
        strm.avail_in = fread(in, 1, CHUNK, source);
        if (ferror(source))
        {
            (void)inflateEnd(&strm);
            return Z_ERRNO;
        }
        if (strm.avail_in == 0)
            break;
        strm.next_in = in;
    
        /* run inflate() on input until output buffer not full */
        do {
    
            strm.avail_out = CHUNK;
            strm.next_out = out;
            
            ret = inflate(&strm, Z_NO_FLUSH);
            assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
            switch (ret) 
            {
                case Z_NEED_DICT:
                    ret = Z_DATA_ERROR;     /* and fall through */
                case Z_DATA_ERROR:
                case Z_MEM_ERROR:
                    (void)inflateEnd(&strm);
                    return ret;
            }
            
            have = CHUNK - strm.avail_out;
            if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
                (void)inflateEnd(&strm);
                return Z_ERRNO;
            }
        } 
        while (strm.avail_out == 0);
    } 
    while (ret != Z_STREAM_END); /* done when inflate() says it's done */

    /* clean up and return */
    (void)inflateEnd(&strm);
    return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}

