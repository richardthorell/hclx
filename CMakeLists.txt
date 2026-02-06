cmake_minimum_required(VERSION 3.20)
project(HCLX LANGUAGES CXX)

option(HCLX_BUILD_TESTS "Build HCLX unit tests" ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Pick up all .cpp in src/
file(GLOB CONFIGURE_DEPENDS HCLX_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_library(hclx STATIC ${HCLX_SOURCES})

target_include_directories(hclx
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/inc
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_features(hclx PUBLIC cxx_std_20)

if (MSVC)
    target_compile_options(hclx PRIVATE /W4 /permissive-)
else()
    target_compile_options(hclx PRIVATE -Wall -Wextra -Wpedantic)
endif()

if (HCLX_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()