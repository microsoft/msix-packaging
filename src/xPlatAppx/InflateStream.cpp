
#include "ZipFileStream.hpp"
#include "InflateStream.hpp"
#include "StreamBase.hpp"

#ifdef WIN32
#define ZLIB_WINAPI
#endif

#include "..\..\lib\zlib\zlib.h"
#include <limits>
#include <algorithm>

namespace xPlat {

    void InflateStream::Write(std::size_t size, const std::uint8_t* bytes)
    {
        throw NotImplementedException();
    }

    std::size_t InflateStream::Read(std::size_t size, const std::uint8_t* bytes)
    {
        /*m_stream->Seek(m_offset + m_relativePosition, StreamBase::Reference::START);

        std::uint64_t amountToRead = std::min(size, (m_compressedSize - m_relativePosition));
        std::uint64_t bytesRead = m_stream->Read(amountToRead, bytes);
        m_relativePosition += bytesRead;
        return bytesRead;*/
        return 0;
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

