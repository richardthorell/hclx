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

#include <hclx/types.h>

namespace hclx
{

/**
 * @brief Result of a parse operation.
 *
 * Contains the parsed abstract syntax tree if parsing succeeded,
 * along with any diagnostics produced during lexing or parsing.
 *
 * Parsing never throws for syntax errors. All issues are reported
 * through the diagnostics container.
 */
struct parse_result
{
    /**
     * @brief Parsed abstract syntax tree.
     *
     * Engaged if parsing completed successfully without fatal errors.
     * If disengaged, diagnostics will contain at least one error.
     */
    std::optional<ast> ast;

    /**
     * @brief Diagnostics emitted during parsing.
     *
     * May contain errors, warnings, or informational messages.
     * Diagnostics are always populated, even if parsing succeeds.
     */
    diagnostics diags;
};


/**
 * @brief Parse HCL input from a byte span.
 *
 * Parses in-memory data provided as a contiguous span of bytes.
 *
 * @param input   Byte span containing HCL source data.
 * @param options Parser configuration options.
 *
 * @return parse_result containing the AST on success and diagnostics.
 */
parse_result parse(std::span<const std::byte> input, const parse_options& options = {});


/**
 * @brief Parse HCL input from a string view.
 *
 * Convenience overload for parsing text-based HCL input.
 * The string_view must remain valid for the duration of the call.
 *
 * @param str     String view containing HCL source text.
 * @param options Parser configuration options.
 *
 * @return parse_result containing the AST on success and diagnostics.
 */
parse_result parse(std::string_view str, const parse_options& options = {});


/**
 * @brief Parse HCL input from a raw character buffer.
 *
 * Parses HCL data from a pointer and explicit size.
 * The memory referenced by @p str must be valid for @p size bytes.
 *
 * @param str     Pointer to HCL source data.
 * @param size    Number of bytes available at @p str.
 * @param options Parser configuration options.
 *
 * @return parse_result containing the AST on success and diagnostics.
 */
parse_result parse(const char* str,  std::size_t size, const parse_options& options = {});


/**
 * @brief Parse HCL input from a file.
 *
 * Loads and parses the contents of the specified file path.
 *
 * @param filename Path to the file containing HCL source.
 * @param options  Parser configuration options.
 *
 * @return parse_result containing the AST on success and diagnostics.
 */
parse_result parse_file(std::string_view filename, const parse_options& options = {});
}