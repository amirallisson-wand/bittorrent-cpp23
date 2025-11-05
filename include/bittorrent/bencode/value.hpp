#pragma once

#include <cstdint>
#include <expected>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace bittorrent::bencode {

struct Value;

using Integer = std::int64_t;
using String = std::string;
using List = std::vector<Value>;
using Dictionary = std::map<std::string, Value>;

class Value {
public:
    Value() : data_(Integer{0}) {}

    template <typename T>
    Value(T&& value) : data_(std::forward<T>(value)) {}

    bool is_integer() const noexcept { return std::holds_alternative<Integer>(data_); }

    bool is_string() const noexcept { return std::holds_alternative<String>(data_); }

    bool is_list() const noexcept { return std::holds_alternative<List>(data_); }

    bool is_dictionary() const noexcept { return std::holds_alternative<Dictionary>(data_); }

    Integer& as_integer() { return std::get<Integer>(data_); }

    const Integer& as_integer() const { return std::get<Integer>(data_); }

    String& as_string() { return std::get<String>(data_); }

    const String& as_string() const { return std::get<String>(data_); }

    List& as_list() { return std::get<List>(data_); }

    const List& as_list() const { return std::get<List>(data_); }

    Dictionary& as_dictionary() { return std::get<Dictionary>(data_); }

    const Dictionary& as_dictionary() const { return std::get<Dictionary>(data_); }

    template <typename T>
    std::expected<T*, std::string> get_if() noexcept {
        if (auto* ptr = std::get_if<T>(&data_)) {
            return ptr;
        }
        return std::unexpected("Type mismatch");
    }

    template <typename T>
    std::expected<const T*, std::string> get_if() const noexcept {
        if (auto* ptr = std::get_if<T>(&data_)) {
            return ptr;
        }
        return std::unexpected("Type mismatch");
    }

private:
    std::variant<Integer, String, List, Dictionary> data_;
};

}  // namespace bittorrent::bencode
