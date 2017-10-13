#include "Exceptions.hpp"
#include "StreamBase.hpp"
#include "StorageObject.hpp"
#include "AppxPackageObject.hpp"

#include <string>
#include <vector>
#include <memory>

namespace xPlat {

    AppxSignatureObject::AppxSignatureObject(std::shared_ptr<StreamBase>&& stream)
    {
        // TODO: Implement
    }

    std::shared_ptr<StreamBase> AppxSignatureObject::GetWholeFileValidationStream(std::string file)
    {
        // TODO: Implement
        throw Exception(Error::NotImplemented);
    }

    AppxBlockMapObject::AppxBlockMapObject(std::shared_ptr<StreamBase>&& stream)
    {
        // TODO: Implement
    }

    std::shared_ptr<StreamBase> AppxBlockMapObject::GetBlockMapValidationStream(std::string file)
    {
        // TODO: Implement
        throw Exception(Error::NotImplemented);
    }

    AppxPackageId::AppxPackageId(
        std::string name,
        std::string version,
        std::string resourceId,
        std::string architecture,
        std::string publisher) :
        Name(name), Version(version), ResourceId(resourceId), Architecture(architecture), PublisherHash(publisher)
    {
        // TODO: Implement validation?
    }

    AppxManifestObject::AppxManifestObject(std::shared_ptr<StreamBase>&& stream)
    {
        // TODO: Implement
    }

    AppxPackageObject::AppxPackageObject(xPlatValidationOptions validation, std::unique_ptr<StorageObject>&& container) :
        m_validation(validation),
        m_container(std::move(container))
    {
        // TODO: wire-up streams based on specified validation options
        for (const auto& fileName : m_container->GetFileNames())
        {
            m_streams[fileName] = m_container->GetFile(fileName);
        }
    }

    void AppxPackageObject::Pack(xPlatPackUnpackOptions options, std::string certFile, StorageObject& from)
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

    std::vector<std::string> AppxPackageObject::GetFootprintFiles()
    {
        // TODO: Implement
        throw Exception(Error::NotImplemented);
    }

    std::string AppxPackageObject::GetPathSeparator() { return "/"; }

    std::vector<std::string> AppxPackageObject::GetFileNames()
    {
        return m_container->GetFileNames();
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
}