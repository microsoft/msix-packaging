//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once

#include "AppxPackaging.hpp"
#include "Exceptions.hpp"
#include "StreamBase.hpp"

#include <utility>

namespace MSIX {
    namespace Helper {

        inline std::vector<std::uint8_t> CreateBufferFromStream(const ComPtr<IStream>& stream)
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

        inline std::pair<std::uint32_t, std::unique_ptr<std::uint8_t[]>> CreateRawBufferFromStream(const ComPtr<IStream>& stream)
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

        inline void WriteStringToStream(const ComPtr<IStream>& stream, const std::string& toWrite)
        {
            ULONG written;
            ThrowHrIfFailed(stream->Write(static_cast<const void*>(toWrite.data()), static_cast<ULONG>(toWrite.size()), &written));
            ThrowErrorIf(Error::FileWrite, (static_cast<ULONG>(toWrite.size()) != written), "write failed");
        }

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