#pragma once

#include <memory>
#include <vector>
#include <algorithm>
#include <iostream>
#include "Exceptions.hpp"
#define UNICODE
#define NOMINMAX
#include "ComHelper.hpp"
#include "AppxPackaging.hpp"
#include "AppxWindows.hpp"

namespace xPlat {
    class StreamBase : public xPlat::ComClass<StreamBase, IAppxFile, IStream>
    {
    public:
        // These are the same values as STREAM_SEEK. See 
        // https://msdn.microsoft.com/en-us/library/windows/desktop/aa380359(v=vs.85).aspx for additional details.
        enum Reference { START = SEEK_SET, CURRENT = SEEK_CUR, END = SEEK_END };

        virtual ~StreamBase() {}

        //
        // IStream methods
        //

        // Creates a new stream object with its own seek pointer that references the same bytes as the original stream.
        virtual HRESULT Clone(IStream**) { return static_cast<HRESULT>(Error::NotSupported); }

        // Ensures that any changes made to a stream object open in transacted mode are reflected in the parent storage.
        // If the stream object is open in direct mode, IStream::Commit has no effect other than flushing all memory buffers
        // to the next-level storage object.
        virtual HRESULT Commit(DWORD)    { return static_cast<HRESULT>(Error::OK); }

        // Copies a specified number of bytes from the current seek pointer in the stream to the current seek pointer in 
        // another stream.
        virtual HRESULT CopyTo(IStream *stream, ULARGE_INTEGER bytesCount, ULARGE_INTEGER *bytesRead, ULARGE_INTEGER *bytesWritten)
        {
            return ResultOf([&] {
                if (bytesRead) { bytesRead->QuadPart = 0; }
                if (bytesWritten) { bytesWritten->QuadPart = 0; }
                ThrowErrorIfNot(Error::InvalidParameter, (stream), "invalid parameter.");

                static const ULONGLONG size = 1024;
                std::vector<std::int8_t> bytes(size);
                std::int64_t read = 0;
                std::int64_t written = 0;
                ULONG length = 0;

                while (0 < bytesCount.QuadPart)
                {
                    ULONGLONG chunk = std::min(bytesCount.QuadPart, size);
                    ThrowHrIfFailed(Read(reinterpret_cast<void*>(bytes.data()), chunk, &length));
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
            });
        }

        virtual HRESULT Read(void*, ULONG, ULONG*) { return static_cast<HRESULT>(Error::NotImplemented); }

        // Restricts access to a specified range of bytes in the stream. Supporting this functionality is optional since
        // some file systems do not provide it.
        virtual HRESULT LockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD)
        {
            return static_cast<HRESULT>(Error::NotSupported);
        }

        // Discards all changes that have been made to a transacted stream since the last IStream::Commit call.
        virtual HRESULT Revert() { return static_cast<HRESULT>(Error::NotSupported); }

        // Changes the seek pointer to a new location. The new location is relative to either the beginning of the
        // stream, the end of the stream, or the current seek pointer.
        virtual HRESULT Seek(LARGE_INTEGER, DWORD, ULARGE_INTEGER*) { return static_cast<HRESULT>(Error::NotImplemented); }

        // Changes the size of the stream object.
        virtual HRESULT SetSize(ULARGE_INTEGER) { return static_cast<HRESULT>(Error::NotSupported); }

        // Retrieves the STATSTG structure for this stream.
        virtual HRESULT Stat(STATSTG* , DWORD) { return static_cast<HRESULT>(Error::NotSupported); }

        // Removes the access restriction on a range of bytes previously restricted with IStream::LockRegion.
        virtual HRESULT UnlockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD)
        {
            return static_cast<HRESULT>(Error::NotSupported);
        }

        // Writes a specified number of bytes into the stream object starting at the current seek pointer.
        HRESULT Write(void const *,ULONG, ULONG) { return static_cast<HRESULT>(Error::NotImplemented); }

        //
        // IAppxFile methods
        //
        HRESULT STDMETHODCALLTYPE GetCompressionOption(APPX_COMPRESSION_OPTION* compressionOption)
        {
            if (compressionOption) { *compressionOption = APPX_COMPRESSION_OPTION_NONE; }
            return static_cast<HRESULT>(Error::OK);
        }

        HRESULT STDMETHODCALLTYPE GetContentType(LPWSTR* contentType)
        {
            return static_cast<HRESULT>(Error::NotImplemented);
        }

        HRESULT STDMETHODCALLTYPE GetName(LPWSTR* fileName)
        {
            return static_cast<HRESULT>(Error::NotImplemented);
        }

        HRESULT STDMETHODCALLTYPE GetSize(UINT64* size)
        {
            return static_cast<HRESULT>(Error::NotImplemented);
        }

        HRESULT STDMETHODCALLTYPE GetStream(IStream** stream)
        {
            UuidOfImpl<IStream> uuid;
            return QueryInterface(uuid.iid, reinterpret_cast<void**>(stream));
        }
    };
}