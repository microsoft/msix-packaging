#pragma once
#include "Exceptions.hpp"
#include "StreamBase.hpp"
#include <string>
#include <map>
#include <functional>


namespace xPlat {
  
    // This represents a subset of a Stream
    class RangeStream : public StreamBase
    {
    public:
        RangeStream(
            std::shared_ptr<StreamBase> stream,
            std::uint64_t beginOffset,
            std::uint64_t endOffset
        );

        ~RangeStream() override;

        void Write(const std::uint8_t* start, const std::uint8_t* end) override;
        std::size_t Read(const std::uint8_t* start, const std::uint8_t* end) override;
        void Seek(std::uint64_t offset, Reference where) override;
        int Ferror() override;
        bool Feof() override;
        std::uint64_t Ftell()  override;

    protected:
        std::shared_ptr<StreamBase> m_stream;
        std::uint64_t m_beginOffset;
        std::uint64_t m_cbLength;
        std::uint64_t m_seekPosition = 0;
    };
}