#include <iostream>
#include "Exceptions.hpp"

namespace xPlat {
    class StreamBase
    {
    public:
        enum Reference { START = SEEK_SET, CURRENT = SEEK_CUR, END = SEEK_END };

        virtual ~StreamBase() { Close(); }

        // just like fwrite
        virtual void Write(std::size_t size, const std::uint8_t* bytes)
        {
            throw NotImplementedException();
        }

        // just like freed
        virtual std::size_t Read(std::size_t size, const std::uint8_t* bytes)
        {
            throw NotImplementedException();
        }

        // just like fseek
        virtual void Seek(long offset, Reference where)
        {
            throw NotImplementedException();
        }

        // just like ferror
        virtual int Ferror()
        {
            throw NotImplementedException();
        }

        // just like feof
        virtual int Feof()
        {
            throw NotImplementedException();
        }

        virtual void CopyTo(StreamBase& to)
        {
            std::uint8_t buffer[1024];  // 1k at a time ought to be sufficient
            std::size_t bytes = Read(sizeof(buffer), buffer);
            while (bytes != 0)
            {
                to.Write(bytes, buffer);
                bytes = Read(sizeof(buffer), buffer);
            }
        }

        virtual void Close() { }
    };
}