#pragma once

#include <boost/asio/awaitable.hpp>
#include <boost/asio/io_context.hpp>
#include <string>
#include <string_view>
#include "bittorrent/core/types.hpp"
#include "tracker_response.hpp"

namespace bittorrent::network {

class HttpTracker {
public:
    explicit HttpTracker(boost::asio::io_context& io_context);

    boost::asio::awaitable<std::expected<TrackerResponse, TrackerError>> announce(
        std::string_view announce_url,
        const core::InfoHash& info_hash,
        const core::PeerID& peer_id,
        std::uint16_t port,
        std::int64_t uploaded,
        std::int64_t downloaded,
        std::int64_t left,
        TrackerEvent event = TrackerEvent::None
    );

    // Public for testing
    static std::expected<TrackerResponse, TrackerError> parse_response(std::string_view response_body);

private:
    boost::asio::io_context& io_context_;
};

}  // namespace bittorrent::network
