//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once
#include <vector>
#include <string>

namespace MsixTest { 

    namespace BlockMap {

        struct BlockMapBlock
        {
            const std::uint32_t compressedSize;
            const std::vector<std::uint8_t> hash;

            BlockMapBlock(std::uint32_t c, const std::vector<std::uint8_t>& h) : compressedSize(c), hash(h)
            {}
        };

        struct BlockMapFileInfo
        {
            const std::string name;
            const std::uint32_t lfh;
            const std::uint64_t size;
            const std::vector<BlockMapBlock> blocks;

            BlockMapFileInfo(const char* n, std::uint32_t l, std::uint64_t s, const std::vector<BlockMapBlock>& b) : 
                name(n), lfh(l), size(s), blocks(b) {}
        };

        const std::vector<BlockMapFileInfo>& GetExpectedBlockMapFiles();
    }
}

