#pragma once

#include <memory>
#include <vector>
#include <iostream>
#include "Exceptions.hpp"
#define UNICODE
#define NOMINMAX
#include "ComHelper.hpp"
#include "AppxPackaging.hpp"

namespace xPlat {
    class StreamBase : public xPlat::ComClass<StreamBase, IAppxFile, IStream>
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

        // IStream
        HRESULT STDMETHODCALLTYPE Read(
            void *pv,
            ULONG cb,
            ULONG *pcbRead)
        {
            return xPlat::ResultOf([&]() {
                // TODO: Implement
                throw Exception(Error::NotImplemented);
            });
        }

        HRESULT STDMETHODCALLTYPE Write(
            const void *pv,
            ULONG cb,
            ULONG *pcbWritten)
        {
            return xPlat::ResultOf([&]() {
                // TODO: Implement
                throw Exception(Error::NotImplemented);
            });
        }

        HRESULT STDMETHODCALLTYPE Seek(
            LARGE_INTEGER dlibMove,
            DWORD dwOrigin,
            ULARGE_INTEGER *plibNewPosition)
        {
            return xPlat::ResultOf([&]() {
                this->Seek(static_cast<std::uint64_t>(dlibMove), static_cast<Reference>(dwOrigin));
                plibNewPosition ? plibNewPosition = static_cast<ULARGE_INTEGER>(this->Ftell()) : nullptr;
            });
        }

        HRESULT STDMETHODCALLTYPE SetSize(ULARGE_INTEGER libNewSize)
        {
            return xPlat::ResultOf([&]() {
                // TODO: Implement
                throw Exception(Error::NotImplemented);
            });
        }

        HRESULT STDMETHODCALLTYPE CopyTo(
            IStream *pstm,
            ULARGE_INTEGER cb,
            ULARGE_INTEGER *pcbRead,
            ULARGE_INTEGER *pcbWritten)
        {
            return xPlat::ResultOf([&]() {
                // TODO: Implement
                throw Exception(Error::NotImplemented);
            });
        }

        HRESULT STDMETHODCALLTYPE Commit(DWORD grfCommitFlags)
        {
            return xPlat::ResultOf([&]() {
                // TODO: Implement
                throw Exception(Error::NotImplemented);
            });
        }

        HRESULT STDMETHODCALLTYPE Revert(void)
        {
            return xPlat::ResultOf([&]() {
                // TODO: Implement
                throw Exception(Error::NotImplemented);
            });
        }

        HRESULT STDMETHODCALLTYPE LockRegion(
            ULARGE_INTEGER libOffset,
            ULARGE_INTEGER cb,
            DWORD dwLockType)
        {
            return xPlat::ResultOf([&]() {
                // TODO: Implement
                throw Exception(Error::NotImplemented);
            });
        }

        HRESULT STDMETHODCALLTYPE UnlockRegion(
            ULARGE_INTEGER libOffset,
            ULARGE_INTEGER cb,
            DWORD dwLockType)
        {
            return xPlat::ResultOf([&]() {
                // TODO: Implement
                throw Exception(Error::NotImplemented);
            });
        }

        HRESULT STDMETHODCALLTYPE Stat(
            STATSTG *pstatstg,
            DWORD grfStatFlag)
        {
            return xPlat::ResultOf([&]() {
                // TODO: Implement
                throw Exception(Error::NotImplemented);
            });
        }

        HRESULT STDMETHODCALLTYPE Clone(IStream **ppstm)
        {
            return xPlat::ResultOf([&]() {
                // TODO: Implement
                throw Exception(Error::NotImplemented);
            });
        }

        // IAppxFile
        HRESULT STDMETHODCALLTYPE GetCompressionOption(APPX_COMPRESSION_OPTION* compressionOption)
        {
            return xPlat::ResultOf([&]() {
                // TODO: Implement
                throw Exception(Error::NotImplemented);
            });
        }

        HRESULT STDMETHODCALLTYPE GetContentType(LPWSTR* contentType)
        {
            return xPlat::ResultOf([&]() {
                // TODO: Implement
                throw Exception(Error::NotImplemented);
            });
        }

        HRESULT STDMETHODCALLTYPE GetName(LPWSTR* fileName)
        {
            return xPlat::ResultOf([&]() {
                // TODO: Implement
                throw Exception(Error::NotImplemented);
            });
        }

        HRESULT STDMETHODCALLTYPE GetSize(UINT64* size)
        {
            return xPlat::ResultOf([&]() {
                // TODO: Implement
                throw Exception(Error::NotImplemented);
            });
        }

        HRESULT STDMETHODCALLTYPE GetStream(IStream** stream)
        {
            UuidOfImpl<IStream> uuid;
            return QueryInterface(uuid.iid, stream);
        }
    };

    typedef std::unique_ptr<StreamBase> StreamPtr;
}