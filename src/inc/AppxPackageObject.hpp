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
};

SpecializeUuidOfImpl(IPackage);

namespace MSIX {
    // The 5-tuple that describes the identity of a package
    struct AppxPackageId
    {
        AppxPackageId(
            const std::string& name,
            const std::string& version,
            const std::string& resourceId,
            const std::string& architecture,
            const std::string& publisher);

        std::string Name;
        std::string Version;
        std::string ResourceId;
        std::string Architecture;
        std::string Publisher;
        std::string PublisherId;

        std::string GetPackageFullName()
        {
            return Name + "_" + Version + "_" + Architecture + "_" + ResourceId + "_" + PublisherId;
        }

        std::string GetPackageFamilyName()
        {
            return Name + "_" + PublisherId;
        }
    };

    struct AppxPackageInBundle
    {
        AppxPackageInBundle(
            const std::string& name,
            const std::string& version,
            const std::uint64_t size,
            const std::uint64_t offset,
            const std::string& resourceId,
            const std::string& architecture,
            bool isApplicationType);

        std::string Name;
        std::string Version;
        std::string ResourceId;
        std::string Architecture;
        std::uint64_t Size;
        std::uint64_t Offset;
        bool IsApplication;
    };

    // Object backed by AppxManifest.xml
    class AppxManifestObject final : public ComClass<AppxManifestObject, IVerifierObject>
    {
    public:
        AppxManifestObject(IXmlFactory* factory, const ComPtr<IStream>& stream);

        // IVerifierObject
        const std::string& GetPublisher() override { return GetPackageId()->Publisher; }
        bool HasStream() override { return !!m_stream; }
        ComPtr<IStream> GetStream() override { return m_stream; }
        ComPtr<IStream> GetValidationStream(const std::string& part, const ComPtr<IStream>&) override { NOTSUPPORTED; }
        const std::string GetPackageFullName() override { return m_packageId->GetPackageFullName(); }

        AppxPackageId* GetPackageId()    { return m_packageId.get(); }

    protected:
        ComPtr<IStream> m_stream;
        std::unique_ptr<AppxPackageId> m_packageId;
    };

    class AppxBundleManifestObject : public ComClass<AppxBundleManifestObject, IVerifierObject>
    {
    public:
        AppxBundleManifestObject(IXmlFactory* factory, const ComPtr<IStream>& stream);

         // IVerifierObject
        const std::string& GetPublisher() override { NOTSUPPORTED; }
        bool HasStream() override { return !!m_stream; }
        ComPtr<IStream> GetStream() override { return m_stream; }
        ComPtr<IStream> GetValidationStream(const std::string& part, const ComPtr<IStream>&) override { NOTSUPPORTED; }
        const std::string GetPackageFullName() override { NOTSUPPORTED; }

    protected:
        ComPtr<IStream> m_stream;
        std::unique_ptr<AppxPackageId> m_packageId;
    };

    // Storage object representing the entire AppxPackage
    class AppxPackageObject final : public ComClass<AppxPackageObject, IAppxPackageReader, IPackage, IStorageObject, IAppxBundleReader>
    {
    public:
        AppxPackageObject(IMSIXFactory* factory, MSIX_VALIDATION_OPTION validation, const ComPtr<IStorageObject>& container);
        ~AppxPackageObject() {}

        // internal IPackage methods
        void Unpack(MSIX_PACKUNPACK_OPTION options, const ComPtr<IStorageObject>& to) override;

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

        // returns a list of the footprint files found within this package.
        std::vector<std::string>& GetFootprintFiles() override { return m_footprintFiles; }

        // IStorageObject methods
        const char*               GetPathSeparator() override;
        std::vector<std::string>  GetFileNames(FileNameOptions options) override;
        ComPtr<IStream>           GetFile(const std::string& fileName) override;

        ComPtr<IStream>           OpenFile(const std::string& fileName, MSIX::FileStream::Mode mode) override;

    protected:
        // Helper methods
        void VerifyFile(const ComPtr<IStream>& stream, const std::string& fileName, const ComPtr<IAppxBlockMapInternal>& blockMapInternal);

        std::map<std::string, ComPtr<IStream>>  m_streams;

        MSIX_VALIDATION_OPTION      m_validation = MSIX_VALIDATION_OPTION::MSIX_VALIDATION_OPTION_FULL;
        ComPtr<IMSIXFactory>        m_factory;
        ComPtr<IVerifierObject>     m_appxSignature;
        ComPtr<IVerifierObject>     m_appxBlockMap;
        ComPtr<IVerifierObject>     m_appxManifest;
        ComPtr<IVerifierObject>     m_appxBundleManifest;
        ComPtr<IStorageObject>      m_container;
        
        std::vector<std::string>    m_payloadFiles;
        std::vector<std::string>    m_footprintFiles;
        std::vector<std::string>    m_payloadPackages;

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