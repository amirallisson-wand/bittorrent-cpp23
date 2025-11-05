#pragma once

#include <string_view>

namespace bittorrent::network {

enum class TrackerError {
    ConnectionFailed,
    InvalidResponse,
    TrackerFailure,
    Timeout,
    ParseError,
};

constexpr std::string_view to_string(TrackerError error) noexcept {
    switch (error) {
        case TrackerError::ConnectionFailed:
            return "Connection failed";
        case TrackerError::InvalidResponse:
            return "Invalid response";
        case TrackerError::TrackerFailure:
            return "Tracker failure";
        case TrackerError::Timeout:
            return "Timeout";
        case TrackerError::ParseError:
            return "Parse error";
    }
    return "Unknown error";
}

}  // namespace bittorrent::network
