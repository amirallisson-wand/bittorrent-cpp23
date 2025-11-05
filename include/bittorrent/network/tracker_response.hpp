#pragma once

#include <chrono>
#include <optional>
#include <string>
#include <vector>
#include "errors.hpp"
#include "peer_info.hpp"

namespace bittorrent::network {

struct TrackerResponse {
    std::chrono::seconds interval;
    std::optional<std::chrono::seconds> min_interval;
    std::optional<std::string> tracker_id;
    std::int64_t complete{0};    // Number of seeders
    std::int64_t incomplete{0};  // Number of leechers
    std::vector<PeerInfo> peers;
    std::optional<std::string> warning_message;
};

}  // namespace bittorrent::network
