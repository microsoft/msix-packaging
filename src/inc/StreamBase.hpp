#pragma once

#include <memory>
#include <iostream>
#include "Exceptions.hpp"

namespace xPlat {
    class StreamBase
    {
    public:
        enum Reference { START = SEEK_SET, CURRENT = SEEK_CUR, END = SEEK_END };

        virtual ~StreamBase()
        {
            Close();
        }

        // This way, derived classes only have to implement what they actually need, and everything else is not implemented.
        virtual void Write(std::size_t size, const std::uint8_t* bytes)       { throw NotImplementedException(); }
        virtual std::size_t Read(std::size_t size, const std::uint8_t* bytes) { throw NotImplementedException(); }
        virtual void Seek(std::uint64_t offset, Reference where)              { throw NotImplementedException(); }
        virtual int Ferror()                                                  { throw NotImplementedException(); }
        virtual int Feof()                                                    { throw NotImplementedException(); }
        virtual std::uint64_t Ftell()                                         { throw NotImplementedException(); }

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

        virtual void Close() {};
    };

    typedef std::unique_ptr<StreamBase> StreamPtr;
}