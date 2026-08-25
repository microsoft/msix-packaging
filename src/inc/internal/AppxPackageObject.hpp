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
#include "AppxManifestObject.hpp"
#include "DirectoryObject.hpp"

// internal interface
// {51b2c456-aaa9-46d6-8ec9-298220559189}
#ifndef WIN32
interface IPackage : public IUnknown
#else
#include "unknwn.h"
#include "objidl.h"
class IPackage : public IUnknown
#endif
{
public:
    virtual void Unpack(MSIX_PACKUNPACK_OPTION options, const MSIX::ComPtr<IDirectoryObject>& to) = 0;
    virtual std::vector<std::string>& GetFootprintFiles() = 0;
};
MSIX_INTERFACE(IPackage, 0x51b2c456,0xaaa9,0x46d6,0x8e,0xc9,0x29,0x82,0x20,0x55,0x91,0x89);

namespace MSIX {
    // Storage object representing the entire AppxPackage
    // Note: This class has is own implmentation of QueryInterface, if a new interface is implemented
    // AppxPackageObject::QueryInterface must also be modified too.
    class AppxPackageObject final : public ComClass<AppxPackageObject, IAppxPackageReader, IPackage, IStorageObject, IAppxBundleReader, IAppxPackageReaderUtf8, IAppxBundleReaderUtf8>
    {
    public:
        AppxPackageObject(IMsixFactory* factory, MSIX_VALIDATION_OPTION validation, MSIX_APPLICABILITY_OPTIONS applicabilityOptions, const ComPtr<IStorageObject>& container);
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
            if (riid == UuidOfImpl<IAppxPackageReaderUtf8>::iid)
            {
                *ppvObject = static_cast<void*>(static_cast<IAppxPackageReaderUtf8*>(this));
                AddRef();
                return S_OK;
            }
            #ifdef BUNDLE_SUPPORT
            if (riid == UuidOfImpl<IAppxBundleReader>::iid && m_isBundle)
            {
                *ppvObject = static_cast<void*>(static_cast<IAppxBundleReader*>(this));
                AddRef();
                return S_OK;
            }
            if (riid == UuidOfImpl<IAppxBundleReaderUtf8>::iid && m_isBundle)
            {
                *ppvObject = static_cast<void*>(static_cast<IAppxBundleReaderUtf8*>(this));
                AddRef();
                return S_OK;
            }
            #endif //BUNDLE_SUPPORT
            if (riid == UuidOfImpl<IUnknown>::iid)
            {
                *ppvObject = static_cast<void*>(reinterpret_cast<IUnknown*>(this));
                AddRef();
                return S_OK;
            }
            return static_cast<HRESULT>(MSIX::Error::NoInterface);
        }

        // internal IPackage methods
        void Unpack(MSIX_PACKUNPACK_OPTION options, const ComPtr<IDirectoryObject>& to) override;
        std::vector<std::string>& GetFootprintFiles() override { return m_footprintFiles; }

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
        // HRESULT STDMETHODCALLTYPE GetBlockMap(IAppxBlockMapReader** blockMapReader) noexcept override; 

        // IStorageObject methods
        std::vector<std::string> GetFileNames(FileNameOptions options) override;
        ComPtr<IStream> GetFile(const std::string& fileName) override;
        std::string GetFileName() override;

        // IAppxPackageReaderUtf8
        HRESULT STDMETHODCALLTYPE GetPayloadFile(LPCSTR fileName, IAppxFile** file) noexcept override;

        // IAppxBundleReaderUtf8
        HRESULT STDMETHODCALLTYPE GetPayloadPackage(LPCSTR fileName, IAppxFile **payloadPackage) noexcept override;

    protected:
        // Helper methods
        void VerifyFile(const ComPtr<IStream>& stream, const std::string& fileName, const ComPtr<IAppxBlockMapInternal>& blockMapInternal);
        ComPtr<IAppxFile> GetAppxFile(const std::string& fileName);

        std::map<std::string, ComPtr<IAppxFile>> m_files;

        MSIX_VALIDATION_OPTION      m_validation = MSIX_VALIDATION_OPTION::MSIX_VALIDATION_OPTION_FULL;
        ComPtr<IMsixFactory>        m_factory;
        ComPtr<IVerifierObject>     m_appxSignature;
        ComPtr<IVerifierObject>     m_appxBlockMap;
        ComPtr<IVerifierObject>     m_appxManifest;
        ComPtr<IVerifierObject>     m_appxBundleManifest;
        ComPtr<IStorageObject>      m_container;
        
        std::vector<std::string>    m_payloadFiles;
        std::vector<std::string>    m_footprintFiles;
        std::vector<std::string>    m_applicablePackagesNames;
        std::vector<ComPtr<IAppxPackageReader>> m_applicablePackages;
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