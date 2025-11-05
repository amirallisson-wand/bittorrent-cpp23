#pragma once

#include "file_info.hpp"
#include "torrent_info.hpp" 
#include "types.hpp"
#include <chrono>
#include <ctime>
#include <optional>
#include <vector>
#include <spdlog/fmt/bundled/format.h>

template <typename T>
struct fmt::formatter<std::optional<T>> {
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const std::optional<T>& opt, FormatContext& ctx) const -> decltype(ctx.out()) {
        if (opt.has_value()) {
            return fmt::format_to(ctx.out(), "{}", *opt);
        }
        return ctx.out();
    }
};

template <typename T>
struct fmt::formatter<std::vector<T>> {
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const std::vector<T>& vec, FormatContext& ctx) const -> decltype(ctx.out()) {
        auto out = ctx.out();
        *out++ = '[';
        for (size_t i = 0; i < vec.size(); ++i) {
            if (i > 0) {
                *out++ = ',';
                *out++ = ' ';
            }
            out = fmt::format_to(out, "{}", vec[i]);
        }
        *out++ = ']';
        return out;
    }
};

template <>
struct fmt::formatter<std::chrono::system_clock::time_point> {
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const std::chrono::system_clock::time_point& tp, FormatContext& ctx) const -> decltype(ctx.out()) {
        auto time = std::chrono::system_clock::to_time_t(tp);
        std::tm tm = *std::localtime(&time);
        return fmt::format_to(ctx.out(), "{:04d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}",
                              tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
                              tm.tm_hour, tm.tm_min, tm.tm_sec);
    }
};

template <>
struct fmt::formatter<bittorrent::core::SHA1Hash> {
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const bittorrent::core::SHA1Hash& hash, FormatContext& ctx) const -> decltype(ctx.out()) {
        return fmt::format_to(ctx.out(), "{}", bittorrent::core::to_hex_string(hash));
    }
};

template <>
struct fmt::formatter<bittorrent::core::FileInfo> {
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const bittorrent::core::FileInfo& file, FormatContext& ctx) const -> decltype(ctx.out()) {
        double mb = file.length / (1024.0 * 1024.0);
        if (mb >= 1.0) {
            return fmt::format_to(ctx.out(), "{} ({:.2f} MB)", file.path.string(), mb);
        }
        return fmt::format_to(ctx.out(), "{} ({} bytes)", file.path.string(), file.length);
    }
};

template <>
struct fmt::formatter<bittorrent::core::TorrentInfo> {
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const bittorrent::core::TorrentInfo& t, FormatContext& ctx) const -> decltype(ctx.out()) {
        auto out = ctx.out();
        
        out = fmt::format_to(out, "TorrentInfo{{\n");
        out = fmt::format_to(out, "  name: '{}'\n", t.name());
        out = fmt::format_to(out, "  announce: {}\n", t.announce());
        out = fmt::format_to(out, "  announce_list: {}\n", t.announce_list());
        out = fmt::format_to(out, "  comment: {}\n", t.comment());
        out = fmt::format_to(out, "  created_by: {}\n", t.created_by());
        out = fmt::format_to(out, "  creation_date: {}\n", t.creation_date());
        out = fmt::format_to(out, "  info_hash: {}\n", t.info_hash());
        out = fmt::format_to(out, "  total_size: {} bytes", t.total_size());
        double mb = t.total_size() / (1024.0 * 1024.0);
        out = fmt::format_to(out, " ({:.2f} MB)\n", mb);
        out = fmt::format_to(out, "  piece_length: {} bytes ({} KB)\n", 
                           t.piece_length(), t.piece_length() / 1024);
        out = fmt::format_to(out, "  piece_count: {}\n", t.piece_count());
        out = fmt::format_to(out, "  is_single_file: {}\n", t.is_single_file());
        out = fmt::format_to(out, "  files: {}\n", t.files());
        out = fmt::format_to(out, "}}");
        
        return out;
    }
};
