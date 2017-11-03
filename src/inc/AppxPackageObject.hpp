#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>

#include "Exceptions.hpp"
#include "StreamBase.hpp"
#include "StorageObject.hpp"
#include "ZipObject.hpp"
#include "ComHelper.hpp"
#include "VerifierObject.hpp"
#include "XmlObject.hpp"
#include "AppxPackaging.hpp"
#include "AppxBlockMapObject.hpp"
#include "AppxSignature.hpp"

namespace xPlat {
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
        std::string PublisherHash;

        std::string GetPackageFullName()
        {
            return Name + "_" + Version + "_" + Architecture + "_" + ResourceId + "_" + PublisherHash;
        }

        std::string GetPackageFamilyName()
        {
            return Name + "_" + PublisherHash;
        }
    };

    // Object backed by AppxManifest.xml
    class AppxManifestObject : public VerifierObject
    {
    public:
        AppxManifestObject(IStream* stream);

        IStream* GetValidationStream(const std::string& part, IStream* stream) override
        {
            throw Exception(Error::NotSupported);
        }

        AppxPackageId* GetPackageId()           { return m_packageId.get(); }
        std::string GetPackageFullName()        { return m_packageId->GetPackageFullName(); }

    protected:
        ComPtr<IStream> m_stream;
        std::unique_ptr<AppxPackageId> m_packageId;
    };

    // internal interface
    #ifndef __IxPlatAppxPackage_INTERFACE_DEFINED__
    #define __IxPlatAppxPackage_INTERFACE_DEFINED__
        EXTERN_C const IID IID_IxPlatAppxPackage;   
        MIDL_INTERFACE("51b2c456-aaa9-46d6-8ec9-298220559189")
        interface IxPlatAppxPackage : public IUnknown
        {
        public:
            virtual void Pack(APPX_PACKUNPACK_OPTION options, const std::string& certFile, StorageObject& from) = 0;
            virtual void Unpack(APPX_PACKUNPACK_OPTION options, StorageObject& to) = 0;
            virtual std::vector<std::string>& GetFootprintFiles() = 0;
        };

        SpecializeUuidOfImpl(IxPlatAppxPackage);
    #endif

    // Storage object representing the entire AppxPackage
    class AppxPackageObject : public xPlat::ComClass<AppxPackageObject, 
        IAppxPackageReader, IxPlatAppxPackage, IxPlatStorageObject, IStorageObject>
    {
    public:
        AppxPackageObject(APPX_VALIDATION_OPTION validation, ComPtr<IStorageObject> container);

        // internal IxPlatAppxPackage methods
        void Pack(APPX_PACKUNPACK_OPTION options, const std::string& certFile, StorageObject& from) override;
        void Unpack(APPX_PACKUNPACK_OPTION options, StorageObject& to) override;

        AppxSignatureObject*      GetAppxSignature() const { return m_appxSignature.get(); }
        AppxBlockMapObject*       GetAppxBlockMap()  const { return m_appxBlockMap.get(); }
        AppxManifestObject*       GetAppxManifest()  const { return m_appxManifest.get(); }

        // IAppxPackageReader
        HRESULT STDMETHODCALLTYPE GetBlockMap(IAppxBlockMapReader** blockMapReader) override;
        HRESULT STDMETHODCALLTYPE GetFootprintFile(APPX_FOOTPRINT_FILE_TYPE type, IAppxFile** file) override;
        HRESULT STDMETHODCALLTYPE GetPayloadFile(LPCWSTR fileName, IAppxFile** file) override;
        HRESULT STDMETHODCALLTYPE GetPayloadFiles(IAppxFilesEnumerator**  filesEnumerator) override;
        HRESULT STDMETHODCALLTYPE GetManifest(IAppxManifestReader**  manifestReader) override;

        // returns a list of the footprint files found within this appx package.
        std::vector<std::string>& GetFootprintFiles() { return m_footprintFiles; }

        // IStorageObject methods
        std::string               GetPathSeparator() override;
        std::vector<std::string>  GetFileNames() override;
        IStream*                  GetFile(const std::string& fileName) override;
        void                      RemoveFile(const std::string& fileName) override;
        IStream*                  OpenFile(const std::string& fileName, FileStream::Mode mode) override;
        void                      CommitChanges() override;

    protected:
        std::map<std::string, ComPtr<IStream>>  m_streams;
        APPX_VALIDATION_OPTION                  m_validation = APPX_VALIDATION_OPTION::APPX_VALIDATION_OPTION_FULL;
        std::unique_ptr<AppxSignatureObject>    m_appxSignature;
        std::unique_ptr<AppxBlockMapObject>     m_appxBlockMap;
        std::unique_ptr<AppxManifestObject>     m_appxManifest;
        ComPtr<IStorageObject>                  m_container;

        std::vector<std::string>                m_payloadFiles;
        std::vector<std::string>                m_footprintFiles;

        std::unique_ptr<XmlObject>              m_contentType;
    };

    class AppxFilesEnumerator : public xPlat::ComClass<AppxFilesEnumerator, IAppxFilesEnumerator>
    {
    public:
        AppxFilesEnumerator(IStorageObject* storage) :
            m_storage(storage),
            m_files(storage->GetFileNames())
        {
        }

        // IAppxFilesEnumerator
        HRESULT STDMETHODCALLTYPE GetCurrent(IAppxFile** file) override
        {
            return ResultOf([&]{
                ThrowErrorIf(Error::InvalidParameter,(file == nullptr || *file != nullptr), "bad pointer");
                ThrowErrorIf(Error::Unexpected, (m_cursor >= m_files.size()), "index out of range");
                *file = m_storage->GetFile(m_files[m_cursor]).As<IAppxFile>().Detach();
            });
        }

        HRESULT STDMETHODCALLTYPE GetHasCurrent(BOOL* hasCurrent) override
        {
            return ResultOf([&]{
                ThrowErrorIfNot(Error:InvalidParameter, (hasCurrent), "bad pointer");
                *hasCurrent = m_cursor != m_files.size() ? TRUE : FALSE;
            });
        }

        HRESULT STDMETHODCALLTYPE MoveNext(BOOL* hasNext) override;        
        {
            return ResultOf([&]{
                ThrowErrorIfNot(Error:InvalidParameter, (hasNext), "bad pointer");
                *hasNext = ++m_cursor != m_files.size() ? TRUE : FALSE;
            });
        }
    protected:
        ComPtr<IStorageObject>      m_storage;
        std::size_t                 m_cursor = 0;
        std::vector<std::string>    m_files;
    };
}