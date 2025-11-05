# Modern C++23 BitTorrent Client (WIP)

A high-performance BitTorrent client built with modern C++23 features, Asio coroutines, and io_uring.

## Features (In Progress)

### ✅ Complete
- **Bencode Parser**: Full implementation with `std::expected` error handling
- **Torrent Parser**: Parse `.torrent` files into structured data

### 🚧 Planned
- **Async I/O**: io_uring integration
- **Coroutines**: C++23 `co_await` for network operations
- **Peer Protocol**: Full BitTorrent wire protocol
- **Download Manager**: Multi-peer coordination

## Building

### Requirements

- C++23 compatible compiler (GCC 13+, Clang 16+)
- CMake 3.14+
- Google Test (automatically fetched)

### Build Commands

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### Running Tests

```bash
cd build
make run_tests
```

## Project Structure

```
bittorrent/
├── include/bittorrent/     # Public API headers
│   ├── bencode.hpp         # Bencode module (all-in-one)
│   ├── bencode/            # Bencode components
│   ├── core.hpp            # Core module (all-in-one)
│   └── core/               # Core components
│       ├── types.hpp       # SHA1, InfoHash, etc.
│       ├── file_info.hpp   # File metadata
│       └── torrent_info.hpp # Torrent parser
├── src/                    # Implementation
│   ├── bencode/
│   └── core/
├── apps/                   # Example applications
│   └── parse_torrent.cpp   # Parse and display torrent info
├── tests/                  # Unit tests
└── torrents/               # Test torrent files
```

## Usage Example

```cpp
#include "bittorrent/core.hpp"
#include <iostream>

int main() {
    // Parse torrent file
    auto result = bittorrent::core::TorrentInfo::from_file("ubuntu.torrent");
    
    if (!result) {
        std::cerr << "Parse error: " << to_string(result.error()) << '\n';
        return 1;
    }
    
    const auto& torrent = *result;
    std::cout << "Name: " << torrent.name() << '\n';
    std::cout << "Size: " << torrent.total_size() << " bytes\n";
    std::cout << "Pieces: " << torrent.piece_count() << '\n';
    std::cout << "Tracker: " << torrent.announce() << '\n';
    
    return 0;
}
```

MIT License

## Status

**Work in Progress** - Currently implementing the peer-to-peer protocol
