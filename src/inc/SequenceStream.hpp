
#include "Exceptions.hpp"
#include "StreamBase.hpp"

#include <initializer_list>
#include <vector>

namespace xPlat {

    // A stream for operating over a set of streams
    class SequenceStream : public StreamBase
    {
    public:
        SequenceStream(std::initializer_list<StreamBase*> streams) : streams(streams) { }

        void Write(std::size_t size, const std::uint8_t* bytes)
        {
            for (auto stream : streams) { stream->Write(size, bytes); }
        }

        std::size_t Read(std::size_t size, const std::uint8_t* bytes)
        {
            std::size_t lastRead = 0;
            for (auto stream : streams)
            {
                lastRead = stream->Read(size, bytes);
            }
            return lastRead;
        }

    protected:
        std::vector<StreamBase*> streams;
    };
}