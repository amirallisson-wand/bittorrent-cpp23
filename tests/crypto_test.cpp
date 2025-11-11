#include "bittorrent/utils/crypto.hpp"
#include <gtest/gtest.h>
#include "bittorrent/core/types.hpp"

using namespace bittorrent;

TEST(CryptoTest, SHA1EmptyString) {
    auto hash = utils::sha1("");
    auto hex = core::to_hex_string(hash);
    EXPECT_EQ(hex, "da39a3ee5e6b4b0d3255bfef95601890afd80709");
}

TEST(CryptoTest, SHA1SimpleString) {
    auto hash = utils::sha1("The quick brown fox jumps over the lazy dog");
    auto hex = core::to_hex_string(hash);
    EXPECT_EQ(hex, "2fd4e1c67a2d28fced849ee1bb76e7391b93eb12");
}

TEST(CryptoTest, SHA1BinaryData) {
    std::string data = std::string("d8:announce41:http://bttracker.debian.org:6969/announcee", 56);
    auto hash = utils::sha1(data);
    EXPECT_EQ(hash.size(), 20);
    auto hash2 = utils::sha1(data);
    EXPECT_EQ(hash, hash2);
}

TEST(CryptoTest, SHA1WithNullBytes) {
    std::string data;
    data += "test";
    data += '\0';
    data += "data";
    auto hash = utils::sha1(data);
    auto hex = core::to_hex_string(hash);
    EXPECT_EQ(hex.length(), 40);
}
