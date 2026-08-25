//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once

#include <memory>
#include <vector>
#include <algorithm>
#include <iostream>
#include <limits>

#include "MSIXWindows.hpp"
#include "AppxPackaging.hpp"
#include "Exceptions.hpp"
#include "ComHelper.hpp"

// {44d2a7a8-a165-4a6e-a56f-c7c24de7505c}
#ifndef WIN32
interface IStreamInternal : public IUnknown
#else
#include "unknwn.h"
#include "objidl.h"
class IStreamInternal : public IUnknown
#endif
{
public:
    virtual std::uint64_t GetSize() = 0;
    virtual bool IsCompressed() = 0;
    virtual std::string GetName() = 0;
};
MSIX_INTERFACE(IStreamInternal, 0x44d2a7a8,0xa165,0x4a6e,0xa5,0x6f,0xc7,0xc2,0x4d,0xe7,0x50,0x5c);

namespace MSIX {
    class StreamBase : public MSIX::ComClass<StreamBase, IStream, IStreamInternal>
    {
    public:
        // These are the same values as STREAM_SEEK. See 
        // https://msdn.microsoft.com/en-us/library/windows/desktop/aa380359(v=vs.85).aspx for additional details.
        enum Reference { START = SEEK_SET, CURRENT = SEEK_CUR, END = SEEK_END };

        virtual ~StreamBase() {}

        // IStream methods
        // Creates a new stream object with its own seek pointer that references the same bytes as the original stream.
        virtual HRESULT STDMETHODCALLTYPE Clone(IStream**) noexcept override { return static_cast<HRESULT>(Error::NotSupported); }

        // Ensures that any changes made to a stream object open in transacted mode are reflected in the parent storage.
        // If the stream object is open in direct mode, IStream::Commit has no effect other than flushing all memory buffers
        // to the next-level storage object.
        virtual HRESULT STDMETHODCALLTYPE Commit(DWORD) noexcept override { return static_cast<HRESULT>(Error::OK); }

        // Copies a specified number of bytes from the current seek pointer in the stream to the current seek pointer in 
        // another stream.
        virtual HRESULT STDMETHODCALLTYPE CopyTo(IStream *stream, ULARGE_INTEGER bytesCount, ULARGE_INTEGER *bytesRead, ULARGE_INTEGER *bytesWritten) noexcept override try
        {
            if (bytesRead) { bytesRead->QuadPart = 0; }
            if (bytesWritten) { bytesWritten->QuadPart = 0; }
            ThrowErrorIf(Error::InvalidParameter, (nullptr == stream), "invalid parameter.");

            static const ULONGLONG size = 1024;
            std::vector<std::int8_t> bytes(size);
            std::int64_t read = 0;
            std::int64_t written = 0;
            ULONG length = 0;

            while (0 < bytesCount.QuadPart)
            {
                ULONGLONG chunk = std::min(bytesCount.QuadPart, static_cast<ULONGLONG>(size));
                ThrowHrIfFailed(Read(reinterpret_cast<void*>(bytes.data()), (ULONG)chunk, &length));
                if (length == 0) { break; }
                read += length;

                ULONG offset = 0;
                while (0 < length)
                {
                    ULONG copy = 0;
                    ThrowHrIfFailed(stream->Write(reinterpret_cast<void*>(&bytes[offset]), length, &copy));
                    offset += copy;
                    written += copy;
                    length -= copy;
                    bytesCount.QuadPart -= copy;
                }
            }

            if (bytesRead)      { bytesRead->QuadPart = read; }
            if (bytesWritten)   { bytesWritten->QuadPart = written;}
            return static_cast<HRESULT>(Error::OK);
        } CATCH_RETURN();

        virtual HRESULT STDMETHODCALLTYPE Read(void*, ULONG, ULONG*) noexcept override { return static_cast<HRESULT>(Error::NotImplemented); }

        // Restricts access to a specified range of bytes in the stream. Supporting this functionality is optional since
        // some file systems do not provide it.
        virtual HRESULT STDMETHODCALLTYPE LockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD) noexcept override
        {
            return static_cast<HRESULT>(Error::NotSupported);
        }

        // Discards all changes that have been made to a transacted stream since the last IStream::Commit call.
        virtual HRESULT STDMETHODCALLTYPE Revert() noexcept override { return static_cast<HRESULT>(Error::NotSupported); }

        // Changes the seek pointer to a new location. The new location is relative to either the beginning of the
        // stream, the end of the stream, or the current seek pointer.
        virtual HRESULT STDMETHODCALLTYPE Seek(LARGE_INTEGER, DWORD, ULARGE_INTEGER*) noexcept override { return static_cast<HRESULT>(Error::NotImplemented); }

        // Changes the size of the stream object.
        virtual HRESULT STDMETHODCALLTYPE SetSize(ULARGE_INTEGER) noexcept override { return static_cast<HRESULT>(Error::NotSupported); }

        // Retrieves the STATSTG structure for this stream.
        virtual HRESULT STDMETHODCALLTYPE Stat(STATSTG* statStg, DWORD /*grfStatFlag*/) noexcept override try
        {
            ThrowErrorIf(Error::InvalidParameter, statStg == nullptr, "bad pointer");
            LARGE_INTEGER start = { 0 };
            ULARGE_INTEGER end = { 0 };
            ThrowHrIfFailed(Seek(start, StreamBase::Reference::END, &end));
            ThrowHrIfFailed(Seek(start, StreamBase::Reference::START, nullptr));
            statStg->type = STGTY_STREAM;
            statStg->cbSize.QuadPart = end.u.LowPart;
            return static_cast<HRESULT>(Error::OK);
        } CATCH_RETURN();

        // Removes the access restriction on a range of bytes previously restricted with IStream::LockRegion.
        virtual HRESULT STDMETHODCALLTYPE UnlockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD) noexcept override
        {
            return static_cast<HRESULT>(Error::NotSupported);
        }

        // Writes a specified number of bytes into the stream object starting at the current seek pointer.
        virtual HRESULT STDMETHODCALLTYPE Write(const void*, ULONG, ULONG*) noexcept override { return static_cast<HRESULT>(Error::NotImplemented); }

        // IStreamInternal
        virtual std::uint64_t GetSize() override { NOTIMPLEMENTED; }
        virtual bool IsCompressed() override { NOTIMPLEMENTED; }
        virtual std::string GetName() override { NOTIMPLEMENTED; }

        template <class T>
        static ULONG Read(const ComPtr<IStream>& stream, T* value)
        {
            ULONG result = 0;
            ThrowHrIfFailed(stream->Read(value, static_cast<ULONG>(sizeof(T)), &result));
            ThrowErrorIf(Error::FileRead, (result != sizeof(T)), "Entire object wasn't read!");
            return result;
        }

        static void ReadData(const ComPtr<IStream>& stream, std::vector<std::uint8_t>& data)
        {
            ULONG size = 0;
            ThrowHrIfFailed(stream->Read(reinterpret_cast<void*>(data.data()), static_cast<ULONG>(data.size()), &size));
            ThrowErrorIfNot(Error::FileRead, size == data.size(), "entire vector not read!");
        }

        static uint64_t Pos(const ComPtr<IStream>& stream)
        {
            ULARGE_INTEGER uPos{0};
            ThrowHrIfFailed(stream->Seek({0}, Reference::CURRENT, &uPos));
            return uPos.QuadPart;
        }

        template <class T>
        static void Write(const ComPtr<IStream>& stream, T* value)
        {
            ULONG result = 0;
            ThrowHrIfFailed(stream->Write(value, static_cast<ULONG>(sizeof(T)), nullptr));
            ThrowErrorIf(Error::FileWrite, (result != sizeof(T)), "Entire object wasn't written!");
        }
    };
}