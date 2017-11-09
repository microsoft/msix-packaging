#pragma once

#include <vector>

namespace xPlat {

    class SHA256
    {
    public:
        static bool ComputeHash(/*in*/ std::uint8_t *buffer, /*in*/ std::uint32_t cbBuffer, /*inout*/ std::vector<byte>& hash);
    };
}