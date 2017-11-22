#pragma once
#define NOMINMAX /* windows.h, or more correctly windef.h, defines min as a macro... */
#include "AppxWindows.hpp"
#include "Exceptions.hpp"
#include "StreamBase.hpp"
#include "RangeStream.hpp"
#include "HashStream.hpp"
#include "ComHelper.hpp"
#include "SHA256.hpp"

#include <string>
#include <map>
#include <functional>
#include <algorithm>
#include <vector>

namespace xPlat {
  
    const std::uint32_t BLOCKMAP_BLOCK_SIZE = 65536; // 64KB

    typedef struct Block
    {
        std::uint32_t size;
        std::uint64_t offset; 
        std::vector<std::uint8_t> hash;
    } Block;

    typedef struct BlockPlusStream : Block
    {
        ComPtr<IStream> stream;
    } BlockPlusStream;

    // This represents a subset of a Stream
    class BlockMapStream : public StreamBase
    {
    public:
        BlockMapStream(IStream* stream, std::vector<Block>& blocks)
        {
            // Build a vector of all HashStream->RangeStream's for the blocks in the blockmap
            std::uint64_t offset = 0;
            for (auto block = blocks.begin(); block != blocks.end(); block++)
            {
                offset += std::min(BLOCKMAP_BLOCK_SIZE, block->size);
                ThrowErrorIfNot(xPlat::Error::AppxSignatureInvalid, 
                    (block->size <= BLOCKMAP_BLOCK_SIZE), 
                    "block size must be less than 65536");
                auto rangeStream = ComPtr<IStream>::Make<RangeStream>(offset, block->size, stream);
                auto hashStream = ComPtr<IStream>::Make<HashStream>(rangeStream.Get(), block->hash);
                
                BlockPlusStream bs;
                bs.offset = offset;
                bs.size   = block->size;
                bs.stream = hashStream;
                bs.hash.assign(&block->hash[0], &block->hash[block->hash.size()]);
                m_blockStreams.push_back(bs);
            }

            // Determine overall stream size
            ULARGE_INTEGER uli;
            LARGE_INTEGER li;
            li.QuadPart = 0;
            ThrowHrIfFailed(stream->Seek(li, STREAM_SEEK_END, &uli));
            
            m_streamSize = uli.QuadPart;

            // Reset seek position to beginning
            li.QuadPart = 0;
            ThrowHrIfFailed(stream->Seek(li, STREAM_SEEK_SET, nullptr));
        }

        HRESULT STDMETHODCALLTYPE Seek(LARGE_INTEGER move, DWORD origin, ULARGE_INTEGER *newPosition) override
        {
            LARGE_INTEGER newPos = { 0 };
            switch (origin)
            {
                case Reference::CURRENT:
                    m_relativePosition += move.QuadPart;
                    break;
                case Reference::START:
                    m_relativePosition = move.QuadPart;
                    break;
                case Reference::END:
                    m_relativePosition = m_streamSize;
                    break;
            }
            m_relativePosition = std::max((std::uint64_t)0, std::min(m_relativePosition, m_streamSize));
            if (newPosition) { newPosition->QuadPart = m_relativePosition; }
            return S_OK;
        }

        HRESULT STDMETHODCALLTYPE Read(void* buffer, ULONG countBytes, ULONG* actualRead) override
        {
            std::uint32_t bytesRead = 0;
            if (m_relativePosition < m_streamSize)
            {
                std::uint32_t bytesToRead = std::min(static_cast<std::uint32_t>(countBytes), static_cast<std::uint32_t>(m_streamSize - m_relativePosition));

                for (auto block = m_blockStreams.begin(); block != m_blockStreams.end() && bytesToRead > 0; block++)
                {
                    if (block->offset <= m_relativePosition)
                    {
                        LARGE_INTEGER li{0};
                        Seek(li, STREAM_SEEK_SET, nullptr);
                        li.QuadPart = (m_relativePosition - block->offset);
                        ThrowHrIfFailed(block->stream.Get()->Seek(li, STREAM_SEEK_SET, nullptr));

                        std::uint32_t count = std::min(bytesToRead, static_cast<std::uint32_t>(block->size - (m_relativePosition - block->offset)));
                        ULONG actual = 0;
                        ThrowHrIfFailed(block->stream.Get()->Read(buffer, count, &actual));

                        buffer = static_cast<std::uint8_t*>(buffer) + actual;
                        m_relativePosition += actual;
                        bytesToRead -= actual;
                        bytesRead += actual;
                    }
                }
            }
            if (actualRead) { *actualRead = bytesRead; }
            return (countBytes == bytesRead) ? S_OK : S_FALSE;
        }
      
    protected:
        std::vector<BlockPlusStream> m_blockStreams;
        std::uint64_t m_relativePosition;
        std::uint64_t m_streamSize;
    };
}