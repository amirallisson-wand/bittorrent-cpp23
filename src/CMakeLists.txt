# Bencode library
add_library(bencode
    bencode/parser.cpp
    bencode/encoder.cpp
)

target_include_directories(bencode PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_compile_features(bencode PUBLIC cxx_std_23)

# Core library
add_library(core
    core/types.cpp
    core/torrent_info.cpp
)

target_link_libraries(core PUBLIC 
    bencode
    spdlog::spdlog
)
target_include_directories(core PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_compile_features(core PUBLIC cxx_std_23)

# Main BitTorrent client executable
add_executable(bittorrent_client
    main.cpp
)

target_link_libraries(bittorrent_client PRIVATE
    core
    bencode
    spdlog::spdlog
)

target_compile_features(bittorrent_client PRIVATE cxx_std_23)

