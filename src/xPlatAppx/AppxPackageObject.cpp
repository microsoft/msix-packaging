#include "Exceptions.hpp"
#include "StreamBase.hpp"
#include "StorageObject.hpp"
#include "AppxPackageObject.hpp"
#include "AppxPackaging.hpp"
#include "ComHelper.hpp"

#include <string>
#include <vector>
#include <memory>

namespace xPlat {

    // names of footprint files.
    #define APPXBLOCKMAP_XML  "AppxBlockMap.xml"
    #define APPXMANIFEST_XML  "AppxManifest.xml"
    #define CODEINTEGRITY_CAT "AppxMetadata/CodeIntegrity.cat"
    #define APPXSIGNATURE_P7X "AppxSignature.p7x"
    #define CONTENT_TYPES_XML "[Content_Types].xml"

    AppxSignatureObject::AppxSignatureObject(std::shared_ptr<StreamBase> stream) :
        m_stream(stream)
    {
        // TODO: Implement
    }

    std::shared_ptr<StreamBase> AppxSignatureObject::GetValidationStream(
        const std::string& file,
        std::shared_ptr<StreamBase> stream)
    {
        // TODO: Implement -- for now, just pass through.
        return stream;
    }

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

    AppxManifestObject::AppxManifestObject(std::shared_ptr<StreamBase> stream) : m_stream(stream)
    {
        // TODO: Implement
    }

    AppxPackageObject::AppxPackageObject(xPlatValidationOptions validation, std::unique_ptr<StorageObject>&& container) :
        m_validation(validation),
        m_container(std::move(container))
    {
        // 1. Get the appx signature from the container and parse it
        // TODO: pass validation flags and other necessary goodness through.
        m_appxSignature = std::make_unique<AppxSignatureObject>(m_container->GetFile(APPXSIGNATURE_P7X));
        ThrowErrorIfNot(Error::AppxMissingSignatureP7X, (m_appxSignature != nullptr), "AppxSignature.p7x not in archive!");

        // 2. Get content type using signature object for validation
        // TODO: switch underlying type of m_contentType to something more specific.
        m_contentType = std::make_unique<XmlObject>(m_appxSignature->GetValidationStream(
            CONTENT_TYPES_XML, m_container->GetFile(CONTENT_TYPES_XML)));
        ThrowErrorIfNot(Error::AppxMissingContentTypesXML, (m_contentType != nullptr), "[Content_Types].xml not in archive!");

        // 3. Get blockmap object using signature object for validation
        m_appxBlockMap = std::make_unique<AppxBlockMapObject>(m_appxSignature->GetValidationStream(
            APPXBLOCKMAP_XML, m_container->GetFile(APPXBLOCKMAP_XML)));
        ThrowErrorIfNot(Error::AppxMissingBlockMapXML, (m_appxBlockMap != nullptr), "AppxBlockMap.xml not in archive!");

        // 4. Get manifest object using blockmap object for validation
        // TODO: pass validation flags and other necessary goodness through.
        m_appxManifest = std::make_unique<AppxManifestObject>(m_appxBlockMap->ValidationStream(
            APPXMANIFEST_XML, m_container->GetFile(APPXMANIFEST_XML)));
        ThrowErrorIfNot(Error::AppxMissingAppxManifestXML, (m_appxBlockMap != nullptr), "AppxManifest.xml not in archive!");

        std::map<std::string, bool> footPrintFileNames = {
            { APPXBLOCKMAP_XML,  false },
            { APPXMANIFEST_XML,  false },
            { APPXSIGNATURE_P7X, false },
            { CODEINTEGRITY_CAT, false },
            { CONTENT_TYPES_XML, false },
        };

        for (const auto& fileName : m_container->GetFileNames())
        {
            std::shared_ptr<StreamBase> stream = nullptr;

            auto footPrintFile = footPrintFileNames.find(fileName);
            if (footPrintFile == footPrintFileNames.end())
            {
                stream = m_appxBlockMap->ValidationStream(fileName, m_container->GetFile(fileName));
                m_payloadFiles.push_back(fileName);
            }
            else
            {
                ThrowErrorIf(Error::AppxDuplicateFootprintFile, (footPrintFile->second), "two or more footprint files with same name!");
                footPrintFile->second = true;

                // CONTENT_TYPES_XML is not a footprint file we surface out via unpack indirectly.
                // However, we will do so for every other footprint file type...
                if (footPrintFile->first == CONTENT_TYPES_XML)
                {
                    stream = nullptr;
                }
                else
                {
                    m_footprintFiles.push_back(fileName);
                }

                if (footPrintFile->first == APPXBLOCKMAP_XML)
                {
                    stream = m_appxBlockMap->GetStream();
                }
                else if (footPrintFile->first == APPXMANIFEST_XML)
                {
                    stream = m_appxManifest->GetStream();
                }
                else if (footPrintFile->first == APPXSIGNATURE_P7X)
                {
                    stream = m_appxSignature->GetStream();
                }
                else if (footPrintFile->first == CODEINTEGRITY_CAT)
                {
                    stream = m_appxSignature->GetValidationStream(footPrintFile->first, m_container->GetFile(fileName));
                }
            }

            if (stream != nullptr) { m_streams[fileName] = stream; }
        }
    }

    void AppxPackageObject::Pack(xPlatPackUnpackOptions options, const std::string& certFile, StorageObject& from)
    {
        // TODO: Implement
        throw Exception(Error::NotImplemented);
    }

    void AppxPackageObject::Unpack(xPlatPackUnpackOptions options, StorageObject& to)
    {
        auto fileNames = GetFileNames();
        for (const auto& fileName : fileNames)
        {
            std::string targetName;
            if (options & xPlatPackUnpackOptionsCreatePackageSubfolder)
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