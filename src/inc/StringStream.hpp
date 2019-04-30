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
        HRESULT STDMETHODCALLTYPE Read(void* buffer, ULONG countBytes, ULONG* bytesRead) noexcept override try
        {
            auto current = m_data.tellp();
            m_data.seekp(0, std::ios_base::end);
            auto available = m_data.tellp() - current;
            m_data.seekp(current); // rewind
            auto buf = m_data.rdbuf();
            ULONG amountToRead = std::min(countBytes, static_cast<ULONG>(available));
            if (amountToRead > 0)
            {
                buf->sgetn(static_cast<char*>(buffer),amountToRead);
                m_data.seekp(static_cast<std::ostringstream::off_type>(amountToRead), std::ios_base::cur);
            }
            if (bytesRead) { *bytesRead = amountToRead; }
            return static_cast<HRESULT>(Error::OK);
        } CATCH_RETURN();

        HRESULT STDMETHODCALLTYPE Seek(LARGE_INTEGER move, DWORD origin, ULARGE_INTEGER *newPosition) noexcept override try
        {
            std::ios_base::seekdir dir;
            switch (origin)
            {
            case Reference::CURRENT:
                dir = std::ios_base::cur;
                break;
            case Reference::START:
                dir = std::ios_base::beg;
                break;
            case Reference::END:
                dir = std::ios_base::end;
                break;
            }
            m_data.seekp(static_cast<std::ostringstream::off_type>(move.QuadPart), dir);
            ThrowErrorIf(Error::FileWrite, m_data.rdstate() != std::ios_base::goodbit, "StringStream Seek failed");
            if (newPosition) { newPosition->QuadPart = static_cast<ULONG>(m_data.tellp()); }
            return static_cast<HRESULT>(Error::OK);
        } CATCH_RETURN();

        HRESULT STDMETHODCALLTYPE Write(const void *buffer, ULONG countBytes, ULONG *bytesWritten) noexcept override try
        {
            if (bytesWritten) { *bytesWritten = 0; }
            m_data.write(static_cast<const char*>(buffer), static_cast<std::streamsize>(countBytes));
            // std::basic_ostream::write : Characters are inserted into the output sequence until one of the following occurs:
            // exactly count characters are inserted or inserting into the output sequence fails (in which case setstate(badbit) is called)
            // If the state is std::ios_base::goodbit we know the exact number of bytes were written.
            ThrowErrorIf(Error::FileWrite, m_data.rdstate() != std::ios_base::goodbit, "StringStream Write failed");
            if (bytesWritten) { *bytesWritten = countBytes; }
            return static_cast<HRESULT>(Error::OK);
        } CATCH_RETURN();

    protected:
        std::stringstream m_data;
    };
} // namespace MSIX