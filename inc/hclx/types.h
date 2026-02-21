#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <span>
#include <unordered_map>
#include <variant>
#include <vector>

namespace hclx
{

/**
 * @brief Represents a position in the source code, including line number, column number, and byte offset.
 */
struct source_position
{
    uint32_t line = 1;
    uint32_t column = 1;
    uint32_t offset = 0;
};


/**
 * @brief Represents a range in the source code, defined by a starting and ending position.
 */
struct source_range
{
    source_position start;
    source_position end;
};

enum class serverity : uint8_t
{
    error,
    warning,
    info
};

struct diagnostic
{
    serverity severity = serverity::error;
    source_range where;
    std::string message;
};

struct diagnostics
{
    std::vector<diagnostic> items;
};

struct expression
{
    using data_type = std::variant<
        std::nullptr_t,
        bool,
        int64_t,
        double,
        std::string,
        std::vector<expression>,
        std::unordered_map<std::string, expression>
    >;

    constexpr expression() noexcept
        : value{ nullptr }
    {}

    constexpr expression(std::nullptr_t, source_range w) noexcept
        : value{ nullptr }
        , where{w}
    {}
    
    constexpr expression(bool v, source_range w) noexcept
        : value{v}
        , where{w}
    {}

    constexpr expression(int64_t v, source_range w) noexcept
        : value{v}
        , where{w}
    {}

    constexpr expression(double v, source_range w) noexcept
        : value{v}
        , where{w}
    {}

    expression(std::string v, source_range w)
        : value{std::move(v)}
        , where{w}
    {}

    expression(std::vector<expression> v, source_range w)
        : value{std::move(v)}
        , where{w}
    {}

    expression(std::unordered_map<std::string, expression> v, source_range w)
        : value{std::move(v)}
        , where{w}
    {}

    data_type value;
    source_range where;
};

struct attribute
{
    std::string key;
    expression exp;
    source_range where;
};

struct block_item;

struct block
{
    std::string type;
    std::string label;
    std::vector<block_item> items;
    source_range where;
};

struct block_item
{
    std::variant<attribute, block> item;
    source_range where;
};

struct ast
{
    std::vector<block_item> items;
};

struct parse_options
{
    bool allow_optional_block_labels = true;
    bool allow_trailing_commas = false;
    bool allow_hash_comments = false;
    bool allow_slash_comments = false;
};

}