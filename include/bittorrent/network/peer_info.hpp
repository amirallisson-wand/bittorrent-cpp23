#pragma once

#include <spdlog/fmt/bundled/format.h>
#include <array>
#include <cstdint>
#include <string>

namespace bittorrent::network {

struct PeerInfo {
    std::array<std::uint8_t, 4> ip;
    std::uint16_t port;

    std::string ip_string() const { return fmt::format("{}.{}.{}.{}", ip[0], ip[1], ip[2], ip[3]); }
};

enum class TrackerEvent {
    Started,
    Stopped,
    Completed,
    None,
};

constexpr std::string_view to_string(TrackerEvent event) noexcept {
    switch (event) {
        case TrackerEvent::Started:
            return "started";
        case TrackerEvent::Stopped:
            return "stopped";
        case TrackerEvent::Completed:
            return "completed";
        case TrackerEvent::None:
            return "";
    }
    return "";
}

}  // namespace bittorrent::network
