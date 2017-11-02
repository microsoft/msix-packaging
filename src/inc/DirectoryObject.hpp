#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>

#include "Exceptions.hpp"
#include "StreamBase.hpp"
#include "StorageObject.hpp"
#include "ComHelper.hpp"

namespace xPlat {

    class DirectoryObject : public StorageObject
    {
    public:
        DirectoryObject(std::string root) : m_root(std::move(root)) {}

        // StorageObject methods
        std::string              GetPathSeparator() override;
        std::vector<std::string> GetFileNames() override;
        IStream*                 GetFile(const std::string& fileName) override;
        void                     RemoveFile(const std::string& fileName) override;
        IStream*                 OpenFile(const std::string& fileName, FileStream::Mode mode) override;
        void                     CommitChanges() override;

    protected:
        std::map<std::string, ComPtr<IStream>> m_streams;
        std::string m_root;

    };//class ZipObject
}