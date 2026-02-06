#pragma once

#include <hclx/types.h>

#include <byte>
#include <span>

namespace hclx
{

struct parse_result
{
    std::optional<ast> ast;
    diagnostics diags;
};


struct parse_options
{
    bool allow_optional_block_labels = false;
    bool allow_trailing_comma = false;
    bool allow_hash_comments = false;
    bool allow_slash_comments = false;
};


parse_result parse(std::span<const std::byte> input, const parse_options& options = {});

parse_result parse_file(std::string_view filename, const parse_options& options = {});
}