# Bencode library
add_library(bencode
    bencode/parser.cpp
    bencode/encoder.cpp
)
target_include_directories(bencode PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_compile_features(bencode PUBLIC cxx_std_23)

# Utils library
add_library(utils
    utils/crypto.cpp
)
target_link_libraries(utils PUBLIC
    OpenSSL::Crypto
)
target_include_directories(utils PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_compile_features(utils PUBLIC cxx_std_23)

# Core library
add_library(core
    core/types.cpp
    core/torrent_info.cpp
)
target_link_libraries(core PUBLIC 
    bencode
    utils
    spdlog::spdlog
)
target_include_directories(core PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_compile_features(core PUBLIC cxx_std_23)

# Network library
find_package(Boost REQUIRED COMPONENTS system url)
add_library(network
    network/tracker/http_tracker.cpp
)
target_link_libraries(network PUBLIC
    core
    bencode
    spdlog::spdlog
    Boost::system
    Boost::url
)
target_include_directories(network PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_compile_features(network PUBLIC cxx_std_23)

# Executable
add_executable(bittorrent_client main.cpp)
target_link_libraries(bittorrent_client PRIVATE
    network
    core
    bencode
    spdlog::spdlog
)
target_compile_features(bittorrent_client PRIVATE cxx_std_23)
