#pragma once

#include <memory>
#include <iostream>
#include "Exceptions.hpp"

namespace xPlat {
    class StreamBase
    {
    public:
        enum Reference { START = SEEK_SET, CURRENT = SEEK_CUR, END = SEEK_END };

        virtual ~StreamBase() {}

        // This way, derived classes only have to implement what they actually need, and everything else is not implemented.
        virtual void Write(std::size_t size, const std::uint8_t* bytes)       { throw Exception(Error::NotImplemented); }
        virtual std::size_t Read(std::size_t size, const std::uint8_t* bytes) { throw Exception(Error::NotImplemented); }
        virtual void Seek(std::uint64_t offset, Reference where)              { throw Exception(Error::NotImplemented); }
        virtual int Ferror()                                                  { throw Exception(Error::NotImplemented); }
        virtual bool Feof()                                                   { throw Exception(Error::NotImplemented); }
        virtual std::uint64_t Ftell()                                         { throw Exception(Error::NotImplemented); }

        virtual void CopyTo(StreamBase* to)
        {
            std::uint8_t buffer[1024];  // 1k at a time ought to be sufficient
            std::size_t bytes = Read(sizeof(buffer), buffer);
            while (bytes != 0)
            {
                to->Write(bytes, buffer);
                bytes = Read(sizeof(buffer), buffer);
            }
        }

        template <class T>
        static std::size_t Read(StreamBase* stream, T* value)
        {
            //static_assert(std::is_pod<T>::value, "specified value type must be both trivial and standard-layout");
            return stream->Read(sizeof(T), reinterpret_cast<std::uint8_t*>(const_cast<T*>(value)));
        }

        template <class T>
        static void Write(StreamBase* stream, T* value)
        {
            //static_assert(std::is_pod<T>::value, "specified value type must be both trivial and standard-layout");
            stream->Write(sizeof(T), reinterpret_cast<std::uint8_t*>(const_cast<T*>(value)));
        }

        virtual void Close() {};
    };

    typedef std::unique_ptr<StreamBase> StreamPtr;
}