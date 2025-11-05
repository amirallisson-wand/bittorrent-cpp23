#pragma once

#include <string_view>

namespace bittorrent::core {

enum class TorrentError {
    InvalidFormat,
    MissingRequiredField,
    InvalidFieldType,
    InvalidPieceLength,
    InvalidPieceHash,
};

constexpr std::string_view to_string(TorrentError error) noexcept {
    switch (error) {
        case TorrentError::InvalidFormat:
            return "Invalid torrent format";
        case TorrentError::MissingRequiredField:
            return "Missing required field";
        case TorrentError::InvalidFieldType:
            return "Invalid field type";
        case TorrentError::InvalidPieceLength:
            return "Invalid piece length";
        case TorrentError::InvalidPieceHash:
            return "Invalid piece hash";
    }
    return "Unknown error";
}

}  // namespace bittorrent::core
