#include "Exceptions.hpp"
#include "SHA256.hpp"
namespace xPlat {
    bool SHA256::ComputeHash(
        /*in*/ std::uint8_t *buffer, 
        /*in*/ std::uint32_t cbBuffer, 
        /*inout*/ std::vector<uint8_t>& hash)
    {
        // TODO: Implement here...
        throw Exception(Error::NotImplemented);
    }
} // namespace xPlat {