//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once
#define NOMINMAX /* windows.h, or more correctly windef.h, defines min as a macro... */
#include "MSIXWindows.hpp"
#include "Exceptions.hpp"
#include "StreamBase.hpp"
#include "RangeStream.hpp"
#include "HashStream.hpp"
#include "ComHelper.hpp"
#include "SHA256.hpp"
#include "AppxFactory.hpp"

#include <string>
#include <map>
#include <functional>
#include <algorithm>
#include <vector>

namespace MSIX {
  
    const std::uint64_t BLOCKMAP_BLOCK_SIZE = 65536; // 64KB

    typedef struct Block
    {
        std::uint64_t compressedSize;
        std::vector<std::uint8_t> hash;
    } Block;

    typedef struct BlockPlusStream : Block
    {
        std::uint64_t   size;
        std::uint64_t   offset;         
        ComPtr<IStream> stream;
    } BlockPlusStream;

    // This represents a subset of a Stream
    class BlockMapStream final : public StreamBase
    {
    public:
        BlockMapStream(IMSIXFactory* factory, std::string decodedName, const ComPtr<IStream>& stream, std::vector<Block>& blocks)
            : m_factory(factory), m_decodedName(decodedName), m_stream(stream)
        {
            // Determine overall stream size
            ULARGE_INTEGER uli;
            LARGE_INTEGER li;
            li.QuadPart = 0;
            ThrowHrIfFailed(stream->Seek(li, STREAM_SEEK_END, &uli));
            
            m_streamSize = uli.QuadPart;

            // Reset seek position to beginning
            li.QuadPart = 0;
            ThrowHrIfFailed(stream->Seek(li, STREAM_SEEK_SET, nullptr));

            // Build a vector of all HashStream->RangeStream's for the blocks in the blockmap
            std::uint64_t offset = 0;
            std::uint64_t sizeRemaining = m_streamSize;
            for (auto block = blocks.begin(); ((sizeRemaining != 0) && (block != blocks.end())); block++)
            {
                auto rangeStream = ComPtr<IStream>::Make<RangeStream>(offset, std::min(sizeRemaining, BLOCKMAP_BLOCK_SIZE), stream);                
                auto hashStream = ComPtr<IStream>::Make<HashStream>(rangeStream, block->hash);
                std::uint64_t blockSize = std::min(sizeRemaining, BLOCKMAP_BLOCK_SIZE);

                BlockPlusStream bs;
                bs.offset = offset;
                bs.size   = blockSize;
                bs.stream = hashStream;
                bs.hash   = block->hash;
                m_blockStreams.emplace_back(std::move(bs));
                
                offset          += blockSize;
                sizeRemaining   -= blockSize;
            }

            // Reset seek position to beginning
            ThrowHrIfFailed(stream->Seek(li, STREAM_SEEK_SET, nullptr));
            ThrowHrIfFailed(Seek(li, STREAM_SEEK_SET, nullptr));
        }

        HRESULT STDMETHODCALLTYPE Seek(LARGE_INTEGER move, DWORD origin, ULARGE_INTEGER *newPosition) noexcept override
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

            m_currentBlock = m_blockStreams.begin();
            return S_OK;
        } CATCH_RETURN();

        HRESULT STDMETHODCALLTYPE Read(void* buffer, ULONG countBytes, ULONG* actualRead) noexcept override
        {
            std::uint32_t bytesRead = 0;
            if (m_relativePosition < m_streamSize)
            {
                std::uint32_t bytesToRead = std::min(static_cast<std::uint32_t>(countBytes), static_cast<std::uint32_t>(m_streamSize - m_relativePosition));
                while (m_currentBlock != m_blockStreams.end() && bytesToRead > 0)
                {
                    if ((m_currentBlock->offset + m_currentBlock->size) <= m_relativePosition)
                    {
                        m_currentBlock++;
                    }
                    else if (m_currentBlock->offset <= m_relativePosition)
                    {
                        std::uint64_t positionInBlock = m_relativePosition - m_currentBlock->offset;
                        LARGE_INTEGER li{0};
                        li.QuadPart = positionInBlock;
                        ThrowHrIfFailed(m_currentBlock->stream->Seek(li, STREAM_SEEK_SET, nullptr));

                        std::uint32_t count = std::min(bytesToRead, static_cast<std::uint32_t>(m_currentBlock->size - positionInBlock));
                        ULONG actual = 0;
                        ThrowHrIfFailed(m_currentBlock->stream->Read(buffer, count, &actual));

                        buffer = static_cast<std::uint8_t*>(buffer) + actual;
                        m_relativePosition += actual;
                        bytesToRead -= actual;
                        bytesRead += actual;
                    }
                    else
                    {
                        m_currentBlock = m_blockStreams.begin();
                    }
                }
            }
            if (actualRead) { *actualRead = bytesRead; }
            return (countBytes == bytesRead) ? S_OK : S_FALSE;
        } CATCH_RETURN();

        HRESULT STDMETHODCALLTYPE GetCompressionOption(APPX_COMPRESSION_OPTION* compressionOption) noexcept override try
        {
            return m_stream.As<IAppxFile>()->GetCompressionOption(compressionOption);
        } CATCH_RETURN();

        HRESULT STDMETHODCALLTYPE GetName(LPWSTR* fileName) noexcept override
        {
            return m_factory->MarshalOutString(m_decodedName, fileName);
        } CATCH_RETURN();

        HRESULT STDMETHODCALLTYPE GetContentType(LPWSTR* contentType) noexcept override try
        {
            return m_stream.As<IAppxFile>()->GetContentType(contentType);
        } CATCH_RETURN();
        
        HRESULT STDMETHODCALLTYPE GetSize(UINT64* size) noexcept override try
        {
            if (size) { *size = m_streamSize; }
            return static_cast<HRESULT>(Error::OK);
        } CATCH_RETURN();
      
    protected:
        std::vector<BlockPlusStream>::iterator m_currentBlock;
        std::vector<BlockPlusStream> m_blockStreams;
        std::uint64_t m_relativePosition;
        std::uint64_t m_streamSize;
        std::string m_decodedName;
        ComPtr<IStream> m_stream;
        IMSIXFactory* m_factory;
    };
}