//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#include "StreamHelper.hpp"
#include "StreamBase.hpp"

namespace MSIX {
    namespace Helper {

        std::vector<std::uint8_t> CreateBufferFromStream(const ComPtr<IStream>& stream)
        {
            // Create buffer from stream
            LARGE_INTEGER start = { 0 };
            ULARGE_INTEGER end = { 0 };
            ThrowHrIfFailed(stream->Seek(start, StreamBase::Reference::END, &end));
            ThrowHrIfFailed(stream->Seek(start, StreamBase::Reference::START, nullptr));

            std::uint32_t streamSize = end.u.LowPart;
            std::vector<std::uint8_t> buffer(streamSize);
            ULONG actualRead = 0;
            ThrowHrIfFailed(stream->Read(buffer.data(), streamSize, &actualRead));
            ThrowErrorIf(Error::FileRead, (actualRead != streamSize), "read error");

            // move the underlying stream back to the beginning.
            ThrowHrIfFailed(stream->Seek(start, StreamBase::Reference::START, nullptr));
            return buffer;
        }

        std::string CreateStringFromStream(IStream* stream)
        {
            // Create buffer from stream
            LARGE_INTEGER start = { 0 };
            ULARGE_INTEGER end = { 0 };
            ThrowHrIfFailed(stream->Seek(start, StreamBase::Reference::END, &end));
            ThrowHrIfFailed(stream->Seek(start, StreamBase::Reference::START, nullptr));

            std::uint32_t streamSize = end.u.LowPart;
            std::string buffer(streamSize, ' ');
            ULONG actualRead = 0;
            ThrowHrIfFailed(stream->Read(&buffer[0], streamSize, &actualRead));
            ThrowErrorIf(Error::FileRead, (actualRead != streamSize), "read error");

            // move the underlying stream back to the beginning.
            ThrowHrIfFailed(stream->Seek(start, StreamBase::Reference::START, nullptr));
            return buffer;
        }

        std::pair<std::uint32_t, std::unique_ptr<std::uint8_t[]>> CreateRawBufferFromStream(const ComPtr<IStream>& stream)
        {
            // Create buffer from stream
            LARGE_INTEGER start = { 0 };
            ULARGE_INTEGER end = { 0 };
            ThrowHrIfFailed(stream->Seek(start, StreamBase::Reference::END, &end));
            ThrowHrIfFailed(stream->Seek(start, StreamBase::Reference::START, nullptr));

            std::uint32_t streamSize = end.u.LowPart;
            std::unique_ptr<std::uint8_t[]> buffer = std::make_unique<std::uint8_t[]>(streamSize);
            ULONG actualRead = 0;
            ThrowHrIfFailed(stream->Read(buffer.get(), streamSize, &actualRead));
            ThrowErrorIf(Error::FileRead, (actualRead != streamSize), "read error");

            // move the underlying stream back to the beginning.
            ThrowHrIfFailed(stream->Seek(start, StreamBase::Reference::START, nullptr));
            return std::make_pair(streamSize, std::move(buffer));
        }

        StreamProcessor::iterator& StreamProcessor::iterator::operator++()
        {
            ReadNextBytes();
            return *this;
        }

        const std::vector<std::uint8_t>& StreamProcessor::iterator::operator*()
        {
            return m_bytes;
        }

        bool StreamProcessor::iterator::operator!=(const iterator& other)
        {
            // The only equality is when both are end
            return !(isEnd && other.isEnd);
        }

        StreamProcessor::iterator::iterator(IStream* stream, size_t blockSize) :
            m_stream(stream), m_blockSize(blockSize)
        {
            m_bytes.resize(m_blockSize);
            ReadNextBytes();
        }

        StreamProcessor::iterator::iterator() :
            isEnd(true) {}

        void StreamProcessor::iterator::ReadNextBytes()
        {
            const ULONG blockSize = static_cast<ULONG>(m_blockSize);

            ULONG bytesRead = 0;
            m_stream->Read(static_cast<void*>(m_bytes.data()), blockSize, &bytesRead);

            if (bytesRead == 0)
            {
                isEnd = true;
                return;
            }
            
            if (bytesRead != blockSize)
            {
                // Try to read more data, to ensure that there is indeed no more.
                // Intentionally ignore any errors that might occur.
                ULONG moreBytesRead = 0;
                do
                {
                    bytesRead += moreBytesRead;
                    moreBytesRead = 0;
                    m_stream->Read(static_cast<void*>(&(m_bytes[bytesRead])), blockSize - bytesRead, &moreBytesRead);
                } while (moreBytesRead && bytesRead < blockSize);

                m_bytes.resize(bytesRead);
            }
        }

    }
}