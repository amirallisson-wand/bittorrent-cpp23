#include "bittorrent/core/types.hpp"
#include <iomanip>
#include <sstream>

namespace bittorrent::core {

std::string to_hex_string(const SHA1Hash& hash) {
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (const auto byte : hash) {
        oss << std::setw(2) << static_cast<unsigned int>(static_cast<std::uint8_t>(byte));
    }
    return oss.str();
}

std::expected<SHA1Hash, std::string> from_hex_string(std::string_view hex) {
    if (hex.size() != 40) {
        return std::unexpected("Invalid hex string length (expected 40 characters)");
    }

    SHA1Hash hash;
    for (std::size_t i = 0; i < 20; ++i) {
        std::string byte_str(hex.substr(i * 2, 2));
        char* end;
        auto value = std::strtol(byte_str.c_str(), &end, 16);
        if (end != byte_str.c_str() + 2) {
            return std::unexpected("Invalid hex character");
        }
        hash[i] = static_cast<std::byte>(value);
    }

    return hash;
}

}  // namespace bittorrent::core
