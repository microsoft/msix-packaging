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
#include "ZipObjectWriter.hpp"
#include "AppxPackageObject.hpp"
#include "Signing.hpp"

#include <map>
#include <memory>
#include <future>

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
    virtual void PackPayloadFiles(const MSIX::ComPtr<IDirectoryObject>& from) = 0;
    // Custom Close used to finish out the signing process
    virtual void Close(
        MSIX_CERTIFICATE_FORMAT signingCertificateFormat,
        IStream* signingCertificate,
        const char* pass,
        IStream* privateKey) = 0;
};
MSIX_INTERFACE(IPackageWriter, 0x32e89da5,0x7cbb,0x4443,0x8c,0xf0,0xb8,0x4e,0xed,0xb5,0x1d,0x0a);

namespace MSIX {
    class AppxPackageWriter final : public ComClass<AppxPackageWriter, IPackageWriter, IAppxPackageWriter,
        IAppxPackageWriterUtf8, IAppxPackageWriter3, IAppxPackageWriter3Utf8>
    {
    public:
        AppxPackageWriter(IMsixFactory* factory, const ComPtr<IZipWriter>& zip, bool enableFileHash);
        AppxPackageWriter(IPackage* packageToSign, std::unique_ptr<SignatureAccumulator>&& accumulator);
        ~AppxPackageWriter() {};

        // IPackageWriter
        void PackPayloadFiles(const ComPtr<IDirectoryObject>& from) override;
        void Close(
            MSIX_CERTIFICATE_FORMAT signingCertificateFormat,
            IStream* signingCertificate,
            const char* pass,
            IStream* privateKey) override;

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

        void ValidateAndAddPayloadFile(const std::string& name, IStream* stream,
            APPX_COMPRESSION_OPTION compressionOpt, const char* contentType);

        void AddFileToPackage(const std::string& name, IStream* stream, bool toCompress,
            bool addToBlockMap, const char* contentType, bool forceContentTypeOverride = false, bool forceDataDescriptor = true);

        void ValidateCompressionOption(APPX_COMPRESSION_OPTION compressionOpt);

        WriterState m_state = WriterState::Open;
        ComPtr<IMsixFactory> m_factory;
        ComPtr<IZipWriter> m_zipWriter;
        BlockMapWriter m_blockMapWriter;
        ContentTypeWriter m_contentTypeWriter;
        std::unique_ptr<SignatureAccumulator> m_signatureAccumulator;
    };
}

