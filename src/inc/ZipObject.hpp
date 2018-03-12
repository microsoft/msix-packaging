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
        ZipObject(IMSIXFactory* factory, IStream* stream);

        // StorageObject methods
        std::string                 GetPathSeparator() override;
        std::vector<std::string>    GetFileNames(FileNameOptions options) override;
        std::pair<bool,IStream*>    GetFile(const std::string& fileName) override;
        void                        RemoveFile(const std::string& fileName) override;
        IStream*                    OpenFile(const std::string& fileName, MSIX::FileStream::Mode mode) override;
        void                        CommitChanges() override;

    protected:
        IMSIXFactory*                          m_factory;
        ComPtr<IStream>                        m_stream;
        std::map<std::string, ComPtr<IStream>> m_streams;
    };//class ZipObject
}
