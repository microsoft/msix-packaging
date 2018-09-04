//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once

#include <iostream>
#include <string>
#include <cstdio>

#include "Exceptions.hpp"
#include "StreamBase.hpp"

namespace MSIX {
    class FileStream final : public StreamBase
    {
    public:
        enum Mode { READ = 0, WRITE, APPEND, READ_UPDATE, WRITE_UPDATE, APPEND_UPDATE };

        FileStream(const std::string& name, Mode mode) : m_name(name)
        {
            static const char* modes[] = { "rb", "wb", "ab", "r+b", "w+b", "a+b" };
            #ifdef WIN32
            errno_t err = fopen_s(&m_file, name.c_str(), modes[mode]);
            std::ostringstream builder;
            builder << "file: '" << name << "' does not exist.";
            ThrowErrorIfNot(Error::FileOpen, (err==0), builder.str().c_str());
            #else
            m_file = std::fopen(name.c_str(), modes[mode]);
            ThrowErrorIfNot(Error::FileOpen, (m_file), name.c_str());
            #endif

            // Get size of the file
            LARGE_INTEGER start = { 0 };
            ULARGE_INTEGER end = { 0 };
            ThrowHrIfFailed(Seek(start, StreamBase::Reference::END, &end));
            ThrowHrIfFailed(Seek(start, StreamBase::Reference::START, nullptr));
            m_size = end.u.LowPart;
        }

        virtual ~FileStream() override
        {
            Close();
        }

        void Close()
        {
            if (m_file)
            {   // the most we would ever do w.r.t. a failure from fclose is *maybe* log something...
                std::fclose(m_file);
                m_file = nullptr;
            }
        }

        // IStream
        HRESULT STDMETHODCALLTYPE Seek(LARGE_INTEGER move, DWORD origin, ULARGE_INTEGER* newPosition) noexcept override try
        {
            int rc = std::fseek(m_file, static_cast<long>(move.QuadPart), origin);
            ThrowErrorIfNot(Error::FileSeek, (rc == 0), "seek failed");
            m_offset = Ftell();
            if (newPosition) { newPosition->QuadPart = m_offset; }
            return static_cast<HRESULT>(Error::OK);
        } CATCH_RETURN();

        HRESULT STDMETHODCALLTYPE Read(void* buffer, ULONG countBytes, ULONG* bytesRead) noexcept override try
        {
            if (bytesRead) { *bytesRead = 0; }
            ULONG result = static_cast<ULONG>(std::fread(buffer, sizeof(std::uint8_t), countBytes, m_file));
            ThrowErrorIfNot(Error::FileRead, (result == countBytes || Feof()), "read failed");
            m_offset = Ftell();
            if (bytesRead) { *bytesRead = result; }
            return static_cast<HRESULT>(Error::OK);
        } CATCH_RETURN();

        HRESULT STDMETHODCALLTYPE Write(const void *buffer, ULONG countBytes, ULONG *bytesWritten) noexcept override try
        {
            if (bytesWritten) { *bytesWritten = 0; }
            ULONG result = static_cast<ULONG>(std::fwrite(buffer, sizeof(std::uint8_t), countBytes, m_file));
            ThrowErrorIfNot(Error::FileWrite, (result == countBytes), "write failed");
            m_offset = Ftell();
            if (bytesWritten) { *bytesWritten = result; }
            return static_cast<HRESULT>(Error::OK);
        } CATCH_RETURN();

        // IStreamInternal
        std::string GetName() override { return m_name; }

    protected:
        inline int Ferror() { return std::ferror(m_file); }
        inline bool Feof()  { return 0 != std::feof(m_file); }
        inline void Flush() { std::fflush(m_file); }

        inline std::uint64_t Ftell()
        {
            auto result = std::ftell(m_file);
            return static_cast<std::uint64_t>(result);
        }

        std::uint64_t m_offset = 0;
        std::uint64_t m_size = 0;
        std::string m_name;
        FILE* m_file;
    };
}
