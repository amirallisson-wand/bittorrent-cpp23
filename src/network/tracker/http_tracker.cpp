#include "bittorrent/network/http_tracker.hpp"
#include <spdlog/spdlog.h>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/url.hpp>
#include <string_view>
#include "bittorrent/bencode/parser.hpp"
#include "bittorrent/bencode/value.hpp"

namespace beast = boost::beast;
namespace http = beast::http;
namespace asio = boost::asio;
using tcp = asio::ip::tcp;

namespace bittorrent::network {

HttpTracker::HttpTracker(asio::io_context& io_context) : io_context_(io_context) {}

std::expected<TrackerResponse, TrackerError> HttpTracker::parse_response(std::string_view response_body) {
    auto result = bencode::Parser::parse(response_body);

    if (!result) {
        spdlog::error("Failed to parse tracker response: {}", bencode::to_string(result.error()));
        return std::unexpected(TrackerError::ParseError);
    }

    const auto& value = *result;
    if (!value.is_dictionary()) {
        spdlog::error("Tracker response is not a dictionary");
        return std::unexpected(TrackerError::InvalidResponse);
    }

    const auto& dict = value.as_dictionary();

    if (dict.contains("failure reason")) {
        const auto& failure = dict.at("failure reason");
        if (failure.is_string()) {
            spdlog::error("Tracker failure: {}", failure.as_string());
        }
        return std::unexpected(TrackerError::TrackerFailure);
    }

    TrackerResponse response;
    if (!dict.contains("interval") || !dict.at("interval").is_integer()) {
        spdlog::error("Missing or invalid 'interval' in tracker response");
        return std::unexpected(TrackerError::InvalidResponse);
    }
    response.interval = std::chrono::seconds(dict.at("interval").as_integer());

    if (dict.contains("min interval") && dict.at("min interval").is_integer()) {
        response.min_interval = std::chrono::seconds(dict.at("min interval").as_integer());
    }

    if (dict.contains("tracker id") && dict.at("tracker id").is_string()) {
        response.tracker_id = dict.at("tracker id").as_string();
    }

    if (dict.contains("complete") && dict.at("complete").is_integer()) {
        response.complete = dict.at("complete").as_integer();
    }

    if (dict.contains("incomplete") && dict.at("incomplete").is_integer()) {
        response.incomplete = dict.at("incomplete").as_integer();
    }

    if (dict.contains("warning message") && dict.at("warning message").is_string()) {
        response.warning_message = dict.at("warning message").as_string();
    }

    if (dict.contains("peers")) {
        const auto& peers_value = dict.at("peers");
        if (peers_value.is_string()) {
            const auto& peers_data = peers_value.as_string();

            if (peers_data.size() % 6 != 0) {
                spdlog::error("Invalid peers data size: {}", peers_data.size());
                return std::unexpected(TrackerError::InvalidResponse);
            }

            for (size_t i = 0; i < peers_data.size(); i += 6) {
                PeerInfo peer;
                peer.ip[0] = static_cast<std::uint8_t>(peers_data[i]);
                peer.ip[1] = static_cast<std::uint8_t>(peers_data[i + 1]);
                peer.ip[2] = static_cast<std::uint8_t>(peers_data[i + 2]);
                peer.ip[3] = static_cast<std::uint8_t>(peers_data[i + 3]);

                // Network byte order
                // https://www.ibm.com/docs/en/zvm/7.3.0?topic=domains-network-byte-order-host-byte-order
                peer.port = (static_cast<std::uint16_t>(static_cast<std::uint8_t>(peers_data[i + 4])) << 8) |
                            static_cast<std::uint16_t>(static_cast<std::uint8_t>(peers_data[i + 5]));

                response.peers.emplace_back(std::move(peer));
            }
        } else if (peers_value.is_list()) {
            spdlog::warn("Dictionary format peers not yet supported");
        }
    }

    spdlog::info(
        "Parsed tracker response: interval={}s, peers={}, seeders={}, leechers={}",
        response.interval.count(),
        response.peers.size(),
        response.complete,
        response.incomplete
    );

    return response;
}

asio::awaitable<std::expected<TrackerResponse, TrackerError>> HttpTracker::announce(
    std::string_view announce_url,
    const core::InfoHash& info_hash,
    const core::PeerID& peer_id,
    std::uint16_t port,
    std::int64_t uploaded,
    std::int64_t downloaded,
    std::int64_t left,
    TrackerEvent event
) {
    try {
        auto parsed = boost::urls::parse_uri(announce_url);
        if (!parsed) {
            spdlog::error("Failed to parse tracker URL: {}", announce_url);
            co_return std::unexpected(TrackerError::InvalidResponse);
        }

        boost::urls::url url = *parsed;
        std::string host{url.host()};
        std::string port_str = url.has_port() ? std::string{url.port()} : "80";
        std::string_view info_hash_view(reinterpret_cast<const char*>(info_hash.data()), info_hash.size());
        std::string_view peer_id_view(reinterpret_cast<const char*>(peer_id.data()), peer_id.size());

        boost::urls::params_ref params = url.params();
        params.append({"info_hash", info_hash_view});
        params.append({"peer_id", peer_id_view});
        params.append({"port", std::to_string(port)});
        params.append({"uploaded", std::to_string(uploaded)});
        params.append({"downloaded", std::to_string(downloaded)});
        params.append({"left", std::to_string(left)});
        params.append({"compact", "1"});

        if (event != TrackerEvent::None) {
            params.append({"event", std::string{to_string(event)}});
        }

        auto target = url.encoded_target();
        spdlog::debug("Announcing to tracker: {}:{}{}", host, port_str, target);

        tcp::resolver resolver(io_context_);
        auto endpoints = co_await resolver.async_resolve(host, port_str, asio::use_awaitable);

        beast::tcp_stream stream(io_context_);
        stream.expires_after(std::chrono::seconds(30));

        co_await stream.async_connect(endpoints, asio::use_awaitable);

        http::request<http::string_body> req{http::verb::get, target, 11};
        req.set(http::field::host, host);
        req.set(http::field::user_agent, "bittorrent-cpp23/1.0");
        req.set(http::field::connection, "close");
        co_await http::async_write(stream, req, asio::use_awaitable);

        beast::flat_buffer buffer;
        http::response<http::string_body> res;
        co_await http::async_read(stream, buffer, res, asio::use_awaitable);
        spdlog::debug("Received tracker response: {} bytes, status={}", res.body().size(), res.result_int());

        beast::error_code ec;
        stream.socket().shutdown(tcp::socket::shutdown_both, ec);

        if (res.result() != http::status::ok) {
            spdlog::error("Tracker returned HTTP {}", res.result_int());
            co_return std::unexpected(TrackerError::TrackerFailure);
        }

        co_return parse_response(res.body());

    } catch (const std::exception& e) {
        spdlog::error("Tracker announce failed: {}", e.what());
        co_return std::unexpected(TrackerError::ConnectionFailed);
    }
}

}  // namespace bittorrent::network
