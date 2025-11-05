#include <iostream>
#include "bittorrent/core.hpp"

int main() {
    auto result = bittorrent::core::TorrentInfo::from_file("torrents/ubuntu-25.10-desktop-amd64.iso.torrent");
    
    if (!result) {
        std::cerr << "Failed to parse torrent\n";
        return 1;
    }
    
    std::cout << "Parsed torrent: " << result->name() << '\n';
    
    return 0;
}
