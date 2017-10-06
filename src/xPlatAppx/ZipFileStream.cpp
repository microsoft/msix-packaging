#include "ZipFileStream.hpp"
#include "StreamBase.hpp"
#include <limits>
#include <algorithm>

namespace xPlat {

    void ZipFileStream::Write(std::size_t size, const std::uint8_t* bytes)
    {
        throw NotImplementedException();
    }

    std::uint64_t ZipFileStream::Read(std::uint64_t size, const std::uint8_t* bytes)
    {
        m_stream->Seek(m_offset + m_relativePosition, StreamBase::Reference::START);

        std::uint64_t amountToRead = std::min(size, (m_compressedSize - m_relativePosition));
        std::uint64_t bytesRead = m_stream->Read(amountToRead, bytes);
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
            newPos = m_offset + m_compressedSize + offset;
            break;
        }

        m_stream->Seek(newPos, Reference::START);
        m_relativePosition = m_stream->Ftell() - m_offset;
    }

    int ZipFileStream::Ferror()
    {
        return 0;
    }

    bool ZipFileStream::Feof()
    {
        return m_relativePosition >= m_compressedSize;
    }

    std::uint64_t ZipFileStream::Ftell()
    {
        return m_relativePosition;
    }

} /* xPlat */