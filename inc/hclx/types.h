/**
 * MIT License
 *
 * Copyright (c) 2026 Richard Thorell
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

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