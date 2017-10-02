#pragma once

#include <iostream>
#include <string>
#include <cstdio>

#include "Exceptions.hpp"
#include "StreamBase.hpp"

namespace xPlat {
    class FileException : public ExceptionBase
    {
    public:
        FileException(std::string message, uint32_t error = 0) :
            reason(message),
            ExceptionBase(ExceptionBase::Facility::FILE)
        {
            SetLastError(error);
        }

        std::string reason;
    };

    // TODO: turns out we DO need some sort of abstraction over storage -- investigate using BOOST?
    class FileStream : public StreamBase
    {
    public:
        enum Mode { READ = 0, WRITE, APPEND, READ_UPDATE, WRITE_UPDATE, APPEND_UPDATE };

        FileStream(std::string&& path, Mode mode) : name(path)
        {
            static const char* modes[] = { "rb", "wb", "ab", "r+b", "w+b", "a+b" };
            file = std::fopen(path.c_str(), modes[mode]);
            if (!file)
            {
                throw FileException(path);
            }
        }

        virtual ~FileStream()
        {
            Close();
        }

        virtual void Close() override
        {
            if (file) {
                int rc = std::fclose(file);
                if (rc != 0) { throw FileException(name, rc); }
                file = nullptr;
            }
        }

        virtual void Seek(std::uint64_t to, StreamBase::Reference whence) override
        {
            int rc = std::fseek(file, to, whence);
            if (rc != 0) { throw FileException(name, rc); }
            offset = Ftell();
        }

        virtual std::size_t Read(std::size_t size, const std::uint8_t* bytes) override
        {
            std::size_t bytesRead = std::fread(
                static_cast<void*>(const_cast<std::uint8_t*>(bytes)), 1, size, file
            );
            if (bytesRead < size && !Feof())
            {
                throw FileException(name, Ferror());
            }
            offset = Ftell();
            return bytesRead;
        }

        virtual int Ferror() override
        {
            return std::ferror(file);
        }

        virtual int Feof() override
        {
            return std::feof(file);
        }

        virtual void Write(std::size_t size, const std::uint8_t* bytes) override
        {
            std::size_t bytesWritten = std::fwrite(
                static_cast<void*>(const_cast<std::uint8_t*>(bytes)), 1, size, file
            );
            if (bytesWritten != size)
            {
                throw FileException(name, std::ferror(file));
            }
            offset = Ftell();
        }

        virtual std::uint64_t Ftell()
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
