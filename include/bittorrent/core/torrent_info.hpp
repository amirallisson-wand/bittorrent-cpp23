#pragma once

#include "bittorrent/bencode.hpp"
#include "errors.hpp"
#include "file_info.hpp"
#include "types.hpp"

#include <chrono>
#include <expected>
#include <optional>
#include <ostream>
#include <string>
#include <vector>

namespace bittorrent::core {

class TorrentInfo {
public:
    [[nodiscard]] static std::expected<TorrentInfo, TorrentError> from_bencode(const bencode::Value& value);

    [[nodiscard]] static std::expected<TorrentInfo, TorrentError> from_file(const std::filesystem::path& path);

    const std::string& name() const noexcept { return name_; }

    std::int64_t total_size() const noexcept { return total_size_; }

    std::int64_t piece_length() const noexcept { return piece_length_; }

    std::size_t piece_count() const noexcept { return piece_hashes_.size(); }

    const std::vector<SHA1Hash>& piece_hashes() const noexcept { return piece_hashes_; }

    const InfoHash& info_hash() const noexcept { return info_hash_; }

    const std::string& announce() const noexcept { return announce_; }

    const std::vector<std::vector<std::string>>& announce_list() const noexcept { return announce_list_; }

    bool is_single_file() const noexcept { return files_.size() == 1; }

    const std::vector<FileInfo>& files() const noexcept { return files_; }

    const std::optional<std::string>& comment() const noexcept { return comment_; }

    const std::optional<std::string>& created_by() const noexcept { return created_by_; }

    const std::optional<std::chrono::system_clock::time_point>& creation_date() const noexcept {
        return creation_date_;
    }

    std::int64_t piece_size(std::size_t piece_index) const noexcept;

    bool verify_piece_hash(std::size_t piece_index, const SHA1Hash& hash) const noexcept;

private:
    TorrentInfo() = default;

    std::string name_;
    std::int64_t total_size_{0};
    std::int64_t piece_length_{0};
    std::vector<SHA1Hash> piece_hashes_;
    InfoHash info_hash_;
    std::string announce_;
    std::vector<FileInfo> files_;

    std::vector<std::vector<std::string>> announce_list_;
    std::optional<std::string> comment_;
    std::optional<std::string> created_by_;
    std::optional<std::chrono::system_clock::time_point> creation_date_;

    std::string info_dict_bencoded_;
};

}  // namespace bittorrent::core
