
#include "Exceptions.hpp"
#include "StreamBase.hpp"

namespace xPlat {
    // used to calculate offsets
    class OffsetStream : public StreamBase
    {
    public:
        void CopyTo(StreamBase& to) { throw NotImplementedException(); }
        virtual void Write(std::size_t size, const std::uint8_t* bytes)
        {
            count += size;
        }

        std::size_t Offset() { return count; }
    protected:

        std::size_t count = 0;

    };
}