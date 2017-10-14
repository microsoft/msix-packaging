#pragma once
#include "Exceptions.hpp"
#include "StreamBase.hpp"

namespace xPlat {
    // used to calculate offsets
    class OffsetStream : public StreamBase
    {
    public:
        virtual void CopyTo(StreamBase* to) override { throw Exception(Error::NotImplemented); }
        virtual void Write(std::size_t size, const std::uint8_t* bytes) override
        {
            count += size;
        }

        std::size_t Offset() { return count; }
    protected:

        std::size_t count = 0;
    };
}