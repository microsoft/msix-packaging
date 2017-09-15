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

        FileStream(std::string& path, Mode& mode) : name(path)
        {
            static char* modes[] = { "r", "w", "a", "r+", "w+", "a+" };
            file = std::fopen(path.c_str(), modes[mode]);
            if (!file)
            {
                throw FileException(path);
            }
        }

        void Close()
        {
            if (file) {
                int rc = std::fclose(file);
                if (rc != 0) { throw FileException(name, rc); }
                file = nullptr;
            }
        }

        void Seek(long offset, Reference where)
        {
            int rc = std::fseek(file, offset, where);
            if (rc != 0) { throw FileException(name, rc); }
        }

        std::size_t Read(std::size_t size, const std::uint8_t* bytes)
        {
            std::size_t bytesRead = std::fread(
                static_cast<void*>(const_cast<std::uint8_t*>(bytes)), 1, size, file
            );
            if (bytesRead < size && !std::feof(file))
            {
                throw FileException(name, std::ferror(file));
            }
            return bytesRead;
        }

        void Write(std::size_t size, const std::uint8_t* bytes)
        {
            std::size_t bytesWritten = std::fwrite(
                static_cast<void*>(const_cast<std::uint8_t*>(bytes)), 1, size, file
            );
            if (bytesWritten != size)
            {
                throw FileException(name, std::ferror(file));
            }
        }

    protected:
        std::string name;
        FILE* file;
    };
}