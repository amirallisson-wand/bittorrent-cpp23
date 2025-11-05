add_executable(bencode_test
    bencode_test.cpp
)

target_link_libraries(bencode_test PRIVATE
    bencode
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(bencode_test)

add_executable(torrent_info_test
    torrent_info_test.cpp
)

target_link_libraries(torrent_info_test PRIVATE
    core
    bencode
    GTest::gtest_main
)

gtest_discover_tests(torrent_info_test)

add_executable(http_tracker_test
    http_tracker_test.cpp
)

target_link_libraries(http_tracker_test PRIVATE
    network
    core
    bencode
    GTest::gtest_main
)

gtest_discover_tests(http_tracker_test)

add_executable(crypto_test
    crypto_test.cpp
)

target_link_libraries(crypto_test PRIVATE
    utils
    core
    bencode
    GTest::gtest_main
)

gtest_discover_tests(crypto_test)
