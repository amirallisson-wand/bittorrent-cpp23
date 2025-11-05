#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include "bittorrent/core.hpp"

int main() {
    auto console = spdlog::stdout_color_mt("console");
    spdlog::set_default_logger(console);
    spdlog::set_level(spdlog::level::debug);
    spdlog::set_pattern("[%^%l%$] %v");
    
    spdlog::info("BitTorrent Client starting...");
    
    auto result = bittorrent::core::TorrentInfo::from_file("/home/amirallisson/bittorrent/torrents/debian-13.1.0-amd64-netinst.iso.torrent");
    
    if (!result) {
        spdlog::error("Failed to parse torrent: {}", to_string(result.error()));
        return 1;
    }

    spdlog::info("Successfully loaded torrent: {}", result->name());
    
    spdlog::info("Torrent info: {}", *result);

    return 0;
}
