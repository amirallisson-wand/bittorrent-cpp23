#pragma once

#include <string>
#include "value.hpp"

namespace bittorrent::bencode {

class Encoder {
public:
    [[nodiscard]] static std::string encode(const Value& value);

private:
    explicit Encoder() = default;

    static void encode_value(std::string& output, const Value& value);
    static void encode_integer(std::string& output, Integer value);
    static void encode_string(std::string& output, const String& value);
    static void encode_list(std::string& output, const List& value);
    static void encode_dictionary(std::string& output, const Dictionary& value);
};

}  // namespace bittorrent::bencode
