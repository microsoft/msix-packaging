//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once
#define NOMINMAX /* windows.h, or more correctly windef.h, defines min as a macro... */
#include "MSIXWindows.hpp"
#include "Exceptions.hpp"
#include "StreamBase.hpp"
#include "ComHelper.hpp"
#include "SHA256.hpp"

#include <string>
#include <map>
#include <functional>
#include <algorithm>

namespace MSIX {
  
    // This represents a subset of a Stream
    class HashStream : public StreamBase
    {
    protected:
        bool m_validated;
        ComPtr<IStream> m_stream;
        std::vector<std::uint8_t>& m_expectedHash;
        std::unique_ptr<std::vector<std::uint8_t>> m_cacheBuffer;
        std::uint64_t m_relativePosition;
        size_t m_streamSize;

    public:
        HashStream(IStream* stream, std::vector<std::uint8_t>& expectedHash) :
            m_validated(false),
            m_stream(stream),
            m_expectedHash(expectedHash),
            m_relativePosition(0),
            m_streamSize(0)
        {
            ULARGE_INTEGER uli;
            LARGE_INTEGER li;
            li.QuadPart = 0;
            
            ThrowHrIfFailed(m_stream->Seek(li, StreamBase::Reference::END, &uli));
            ThrowHrIfFailed(m_stream->Seek(li, StreamBase::Reference::START, nullptr));
            m_streamSize = static_cast<size_t>(uli.u.LowPart);
        }

        void Validate()
        {
            if (m_validated) { return; }

            // read stream into cache buffer
            m_cacheBuffer = std::make_unique<std::vector<std::uint8_t>>(m_streamSize);            
            ULONG bytesRead = 0;
            ThrowHrIfFailed(m_stream->Read(m_cacheBuffer->data(), m_cacheBuffer->size(), &bytesRead));
            ThrowErrorIfNot(MSIX::Error::SignatureInvalid, bytesRead == m_streamSize, "read failed");
            
            // compute digest and compare against expected digest
            std::vector<std::uint8_t> hash;
            ThrowErrorIfNot(MSIX::Error::SignatureInvalid, 
                MSIX::SHA256::ComputeHash(m_cacheBuffer->data(), m_cacheBuffer->size(), hash), 
                "Invalid signature");
            ThrowErrorIfNot(MSIX::Error::SignatureInvalid, m_expectedHash.size() == hash.size(), "Signature is corrupt");
            ThrowErrorIfNot(
                MSIX::Error::SignatureInvalid,
                memcmp(m_expectedHash.data(), hash.data(), hash.size()) == 0,
                "Signature hash doesn't match digest hash"); //TODO: better exception

            m_validated = true;
        }

        void CacheSeek(LARGE_INTEGER move, DWORD origin, ULARGE_INTEGER *newPosition)
        {
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
                    m_relativePosition = m_streamSize;
                    break;
            }
            m_relativePosition = std::max((std::uint64_t)0, std::min(m_relativePosition, static_cast<std::uint64_t>(m_streamSize)));
            if (newPosition) { newPosition->QuadPart = (std::uint64_t)m_relativePosition; }
        }        

        HRESULT STDMETHODCALLTYPE Seek(LARGE_INTEGER move, DWORD origin, ULARGE_INTEGER *newPosition) override
        {
            return ResultOf([&]{
                if (m_cacheBuffer.get() == nullptr)
                {   ThrowHrIfFailed(m_stream->Seek(move, origin, newPosition));
                }
                // always call into cache seek to keep cache state aligned with the underlying stream state.
                CacheSeek(move, origin, newPosition);
                return static_cast<HRESULT>(Error::OK);
            });
        }

        void CacheRead(void* buffer, ULONG countBytes, ULONG* actualRead)
        {
            ThrowErrorIf(Error::Stg_E_Invalidpointer, (buffer == nullptr), "bad input");
            ULONG bytesToRead = std::min((std::uint32_t)countBytes, static_cast<std::uint32_t>((std::uint64_t)m_cacheBuffer->size() - m_relativePosition));
            if (bytesToRead)
            {
                memcpy(buffer, reinterpret_cast<BYTE*>(m_cacheBuffer->data()) + m_relativePosition, bytesToRead);
            }

            m_relativePosition += bytesToRead;
            if (m_streamSize == m_relativePosition) { m_cacheBuffer = nullptr; }
            if (actualRead) { *actualRead = bytesToRead; }
        }

        HRESULT STDMETHODCALLTYPE Read(void* buffer, ULONG countBytes, ULONG* actualRead) override
        {
            return ResultOf([&]{
                Validate();
                if (m_cacheBuffer.get() == nullptr)
                {   ThrowHrIfFailed(m_stream->Read(buffer, countBytes, actualRead));
                }
                else
                {   CacheRead(buffer, countBytes, actualRead);
                }
                return static_cast<HRESULT>(Error::OK);
            });            
        }

        HRESULT STDMETHODCALLTYPE GetCompressionOption(APPX_COMPRESSION_OPTION* compressionOption) override
        {
            return ResultOf([&]{ return m_stream.As<IAppxFile>()->GetCompressionOption(compressionOption); });
        }

        HRESULT STDMETHODCALLTYPE GetName(LPWSTR* fileName) override
        {
            return ResultOf([&]{ return m_stream.As<IAppxFile>()->GetName(fileName); });
        }

        HRESULT STDMETHODCALLTYPE GetContentType(LPWSTR* contentType) override
        {
            return ResultOf([&]{ return m_stream.As<IAppxFile>()->GetContentType(contentType); });
        }
        
        HRESULT STDMETHODCALLTYPE GetSize(UINT64* size) override
        {
            return ResultOf([&]{
                if (size) { *size = m_streamSize; }
                return static_cast<HRESULT>(Error::OK);
                });
        }
    };
}