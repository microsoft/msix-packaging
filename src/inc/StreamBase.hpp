#pragma once

#include <memory>
#include <vector>
#include <iostream>
#include "Exceptions.hpp"

namespace xPlat {
    class StreamBase
    {
    public:
        enum Reference { START = SEEK_SET, CURRENT = SEEK_CUR, END = SEEK_END };

        virtual ~StreamBase() {}

        // This way, derived classes only have to implement what they actually need, and everything else is not implemented.
        virtual void Write(const std::uint8_t* start, const std::uint8_t* end)       { throw Exception(Error::NotImplemented); }
        virtual std::size_t Read(const std::uint8_t* start, const std::uint8_t* end) { throw Exception(Error::NotImplemented); }
        virtual void Seek(std::uint64_t offset, Reference where)                     { throw Exception(Error::NotImplemented); }
        virtual int Ferror()                                                         { throw Exception(Error::NotImplemented); }
        virtual bool Feof()                                                          { throw Exception(Error::NotImplemented); }
        virtual std::uint64_t Ftell()                                                { throw Exception(Error::NotImplemented); }
        virtual void Flush()                                                         { }
        virtual void CopyTo(StreamBase* to)
        {
            std::vector<std::uint8_t> buffer(1024);  // 1k at a time ought to be sufficient
            std::size_t bytes = Read(buffer.data(), buffer.data() + buffer.size());
            while (bytes != 0)
            {
                to->Write(buffer.data(), buffer.data() + bytes);
                to->Flush();
                bytes = Read(buffer.data(), buffer.data() + buffer.size());
            }
        }

        template <class T>
        static std::size_t Read(StreamBase* stream, T* value)
        {
            const std::uint8_t* start = reinterpret_cast<std::uint8_t*>(const_cast<T*>(value));
            const std::uint8_t* end = start + sizeof(T);
            return stream->Read(start, end);
        }

        template <class T>
        static void Write(StreamBase* stream, T* value)
        {
            const std::uint8_t* start = reinterpret_cast<std::uint8_t*>(const_cast<T*>(value));
            const std::uint8_t* end = start + sizeof(T);
            stream->Write(start, end);
        }

        virtual void Close() {};
    };

    typedef std::unique_ptr<StreamBase> StreamPtr;
}