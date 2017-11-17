#pragma once

#include <vector>

namespace xPlat {

    const unsigned HASH_BYTES = 32;

    class SHA256
    {
    public:
        static bool ComputeHash(/*in*/ std::uint8_t *buffer, /*in*/ std::uint32_t cbBuffer, /*inout*/ std::vector<uint8_t>& hash);
    };
}