//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>

#include "AppxPackaging.hpp"
#include "MSIXWindows.hpp"
#include "Exceptions.hpp"
#include "ComHelper.hpp"
#include "StreamBase.hpp"
#include "StorageObject.hpp"
#include "ZipObject.hpp"
#include "VerifierObject.hpp"
#include "IXml.hpp"
#include "AppxBlockMapObject.hpp"
#include "AppxSignature.hpp"
#include "AppxFactory.hpp"
#include "AppxPackageInfo.hpp"

// internal interface
EXTERN_C const IID IID_IPackage;
#ifndef WIN32
// {51b2c456-aaa9-46d6-8ec9-298220559189}
interface IPackage : public IUnknown
#else
#include "Unknwn.h"
#include "Objidl.h"
class IPackage : public IUnknown
#endif
{
public:
    virtual void Unpack(MSIX_PACKUNPACK_OPTION options, const MSIX::ComPtr<IStorageObject>& to) = 0;
    virtual std::vector<std::string>& GetFootprintFiles() = 0;
    virtual MSIX::ComPtr<IVerifierObject> GetAppxManifestObject() = 0;
};

EXTERN_C const IID IID_IBundleInfo;
#ifndef WIN32
// {ff82ffcd-747a-4df9-8879-853ab9dd15a1}
interface IBundleInfo : public IUnknown
#else
#include "Unknwn.h"
#include "Objidl.h"
class IBundleInfo : public IUnknown
#endif
{
public:
    virtual std::vector<std::unique_ptr<MSIX::AppxPackageInBundle>>& GetPackages() = 0;
};

SpecializeUuidOfImpl(IPackage);
SpecializeUuidOfImpl(IBundleInfo);

namespace MSIX {

    // Object backed by AppxManifest.xml
    class AppxManifestObject final : public ComClass<AppxManifestObject, IVerifierObject>
    {
    public:
        AppxManifestObject(IXmlFactory* factory, const ComPtr<IStream>& stream);

        // IVerifierObject
        const std::string& GetPublisher() override { return GetPackageId()->PublisherId; }
        bool HasStream() override { return !!m_stream; }
        ComPtr<IStream> GetStream() override { return m_stream; }
        ComPtr<IStream> GetValidationStream(const std::string& part, const ComPtr<IStream>&) override { NOTSUPPORTED; }
        const std::string GetPackageFullName() override { return m_packageId->GetPackageFullName(); }
        const std::string& GetVersion() override { return GetPackageId()->Version; }
        const std::string& GetName() override { return GetPackageId()->Name; }
        const std::string& GetArchitecture() override { return GetPackageId()->Architecture; }
        const MSIX_PLATFORM GetPlatform() override { return m_platform; }

        AppxPackageId* GetPackageId() { return m_packageId.get(); }

    protected:
        ComPtr<IStream> m_stream;
        std::unique_ptr<AppxPackageId> m_packageId;
        MSIX_PLATFORM m_platform = MSIX_PLATFORM_NONE;
    };

    class AppxBundleManifestObject final : public ComClass<AppxBundleManifestObject, IVerifierObject, IBundleInfo>
    {
    public:
        AppxBundleManifestObject(IXmlFactory* factory, const ComPtr<IStream>& stream);

         // IVerifierObject
        const std::string& GetPublisher() override { return GetPackageId()->PublisherId; }
        bool HasStream() override { return !!m_stream; }
        ComPtr<IStream> GetStream() override { return m_stream; }
        ComPtr<IStream> GetValidationStream(const std::string& part, const ComPtr<IStream>&) override { NOTSUPPORTED; }
        const std::string GetPackageFullName() override { NOTSUPPORTED; }
        const std::string& GetVersion() override { return GetPackageId()->Version; }
        const std::string& GetName() override { return GetPackageId()->Name; }
        const std::string& GetArchitecture() override { NOTSUPPORTED; }
        const MSIX_PLATFORM GetPlatform() override { NOTSUPPORTED; }

        AppxPackageId* GetPackageId() { return m_packageId.get(); }

        // IBundleInfo
        std::vector<std::unique_ptr<AppxPackageInBundle>>& GetPackages() override { return m_packages; }

    protected:
        ComPtr<IStream> m_stream;
        std::unique_ptr<AppxPackageId> m_packageId;
        std::vector<std::unique_ptr<AppxPackageInBundle>> m_packages;
    };

    // Storage object representing the entire AppxPackage
    // Note: This class has is own implmentation of QueryInterface, if a new interface is implemented
    // AppxPackageObject::QueryInterface must also be modified too.
    class AppxPackageObject final : public ComClass<AppxPackageObject, IAppxPackageReader, IPackage, IStorageObject, IAppxBundleReader>
    {
    public:
        AppxPackageObject(IMSIXFactory* factory, MSIX_VALIDATION_OPTION validation, const ComPtr<IStorageObject>& container);
        ~AppxPackageObject() {}

        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) noexcept override
        {
            if (ppvObject == nullptr || *ppvObject != nullptr)
            {
                return static_cast<HRESULT>(Error::InvalidParameter);
            }
            *ppvObject = nullptr;
            if (riid == UuidOfImpl<IAppxPackageReader>::iid)
            {
                *ppvObject = static_cast<void*>(static_cast<IAppxPackageReader*>(this));
                AddRef();
                return S_OK;
            }
            if (riid == UuidOfImpl<IPackage>::iid)
            {
                *ppvObject = static_cast<void*>(static_cast<IPackage*>(this));
                AddRef();
                return S_OK;
            }
            if (riid == UuidOfImpl<IStorageObject>::iid)
            {
                *ppvObject = static_cast<void*>(static_cast<IStorageObject*>(this));
                AddRef();
                return S_OK;
            }
            if (riid == UuidOfImpl<IAppxBundleReader>::iid && m_isBundle)
            {
                *ppvObject = static_cast<void*>(static_cast<IAppxBundleReader*>(this));
                AddRef();
                return S_OK;
            }
            return static_cast<HRESULT>(MSIX::Error::NoInterface);
        }

        // internal IPackage methods
        void Unpack(MSIX_PACKUNPACK_OPTION options, const ComPtr<IStorageObject>& to) override;
        std::vector<std::string>& GetFootprintFiles() override { return m_footprintFiles; }
        ComPtr<IVerifierObject> GetAppxManifestObject() override { return m_appxManifest; }

        // IAppxPackageReader
        HRESULT STDMETHODCALLTYPE GetBlockMap(IAppxBlockMapReader** blockMapReader) noexcept override;
        HRESULT STDMETHODCALLTYPE GetFootprintFile(APPX_FOOTPRINT_FILE_TYPE type, IAppxFile** file) noexcept override;
        HRESULT STDMETHODCALLTYPE GetPayloadFile(LPCWSTR fileName, IAppxFile** file) noexcept override;
        HRESULT STDMETHODCALLTYPE GetPayloadFiles(IAppxFilesEnumerator**  filesEnumerator) noexcept override;
        HRESULT STDMETHODCALLTYPE GetManifest(IAppxManifestReader**  manifestReader) noexcept override;

        // IAppxBundleReader
        HRESULT STDMETHODCALLTYPE GetFootprintFile(APPX_BUNDLE_FOOTPRINT_FILE_TYPE fileType, IAppxFile **footprintFile) noexcept override;
        HRESULT STDMETHODCALLTYPE GetManifest(IAppxBundleManifestReader **manifestReader) noexcept override;
        HRESULT STDMETHODCALLTYPE GetPayloadPackages(IAppxFilesEnumerator **payloadPackages) noexcept override;
        HRESULT STDMETHODCALLTYPE GetPayloadPackage(LPCWSTR fileName, IAppxFile **payloadPackage) noexcept override;
        // Same signature as IAppxPackageReader
        // HRESULT STDMETHODCALLTYPE GetBlockMap(IAppxBlockMapReader** blockMapReader) override; 

        // IStorageObject methods
        const char*               GetPathSeparator() override;
        std::vector<std::string>  GetFileNames(FileNameOptions options) override;
        ComPtr<IStream>           GetFile(const std::string& fileName) override;
        ComPtr<IStream>           OpenFile(const std::string& fileName, MSIX::FileStream::Mode mode) override;

    protected:
        // Helper methods
        void VerifyFile(const ComPtr<IStream>& stream, const std::string& fileName, const ComPtr<IAppxBlockMapInternal>& blockMapInternal);

        std::map<std::string, ComPtr<IStream>> m_streams;

        MSIX_VALIDATION_OPTION      m_validation = MSIX_VALIDATION_OPTION::MSIX_VALIDATION_OPTION_FULL;
        ComPtr<IMSIXFactory>        m_factory;
        ComPtr<IVerifierObject>     m_appxSignature;
        ComPtr<IVerifierObject>     m_appxBlockMap;
        ComPtr<IVerifierObject>     m_appxManifest;
        ComPtr<IVerifierObject>     m_appxBundleManifest;
        ComPtr<IStorageObject>      m_container;
        
        std::vector<std::string>    m_payloadFiles;
        std::vector<std::string>    m_footprintFiles;
        std::vector<std::string>    m_payloadPackagesNames;

        std::vector<ComPtr<IAppxPackageReader>> m_payloadPackages;
        bool                        m_isBundle = false;
    };

    class AppxFilesEnumerator final : public MSIX::ComClass<AppxFilesEnumerator, IAppxFilesEnumerator>
    {
    protected:
        ComPtr<IStorageObject>      m_storage;
        std::size_t                 m_cursor = 0;
        std::vector<std::string>    m_files;

    public:
        AppxFilesEnumerator(const ComPtr<IStorageObject>& storage) : 
            m_storage(storage)
        {
            m_files = storage->GetFileNames(FileNameOptions::PayloadOnly);            
        }

        // IAppxFilesEnumerator
        HRESULT STDMETHODCALLTYPE GetCurrent(IAppxFile** file) noexcept override try
        {
            ThrowErrorIf(Error::InvalidParameter,(file == nullptr || *file != nullptr), "bad pointer");
            ThrowErrorIf(Error::Unexpected, (m_cursor >= m_files.size()), "index out of range");
            *file = m_storage->GetFile(m_files[m_cursor]).As<IAppxFile>().Detach();
            return static_cast<HRESULT>(Error::OK);
        } CATCH_RETURN();

        HRESULT STDMETHODCALLTYPE GetHasCurrent(BOOL* hasCurrent) noexcept override
        {   
            if (nullptr == hasCurrent) {
                return static_cast<HRESULT>(Error::InvalidParameter);
            }
            *hasCurrent = (m_cursor != m_files.size()) ? TRUE : FALSE;
            return static_cast<HRESULT>(Error::OK);
        }

        HRESULT STDMETHODCALLTYPE MoveNext(BOOL* hasNext) noexcept override      
        {   
            if (nullptr == hasNext) {
                return static_cast<HRESULT>(Error::InvalidParameter);
            }            
            *hasNext = (++m_cursor != m_files.size()) ? TRUE : FALSE;
            return static_cast<HRESULT>(Error::OK);
        }
    };
}