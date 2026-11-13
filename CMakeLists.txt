cmake_minimum_required(VERSION 3.16)
project(loomchannel LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---------------------------------------------------------------
# Static library (pure, no GUI dependency)
# ---------------------------------------------------------------
add_library(loomchannel STATIC
    src/bus.cpp
    src/channel.cpp
    src/ordered_channel.cpp
)

target_include_directories(loomchannel PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(NOT MSVC)
    target_compile_options(loomchannel PUBLIC -Wall -Wextra -Wpedantic)
endif()

# ---------------------------------------------------------------
# Console examples
# ---------------------------------------------------------------
add_executable(loomchannel_example example/main.cpp)
target_link_libraries(loomchannel_example PRIVATE loomchannel)

add_executable(loomchannel_advanced example/advanced.cpp)
target_link_libraries(loomchannel_advanced PRIVATE loomchannel)

if(NOT MSVC)
    target_compile_options(loomchannel_example PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(loomchannel_advanced PRIVATE -Wall -Wextra -Wpedantic)
endif()

# ---------------------------------------------------------------
# Unit tests (Google Test)
# ---------------------------------------------------------------
option(BUILD_TESTS "Build unit tests" OFF)
if(BUILD_TESTS)
    find_package(GTest REQUIRED)
    include(GoogleTest)

    add_executable(loomchannel_tests tests/test_bus.cpp)
    target_link_libraries(loomchannel_tests PRIVATE loomchannel GTest::gtest_main)
    target_include_directories(loomchannel_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
    if(NOT MSVC)
        target_compile_options(loomchannel_tests PRIVATE -Wall -Wextra -Wpedantic)
    endif()

    gtest_discover_tests(loomchannel_tests)
endif()

# ---------------------------------------------------------------
# Installation
# ---------------------------------------------------------------
install(TARGETS loomchannel loomchannel_example
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(DIRECTORY include/ DESTINATION include/loomchannel)
install(FILES LICENSE DESTINATION share/loomchannel)
