#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <random>
#include "bittorrent/core.hpp"
#include "bittorrent/core/formatters.hpp"
#include "bittorrent/network.hpp"

namespace asio = boost::asio;
using namespace bittorrent;

core::PeerID generate_peer_id() {
    core::PeerID peer_id;

    // Format: -BT0001-<12 random bytes>
    // BT = BitTorrent client, 0001 = version
    std::string prefix = "-BT0001-";
    std::copy(prefix.begin(), prefix.end(), reinterpret_cast<char*>(peer_id.data()));

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);

    for (size_t i = prefix.size(); i < peer_id.size(); ++i) {
        peer_id[i] = static_cast<std::byte>(dis(gen));
    }

    return peer_id;
}

asio::awaitable<void> run_client() {
    spdlog::info("BitTorrent Client starting...");

    auto torrent =
        core::TorrentInfo::from_file("/home/amirallisson/bittorrent/torrents/debian-13.1.0-amd64-netinst.iso.torrent");

    if (!torrent) {
        spdlog::error("Failed to parse torrent: {}", to_string(torrent.error()));
        co_return;
    }

    spdlog::info("Successfully loaded torrent:\n{}", *torrent);
    spdlog::info("Info hash: {}", core::to_hex_string(torrent->info_hash()));

    auto peer_id = generate_peer_id();
    spdlog::info("Generated peer ID: {}", core::to_hex_string(peer_id));

    auto executor = co_await asio::this_coro::executor;
    auto& io_context = static_cast<boost::asio::io_context&>(executor.context());
    network::HttpTracker tracker(io_context);

    spdlog::info("Announcing to tracker: {}", torrent->announce());

    auto response = co_await tracker.announce(
        torrent->announce(),
        torrent->info_hash(),
        peer_id,
        /* port */ 6881,
        /* uploaded */ 0,
        /* downloaded */ 0,
        /* left */ torrent->total_size(),
        /* event */ network::TrackerEvent::Started
    );

    if (!response) {
        spdlog::error("Tracker announce failed: {}", network::to_string(response.error()));
        co_return;
    }

    spdlog::info("Tracker announce successful!");
    if (response->warning_message) {
        spdlog::warn("Tracker warning: {}", *response->warning_message);
    }

    for (const auto& peer : response->peers) {
        spdlog::info("  Peer: {}:{}", peer.ip_string(), peer.port);
    }
}

int main() {
    // Setup logging
    auto console = spdlog::stdout_color_mt("console");
    spdlog::set_default_logger(console);
    spdlog::set_level(spdlog::level::debug);
    spdlog::set_pattern("[%^%l%$] %v");

    try {
        asio::io_context io_context;

        asio::co_spawn(io_context, run_client(), [](std::exception_ptr e) {
            if (!e) {
                return;
            }
            try {
                std::rethrow_exception(e);
            } catch (const std::exception& ex) {
                spdlog::error("Exception in client: {}", ex.what());
            }
        });

        io_context.run();

        spdlog::info("Client finished.");
        return 0;

    } catch (const std::exception& e) {
        spdlog::error("Fatal error: {}", e.what());
        return 1;
    }
}
