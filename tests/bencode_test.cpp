#include "bittorrent/bencode.hpp"
#include <gtest/gtest.h>

using namespace bittorrent::bencode;

TEST(BencodeParser, ParsePositiveInteger) {
    auto result = Parser::parse("i42e");
    ASSERT_TRUE(result.has_value());
    ASSERT_TRUE(result->is_integer());
    EXPECT_EQ(result->as_integer(), 42);
}

TEST(BencodeParser, ParseNegativeInteger) {
    auto result = Parser::parse("i-42e");
    ASSERT_TRUE(result.has_value());
    ASSERT_TRUE(result->is_integer());
    EXPECT_EQ(result->as_integer(), -42);
}

TEST(BencodeParser, ParseZero) {
    auto result = Parser::parse("i0e");
    ASSERT_TRUE(result.has_value());
    ASSERT_TRUE(result->is_integer());
    EXPECT_EQ(result->as_integer(), 0);
}

TEST(BencodeParser, ParseLargeInteger) {
    auto result = Parser::parse("i9223372036854775807e");
    ASSERT_TRUE(result.has_value());
    ASSERT_TRUE(result->is_integer());
    EXPECT_EQ(result->as_integer(), 9223372036854775807LL);
}

TEST(BencodeParser, RejectLeadingZero) {
    auto result = Parser::parse("i042e");
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), ParseError::InvalidInteger);
}

TEST(BencodeParser, RejectNegativeZero) {
    auto result = Parser::parse("i-0e");
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), ParseError::InvalidInteger);
}

TEST(BencodeParser, RejectIncompleteInteger) {
    auto result = Parser::parse("i42");
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), ParseError::UnexpectedEnd);
}

TEST(BencodeParser, ParseString) {
    auto result = Parser::parse("4:spam");
    ASSERT_TRUE(result.has_value());
    ASSERT_TRUE(result->is_string());
    EXPECT_EQ(result->as_string(), "spam");
}

TEST(BencodeParser, ParseEmptyString) {
    auto result = Parser::parse("0:");
    ASSERT_TRUE(result.has_value());
    ASSERT_TRUE(result->is_string());
    EXPECT_EQ(result->as_string(), "");
}

TEST(BencodeParser, ParseStringWithSpaces) {
    auto result = Parser::parse("11:hello world");
    ASSERT_TRUE(result.has_value());
    ASSERT_TRUE(result->is_string());
    EXPECT_EQ(result->as_string(), "hello world");
}

TEST(BencodeParser, ParseBinaryString) {
    std::string input;
    input += "4:\x01\x02\x03\x04";
    auto result = Parser::parse(std::string_view{input});
    ASSERT_TRUE(result.has_value());
    ASSERT_TRUE(result->is_string());
    EXPECT_EQ(result->as_string(), "\x01\x02\x03\x04");
}

TEST(BencodeParser, RejectStringTooShort) {
    auto result = Parser::parse("10:spam");
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), ParseError::UnexpectedEnd);
}

TEST(BencodeParser, RejectInvalidStringLength) {
    auto result = Parser::parse("abc:spam");
    EXPECT_FALSE(result.has_value());
}

TEST(BencodeParser, ParseEmptyList) {
    auto result = Parser::parse("le");
    ASSERT_TRUE(result.has_value());
    ASSERT_TRUE(result->is_list());
    EXPECT_EQ(result->as_list().size(), 0);
}

TEST(BencodeParser, ParseListOfIntegers) {
    auto result = Parser::parse("li1ei2ei3ee");
    ASSERT_TRUE(result.has_value());
    ASSERT_TRUE(result->is_list());
    const auto& list = result->as_list();
    ASSERT_EQ(list.size(), 3);
    EXPECT_EQ(list[0].as_integer(), 1);
    EXPECT_EQ(list[1].as_integer(), 2);
    EXPECT_EQ(list[2].as_integer(), 3);
}

TEST(BencodeParser, ParseListOfStrings) {
    auto result = Parser::parse("l4:spam4:eggse");
    ASSERT_TRUE(result.has_value());
    ASSERT_TRUE(result->is_list());
    const auto& list = result->as_list();
    ASSERT_EQ(list.size(), 2);
    EXPECT_EQ(list[0].as_string(), "spam");
    EXPECT_EQ(list[1].as_string(), "eggs");
}

TEST(BencodeParser, ParseMixedList) {
    auto result = Parser::parse("li42e4:spam3:fooe");
    ASSERT_TRUE(result.has_value());
    ASSERT_TRUE(result->is_list());
    const auto& list = result->as_list();
    ASSERT_EQ(list.size(), 3);
    EXPECT_EQ(list[0].as_integer(), 42);
    EXPECT_EQ(list[1].as_string(), "spam");
    EXPECT_EQ(list[2].as_string(), "foo");
}

TEST(BencodeParser, ParseNestedList) {
    auto result = Parser::parse("lli1ei2eeli3ei4eee");
    ASSERT_TRUE(result.has_value());
    ASSERT_TRUE(result->is_list());
    const auto& list = result->as_list();
    ASSERT_EQ(list.size(), 2);
    ASSERT_TRUE(list[0].is_list());
    ASSERT_TRUE(list[1].is_list());
    EXPECT_EQ(list[0].as_list()[0].as_integer(), 1);
    EXPECT_EQ(list[1].as_list()[1].as_integer(), 4);
}

TEST(BencodeParser, RejectIncompleteList) {
    auto result = Parser::parse("li1ei2e");
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), ParseError::UnexpectedEnd);
}

TEST(BencodeParser, ParseEmptyDictionary) {
    auto result = Parser::parse("de");
    ASSERT_TRUE(result.has_value());
    ASSERT_TRUE(result->is_dictionary());
    EXPECT_EQ(result->as_dictionary().size(), 0);
}

TEST(BencodeParser, ParseSimpleDictionary) {
    auto result = Parser::parse("d3:cow3:moo4:spam4:eggse");
    ASSERT_TRUE(result.has_value());
    ASSERT_TRUE(result->is_dictionary());
    const auto& dict = result->as_dictionary();
    ASSERT_EQ(dict.size(), 2);
    EXPECT_EQ(dict.at("cow").as_string(), "moo");
    EXPECT_EQ(dict.at("spam").as_string(), "eggs");
}

TEST(BencodeParser, ParseDictionaryWithInteger) {
    auto result = Parser::parse("d3:agei25e4:name4:Johne");
    ASSERT_TRUE(result.has_value());
    ASSERT_TRUE(result->is_dictionary());
    const auto& dict = result->as_dictionary();
    EXPECT_EQ(dict.at("age").as_integer(), 25);
    EXPECT_EQ(dict.at("name").as_string(), "John");
}

TEST(BencodeParser, ParseNestedDictionary) {
    auto result = Parser::parse("d4:infod4:name4:Johne5:valuei42ee");
    ASSERT_TRUE(result.has_value());
    ASSERT_TRUE(result->is_dictionary());
    const auto& dict = result->as_dictionary();
    ASSERT_TRUE(dict.at("info").is_dictionary());
    EXPECT_EQ(dict.at("info").as_dictionary().at("name").as_string(), "John");
    EXPECT_EQ(dict.at("value").as_integer(), 42);
}

TEST(BencodeParser, ParseDictionaryWithList) {
    auto result = Parser::parse("d4:listli1ei2ei3eee");
    ASSERT_TRUE(result.has_value());
    ASSERT_TRUE(result->is_dictionary());
    const auto& dict = result->as_dictionary();
    ASSERT_TRUE(dict.at("list").is_list());
    EXPECT_EQ(dict.at("list").as_list().size(), 3);
}

TEST(BencodeParser, RejectUnsortedDictionary) {
    auto result = Parser::parse("d4:spam4:eggs3:cow3:mooe");
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), ParseError::InvalidFormat);
}

TEST(BencodeParser, RejectDuplicateKey) {
    auto result = Parser::parse("d3:cow3:moo3:cow3:baae");
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), ParseError::InvalidFormat);
}

TEST(BencodeParser, RejectNonStringKey) {
    auto result = Parser::parse("di42e5:valuee");
    EXPECT_FALSE(result.has_value());
}

TEST(BencodeParser, RejectIncompleteDictionary) {
    auto result = Parser::parse("d3:cow3:moo");
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), ParseError::UnexpectedEnd);
}

TEST(BencodeEncoder, EncodeInteger) {
    Value value{Integer{42}};
    EXPECT_EQ(Encoder::encode(value), "i42e");
}

TEST(BencodeEncoder, EncodeNegativeInteger) {
    Value value{Integer{-42}};
    EXPECT_EQ(Encoder::encode(value), "i-42e");
}

TEST(BencodeEncoder, EncodeString) {
    Value value{String{"spam"}};
    EXPECT_EQ(Encoder::encode(value), "4:spam");
}

TEST(BencodeEncoder, EncodeEmptyString) {
    Value value{String{""}};
    EXPECT_EQ(Encoder::encode(value), "0:");
}

TEST(BencodeEncoder, EncodeList) {
    List list;
    list.push_back(Value{Integer{1}});
    list.push_back(Value{String{"spam"}});
    Value value{std::move(list)};
    EXPECT_EQ(Encoder::encode(value), "li1e4:spame");
}

TEST(BencodeEncoder, EncodeEmptyList) {
    Value value{List{}};
    EXPECT_EQ(Encoder::encode(value), "le");
}

TEST(BencodeEncoder, EncodeDictionary) {
    Dictionary dict;
    dict["spam"] = Value{String{"eggs"}};
    dict["number"] = Value{Integer{42}};
    Value value{std::move(dict)};
    // Note: map keeps keys sorted
    EXPECT_EQ(Encoder::encode(value), "d6:numberi42e4:spam4:eggse");
}

TEST(BencodeEncoder, EncodeEmptyDictionary) {
    Value value{Dictionary{}};
    EXPECT_EQ(Encoder::encode(value), "de");
}

TEST(BencodeRoundTrip, IntegerRoundTrip) {
    std::string original = "i12345e";
    auto parsed = Parser::parse(original);
    ASSERT_TRUE(parsed.has_value());
    auto encoded = Encoder::encode(*parsed);
    EXPECT_EQ(encoded, original);
}

TEST(BencodeRoundTrip, StringRoundTrip) {
    std::string original = "12:hello world!";
    auto parsed = Parser::parse(original);
    ASSERT_TRUE(parsed.has_value());
    auto encoded = Encoder::encode(*parsed);
    EXPECT_EQ(encoded, original);
}

TEST(BencodeRoundTrip, ListRoundTrip) {
    std::string original = "li1e4:test3:fooe";
    auto parsed = Parser::parse(original);
    ASSERT_TRUE(parsed.has_value());
    auto encoded = Encoder::encode(*parsed);
    EXPECT_EQ(encoded, original);
}

TEST(BencodeRoundTrip, DictionaryRoundTrip) {
    std::string original = "d3:agei30e4:name4:Johne";
    auto parsed = Parser::parse(original);
    ASSERT_TRUE(parsed.has_value());
    auto encoded = Encoder::encode(*parsed);
    EXPECT_EQ(encoded, original);
}

TEST(BencodeRoundTrip, ComplexRoundTrip) {
    std::string original = "d4:listli1ei2ee6:nestedd3:key5:valuee6:numberi42e6:string4:teste";
    auto parsed = Parser::parse(original);
    ASSERT_TRUE(parsed.has_value());
    auto encoded = Encoder::encode(*parsed);
    EXPECT_EQ(encoded, original);
}

TEST(BencodeParser, RejectEmptyInput) {
    auto result = Parser::parse("");
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), ParseError::UnexpectedEnd);
}

TEST(BencodeParser, RejectInvalidInput) {
    auto result = Parser::parse("invalid");
    EXPECT_FALSE(result.has_value());
}

TEST(BencodeParser, HandleUTF8String) {
    auto result = Parser::parse("12:Hello 世界");
    ASSERT_TRUE(result.has_value());
    ASSERT_TRUE(result->is_string());
    EXPECT_EQ(result->as_string(), "Hello 世界");
}
