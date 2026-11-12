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
    ${CMAKE_SOURCE_DIR}/include
)

if(NOT MSVC)
    target_compile_options(loomchannel PUBLIC -Wall -Wextra -Wpedantic)
endif()

# ---------------------------------------------------------------
# Console example
# ---------------------------------------------------------------
add_executable(loomchannel_example example/main.cpp)
target_link_libraries(loomchannel_example PRIVATE loomchannel)

if(NOT MSVC)
    target_compile_options(loomchannel_example PRIVATE -Wall -Wextra -Wpedantic)
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
