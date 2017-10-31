#pragma once

#include <iostream>
#include <string>
#include <cstdio>

#include "Exceptions.hpp"
#include "StreamBase.hpp"

namespace xPlat {
    class FileStream : public StreamBase
    {
    public:
        enum Mode { READ = 0, WRITE, APPEND, READ_UPDATE, WRITE_UPDATE, APPEND_UPDATE };

        FileStream(const std::string& path, Mode mode)
        {
            static const char* modes[] = { "rb", "wb", "ab", "r+b", "w+b", "a+b" };
            file = std::fopen(path.c_str(), modes[mode]);
            ThrowErrorIfNot(Error::FileOpen, (file), path.c_str());
        }

        virtual ~FileStream() override
        {
            Close();
        }

        void Close()
        {
            if (file)
            {   // the most we would ever do w.r.t. a failure from fclose is *maybe* log something...
                std::fclose(file);
                file = nullptr;
            }
        }

        HRESULT Seek(LARGE_INTEGER move, DWORD origin, ULARGE_INTEGER *newPosition) override
        {
            return ResultOf([&] {
                int rc = std::fseek(file, move.QuadPart, origin);
                ThrowErrorIfNot(Error::FileSeek, (rc == 0), "seek failed");
                offset = Ftell();
                if (newPosition) { newPosition->QuadPart = offset; }
            });
        }

        HRESULT Read(void* buffer, ULONG countBytes, ULONG* bytesRead) override
        {
            if (bytesRead) { *bytesRead = 0; }
            return ResultOf([&] {
                ULONG result = static_cast<ULONG>(std::fread(buffer, sizeof(std::uint8_t), countBytes, file));
                ThrowErrorIfNot(Error::FileRead, (result == countBytes || Feof()), "read failed");
                offset = Ftell();
                if (bytesRead) { *bytesRead = result; }
            });
        }

        HRESULT Write(void const *buffer, ULONG countBytes, ULONG *bytesWritten) override
        {
            if (bytesWritten) { *bytesWritten = 0; }
            return ResultOf([&] {
                ULONG result = static_cast<ULONG>(std::fwrite(buffer, sizeof(std::uint8_t), countBytes, file));
                ThrowErrorIfNot(Error::FileWrite, (result == countBytes), "write failed");
                offset = Ftell();
                if (bytesWritten) { *bytesWritten = result; }
            });
        }

    protected:
        inline int Ferror() { return std::ferror(file); }
        inline bool Feof()  { return 0 != std::feof(file); }
        inline void Flush() { std::fflush(file); }

        inline std::uint64_t Ftell()
        {
            auto result = ftell(file);
            return static_cast<std::uint64_t>(result);
        }

        std::uint64_t offset = 0;
        std::string name;
        FILE* file;
    };
}
