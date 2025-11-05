#pragma once

#include <array>
#include <cstdint>
#include <expected>
#include <string>
#include <vector>

namespace bittorrent::core {

// https://www.bittorrent.org/beps/bep_0003.html#:~:text=trackers,they%20cease%20downloading.
using SHA1Hash = std::array<std::byte, 20>;

using InfoHash = SHA1Hash;

using PeerID = std::array<std::byte, 20>;

std::string to_hex_string(const SHA1Hash& hash);

std::expected<SHA1Hash, std::string> from_hex_string(std::string_view hex);

}  // namespace bittorrent::core
