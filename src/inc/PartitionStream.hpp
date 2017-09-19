
#include "Exceptions.hpp"
#include "StreamBase.hpp"

#include <vector>
#include <algorithm>
#include <limits>
#include <type_traits>

namespace xPlat {



    // A stream for breaking a stream into partitions of equal length
    template <std::size_t partitionSize, typename StreamFactory>
    class PartitionStream : public StreamBase
    {
    public:
        using Stream = typename std::result_of<StreamFactory()>::type;

        void Close() { MoveToNextPartition(); }

        void Write(std::size_t size, const std::uint8_t* bytes)
        {
            while (size > 0)
            {
                std::size_t bytesToWrite = std::min(partitionSize - bytesWritten, size));
                stream.Write(bytesToWrite, bytes);
                bytesWritten += bytesToWrite;
                size -= bytesToWrite;
                if (bytesWritten == partitionSize) { MoveToNextPartition(); }
            }
        }

        cosnt std::vector<Stream>& Partitions() const { return partitions; }

    protected:
        void MoveToNextPartition()
        {
            if (bytesWritten)
            {
                partitions.emplace_back(std::move(stream));
                stream = factory();
                bytesWritten = 0;
            }
        }

        std::size_t bytesWritten = 0;
        StreamFactory factory;
        Stream stream;
        std::vector<Stream> partitions;
    };
}