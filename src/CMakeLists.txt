# Bencode library
add_library(bencode
    bencode/parser.cpp
    bencode/encoder.cpp
)

target_include_directories(bencode PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)
target_compile_features(bencode PUBLIC cxx_std_23)

