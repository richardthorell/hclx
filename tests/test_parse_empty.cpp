#include <gtest/gtest.h>

#include <cstddef>
#include <span>

#include "hclx/parser.h"

TEST(Parse, AcceptsEmptyInput)
{
    hclx::parse_options opt{};
    std::span<const std::byte> empty{};
    hclx::parse_result r = hclx::parse(empty, opt);

    // This should not crash. If your parser reports an error on empty input,
    // change this expectation accordingly.
    EXPECT_FALSE(r.diags.has_errors());
}