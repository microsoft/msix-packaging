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
        DirectoryObject(std::string root) : m_root(root) {}

        // StorageObject methods
        virtual std::vector<std::string>    GetFileNames() override;
        virtual std::shared_ptr<StreamBase> GetFile(std::string& fileName) override;
        virtual void                        RemoveFile(std::string& fileName) override;
        virtual std::shared_ptr<StreamBase> OpenFile(std::string& fileName, FileStream::Mode mode) override;
        virtual void                        CommitChanges() override;

    protected:
        std::map<std::string, std::shared_ptr<StreamBase>> m_streams;
        std::string m_root;

    };//class ZipObject
}