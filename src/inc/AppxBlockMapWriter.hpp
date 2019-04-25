//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once

#include "XmlWriter.hpp"
#include "ComHelper.hpp"

#include <vector>

namespace MSIX {

    class BlockMapWriter final
    {
    public:
        BlockMapWriter();

        void AddFile(const std::string& name, std::uint64_t uncompressedSize, std::uint32_t lfh);
        void AddBlock(const std::vector<std::uint8_t>& hash, std::size_t size = 0);
        void CloseFile();
        void Close();
        ComPtr<IStream> GetStream() { return m_xmlWriter.GetStream(); }

    protected:
        XmlWriter m_xmlWriter;
    };
}