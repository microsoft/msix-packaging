//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once

#include "AppxPackaging.hpp"
#include "ComHelper.hpp"
#include "DirectoryObject.hpp"
#include "AppxBlockMapWriter.hpp"
#include "ContentTypeWriter.hpp"

#include <map>
#include <memory>

// internal interface
// {32e89da5-7cbb-4443-8cf0-b84eedb51d0a}
#ifndef WIN32
interface IPackageWriter : public IUnknown
#else
#include "Unknwn.h"
#include "Objidl.h"
class IPackageWriter : public IUnknown
#endif
{
public:
    // TODO: add options if needed
    virtual void Pack(const MSIX::ComPtr<IDirectoryObject>& from) = 0;
};
MSIX_INTERFACE(IPackageWriter, 0x32e89da5,0x7cbb,0x4443,0x8c,0xf0,0xb8,0x4e,0xed,0xb5,0x1d,0x0a);

namespace MSIX {

    typedef struct BlockAndHash
    {
        std::vector<std::uint8_t> block;
        std::string hashValue;
    } BlockAndHash;

    typedef struct PayloadFile // rename this
    {
        APPX_COMPRESSION_OPTION compressionOption;
        std::uint32_t crc = 0;
        std::uint64_t fileSize;
        std::string relativeName;
        std::vector<std::unique_ptr<BlockAndHash>> fileBlocks;
    } PayloadFile;

    class AppxPackageWriter final : public ComClass<AppxPackageWriter, IPackageWriter, IAppxPackageWriter,
        IAppxPackageWriterUtf8, IAppxPackageWriter3, IAppxPackageWriter3Utf8>
    {
    public:
        AppxPackageWriter(IStream* outputStream);
        ~AppxPackageWriter() {};

        // IPackageWriter
        void Pack(const ComPtr<IDirectoryObject>& from) override;

        // IAppxPackageWriter
        HRESULT STDMETHODCALLTYPE AddPayloadFile(LPCWSTR fileName, LPCWSTR contentType,
            APPX_COMPRESSION_OPTION compressionOption, IStream *inputStream) noexcept override;
        HRESULT STDMETHODCALLTYPE Close(IStream *manifest) noexcept override;

        // IAppxPackageWriterUtf8
        HRESULT STDMETHODCALLTYPE AddPayloadFile(LPCSTR fileName, LPCSTR contentType,
            APPX_COMPRESSION_OPTION compressionOption, IStream* inputStream) noexcept override;

        // IAppxPackageWriter3
        HRESULT STDMETHODCALLTYPE AddPayloadFiles(UINT32 fileCount, APPX_PACKAGE_WRITER_PAYLOAD_STREAM* payloadFiles,
            UINT64 memoryLimit) noexcept override;

        // IAppxPackageWriter3Utf8
        HRESULT STDMETHODCALLTYPE AddPayloadFiles(UINT32 fileCount, APPX_PACKAGE_WRITER_PAYLOAD_STREAM_UTF8* payloadFiles,
            UINT64 memoryLimit) noexcept override;

    protected:
        typedef enum
        {
            Open = 1,
            Closed = 2,
            Failed = 3
        }
        WriterState;

        std::unique_ptr<PayloadFile> BuildPayloadFile(const std::string& name, const ComPtr<IStream>& stream, 
            const std::string& contentType, APPX_COMPRESSION_OPTION compressionOpt);

        WriterState m_state;
        ComPtr<IStream> m_outputStream;
        std::unique_ptr<BlockMapWriter> m_blockMapWriter;
        std::unique_ptr<ContentTypeWriter> m_contentTypeWriter;
    };
}

