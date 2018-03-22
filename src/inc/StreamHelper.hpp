//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once

#include "AppxPackaging.hpp"
#include "Exceptions.hpp"

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
    }
}