
#include "Exceptions.hpp"
#include "StreamBase.hpp"

#include <initializer_list>
#include <vector>
#include <algorithm>

namespace xPlat {

    // A stream for operating over a set of streams
    class SequenceStream : public StreamBase
    {
    public:
        SequenceStream(std::initializer_list<StreamBase*> streams) : streams(streams) { }

        void Write(std::size_t size, const std::uint8_t* bytes)
        {
            std::for_each(streams.begin(), streams.end(), [&](auto stream) { stream->Write(size, bytes); });
        }

        std::size_t Read(std::size_t size, const std::uint8_t* bytes)
        {
            std::size_t lastRead = 0;
            std::for_each(streams.begin(), streams.end(), [&](auto stream) { lastRead = stream->Read(size, bytes); });
            return lastRead;
        }

    protected:
        std::vector<StreamBase*> streams;
    };
}