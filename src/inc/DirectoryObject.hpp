#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>

#include "Exceptions.hpp"
#include "StreamBase.hpp"
#include "StorageObject.hpp"

namespace xPlat {

    class DirectoryObject : public StorageObject
    {
    public:
        DirectoryObject(std::string&& root) : m_root(std::move(root)) {}

        // StorageObject methods
        std::vector<std::string>    GetFileNames() override;
        std::shared_ptr<StreamBase> GetFile(const std::string& fileName) override;
        void                        RemoveFile(const std::string& fileName) override;
        std::shared_ptr<StreamBase> OpenFile(const std::string& fileName, FileStream::Mode mode) override;
        void                        CommitChanges() override;

    protected:
        std::map<std::string, std::shared_ptr<StreamBase>> m_streams;
        std::string m_root;

    };//class ZipObject
}