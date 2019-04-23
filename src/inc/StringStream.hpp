//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once

#include <Exceptions.hpp>
#include <StreamBase.hpp>

#include <sstream>

namespace MSIX {

    class StringStream final : public StreamBase
    {
    public:
        StringStream() {}
        ~StringStream() {}

        HRESULT STDMETHODCALLTYPE Read(void* buffer, ULONG countBytes, ULONG* bytesRead) noexcept override try
        {
            ULONG amountToRead = std::min(countBytes, static_cast<ULONG>(m_data.tellp()) - m_offset);
            auto buf = m_data.rdbuf();
            if (amountToRead > 0) { memcpy(buffer, &(buf), amountToRead); }
            m_offset += amountToRead;
            if (bytesRead) { *bytesRead = amountToRead; }
            m_data.seekp(m_offset);
            return static_cast<HRESULT>(Error::OK);
        } CATCH_RETURN();

        HRESULT STDMETHODCALLTYPE Seek(LARGE_INTEGER move, DWORD origin, ULARGE_INTEGER *newPosition) noexcept override try
        {
            LARGE_INTEGER newPos {0};
            switch (origin)
            {
            case Reference::CURRENT:
                newPos.QuadPart = m_offset + move.QuadPart;
                break;
            case Reference::START:
                newPos.QuadPart = move.QuadPart;
                break;
            case Reference::END:
                newPos.QuadPart = static_cast<ULONG>(m_data.tellp()) + move.QuadPart;
                break;
            }
            m_offset = std::min(newPos.u.LowPart, static_cast<ULONG>(m_data.tellp()));
            m_data.seekp(m_offset);
            if (newPosition) { newPosition->QuadPart = newPos.QuadPart; }
            return static_cast<HRESULT>(Error::OK);
        } CATCH_RETURN();

        HRESULT STDMETHODCALLTYPE Write(const void *buffer, ULONG countBytes, ULONG *bytesWritten) noexcept override try
        {
            if (bytesWritten) { *bytesWritten = 0; }
            ULONG oldOffset = m_offset;
            m_data.write(static_cast<const char*>(buffer), static_cast<std::streamsize>(countBytes));
            m_offset = std::min(m_offset + countBytes, static_cast<ULONG>(m_data.tellp()));
            ThrowErrorIfNot(Error::FileWrite, ((m_offset - oldOffset) == countBytes), "write failed");
            if (bytesWritten) { *bytesWritten = countBytes; }
            m_data.seekp(m_offset);
            return static_cast<HRESULT>(Error::OK);
        } CATCH_RETURN();

    protected:
        ULONG m_offset = 0;
        std::ostringstream m_data;
    };
} // namespace MSIX