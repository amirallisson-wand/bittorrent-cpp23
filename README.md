# Modern C++23 BitTorrent Client (WIP)

A high-performance BitTorrent client built with modern C++23 features, Asio coroutines, and io_uring.

## Features (In Progress)

- **Bencode Parser**: Complete implementation with `std::expected` error handling
- **Async I/O**: io_uring integration (planned)
- **Coroutines**: C++23 `co_await` for network operations (planned)
- **Peer Protocol**: Full BitTorrent wire protocol (planned)

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
├── include/bittorrent/    # Public API headers
│   ├── bencode.hpp        # All-in-one bencode header
│   └── bencode/           # Bencode module
│       ├── errors.hpp     # Error types
│       ├── value.hpp      # Value type (variant)
│       ├── parser.hpp     # Bencode parser
│       └── encoder.hpp    # Bencode encoder
├── src/                   # Implementation
│   └── bencode/
├── tests/                 # Unit tests
└── torrents/              # Test torrent files
```

MIT License

## Status

**Work in Progress** - Currently implementing the bencode parser module.
