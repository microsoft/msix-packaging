#include "ZipFileStream.hpp"
#include "StreamBase.hpp"
#include <limits>
#include <algorithm>

namespace xPlat {

    void ZipFileStream::Write(const std::uint8_t*, const std::uint8_t*)
    {
        throw Exception(Error::NotImplemented);
    }

    std::size_t ZipFileStream::Read(const std::uint8_t* start, const std::uint8_t* end)
    {
        m_stream->Seek(m_offset + m_relativePosition, StreamBase::Reference::START);

        std::size_t amountToRead = std::min(static_cast<size_t>(end-start), static_cast<size_t>(m_size - m_relativePosition));
        std::size_t bytesRead = m_stream->Read(start, start + amountToRead);
        m_relativePosition += bytesRead;
        return bytesRead;
    }

    void ZipFileStream::Seek(std::uint64_t offset, Reference where)
    {
        std::uint64_t newPos = 0;
        switch (where)
        {
        case Reference::CURRENT:
            newPos = m_offset + m_relativePosition + offset;
            break;
        case Reference::START:
            newPos = m_offset + offset;
            break;
        case Reference::END:
            newPos = m_offset + m_size + offset;
            break;
        }
        //TODO: We need to constrain newPos so that it can't exceed the end of the stream
        m_stream->Seek(newPos, Reference::START);
        m_relativePosition = m_stream->Ftell() - m_offset;
    }

    int ZipFileStream::Ferror()
    {
        return 0;
    }

    bool ZipFileStream::Feof()
    {
        return m_relativePosition >= m_size;
    }

    std::uint64_t ZipFileStream::Ftell()
    {
        return m_relativePosition;
    }

    bool ZipFileStream::IsCompressed()
    {
        return (m_isCompressed);
    }

    std::uint64_t ZipFileStream::Size()
    {
        return m_size;
    }
} /* xPlat */