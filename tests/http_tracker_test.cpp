#include <gtest/gtest.h>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include "bittorrent/core.hpp"
#include "bittorrent/network.hpp"

using namespace bittorrent;

TEST(HttpTrackerTest, ParseCompactPeers) {
    std::string response = "d8:intervali1800e5:peers6:";

    // Add 1 peer: IP 192.168.1.1, port 6881 (0x1AE1)
    response += std::string("\xC0\xA8\x01\x01\x1A\xE1", 6);
    response += "e";

    auto result = network::HttpTracker::parse_response(response);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->interval.count(), 1800);
    EXPECT_EQ(result->peers.size(), 1);
    EXPECT_EQ(result->peers[0].ip[0], 192);
    EXPECT_EQ(result->peers[0].ip[1], 168);
    EXPECT_EQ(result->peers[0].ip[2], 1);
    EXPECT_EQ(result->peers[0].ip[3], 1);
    EXPECT_EQ(result->peers[0].port, 6881);
    EXPECT_EQ(result->peers[0].ip_string(), "192.168.1.1");
}

// Test parsing a response with seeders/leechers
TEST(HttpTrackerTest, ParseSeedersLeechers) {
    std::string response = "d8:intervali900e8:completei50e10:incompletei25e5:peers0:e";

    auto result = network::HttpTracker::parse_response(response);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->interval.count(), 900);
    EXPECT_EQ(result->complete, 50);
    EXPECT_EQ(result->incomplete, 25);
    EXPECT_EQ(result->peers.size(), 0);
}

// Test parsing a tracker failure response
TEST(HttpTrackerTest, ParseFailureReason) {
    std::string response = "d14:failure reason12:Invalid infoe";

    auto result = network::HttpTracker::parse_response(response);

    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), network::TrackerError::TrackerFailure);
}

// Test parsing invalid bencode
TEST(HttpTrackerTest, ParseInvalidBencode) {
    std::string response = "not bencode at all";

    auto result = network::HttpTracker::parse_response(response);

    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), network::TrackerError::ParseError);
}

// Test parsing response without interval
TEST(HttpTrackerTest, ParseMissingInterval) {
    std::string response = "d5:peers0:e";

    auto result = network::HttpTracker::parse_response(response);

    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), network::TrackerError::InvalidResponse);
}
