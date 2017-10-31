#include "Exceptions.hpp"
#include "StreamBase.hpp"
#include "StorageObject.hpp"
#include "AppxPackaging.hpp"
#include "AppxPackageObject.hpp"
#include "ComHelper.hpp"

#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace xPlat {

    // names of footprint files.
    #define APPXBLOCKMAP_XML  "AppxBlockMap.xml"
    #define APPXMANIFEST_XML  "AppxManifest.xml"
    #define CODEINTEGRITY_CAT "AppxMetadata/CodeIntegrity.cat"
    #define APPXSIGNATURE_P7X "AppxSignature.p7x"
    #define CONTENT_TYPES_XML "[Content_Types].xml"

    AppxPackageId::AppxPackageId(
        const std::string& name,
        const std::string& version,
        const std::string& resourceId,
        const std::string& architecture,
        const std::string& publisher) :
        Name(name), Version(version), ResourceId(resourceId), Architecture(architecture), PublisherHash(publisher)
    {
        // TODO: Implement validation?
    }

    AppxManifestObject::AppxManifestObject(std::shared_ptr<StreamBase> stream) : VerifierObject(std::move(stream))
    {
        // TODO: Implement
    }

    AppxPackageObject::AppxPackageObject(APPX_VALIDATION_OPTION validation, std::unique_ptr<StorageObject>&& container) :
        m_validation(validation),
        m_container(std::move(container))
    {
        // 1. Get the appx signature from the container and parse it
        // TODO: pass validation flags and other necessary goodness through.
        m_appxSignature = std::make_unique<AppxSignatureObject>(validation, m_container->GetFile(APPXSIGNATURE_P7X));
        ThrowErrorIfNot(Error::AppxMissingSignatureP7X, (m_appxSignature->HasStream()), "AppxSignature.p7x not in archive!");

        // 2. Get content type using signature object for validation
        // TODO: switch underlying type of m_contentType to something more specific.
        m_contentType = std::make_unique<XmlObject>(m_appxSignature->GetValidationStream(
            CONTENT_TYPES_XML, m_container->GetFile(CONTENT_TYPES_XML)));
        ThrowErrorIfNot(Error::AppxMissingContentTypesXML, (m_contentType->HasStream()), "[Content_Types].xml not in archive!");

        // 3. Get blockmap object using signature object for validation
        m_appxBlockMap = std::make_unique<AppxBlockMapObject>(m_appxSignature->GetValidationStream(
            APPXBLOCKMAP_XML, m_container->GetFile(APPXBLOCKMAP_XML)));
        ThrowErrorIfNot(Error::AppxMissingBlockMapXML, (m_appxBlockMap->HasStream()), "AppxBlockMap.xml not in archive!");

        // 4. Get manifest object using blockmap object for validation
        // TODO: pass validation flags and other necessary goodness through.
        m_appxManifest = std::make_unique<AppxManifestObject>(m_appxBlockMap->GetValidationStream(
            APPXMANIFEST_XML, m_container->GetFile(APPXMANIFEST_XML)));
        ThrowErrorIfNot(Error::AppxMissingAppxManifestXML, (m_appxBlockMap->HasStream()), "AppxManifest.xml not in archive!");

        struct Config
        {
            using lambda = std::function<std::shared_ptr<StreamBase>()>;
            Config(lambda f) : GetValidationStream(f) {}
            lambda GetValidationStream;
        };

        std::map<std::string, Config> footPrintFileNames = {
            { APPXBLOCKMAP_XML,  Config([&](){ m_footprintFiles.push_back(APPXBLOCKMAP_XML);  return m_appxBlockMap->GetStream();})  },
            { APPXMANIFEST_XML,  Config([&](){ m_footprintFiles.push_back(APPXMANIFEST_XML);  return m_appxManifest->GetStream();})  },
            { APPXSIGNATURE_P7X, Config([&](){ m_footprintFiles.push_back(APPXSIGNATURE_P7X); return m_appxSignature->GetStream();}) },
            { CODEINTEGRITY_CAT, Config([&](){ m_footprintFiles.push_back(CODEINTEGRITY_CAT); return m_appxSignature->GetValidationStream(CODEINTEGRITY_CAT, m_container->GetFile(CODEINTEGRITY_CAT));}) },
            { CONTENT_TYPES_XML, Config([&]()->std::shared_ptr<StreamBase>{ return nullptr;}) }, // content types is never implicitly unpacked
        };

        // 5. Ensure that the stream collection contains streams wired up for their appropriate validation
        // and partition the container's file names into footprint and payload files.
        for (const auto& fileName : m_container->GetFileNames())
        {
            std::shared_ptr<StreamBase> stream = nullptr;

            auto footPrintFile = footPrintFileNames.find(fileName);
            if (footPrintFile != footPrintFileNames.end())
            {
                stream = footPrintFile->second.GetValidationStream();
            }
            else
            {
                m_payloadFiles.push_back(fileName);
                stream = m_appxBlockMap->GetValidationStream(fileName, m_container->GetFile(fileName));
            }

            if (stream != nullptr) { m_streams[fileName] = stream; }
        }
    }

    void AppxPackageObject::Pack(APPX_PACKUNPACK_OPTION options, const std::string& certFile, StorageObject& from)
    {
        // TODO: Implement
        throw Exception(Error::NotImplemented);
    }

    void AppxPackageObject::Unpack(APPX_PACKUNPACK_OPTION options, StorageObject& to)
    {
        auto fileNames = GetFileNames();
        for (const auto& fileName : fileNames)
        {
            std::string targetName;
            if (options & APPX_PACKUNPACK_OPTION_CREATEPACKAGESUBFOLDER)
            {
                targetName = GetAppxManifest()->GetPackageFullName() + to.GetPathSeparator() + fileName;
            }
            else
            {
                targetName = fileName;
            }

            auto targetFile = to.OpenFile(targetName, xPlat::FileStream::Mode::WRITE_UPDATE);
            auto sourceFile = GetFile(fileName);
            sourceFile->CopyTo(targetFile.get());
            targetFile->Close();
        }
    }

    std::string AppxPackageObject::GetPathSeparator() { return "/"; }

    std::vector<std::string> AppxPackageObject::GetFileNames()
    {
        std::vector<std::string> result(m_footprintFiles.begin(), m_footprintFiles.end());
        result.insert(result.end(), m_payloadFiles.begin(), m_payloadFiles.end());
        return result;
    }

    std::shared_ptr<StreamBase> AppxPackageObject::GetFile(const std::string& fileName)
    {
        // TODO: add input validation.
        return m_streams[fileName];
    }

    void AppxPackageObject::RemoveFile(const std::string& fileName)
    {
        // TODO: Implement
        throw Exception(Error::NotImplemented);
    }

    std::shared_ptr<StreamBase> AppxPackageObject::OpenFile(const std::string& fileName, FileStream::Mode mode)
    {
        // TODO: Implement
        throw Exception(Error::NotImplemented);
    }

    void AppxPackageObject::CommitChanges()
    {
        // TODO: Implement
        throw Exception(Error::NotImplemented);
    }

    // IAppxPackageReader
    HRESULT STDMETHODCALLTYPE AppxPackageObject::GetBlockMap(IAppxBlockMapReader**  blockMapReader)
    {
        return xPlat::ResultOf([&]() {
            // TODO: Implement
            throw Exception(Error::NotImplemented);
        });
    }

    HRESULT STDMETHODCALLTYPE AppxPackageObject::GetFootprintFile(APPX_FOOTPRINT_FILE_TYPE type, IAppxFile** file)
    {
        return xPlat::ResultOf([&]() {
            // TODO: Implement
            throw Exception(Error::NotImplemented);
        });
    }

    HRESULT STDMETHODCALLTYPE AppxPackageObject::GetPayloadFile(LPCWSTR fileName, IAppxFile** file)
    {
        return xPlat::ResultOf([&]() {
            // TODO: Implement
            throw Exception(Error::NotImplemented);
        });
    }

    HRESULT STDMETHODCALLTYPE AppxPackageObject::GetPayloadFiles(IAppxFilesEnumerator** filesEnumerator)
    {
        return xPlat::ResultOf([&]() {
            // TODO: Implement
            throw Exception(Error::NotImplemented);
        });
    }

    HRESULT STDMETHODCALLTYPE AppxPackageObject::GetManifest(IAppxManifestReader** manifestReader)
    {
        return xPlat::ResultOf([&]() {
            // TODO: Implement
            throw Exception(Error::NotImplemented);
        });
    }

    // IAppxFilesEnumerator
    HRESULT STDMETHODCALLTYPE AppxPackageObject::GetCurrent(IAppxFile** file)
    {
        return xPlat::ResultOf([&]() {
            // TODO: Implement
            throw Exception(Error::NotImplemented);
        });
    }

    HRESULT STDMETHODCALLTYPE AppxPackageObject::GetHasCurrent(BOOL* hasCurrent)
    {
        return xPlat::ResultOf([&]() {
            // TODO: Implement
            throw Exception(Error::NotImplemented);
        });
    }

    HRESULT STDMETHODCALLTYPE AppxPackageObject::MoveNext(BOOL* hasNext)
    {
        return xPlat::ResultOf([&]() {
            // TODO: Implement
            throw Exception(Error::NotImplemented);
        });
    }
}