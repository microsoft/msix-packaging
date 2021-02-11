//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once

#include "ComHelper.hpp"
#include "StreamBase.hpp"

#include <vector>
#include <zlib.h>

namespace MSIX {

    class DeflateStream final : public StreamBase
    {
    public:
        DeflateStream(const ComPtr<IStream>& stream);
        ~DeflateStream();

        // IStream
        HRESULT STDMETHODCALLTYPE Seek(LARGE_INTEGER move, DWORD origin, ULARGE_INTEGER *newPosition) noexcept override;
        HRESULT STDMETHODCALLTYPE Read(void* buffer, ULONG countBytes, ULONG* bytesRead) noexcept override;
        HRESULT STDMETHODCALLTYPE Write(void const *buffer, ULONG countBytes, ULONG *bytesWritten) noexcept override;

        // IStreamInternal
        std::uint64_t GetSize() override { return m_stream.As<IStreamInternal>()->GetSize(); }
        bool IsCompressed() override { return m_stream.As<IStreamInternal>()->IsCompressed(); }
        std::string GetName() override { return m_stream.As<IStreamInternal>()->GetName(); }
    
    protected:
        std::vector<std::uint8_t> Deflate(int disposition);

        typedef enum
        {
            Open,
            Closed,
        }
        State;

        State m_state = State::Open;
        z_stream m_zstrm;
        ComPtr<IStream> m_stream;
    };
}
