
#include "Exceptions.hpp"
#include "StreamBase.hpp"
#include "StreamBase.hpp"

#include <string>

namespace xPlat {

    // This represents a raw stream over a file contained in a .zip file.
    class InflateStream : public StreamBase
    {
    public:
        InflateStream(
            std::shared_ptr<StreamBase> stream,
            std::uint64_t compressedSize,
            std::uint64_t uncompressedSize
        ) : m_stream(stream)
        {
       
        }

        void Write(std::size_t size, const std::uint8_t* bytes) override;
        std::size_t Read(std::size_t size, const std::uint8_t* bytes) override;
        void Seek(std::uint64_t offset, Reference where) override;
        int Ferror() override;
        bool Feof() override;
        std::uint64_t Ftell()  override;

    protected:
        std::uint64_t m_seekPosition = 0;
        std::shared_ptr<StreamBase> m_stream;
        std::uint64_t m_compressedSize;
        std::uint64_t m_uncompressedSize;
    };
}