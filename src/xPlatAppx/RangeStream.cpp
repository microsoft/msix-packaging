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

    void RangeStream::Write(const std::uint8_t* start, const std::uint8_t* end)
    {
        throw Exception(Error::NotImplemented);
    }

    std::size_t RangeStream::Read(const std::uint8_t* start, const std::uint8_t* end)
    {
        size_t bytesRead = 0;
        size_t cbReadBuffer = std::min(static_cast<size_t>(end-start), static_cast<size_t>(m_cbLength - m_seekPosition));
        if (cbReadBuffer)
        {
            m_stream->Seek(m_beginOffset + m_seekPosition, xPlat::StreamBase::Reference::START);
            bytesRead = m_stream->Read(start, start + cbReadBuffer);
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