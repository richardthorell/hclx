#pragma once

#include <hclx/types.h>

namespace hclx
{

/**
 * @brief Checks if the diagnostics contain any errors.
 * 
 * @param diags The diagnostics to check.
 * @return true if there are errors, false otherwise.
 */
[[nodiscard]] constexpr bool has_errors(const diagnostics& diags) noexcept
{
    for (const auto& diag : diags.items)
    {
        if (diag.severity == serverity::error)
            return true;
    }

    return false;
}

inline void error(diagnostics& diags, std::string_view message, source_range where)
{
    diags.items.push_back({serverity::error, where, message});
}

}