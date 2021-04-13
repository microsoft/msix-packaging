//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once

#include "Crypto.hpp"
#include "XmlWriter.hpp"
#include "ComHelper.hpp"

#include <vector>

namespace MSIX {

    static const std::uint32_t DefaultBlockSize = 65536;

    class BlockMapWriter final
    {
    public:
        BlockMapWriter();

        void EnableFileHash();
        void AddFile(const std::string& name, std::uint64_t uncompressedSize, std::uint32_t lfh);
        void AddBlock(const std::vector<std::uint8_t>& block, ULONG size, bool isCompressed);
        void CloseFile();
        void Close();
        ComPtr<IStream> GetStream() { return m_xmlWriter.GetStream(); }

    protected:
        XmlWriter m_xmlWriter;

    private:
        MSIX::SHA256 m_fileHashEngine;
        bool m_enableFileHash = false;
        bool m_addFileHash = false;
    };
}