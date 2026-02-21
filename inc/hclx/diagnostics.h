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
 * @brief Determine whether the given diagnostics contain any errors.
 *
 * Performs a linear scan over the diagnostic collection and returns
 * true if at least one diagnostic has severity::error.
 *
 * @param diags Diagnostics container to inspect.
 *
 * @return true if at least one error diagnostic is present.
 * @return false if no error diagnostics are present.
 *
 * @note This function does not allocate and does not modify @p diags.
 */
[[nodiscard]] constexpr bool has_errors(const diagnostics& diags) noexcept
{
    for (const auto& diag : diags.items)
    {
        if (diag.severity == severity::error)
            return true;
    }

    return false;
}


/**
 * @brief Append an error diagnostic.
 *
 * Adds a new diagnostic with severity::error to the provided
 * diagnostics container.
 *
 * @param diags   Diagnostics container to modify.
 * @param message Error message to associate with the diagnostic.
 * @param where   Source location range associated with the error.
 *
 * @note The message string is moved into the diagnostics container.
 */
inline void error(diagnostics& diags, std::string message, source_range where)
{
    diags.items.push_back({severity::error, where, std::move(message)});
}


/**
 * @brief Append an error diagnostic from a string view.
 *
 * Convenience overload that copies the provided message into
 * an owning std::string before appending the diagnostic.
 *
 * @param diags   Diagnostics container to modify.
 * @param message Error message text.
 * @param where   Source location range associated with the error.
 */
inline void error(diagnostics& diags, std::string_view message, source_range where)
{
    error(diags, std::string{message}, where);
}


/**
 * @brief Append an error diagnostic from a null-terminated string.
 *
 * Convenience overload that copies the provided C-string into
 * an owning std::string before appending the diagnostic.
 *
 * @param diags   Diagnostics container to modify.
 * @param message Null-terminated error message string.
 * @param where   Source location range associated with the error.
 */
inline void error(diagnostics& diags, const char* message, source_range where)
{
    error(diags, std::string{message}, where);
}

}