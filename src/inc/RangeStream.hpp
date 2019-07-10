//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once
#include "Exceptions.hpp"
#include "StreamBase.hpp"
#include "ComHelper.hpp"
#include "MsixFeatureSelector.hpp"

#include <string>
#include <map>
#include <functional>

namespace MSIX {

    // This represents a subset of a Stream
    class RangeStream : public StreamBase
    {
    public:
        RangeStream(std::uint64_t offset, std::uint64_t size, IStream* stream) :
            m_offset(offset),
            m_size(size),
            m_stream(stream)
        {
        }

        // For writing/pack
        // This simply keeps track of the amount of data written to the stream, as well as
        // limiting any Read/Seek to the data written, rather than the entire underlying stream.
        RangeStream(IStream* stream) : m_stream(stream), m_size(0)
        {
            THROW_IF_PACK_NOT_ENABLED
            ULARGE_INTEGER pos = { 0 };
            ThrowHrIfFailed(m_stream->Seek({ 0 }, Reference::CURRENT, &pos));
            m_offset = pos.QuadPart;
        }

        HRESULT STDMETHODCALLTYPE Seek(LARGE_INTEGER move, DWORD origin, ULARGE_INTEGER *newPosition) noexcept override try
        {
            // Determine new range relative position
            LARGE_INTEGER newPos = { 0 };
            switch (origin)
            {
            case Reference::CURRENT:
                newPos.QuadPart = m_relativePosition + move.QuadPart;
                break;
            case Reference::START:
                newPos.QuadPart = move.QuadPart;
                break;
            case Reference::END:
                newPos.QuadPart = m_size + move.QuadPart;
                break;
            }

            // Constrain newPos to range relative values
            if (newPos.QuadPart < 0)
            {
                newPos.QuadPart = 0;
            }
            else if (static_cast<uint64_t>(newPos.QuadPart) > m_size)
            {
                newPos.QuadPart = m_size;
            }

            // Add in the underlying stream offset
            newPos.QuadPart += m_offset;

            ULARGE_INTEGER pos = { 0 };
            ThrowHrIfFailed(m_stream->Seek(newPos, Reference::START, &pos));
            m_relativePosition = std::min(static_cast<std::uint64_t>(pos.QuadPart - m_offset), m_size);
            if (newPosition) { newPosition->QuadPart = m_relativePosition; }
            return static_cast<HRESULT>(Error::OK);
        } CATCH_RETURN();

        HRESULT STDMETHODCALLTYPE Read(void* buffer, ULONG countBytes, ULONG* bytesRead) noexcept override try
        {
            LARGE_INTEGER offset = {0};
            offset.QuadPart = m_relativePosition + m_offset;
            ThrowHrIfFailed(m_stream->Seek(offset, StreamBase::START, nullptr));
            ULONG amountToRead = std::min(countBytes, static_cast<ULONG>(m_size - m_relativePosition));
            ULONG amountRead = 0;
            ThrowHrIfFailed(m_stream->Read(buffer, amountToRead, &amountRead));
            ThrowErrorIf(Error::FileRead, (amountToRead != amountRead), "Did not read as much as requesteed.");
            m_relativePosition += amountRead;
            if (bytesRead) { *bytesRead = amountRead; }
            ThrowErrorIf(Error::FileSeekOutOfRange, (m_relativePosition > m_size), "seek pointer out of bounds.");
            return static_cast<HRESULT>(Error::OK);
        } CATCH_RETURN();

        HRESULT STDMETHODCALLTYPE Write(const void *buffer, ULONG countBytes, ULONG *bytesWritten) noexcept override try
        {
            THROW_IF_PACK_NOT_ENABLED
            LARGE_INTEGER offset = { 0 };
            offset.QuadPart = m_relativePosition + m_offset;
            ThrowHrIfFailed(m_stream->Seek(offset, StreamBase::START, nullptr));
            ULONG amountWritten = 0;
            ThrowHrIfFailed(m_stream->Write(buffer, countBytes, &amountWritten));
            ThrowErrorIf(Error::FileWrite, (countBytes != amountWritten), "Did not write as much as requesteed.");
            m_relativePosition += amountWritten;
            m_size = std::max(m_size, m_relativePosition);
            if (bytesWritten) { *bytesWritten = amountWritten; }
            return static_cast<HRESULT>(Error::OK);
        } CATCH_RETURN();

        std::uint64_t Size() { return m_size; }

    protected:
        std::uint64_t m_offset;
        std::uint64_t m_size;
        std::uint64_t m_relativePosition = 0;
        ComPtr<IStream> m_stream;
    };
}