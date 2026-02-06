include(FetchContent)

# GoogleTest
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)

# Prevent overriding the parent project's compiler/linker settings (Windows)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

add_executable(hclx_tests
    test_parse_empty.cpp
)

target_link_libraries(hclx_tests
    PRIVATE
        hclx
        GTest::gtest_main
)

target_compile_features(hclx_tests PRIVATE cxx_std_20)

include(GoogleTest)
gtest_discover_tests(hclx_tests)