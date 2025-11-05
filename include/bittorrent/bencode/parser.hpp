#pragma once

#include "errors.hpp"
#include "value.hpp"

#include <expected>
#include <string>
#include <string_view>

namespace bittorrent::bencode {

class Parser {
public:
    [[nodiscard]] static std::expected<Value, ParseError> parse(std::string_view data);

private:
    explicit Parser(std::string_view data) : data_(data), pos_(0) {}

    std::expected<Value, ParseError> parse_value();

    std::expected<Integer, ParseError> parse_integer();
    std::expected<String, ParseError> parse_string();
    std::expected<List, ParseError> parse_list();
    std::expected<Dictionary, ParseError> parse_dictionary();

    bool has_more() const noexcept { return pos_ < data_.size(); }

    char peek() const noexcept { return has_more() ? data_[pos_] : '\0'; }

    char consume() noexcept { return has_more() ? data_[pos_++] : '\0'; }

    void skip(size_t count) noexcept { pos_ = std::min(pos_ + count, data_.size()); }

    std::string_view remaining() const noexcept { return data_.substr(pos_); }

private:
    std::string_view data_;
    size_t pos_;
};

}  // namespace bittorrent::bencode
