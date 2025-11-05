#include "bittorrent/bencode/parser.hpp"
#include <cctype>
#include <charconv>
#include <iostream>

namespace bittorrent::bencode {

std::expected<Value, ParseError> Parser::parse(std::string_view data) {
    Parser parser(data);
    return parser.parse_value();
}

std::expected<Value, ParseError> Parser::parse_value() {
    if (!has_more()) {
        return std::unexpected(ParseError::UnexpectedEnd);
    }

    const char c = peek();

    if (c == 'i') {
        auto result = parse_integer();
        if (!result) {
            return std::unexpected(result.error());
        }
        return Value{*result};
    }

    if (std::isdigit(static_cast<unsigned char>(c))) {
        auto result = parse_string();
        if (!result) {
            return std::unexpected(result.error());
        }
        return Value{*result};
    }

    if (c == 'l') {
        auto result = parse_list();
        if (!result) {
            return std::unexpected(result.error());
        }
        return Value{*result};
    }

    if (c == 'd') {
        auto result = parse_dictionary();
        if (!result) {
            return std::unexpected(result.error());
        }
        return Value{*result};
    }

    return std::unexpected(ParseError::UnexpectedCharacter);
}

std::expected<Integer, ParseError> Parser::parse_integer() {
    if (consume() != 'i') {
        return std::unexpected(ParseError::InvalidInteger);
    }

    size_t start = pos_;
    while (has_more() && peek() != 'e') {
        consume();
    }

    if (!has_more()) {
        return std::unexpected(ParseError::UnexpectedEnd);
    }

    std::string_view number_str = data_.substr(start, pos_ - start);

    if (number_str.size() > 1 && number_str[0] == '0') {
        return std::unexpected(ParseError::InvalidInteger);
    }

    if (number_str == "-0") {
        return std::unexpected(ParseError::InvalidInteger);
    }

    Integer value;
    auto [ptr, ec] = std::from_chars(number_str.data(), number_str.data() + number_str.size(), value);

    if (ec != std::errc{} || ptr != number_str.data() + number_str.size()) {
        return std::unexpected(ParseError::InvalidInteger);
    }

    consume();
    return value;
}

std::expected<String, ParseError> Parser::parse_string() {
    size_t start = pos_;
    while (has_more() && std::isdigit(static_cast<unsigned char>(peek()))) {
        consume();
    }

    if (!has_more() || peek() != ':') {
        std::cerr << "Invalid string at pos " << pos_ << ": expected ':', got '" << peek() << "'\n";
        return std::unexpected(ParseError::InvalidString);
    }

    std::string_view length_str = data_.substr(start, pos_ - start);
    size_t length;
    auto [ptr, ec] = std::from_chars(length_str.data(), length_str.data() + length_str.size(), length);

    if (ec != std::errc{}) {
        std::cerr << "Invalid string length at pos " << start << "\n";
        return std::unexpected(ParseError::InvalidLength);
    }

    consume();

    if (pos_ + length > data_.size()) {
        std::cerr << "String too long: trying to read " << length << " bytes at pos " << pos_ 
                  << " but only " << (data_.size() - pos_) << " bytes remaining\n";
        return std::unexpected(ParseError::UnexpectedEnd);
    }

    std::string result(data_.substr(pos_, length));
    skip(length);

    return result;
}

std::expected<List, ParseError> Parser::parse_list() {
    if (consume() != 'l') {
        std::cerr << "Invalid format in list: " << peek() << std::endl;
        return std::unexpected(ParseError::InvalidFormat);
    }

    List list;

    while (has_more() && peek() != 'e') {
        auto value = parse_value();
        if (!value) {
            return std::unexpected(value.error());
        }
        list.push_back(std::move(*value));
    }

    if (!has_more()) {
        return std::unexpected(ParseError::UnexpectedEnd);
    }

    consume();
    return list;
}

std::expected<Dictionary, ParseError> Parser::parse_dictionary() {
    if (consume() != 'd') {
        std::cerr << "Invalid format in dictionary: " << peek() << std::endl;
        return std::unexpected(ParseError::InvalidFormat);
    }

    Dictionary dict;
    std::string last_key;

    while (has_more() && peek() != 'e') {
        if (!std::isdigit(peek())) {
            std::cerr << "Invalid format in dictionary key: " << pos_ << " " << peek() << std::endl;
            return std::unexpected(ParseError::InvalidFormat);
        }

        auto key = parse_string();
        if (!key) {
            return std::unexpected(key.error());
        }

        if (!last_key.empty() && *key <= last_key) {
            std::cerr << "Invalid format in dictionary key 2: " << pos_ << " " << peek() << std::endl;
            return std::unexpected(ParseError::InvalidFormat);
        }
        last_key = *key;

        auto value = parse_value();
        if (!value) {
            return std::unexpected(value.error());
        }

        dict.emplace(std::move(*key), std::move(*value));
    }

    if (!has_more()) {
        return std::unexpected(ParseError::UnexpectedEnd);
    }

    consume();
    return dict;
}

}  // namespace bittorrent::bencode
