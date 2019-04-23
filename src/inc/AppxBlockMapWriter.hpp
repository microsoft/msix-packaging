//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once

#include "XmlWriter.hpp"
#include "ComHelper.hpp"

namespace MSIX {

    class BlockMapWriter final
    {
    public:
        BlockMapWriter();
        ~BlockMapWriter() {}

        void AddFileElement(const std::string& name, std::uint64_t uncompressedSize, std::uint32_t lfh);
        void AddBlockElement(const std::string& hash, std::size_t size);
        void CloseFileElement();
        void Close();
        ComPtr<IStream> GetStream() { return m_xmlWriter->GetStream(); }

    protected:
        std::unique_ptr<XmlWriter> m_xmlWriter;
    };
}