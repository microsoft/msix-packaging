#include "Exceptions.hpp"
#include "StreamBase.hpp"
#include "RangeStream.hpp"

#include <vector>
#include <algorithm>
#include <limits>
#include <type_traits>

namespace xPlat {

    RangeStream::RangeStream(
        std::shared_ptr<StreamBase> stream,
        std::uint64_t beginOffset,
        std::uint64_t cbLength
    ) : m_stream(stream), 
        m_beginOffset(beginOffset), 
        m_cbLength(cbLength),
        m_seekPosition(0)
    {
    }

    RangeStream::~RangeStream()
    {
    }

    void RangeStream::Write(std::size_t size, const std::uint8_t* bytes)
    {
        throw Exception(Error::NotImplemented);
    }

    std::size_t RangeStream::Read(std::size_t cbReadBuffer, const std::uint8_t* readBuffer)
    {
        size_t bytesRead = 0;
        cbReadBuffer = std::min((std::uint64_t)cbReadBuffer, m_cbLength - m_seekPosition);
        if (cbReadBuffer)
        {
            m_stream->Seek(m_beginOffset + m_seekPosition, xPlat::StreamBase::Reference::START);
            bytesRead = m_stream->Read(cbReadBuffer, readBuffer);
            m_seekPosition += bytesRead;
        }
        return bytesRead;
    }

    void RangeStream::Seek(std::uint64_t offset, xPlat::StreamBase::Reference where)
    {
        std::uint64_t seekPosition = 0;
        switch (where)
        {
        case Reference::CURRENT:
            seekPosition = m_beginOffset + m_seekPosition + offset;
            break;
        case Reference::START:
            seekPosition = m_seekPosition = offset;
            break;
        case Reference::END:
            seekPosition = m_beginOffset + m_cbLength;
            break;
        }
        
        // Can't seek beyond the end of the range
        m_seekPosition = std::min(seekPosition, m_beginOffset + m_cbLength);
    }

    int RangeStream::Ferror()
    {
        return 0;
    }

    bool RangeStream::Feof()
    {
        return m_seekPosition >= m_cbLength;
    }

    std::uint64_t RangeStream::Ftell()
    {
        return m_seekPosition;
    }
} /* xPlat */

