#include "bittorrent/bencode/encoder.hpp"
#include <string>

namespace bittorrent::bencode {

std::string Encoder::encode(const Value& value) {
    std::string result;
    encode_value(result, value);
    return result;
}

void Encoder::encode_value(std::string& output, const Value& value) {
    if (value.is_integer()) {
        encode_integer(output, value.as_integer());
    } else if (value.is_string()) {
        encode_string(output, value.as_string());
    } else if (value.is_list()) {
        encode_list(output, value.as_list());
    } else if (value.is_dictionary()) {
        encode_dictionary(output, value.as_dictionary());
    }
}

void Encoder::encode_integer(std::string& output, Integer value) {
    output += 'i';
    output += std::to_string(value);
    output += 'e';
}

void Encoder::encode_string(std::string& output, const String& value) {
    output += std::to_string(value.size());
    output += ':';
    output += value;
}

void Encoder::encode_list(std::string& output, const List& value) {
    output += 'l';
    for (const auto& item : value) {
        encode_value(output, item);
    }
    output += 'e';
}

void Encoder::encode_dictionary(std::string& output, const Dictionary& value) {
    output += 'd';
    for (const auto& [key, val] : value) {
        encode_string(output, key);
        encode_value(output, val);
    }
    output += 'e';
}

}  // namespace bittorrent::bencode
