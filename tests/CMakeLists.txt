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
