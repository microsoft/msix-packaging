#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>

#include "Exceptions.hpp"
#include "StreamBase.hpp"
#include "StorageObject.hpp"
#include "ZipObject.hpp"
#include "xPlatAppx.hpp"
#include "ComHelper.hpp"
#include "VerifierObject.hpp"
#include "XmlObject.hpp"
#include "AppxPackaging.hpp"
#include "AppxBlockMapObject.hpp"

namespace xPlat {
    // Object backed by AppxSignature.p7x
    class AppxSignatureObject : public VerifierObject
    {
    public:
        AppxSignatureObject(std::shared_ptr<StreamBase> stream);

        std::shared_ptr<StreamBase> GetValidationStream(const std::string& part, std::shared_ptr<StreamBase> stream) override;

    protected:
        std::map<std::string, std::vector<std::uint8_t>> m_digests;
    };

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
        AppxManifestObject(std::shared_ptr<StreamBase> stream);

        std::shared_ptr<StreamBase> GetValidationStream(const std::string& part, std::shared_ptr<StreamBase> stream) override
        {
            throw Exception(Error::NotSupported);
        }

        AppxPackageId* GetPackageId()           { return m_packageId.get(); }
        std::string GetPackageFullName()        { return m_packageId->GetPackageFullName(); }

    protected:
        std::shared_ptr<StreamBase> m_stream;
        std::unique_ptr<AppxPackageId> m_packageId;
    };

    // Storage object representing the entire AppxPackage
    class AppxPackageObject : public xPlat::ComClass<AppxPackageObject, IAppxPackageReader, IAppxFilesEnumerator>,
                              public StorageObject
    {
    public:
        AppxPackageObject(xPlatValidationOptions validation, std::unique_ptr<StorageObject>&& container);

        void Pack(xPlatPackUnpackOptions options, const std::string& certFile, StorageObject& from);
        void Unpack(xPlatPackUnpackOptions options, StorageObject& to);

        AppxSignatureObject*        GetAppxSignature() const { return m_appxSignature.get(); }
        AppxBlockMapObject*         GetAppxBlockMap()  const { return m_appxBlockMap.get(); }
        AppxManifestObject*         GetAppxManifest()  const { return m_appxManifest.get(); }

        // IAppxPackageReader
        HRESULT STDMETHODCALLTYPE GetBlockMap(IAppxBlockMapReader** blockMapReader) override;
        HRESULT STDMETHODCALLTYPE GetFootprintFile(APPX_FOOTPRINT_FILE_TYPE type, IAppxFile** file) override;
        HRESULT STDMETHODCALLTYPE GetPayloadFile(LPCWSTR fileName, IAppxFile** file) override;
        HRESULT STDMETHODCALLTYPE GetPayloadFiles(IAppxFilesEnumerator**  filesEnumerator) override;
        HRESULT STDMETHODCALLTYPE GetManifest(IAppxManifestReader**  manifestReader) override;

        // returns a list of the footprint files found within this appx package.
        std::vector<std::string>&    GetFootprintFiles() { return m_footprintFiles; }

        // StorageObject methods
        std::string                 GetPathSeparator() override;
        std::vector<std::string>    GetFileNames() override;
        std::shared_ptr<StreamBase> GetFile(const std::string& fileName) override;
        void                        RemoveFile(const std::string& fileName) override;
        std::shared_ptr<StreamBase> OpenFile(const std::string& fileName, FileStream::Mode mode) override;
        void                        CommitChanges() override;

        // IAppxFilesEnumerator
        HRESULT STDMETHODCALLTYPE GetCurrent(IAppxFile** file) override;
        HRESULT STDMETHODCALLTYPE GetHasCurrent(BOOL* hasCurrent) override;
        HRESULT STDMETHODCALLTYPE MoveNext(BOOL* hasNext) override;

    protected:
        std::map<std::string, std::shared_ptr<StreamBase>>  m_streams;
        xPlatValidationOptions                  m_validation = xPlatValidationOptions::xPlatValidationOptionFull;
        std::unique_ptr<AppxSignatureObject>    m_appxSignature;
        std::unique_ptr<AppxBlockMapObject>     m_appxBlockMap;
        std::unique_ptr<AppxManifestObject>     m_appxManifest;
        std::unique_ptr<StorageObject>          m_container;

        std::vector<std::string>                m_payloadFiles;
        std::vector<std::string>                m_footprintFiles;

        std::unique_ptr<XmlObject>              m_contentType;
    };
}