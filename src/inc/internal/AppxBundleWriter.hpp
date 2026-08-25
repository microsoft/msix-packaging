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
#include "BundleWriterHelper.hpp"
#include "BundleManifestWriter.hpp"
#include "AppxPackageInfo.hpp"

// internal interface
// {ca90bcd9-78a2-4773-820c-0b687de49f99}
#ifndef WIN32
interface IBundleWriter : public IUnknown
#else
#include "unknwn.h"
#include "objidl.h"
class IBundleWriter : public IUnknown
#endif
{
public:
    virtual void ProcessBundlePayload(const MSIX::ComPtr<IDirectoryObject>& from, bool flatBundle) = 0;
    virtual void ProcessBundlePayloadFromMappingFile(std::map<std::string, std::string> fileList, bool flatBundle) = 0;
    virtual void ProcessExternalPackages(std::map<std::string, std::string> externalPackagesList) = 0;
};
MSIX_INTERFACE(IBundleWriter, 0xca90bcd9,0x78a2,0x4773,0x82,0x0c,0x0b,0x68,0x7d,0xe4,0x9f,0x99);

namespace MSIX {
    class AppxBundleWriter final : public ComClass<AppxBundleWriter, IBundleWriter, IAppxBundleWriter, IAppxBundleWriter4>
    {
    public:
        AppxBundleWriter(IMsixFactory* factory, const ComPtr<IZipWriter>& zip, std::uint64_t bundleVersion);
        ~AppxBundleWriter() {};

        // IBundleWriter
        void ProcessBundlePayload(const ComPtr<IDirectoryObject>& from, bool flatBundle) override;
        void ProcessBundlePayloadFromMappingFile(std::map<std::string, std::string> fileList, bool flatBundle) override;
        void ProcessExternalPackages(std::map<std::string, std::string> externalPackagesList) override;

        // IAppxBundleWriter
        HRESULT STDMETHODCALLTYPE AddPayloadPackage(LPCWSTR fileName, IStream* packageStream) noexcept override;
        HRESULT STDMETHODCALLTYPE Close() noexcept override;

        // IAppxBundleWriter4
        HRESULT STDMETHODCALLTYPE AddPackageReference(LPCWSTR fileName, IStream* inputStream, 
            BOOL isDefaultApplicablePackage) noexcept override;
        HRESULT STDMETHODCALLTYPE AddPayloadPackage(LPCWSTR fileName, IStream* packageStream, 
            BOOL isDefaultApplicablePackage) noexcept override;
        HRESULT STDMETHODCALLTYPE AddExternalPackageReference(LPCWSTR fileName, IStream* inputStream,
            BOOL isDefaultApplicablePackage) noexcept override;

    protected:
        typedef enum
        {
            Open = 1,
            Closed = 2,
            Failed = 3
        }
        WriterState;

        void AddFileToPackage(const std::string& name, IStream* stream, bool toCompress,
            bool addToBlockMap, const char* contentType, bool forceContentTypeOverride = false);

        void AddPackageReferenceInternal(std::string fileName, IStream* packageStream, bool isDefaultApplicablePackage);

        void AddExternalPackageReferenceInternal(std::string fileName, IStream* packageStream, bool isDefaultApplicablePackage);
            
        void ValidateAndAddPayloadFile(const std::string& name, IStream* stream, APPX_COMPRESSION_OPTION compressionOpt, const char* contentType);

        void ValidateCompressionOption(APPX_COMPRESSION_OPTION compressionOpt);

        WriterState m_state;
        ComPtr<IMsixFactory> m_factory;
        ComPtr<IZipWriter> m_zipWriter;
        BlockMapWriter m_blockMapWriter;
        ContentTypeWriter m_contentTypeWriter;
        BundleWriterHelper m_bundleWriterHelper;
    };
}

