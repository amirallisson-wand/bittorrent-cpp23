#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace bittorrent::core {

struct FileInfo {
    std::filesystem::path path;
    std::int64_t length;
    std::int64_t offset{0};
};

}  // namespace bittorrent::core
