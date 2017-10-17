#pragma once

#include "Exceptions.hpp"
#include "StreamBase.hpp"
#include "StorageObject.hpp"

#include <vector>
#include <map>
#include <memory>

namespace xPlat {
    // forward declarations
    class CentralDirectoryFileHeader;
    class LocalFileHeader;

    // This represents a raw stream over a.zip file.
    class ZipObject : public StorageObject
    {
    public:
        ZipObject(std::unique_ptr<StreamBase>&& stream);

        // StorageObject methods
        std::string                 GetPathSeparator() override;
        std::vector<std::string>    GetFileNames() override;
        std::shared_ptr<StreamBase> GetFile(const std::string& fileName) override;
        void                        RemoveFile(const std::string& fileName) override;
        std::shared_ptr<StreamBase> OpenFile(const std::string& fileName, FileStream::Mode mode) override;
        void                        CommitChanges() override;

    protected:
        std::unique_ptr<StreamBase>                                          m_stream;
        std::map<std::string, std::shared_ptr<StreamBase>>                   m_streams;
    };//class ZipObject
}
