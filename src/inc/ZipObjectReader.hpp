//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once

#include "Exceptions.hpp"
#include "ComHelper.hpp"
#include "ZipObject.hpp"

#include <vector>
#include <map>
#include <memory>

namespace MSIX {
    // This represents a raw stream over a.zip file.
    class ZipObjectReader final : public ZipObject
    {
    public:
        ZipObjectReader(const ComPtr<IStream>& stream);

        // IStorageObject methods
        std::vector<std::string> GetFileNames(FileNameOptions options) override;
        ComPtr<IStream> GetFile(const std::string& fileName) override;
        std::string GetFileName() override;

    protected:
        std::map<std::string, ComPtr<IStream>> m_streams;
    };
}
