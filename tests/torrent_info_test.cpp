#include <gtest/gtest.h>
#include "bittorrent/bencode.hpp"
#include "bittorrent/core.hpp"

using namespace bittorrent::core;
using namespace bittorrent::bencode;

TEST(TorrentInfo, ParseSimpleTorrent) {
    Dictionary root;
    root["announce"] = Value{String{"http://tracker.example.com:8080/announce"}};

    Dictionary info;
    info["name"] = Value{String{"test.txt"}};
    info["length"] = Value{Integer{1024}};
    info["piece length"] = Value{Integer{256}};

    std::string pieces;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 20; ++j) {
            pieces += static_cast<char>(i);
        }
    }
    info["pieces"] = Value{String{pieces}};

    root["info"] = Value{std::move(info)};

    Value torrent_value{std::move(root)};

    auto result = TorrentInfo::from_bencode(torrent_value);
    ASSERT_TRUE(result.has_value());

    const auto& torrent = *result;
    EXPECT_EQ(torrent.name(), "test.txt");
    EXPECT_EQ(torrent.total_size(), 1024);
    EXPECT_EQ(torrent.piece_length(), 256);
    EXPECT_EQ(torrent.piece_count(), 4);
    EXPECT_EQ(torrent.announce(), "http://tracker.example.com:8080/announce");
    EXPECT_TRUE(torrent.is_single_file());
    EXPECT_EQ(torrent.files().size(), 1);
}

TEST(TorrentInfo, ParseMultiFileTorrent) {
    Dictionary root;
    root["announce"] = Value{String{"http://tracker.example.com:8080/announce"}};

    Dictionary info;
    info["name"] = Value{String{"test_dir"}};
    info["piece length"] = Value{Integer{256}};

    List files_list;

    Dictionary file1;
    file1["length"] = Value{Integer{512}};
    List path1;
    path1.push_back(Value{String{"file1.txt"}});
    file1["path"] = Value{std::move(path1)};
    files_list.push_back(Value{std::move(file1)});

    Dictionary file2;
    file2["length"] = Value{Integer{1024}};
    List path2;
    path2.push_back(Value{String{"subdir"}});
    path2.push_back(Value{String{"file2.txt"}});
    file2["path"] = Value{std::move(path2)};
    files_list.push_back(Value{std::move(file2)});

    info["files"] = Value{std::move(files_list)};

    std::string pieces;
    for (int i = 0; i < 6; ++i) {
        for (int j = 0; j < 20; ++j) {
            pieces += static_cast<char>(i);
        }
    }
    info["pieces"] = Value{String{pieces}};

    root["info"] = Value{std::move(info)};
    Value torrent_value{std::move(root)};

    auto result = TorrentInfo::from_bencode(torrent_value);
    ASSERT_TRUE(result.has_value());

    const auto& torrent = *result;
    EXPECT_EQ(torrent.name(), "test_dir");
    EXPECT_EQ(torrent.total_size(), 1536);
    EXPECT_EQ(torrent.piece_length(), 256);
    EXPECT_EQ(torrent.piece_count(), 6);
    EXPECT_FALSE(torrent.is_single_file());
    EXPECT_EQ(torrent.files().size(), 2);

    EXPECT_EQ(torrent.files()[0].length, 512);
    EXPECT_EQ(torrent.files()[0].offset, 0);

    EXPECT_EQ(torrent.files()[1].length, 1024);
    EXPECT_EQ(torrent.files()[1].offset, 512);
}

TEST(TorrentInfo, ParseWithOptionalFields) {
    Dictionary root;
    root["announce"] = Value{String{"http://tracker.example.com:8080/announce"}};
    root["comment"] = Value{String{"Test torrent"}};
    root["created by"] = Value{String{"test/1.0"}};
    root["creation date"] = Value{Integer{1234567890}};

    List announce_list;
    List tier1;
    tier1.push_back(Value{String{"http://tracker1.example.com/announce"}});
    tier1.push_back(Value{String{"http://tracker2.example.com/announce"}});
    announce_list.push_back(Value{std::move(tier1)});
    root["announce-list"] = Value{std::move(announce_list)};

    Dictionary info;
    info["name"] = Value{String{"test.txt"}};
    info["length"] = Value{Integer{1024}};
    info["piece length"] = Value{Integer{512}};

    std::string pieces;
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 20; ++j) {
            pieces += static_cast<char>(i);
        }
    }
    info["pieces"] = Value{String{pieces}};

    root["info"] = Value{std::move(info)};
    Value torrent_value{std::move(root)};

    auto result = TorrentInfo::from_bencode(torrent_value);
    ASSERT_TRUE(result.has_value());

    const auto& torrent = *result;
    EXPECT_TRUE(torrent.comment().has_value());
    EXPECT_EQ(*torrent.comment(), "Test torrent");

    EXPECT_TRUE(torrent.created_by().has_value());
    EXPECT_EQ(*torrent.created_by(), "test/1.0");

    EXPECT_TRUE(torrent.creation_date().has_value());

    EXPECT_EQ(torrent.announce_list().size(), 1);
    EXPECT_EQ(torrent.announce_list()[0].size(), 2);
}

TEST(TorrentInfo, PieceSizeCalculation) {
    Dictionary root;
    root["announce"] = Value{String{"http://tracker.example.com:8080/announce"}};

    Dictionary info;
    info["name"] = Value{String{"test.txt"}};
    info["length"] = Value{Integer{1000}};
    info["piece length"] = Value{Integer{256}};

    std::string pieces;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 20; ++j) {
            pieces += static_cast<char>(i);
        }
    }
    info["pieces"] = Value{String{pieces}};

    root["info"] = Value{std::move(info)};
    Value torrent_value{std::move(root)};

    auto result = TorrentInfo::from_bencode(torrent_value);
    ASSERT_TRUE(result.has_value());

    const auto& torrent = *result;
    EXPECT_EQ(torrent.piece_size(0), 256);
    EXPECT_EQ(torrent.piece_size(1), 256);
    EXPECT_EQ(torrent.piece_size(2), 256);
    EXPECT_EQ(torrent.piece_size(3), 232);
    EXPECT_EQ(torrent.piece_size(4), 0);
}

TEST(TorrentInfo, RejectInvalidTorrent) {
    {
        Dictionary root;
        Dictionary info;
        info["name"] = Value{String{"test.txt"}};
        info["length"] = Value{Integer{1024}};
        info["piece length"] = Value{Integer{256}};
        info["pieces"] = Value{String(80, '\0')};
        root["info"] = Value{std::move(info)};

        Value torrent_value{std::move(root)};
        auto result = TorrentInfo::from_bencode(torrent_value);
        EXPECT_FALSE(result.has_value());
        EXPECT_EQ(result.error(), TorrentError::MissingRequiredField);
    }

    {
        Dictionary root;
        root["announce"] = Value{String{"http://tracker.example.com/announce"}};

        Value torrent_value{std::move(root)};
        auto result = TorrentInfo::from_bencode(torrent_value);
        EXPECT_FALSE(result.has_value());
        EXPECT_EQ(result.error(), TorrentError::MissingRequiredField);
    }

    {
        Dictionary root;
        root["announce"] = Value{String{"http://tracker.example.com/announce"}};

        Dictionary info;
        info["name"] = Value{String{"test.txt"}};
        info["length"] = Value{Integer{1024}};
        info["piece length"] = Value{Integer{0}};
        info["pieces"] = Value{String(20, '\0')};
        root["info"] = Value{std::move(info)};

        Value torrent_value{std::move(root)};
        auto result = TorrentInfo::from_bencode(torrent_value);
        EXPECT_FALSE(result.has_value());
        EXPECT_EQ(result.error(), TorrentError::InvalidPieceLength);
    }

    {
        Dictionary root;
        root["announce"] = Value{String{"http://tracker.example.com/announce"}};

        Dictionary info;
        info["name"] = Value{String{"test.txt"}};
        info["length"] = Value{Integer{1024}};
        info["piece length"] = Value{Integer{256}};
        info["pieces"] = Value{String(19, '\0')};
        root["info"] = Value{std::move(info)};

        Value torrent_value{std::move(root)};
        auto result = TorrentInfo::from_bencode(torrent_value);
        EXPECT_FALSE(result.has_value());
        EXPECT_EQ(result.error(), TorrentError::InvalidPieceHash);
    }
}

TEST(Types, SHA1HexConversion) {
    SHA1Hash hash;
    for (int i = 0; i < 20; ++i) {
        hash[i] = static_cast<std::byte>(i);
    }

    auto hex = to_hex_string(hash);
    EXPECT_EQ(hex.size(), 40);
    EXPECT_EQ(hex, "000102030405060708090a0b0c0d0e0f10111213");

    auto parsed = from_hex_string(hex);
    ASSERT_TRUE(parsed.has_value());
    EXPECT_EQ(*parsed, hash);
}

TEST(Types, InvalidHexString) {
    auto result = from_hex_string("invalid");
    EXPECT_FALSE(result.has_value());

    auto result2 = from_hex_string("zz0102030405060708090a0b0c0d0e0f10111213");
    EXPECT_FALSE(result2.has_value());
}
