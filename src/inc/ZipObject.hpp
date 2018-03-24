//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once

#include "Exceptions.hpp"
#include "ComHelper.hpp"
#include "StreamBase.hpp"
#include "StorageObject.hpp"
#include "AppxFactory.hpp"

#include <vector>
#include <map>
#include <memory>

namespace MSIX {
    // This represents a raw stream over a.zip file.
    class ZipObject : public ComClass<ZipObject, IStorageObject>
    {
    public:
        ZipObject(IMSIXFactory* factory, const ComPtr<IStream>& stream);

        // StorageObject methods
        const char*                 GetPathSeparator() override { return "/"; }
        std::vector<std::string>    GetFileNames(FileNameOptions options) override;
        ComPtr<IStream>             GetFile(const std::string& fileName) override;

        ComPtr<IStream>             OpenFile(const std::string& fileName, MSIX::FileStream::Mode mode) override { NOTIMPLEMENTED; }
        void                        CommitChanges() override { NOTIMPLEMENTED; }

    protected:
        IMSIXFactory*                          m_factory;
        ComPtr<IStream>                        m_stream;
        std::map<std::string, ComPtr<IStream>> m_streams;
    };//class ZipObject
}
