#pragma once

#include <iostream>
#include <string>
#include <cstdio>

#include "Exceptions.hpp"
#include "StreamBase.hpp"

namespace xPlat {
    // TODO: turns out we DO need some sort of abstraction over storage -- investigate using BOOST?
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

        virtual void Close() override
        {
            if (file)
            {   // the most we would ever do w.r.t. a failure from fclose is *maybe* log something...
                std::fclose(file);
                file = nullptr;
            }
        }

        virtual void Seek(std::uint64_t to, StreamBase::Reference whence) override
        {
            int rc = std::fseek(file, to, whence);
            ThrowErrorIfNot(Error::FileSeek, (rc == 0), "seek failed");
            offset = Ftell();
        }

        virtual std::size_t Read(const std::uint8_t* start, const std::uint8_t* end) override
        {
            std::size_t bytesRead = std::fread(
                reinterpret_cast<void*>(const_cast<std::uint8_t*>(start)), 1, end-start, file
            );
            ThrowErrorIfNot(Error::FileRead, (bytesRead == (end-start) || Feof()), "read failed");
            offset = Ftell();
            return bytesRead;
        }

        virtual int Ferror() override
        {
            return std::ferror(file);
        }

        virtual bool Feof() override
        {
            return 0 != std::feof(file);
        }

        virtual void Write(const std::uint8_t* start, const std::uint8_t* end) override
        {
            std::size_t bytesWritten = std::fwrite(
                static_cast<void*>(const_cast<std::uint8_t*>(start)), 1, end-start, file
            );
            ThrowErrorIfNot(Error::FileWrite, (bytesWritten == (end-start)), "write failed");
            offset = Ftell();
        }

        void Flush() override
        {
            std::fflush(file);
        }

        virtual std::uint64_t Ftell() override
        {
            auto result = ftell(file);
            return static_cast<std::uint64_t>(result);
        }

    protected:
        std::uint64_t offset = 0;
        std::string name;
        FILE* file;
    };
}
