//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once

#include "AppxPackaging.hpp"
#include "Exceptions.hpp"
#include "ComHelper.hpp"
#include "StreamBase.hpp"

#include <string>
#include <utility>
#include <vector>

namespace MSIX {
    namespace Helper {

        std::vector<std::uint8_t> CreateBufferFromStream(const ComPtr<IStream>& stream);

        std::string CreateStringFromStream(IStream* stream);

        std::pair<std::uint32_t, std::unique_ptr<std::uint8_t[]>> CreateRawBufferFromStream(const ComPtr<IStream>& stream);

        inline void WriteStringToStream(const ComPtr<IStream>& stream, const std::string& toWrite)
        {
            ULONG written = 0;
            ThrowHrIfFailed(stream->Write(static_cast<const void*>(toWrite.data()), static_cast<ULONG>(toWrite.size()), &written));
            ThrowErrorIf(Error::FileWrite, (static_cast<ULONG>(toWrite.size()) != written), "write failed");
        }

        // Helper struct that allows range based for loops to operate on blocks of data from the given stream.
        struct StreamProcessor
        {
            // Forward only iterator; advancing overwrites old data
            struct iterator
            {
                friend StreamProcessor;

                iterator& operator++();
                const std::vector<std::uint8_t>& operator*();
                bool operator!=(const iterator& other);

            private:
                iterator(IStream* stream, size_t blockSize);
                iterator();

                void ReadNextBytes();

                ComPtr<IStream> m_stream;
                size_t m_blockSize;
                std::vector<std::uint8_t> m_bytes;
                bool isEnd = false;
            };

            StreamProcessor(IStream* stream, size_t blockSize) :
                m_stream(stream), m_blockSize(blockSize) {}

            iterator begin()
            {
                return { m_stream.Get(), m_blockSize };
            }

            iterator end()
            {
                return {};
            }

        private:
            ComPtr<IStream> m_stream;
            size_t m_blockSize;
        };

        // Reverts a stream's position on destruction
        struct StreamPositionReset
        {
            StreamPositionReset(IStream* stream) : m_stream(stream)
            {
                ThrowHrIfFailed(m_stream->Seek({ 0 }, StreamBase::Reference::CURRENT, &m_pos));
            }

            ~StreamPositionReset()
            {
                Reset();
            }

            void Reset()
            {
                if (m_stream)
                {
                    LARGE_INTEGER target;
                    target.QuadPart = static_cast<LONGLONG>(m_pos.QuadPart);
                    ThrowHrIfFailed(m_stream->Seek(target, StreamBase::Reference::START, nullptr));
                    m_stream = nullptr;
                }
            }

            void Release()
            {
                m_stream = nullptr;
            }

        private:
            ComPtr<IStream> m_stream;
            ULARGE_INTEGER m_pos;
        };
    }
}