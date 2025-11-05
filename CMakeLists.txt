cmake_minimum_required(VERSION 3.14)
project(awesome_concurrency VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

add_subdirectory(src)

# testing
option(BUILD_TESTS "Build tests" ON)
if(BUILD_TESTS)
    enable_testing()
    
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
    )
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
    
    add_subdirectory(tests)
    
    # Add custom target to run all tests
    add_custom_target(run_tests
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Running all tests..."
        DEPENDS bencode_test
    )
endif()

add_custom_target(format
    COMMAND find . -type f \( -name "*.cpp" -o -name "*.hpp" -o -name "*.cc" -o -name "*.hh" -o -name "*.h" -o -name "*.cxx" -o -name "*.hxx" \) -exec clang-format -i {} +
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Formatting all C++ source files..."
    VERBATIM
)
