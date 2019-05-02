//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 

#include "DeflateStream.hpp"
#include "Exceptions.hpp"

#include <vector>

namespace MSIX {

    DeflateStream::DeflateStream(const ComPtr<IStream>& stream) : m_stream(stream)
    {
        m_zstrm.zalloc = Z_NULL;
        m_zstrm.zfree = Z_NULL;
        m_zstrm.opaque = Z_NULL;
        auto result = deflateInit2(&m_zstrm, Z_BEST_COMPRESSION, Z_DEFLATED, -MAX_WBITS, MAX_MEM_LEVEL, Z_DEFAULT_STRATEGY);
        ThrowErrorIf(Error::DeflateInitialize, result != Z_OK, "Error calling deflateinit2");
    }

    DeflateStream::~DeflateStream()
    {
        deflateEnd(&m_zstrm);
    }

    // IStream
    HRESULT STDMETHODCALLTYPE DeflateStream::Seek(LARGE_INTEGER move, DWORD origin, ULARGE_INTEGER *newPosition) noexcept try
    {
        // just forward to the VectorStream
        ThrowHrIfFailed(m_stream->Seek(move, origin, newPosition));
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    HRESULT STDMETHODCALLTYPE DeflateStream::Read(void* buffer, ULONG countBytes, ULONG* bytesRead) noexcept try
    {
        // just forward to the VectorStream
        ThrowHrIfFailed(m_stream->Read(buffer, countBytes, bytesRead));
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    // Caller should NOT assume that bytesWritten returned is going to be equal to countBytes
    HRESULT STDMETHODCALLTYPE DeflateStream::Write(void const *buffer, ULONG countBytes, ULONG *bytesWritten) noexcept try
    {
        // Important! If this stream is asked to write with 0 bytes, then it means that we are done.
        // We need to terminate the stream anc call deflate with Z_FINISH.
        int disposition = (countBytes > 0) ? Z_FULL_FLUSH : Z_FINISH;
        m_zstrm.next_in = reinterpret_cast<Bytef *>(const_cast<void*>(buffer));
        m_zstrm.avail_in = static_cast<std::uint32_t>(countBytes);
        auto toWrite = Deflate(disposition);
        ThrowHrIfFailed(m_stream->Write(toWrite.data(), static_cast<ULONG>(toWrite.size()), bytesWritten));
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    std::vector<std::uint8_t> DeflateStream::Deflate(int disposition)
    {
        std::vector<std::uint8_t> compressedBuffer;
        std::vector<std::uint8_t> deflateBuffer(1024);
        do
        {
            m_zstrm.next_out = deflateBuffer.data();
            m_zstrm.avail_out = static_cast<std::uint32_t>(deflateBuffer.size());
            auto result = deflate(&m_zstrm, disposition);
            if (disposition == Z_FINISH && result == Z_STREAM_END)
            {
                result = Z_OK;
            }
            ThrowErrorIf(Error::DeflateWrite, result != Z_OK, "Error deflating stream");
            auto have = deflateBuffer.size() - m_zstrm.avail_out;
            compressedBuffer.insert(compressedBuffer.end(), deflateBuffer.data(), deflateBuffer.data() + have);
       } while (m_zstrm.avail_out == 0);
       return compressedBuffer;
    }

}