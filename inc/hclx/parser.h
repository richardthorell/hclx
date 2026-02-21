#pragma once

#include <hclx/types.h>

namespace hclx
{

struct parse_result
{
    std::optional<ast> ast;
    diagnostics diags;
};

parse_result parse(std::span<const std::byte> input, const parse_options& options = {});

parse_result parse_file(std::string_view filename, const parse_options& options = {});
}