#include "bittorrent/core/torrent_info.hpp"
#include <fstream>
#include <sstream>
#include "bittorrent/bencode.hpp"

namespace bittorrent::core {

namespace {

std::expected<std::string, TorrentError> get_string_field(const bencode::Dictionary& dict, const std::string& key) {
    auto it = dict.find(key);
    if (it == dict.end()) {
        return std::unexpected(TorrentError::MissingRequiredField);
    }
    if (!it->second.is_string()) {
        return std::unexpected(TorrentError::InvalidFieldType);
    }
    return it->second.as_string();
}

std::expected<std::int64_t, TorrentError> get_integer_field(const bencode::Dictionary& dict, const std::string& key) {
    auto it = dict.find(key);
    if (it == dict.end()) {
        return std::unexpected(TorrentError::MissingRequiredField);
    }
    if (!it->second.is_integer()) {
        return std::unexpected(TorrentError::InvalidFieldType);
    }
    return it->second.as_integer();
}

std::expected<std::vector<SHA1Hash>, TorrentError> parse_piece_hashes(const std::string& pieces_str) {
    if (pieces_str.size() % 20 != 0) {
        return std::unexpected(TorrentError::InvalidPieceHash);
    }

    std::vector<SHA1Hash> hashes;
    hashes.reserve(pieces_str.size() / 20);

    for (std::size_t i = 0; i < pieces_str.size(); i += 20) {
        SHA1Hash hash;
        for (std::size_t j = 0; j < 20; ++j) {
            hash[j] = static_cast<std::byte>(static_cast<unsigned char>(pieces_str[i + j]));
        }
        hashes.push_back(hash);
    }

    return hashes;
}

std::expected<std::vector<FileInfo>, TorrentError>
parse_files(const bencode::Dictionary& info_dict, const std::string& name) {
    std::vector<FileInfo> files;

    // Check if it's a single-file torrent
    auto length_it = info_dict.find("length");
    if (length_it != info_dict.end()) {
        if (!length_it->second.is_integer()) {
            return std::unexpected(TorrentError::InvalidFieldType);
        }
        files.emplace_back(name, length_it->second.as_integer());
        return files;
    }

    // Multi-file torrent
    auto files_it = info_dict.find("files");
    if (files_it == info_dict.end() || !files_it->second.is_list()) {
        return std::unexpected(TorrentError::MissingRequiredField);
    }

    const auto& files_list = files_it->second.as_list();
    std::int64_t offset = 0;

    for (const auto& file_value : files_list) {
        if (!file_value.is_dictionary()) {
            return std::unexpected(TorrentError::InvalidFieldType);
        }

        const auto& file_dict = file_value.as_dictionary();

        auto length = get_integer_field(file_dict, "length");
        if (!length) {
            return std::unexpected(length.error());
        }

        auto path_it = file_dict.find("path");
        if (path_it == file_dict.end() || !path_it->second.is_list()) {
            return std::unexpected(TorrentError::MissingRequiredField);
        }

        const auto& path_list = path_it->second.as_list();
        std::filesystem::path file_path = name;

        for (const auto& component : path_list) {
            if (!component.is_string()) {
                return std::unexpected(TorrentError::InvalidFieldType);
            }
            file_path /= component.as_string();
        }

        FileInfo file_info(file_path, *length);
        file_info.offset = offset;
        offset += *length;

        files.push_back(std::move(file_info));
    }

    return files;
}

std::vector<std::vector<std::string>> parse_announce_list(const bencode::Dictionary& root) {
    std::vector<std::vector<std::string>> announce_list;

    auto it = root.find("announce-list");
    if (it == root.end() || !it->second.is_list()) {
        return announce_list;
    }

    const auto& outer_list = it->second.as_list();
    for (const auto& inner_value : outer_list) {
        if (!inner_value.is_list()) {
            continue;
        }

        std::vector<std::string> tier;
        const auto& inner_list = inner_value.as_list();
        for (const auto& url_value : inner_list) {
            if (url_value.is_string()) {
                tier.push_back(url_value.as_string());
            }
        }

        if (!tier.empty()) {
            announce_list.push_back(std::move(tier));
        }
    }

    return announce_list;
}

}  // anonymous namespace

std::expected<TorrentInfo, TorrentError> TorrentInfo::from_bencode(const bencode::Value& value) {
    if (!value.is_dictionary()) {
        return std::unexpected(TorrentError::InvalidFormat);
    }

    const auto& root = value.as_dictionary();
    TorrentInfo info;

    auto announce = get_string_field(root, "announce");
    if (!announce) {
        return std::unexpected(announce.error());
    }
    info.announce_ = *announce;
    info.announce_list_ = parse_announce_list(root);

    if (auto it = root.find("comment"); it != root.end() && it->second.is_string()) {
        info.comment_ = it->second.as_string();
    }

    if (auto it = root.find("created by"); it != root.end() && it->second.is_string()) {
        info.created_by_ = it->second.as_string();
    }

    if (auto it = root.find("creation date"); it != root.end() && it->second.is_integer()) {
        auto timestamp = it->second.as_integer();
        info.creation_date_ = std::chrono::system_clock::from_time_t(timestamp);
    }

    auto info_it = root.find("info");
    if (info_it == root.end() || !info_it->second.is_dictionary()) {
        return std::unexpected(TorrentError::MissingRequiredField);
    }

    const auto& info_dict = info_it->second.as_dictionary();
    auto name = get_string_field(info_dict, "name");
    if (!name) {
        return std::unexpected(name.error());
    }
    info.name_ = *name;

    auto piece_length = get_integer_field(info_dict, "piece length");
    if (!piece_length || *piece_length <= 0) {
        return std::unexpected(TorrentError::InvalidPieceLength);
    }
    info.piece_length_ = *piece_length;

    auto pieces = get_string_field(info_dict, "pieces");
    if (!pieces) {
        return std::unexpected(pieces.error());
    }

    auto piece_hashes = parse_piece_hashes(*pieces);
    if (!piece_hashes) {
        return std::unexpected(piece_hashes.error());
    }
    info.piece_hashes_ = std::move(*piece_hashes);

    auto files = parse_files(info_dict, info.name_);
    if (!files) {
        return std::unexpected(files.error());
    }
    info.files_ = std::move(*files);

    info.total_size_ = 0;
    for (const auto& file : info.files_) {
        info.total_size_ += file.length;
    }

    info.info_dict_bencoded_ = bencode::Encoder::encode(info_it->second);

    // TODO: Calculate actual info_hash using SHA-1
    info.info_hash_ = {};

    return info;
}

std::expected<TorrentInfo, TorrentError> TorrentInfo::from_file(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        return std::unexpected(TorrentError::InvalidFormat);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();

    auto parse_result = bencode::Parser::parse(content);
    if (!parse_result) {
        return std::unexpected(TorrentError::InvalidFormat);
    }

    return from_bencode(*parse_result);
}

std::int64_t TorrentInfo::piece_size(std::size_t piece_index) const noexcept {
    if (piece_index >= piece_hashes_.size()) {
        return 0;
    }

    if (piece_index == piece_hashes_.size() - 1) {
        auto remainder = total_size_ % piece_length_;
        return remainder == 0 ? piece_length_ : remainder;
    }

    return piece_length_;
}

bool TorrentInfo::verify_piece_hash(std::size_t piece_index, const SHA1Hash& hash) const noexcept {
    if (piece_index >= piece_hashes_.size()) {
        return false;
    }
    return piece_hashes_[piece_index] == hash;
}

}  // namespace bittorrent::core
