#pragma once

#include <hclx/types.h>

namespace hclx
{

enum class token_kind : uint8_t
{
    eof,
    ident,
    string,
    integer,
    floating,
    lbrace, rbrace, lbracket, rbracket,
    equal,
    comma,
};


struct token
{
    token_kind kind{token_kind::eof};
    source_range where;
    std::variant<std::monostate, std::string, int64_t, double> value;
};


struct lexer
{
    lexer(std::span<const std::byte> input, const parse_options& options, diagnostics& diags) noexcept
        : input_{input}
        , options_{options}
        , diags_{diags}
    {
    }

    [[nodiscard]] token next_token();

    [[nodiscard]] token peek_token();

    void consume_token() noexcept;

    std::span<const std::byte> input_;
    const parse_options& options_;
    diagnostics& diags_;
    std::size_t index_ = 0;
    source_position position_;
    std::optional<token> peek_;
};
}