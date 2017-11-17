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
#include <set>

namespace xPlat {
  
    const std::uint32_t BLOCKMAP_BLOCK_SIZE = 65535;

    typedef struct Block
    {
        std::uint64_t size;
        std::uint64_t offset;
        std::vector<std::uint8_t> hash;
    } Block;

    typedef struct BlockCompare 
    {
        bool operator()(const Block& lhs, const Block& rhs) const
        {
            return lhs.offset < rhs.offset;
        }
    } BlockCompare;

    // This represents a subset of a Stream
    class BlockMapStream : public StreamBase
    {
    public:
        BlockMapStream(/*[In]*/ IStream* stream, /*[In]*/ std::set<Block, BlockCompare>& blocks)
        {
            for (auto block = blocks.begin(); block != blocks.end(); block++)
            {
                auto rangeStream = ComPtr<IStream>::Make<RangeStream>(block->offset, block->size, stream);
                auto hashStream = ComPtr<IStream>::Make<HashStream>(rangeStream.Get(), block->hash);
                m_blockStreams.push_back(hashStream);
            }
        }

        HRESULT STDMETHODCALLTYPE Seek(LARGE_INTEGER move, DWORD origin, ULARGE_INTEGER *newPosition) override
        {
#ifdef DISABLED
            LARGE_INTEGER newPos = { 0 };
            switch (origin)
            {
                case Reference::CURRENT:
                    m_relativePosition += move.u.LowPart;
                    break;
                case Reference::START:
                    m_relativePosition = move.u.LowPart;
                    break;
                case Reference::END:
                    m_relativePosition = m_cacheBuffer.size();
                    break;
            }
            m_relativePosition = std::max((std::uint32_t)0, std::min(m_relativePosition, (std::uint32_t)m_cacheBuffer.size()));
            if (newPosition) { newPosition->QuadPart = (std::uint64_t)m_relativePosition; }
#endif
            return S_OK;
        }

        HRESULT STDMETHODCALLTYPE Read(void* buffer, ULONG countBytes, ULONG* actualRead) override
        {
            return 0;
        }
      
    protected:
        std::vector<ComPtr<IStream>> m_blockStreams;
        std::uint32_t m_relativePosition;
    };
}