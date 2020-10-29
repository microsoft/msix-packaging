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

#include <map>
#include <memory>
#include <future>

// internal interface
// {32e89da5-7cbb-4443-8cf0-b84eedb51d0a}
#ifndef WIN32
interface IBundleWriter : public IUnknown
#else
#include "Unknwn.h"
#include "Objidl.h"
class IBundleWriter : public IUnknown
#endif
{
public:
    virtual void ProcessBundlePayload(const MSIX::ComPtr<IDirectoryObject>& from, bool flatBundle) = 0;

};
MSIX_INTERFACE(IBundleWriter, 0x32e89da5,0x7cbb,0x4443,0x8c,0xf0,0xb8,0x4e,0xed,0xb5,0x1d,0x0a);

namespace MSIX {
    class AppxBundleWriter final : public ComClass<AppxBundleWriter, IBundleWriter, IAppxBundleWriter, IAppxBundleWriter4>
    {
    public:
        AppxBundleWriter(IMsixFactory* factory, const ComPtr<IZipWriter>& zip);
        ~AppxBundleWriter() {};

        // IBundleWriter
        void ProcessBundlePayload(const ComPtr<IDirectoryObject>& from, bool flatBundle) override;

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

        void ValidateAndAddPayloadFile(const std::string& name, IStream* stream,
            APPX_COMPRESSION_OPTION compressionOpt, const char* contentType);

        void AddFileToPackage(const std::string& name, IStream* stream, bool toCompress,
            bool addToBlockMap, const char* contentType, bool forceContentTypeOverride = false);

        void ValidateCompressionOption(APPX_COMPRESSION_OPTION compressionOpt);

        void CloseInternal();

        HRESULT AddPackageReferenceInternal(_In_ LPCWSTR fileName, _In_ IStream* packageStream,
            _In_ bool isDefaultApplicablePackage);
            
        WriterState m_state;
        ComPtr<IMsixFactory> m_factory;
        ComPtr<IZipWriter> m_zipWriter;
        BlockMapWriter m_blockMapWriter;
        ContentTypeWriter m_contentTypeWriter;
        BundleWriterHelper m_bundleWriterHelper;
        BundleManifestWriter m_bundleManifestWriter;
    };
}

