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
    class ZipObject final : public ComClass<ZipObject, IStorageObject>
    {
    public:
        ZipObject(IMsixFactory* factory, const ComPtr<IStream>& stream);

        // IStorageObject methods
        std::vector<std::string> GetFileNames(FileNameOptions options) override;
        ComPtr<IStream> GetFile(const std::string& fileName) override;
        std::string GetFileName() override;

    protected:
        IMsixFactory*                          m_factory;
        ComPtr<IStream>                        m_stream;
        std::map<std::string, ComPtr<IStream>> m_streams;
    };//class ZipObject
}
