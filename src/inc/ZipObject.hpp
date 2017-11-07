#pragma once

#include "Exceptions.hpp"
#include "ComHelper.hpp"
#include "StreamBase.hpp"
#include "StorageObject.hpp"
#include "AppxFactory.hpp"

#include <vector>
#include <map>
#include <memory>

namespace xPlat {
    // This represents a raw stream over a.zip file.
    class ZipObject : public ComClass<ZipObject, IStorageObject>
    {
    public:
        ZipObject(IxPlatFactory* appxFactory, IStream* stream);

        // StorageObject methods
        std::string                 GetPathSeparator() override;
        std::vector<std::string>    GetFileNames() override;
        IStream*                    GetFile(const std::string& fileName) override;
        void                        RemoveFile(const std::string& fileName) override;
        IStream*                    OpenFile(const std::string& fileName, xPlat::FileStream::Mode mode) override;
        void                        CommitChanges() override;

    protected:
        ComPtr<IxPlatFactory>                  m_factory;
        ComPtr<IStream>                        m_stream;
        std::map<std::string, ComPtr<IStream>> m_streams;
    };//class ZipObject
}
