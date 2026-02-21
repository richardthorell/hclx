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
 * @brief Represents a position within the source input.
 *
 * Stores line number, column number, and absolute byte offset.
 * Line and column numbers are 1-based.
 */
struct source_position
{
    uint32_t line = 1;
    uint32_t column = 1;
    uint32_t offset = 0;
};


/**
 * @brief Represents a half-open range in the source input.
 *
 * The range is defined by a starting and ending position.
 * The interpretation of inclusivity is defined by the parser.
 */
struct source_range
{
    source_position start;
    source_position end;
};


/**
 * @brief Severity level of a diagnostic message.
 */
enum class severity : uint8_t
{
    error,   ///< Parsing or lexical error.
    warning, ///< Non-fatal issue.
    info     ///< Informational message.
};


/**
 * @brief Represents a single diagnostic entry.
 *
 * Diagnostics are emitted during lexing and parsing.
 */
struct diagnostic
{
    /**
     * @brief Severity classification of the diagnostic.
     */
    severity severity = severity::error;

    /**
     * @brief Source range associated with the diagnostic.
     */
    source_range where;

    /**
     * @brief Human-readable diagnostic message.
     */
    std::string message;
};


/**
 * @brief Collection of diagnostics produced during parsing.
 *
 * Diagnostics are accumulated and returned to the caller
 * as part of parse_result.
 */
struct diagnostics
{
    /**
     * @brief Ordered list of diagnostic entries.
     */
    std::vector<diagnostic> items;
};


/**
 * @brief Represents an HCL expression node.
 *
 * An expression may contain primitive values, arrays, or objects.
 * Each expression stores its associated source range.
 */
struct expression
{
    /**
     * @brief Variant storage for all supported expression types.
     */
    using data_type = std::variant<
        std::nullptr_t,
        bool,
        int64_t,
        double,
        std::string,
        std::vector<expression>,
        std::unordered_map<std::string, expression>
    >;

    /**
     * @brief Constructs a null expression with default source range.
     */
    constexpr expression() noexcept
        : value{ nullptr }
    {}

    /**
     * @brief Constructs a null expression with explicit source range.
     */
    constexpr expression(std::nullptr_t, source_range w) noexcept
        : value{ nullptr }
        , where{ w }
    {}

    /**
     * @brief Constructs a boolean expression.
     */
    constexpr expression(bool v, source_range w) noexcept
        : value{ v }
        , where{ w }
    {}

    /**
     * @brief Constructs an integer expression.
     */
    constexpr expression(int64_t v, source_range w) noexcept
        : value{ v }
        , where{ w }
    {}

    /**
     * @brief Constructs a floating-point expression.
     */
    constexpr expression(double v, source_range w) noexcept
        : value{ v }
        , where{ w }
    {}

    /**
     * @brief Constructs a string expression.
     *
     * The string is moved into the expression.
     */
    expression(std::string v, source_range w)
        : value{ std::move(v) }
        , where{ w }
    {}

    /**
     * @brief Constructs an array expression.
     *
     * The vector is moved into the expression.
     */
    expression(std::vector<expression> v, source_range w)
        : value{ std::move(v) }
        , where{ w }
    {}

    /**
     * @brief Constructs an object expression.
     *
     * The unordered_map is moved into the expression.
     */
    expression(std::unordered_map<std::string, expression> v, source_range w)
        : value{ std::move(v) }
        , where{ w }
    {}

    /**
     * @brief Stored expression value.
     */
    data_type value;

    /**
     * @brief Source range of this expression.
     */
    source_range where;
};


/**
 * @brief Represents a key-value attribute inside a block.
 */
struct attribute
{
    /**
     * @brief Attribute key.
     */
    std::string key;

    /**
     * @brief Attribute value expression.
     */
    expression exp;

    /**
     * @brief Source range of the attribute.
     */
    source_range where;
};


struct block_item;


/**
 * @brief Represents a block construct.
 *
 * Blocks contain a type, optional label, and nested items.
 */
struct block
{
    /**
     * @brief Block type identifier.
     */
    std::string type;

    /**
     * @brief Optional block label.
     */
    std::string label;

    /**
     * @brief Nested attributes and blocks.
     */
    std::vector<block_item> items;

    /**
     * @brief Source range of the block.
     */
    source_range where;
};


/**
 * @brief Represents a single item within a block.
 *
 * A block item is either an attribute or a nested block.
 */
struct block_item
{
    /**
     * @brief Variant holding either an attribute or a block.
     */
    std::variant<attribute, block> item;

    /**
     * @brief Source range of the item.
     */
    source_range where;
};


/**
 * @brief Root abstract syntax tree.
 *
 * Contains all top-level block items parsed from the input.
 */
struct ast
{
    /**
     * @brief Top-level items in the document.
     */
    std::vector<block_item> items;
};


/**
 * @brief Configuration options controlling parser behavior.
 *
 * These options enable or restrict certain language features.
 */
struct parse_options
{
    /**
     * @brief Allow blocks without explicit labels.
     */
    bool allow_optional_block_labels = true;

    /**
     * @brief Allow trailing commas in arrays and objects.
     */
    bool allow_trailing_commas = false;

    /**
     * @brief Allow '#' style comments.
     */
    bool allow_hash_comments = false;

    /**
     * @brief Allow C and C++ style comments.
     */
    bool allow_slash_comments = false;
};

}