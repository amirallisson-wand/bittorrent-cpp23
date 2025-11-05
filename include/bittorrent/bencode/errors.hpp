#pragma once

#include <string_view>

namespace bittorrent::bencode {

enum class ParseError {
    UnexpectedEnd,
    InvalidInteger,
    InvalidString,
    InvalidLength,
    InvalidFormat,
    UnexpectedCharacter,
};

constexpr std::string_view to_string(ParseError error) noexcept {
    switch (error) {
        case ParseError::UnexpectedEnd:
            return "Unexpected end of input";
        case ParseError::InvalidInteger:
            return "Invalid integer format";
        case ParseError::InvalidString:
            return "Invalid string format";
        case ParseError::InvalidLength:
            return "Invalid length value";
        case ParseError::InvalidFormat:
            return "Invalid bencode format";
        case ParseError::UnexpectedCharacter:
            return "Unexpected character";
    }
    return "Unknown error";
}

}  // namespace bittorrent::bencode
