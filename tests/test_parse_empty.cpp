#include <gtest/gtest.h>

#include <cstddef>
#include <span>

#include "hclx/diagnostics.h"
#include "hclx/parser.h"

inline auto as_bytes(std::string_view str)
{
    std::span view = str;
    return std::as_bytes(view);
}

TEST(Parse, AcceptsEmptyInput)
{
    hclx::parse_options opt{};
    std::span<const std::byte> empty{};
    hclx::parse_result r = hclx::parse(empty, opt);

    // This should not crash. If your parser reports an error on empty input,
    // change this expectation accordingly.
    EXPECT_FALSE(has_errors(r.diags));
}

TEST(Parse, ParsesAttributesAndBlocks)
{
    // Top-level attribute + a labeled block with nested attributes.
    std::string_view src = R"(
name = "demo"

tool "clang" {
  cmd = "clang++"
  workdir = "."
}
)";

    hclx::parse_options opt{};
    auto r = hclx::parse(as_bytes(src), opt);

    ASSERT_FALSE(has_errors(r.diags));

    // Expect 2 top-level items: `name = ...` and `tool "clang" { ... }`
    ASSERT_EQ(r.ast->items.size(), 2u);

    // 1) name attribute
    {
        const auto& it = r.ast->items[0].item;
        ASSERT_TRUE(std::holds_alternative<hclx::attribute>(it));

        const auto& a = std::get<hclx::attribute>(it);
        EXPECT_EQ(a.key, "name");

        // expression should be a string "demo"
        ASSERT_TRUE(std::holds_alternative<std::string>(a.exp.value));
        EXPECT_EQ(std::get<std::string>(a.exp.value), "demo");
    }

    // 2) tool block with label "clang"
    {
        const auto& it = r.ast->items[1].item;
        ASSERT_TRUE(std::holds_alternative<hclx::block>(it));

        const auto& b = std::get<hclx::block>(it);
        EXPECT_EQ(b.type, "tool");
        //ASSERT_TRUE(b.label.has_value());
        //EXPECT_EQ(*b.label, "clang");

        // inside: cmd + workdir (2 items)
        ASSERT_EQ(b.items.size(), 2u);

        // cmd = "clang++"
        {
            const auto& inner = b.items[0].item;
            ASSERT_TRUE(std::holds_alternative<hclx::attribute>(inner));
            const auto& a = std::get<hclx::attribute>(inner);
            EXPECT_EQ(a.key, "cmd");
            ASSERT_TRUE(std::holds_alternative<std::string>(a.exp.value));
            EXPECT_EQ(std::get<std::string>(a.exp.value), "clang++");
        }

        // workdir = "."
        {
            const auto& inner = b.items[1].item;
            ASSERT_TRUE(std::holds_alternative<hclx::attribute>(inner));
            const auto& a = std::get<hclx::attribute>(inner);
            EXPECT_EQ(a.key, "workdir");
            ASSERT_TRUE(std::holds_alternative<std::string>(a.exp.value));
            EXPECT_EQ(std::get<std::string>(a.exp.value), ".");
        }
    }
}