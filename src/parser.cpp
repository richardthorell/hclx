#include <hclx/types.h>
#include <hclx/diagnostics.h>
#include <hclx/parser.h>

#include "lexer.h"
#include "mapped_file.h"

namespace hclx
{

struct parser_context
{
    parser_context(std::span<const std::byte> input, const parse_options& options)
        : lx_{input, options, diags_}
        , options_{options}
    {
    }

    lexer lx_;
    diagnostics diags_;
    const parse_options& options_;
};

[[nodiscard]] inline source_position end_of_block_item(const block_item& blk) noexcept
{
    return std::visit([](const auto& item) { return item.where.end; }, blk.item);
}

[[nodiscard]] inline token peek_token(parser_context& ctx)
{
    return ctx.lx_.peek_token();
}

[[nodiscard]] inline token next_token(parser_context& ctx)
{
    return ctx.lx_.next_token();
}

inline void consume_token(parser_context& ctx) noexcept
{
    ctx.lx_.consume_token();
}

[[nodiscard]] inline token expect_token(parser_context& ctx, token_kind expected, std::string_view message = "Unexpected token")
{
    token tok = peek_token(ctx);
    if (tok.kind != expected)
    {
        error(ctx.diags_, message, tok.where);
        return {expected, tok.where, std::monostate{}};
    }

    return next_token(ctx);
}

inline void error_here(parser_context& ctx, std::string_view message)
{
    token tok = peek_token(ctx);
    error(ctx.diags_, message, tok.where);
}

[[nodiscard]] inline bool looks_like_item_start(parser_context& ctx) noexcept
{
    return peek_token(ctx).kind == token_kind::ident;
}

inline void recover_to_item_or_rbrace(parser_context& ctx)
{
    while (true)
    {
        const auto k = peek_token(ctx).kind;

        if (k == token_kind::ident || k == token_kind::rbrace || k == token_kind::eof)
            break;

        consume_token(ctx);
    }
}

static block_item parse_item(parser_context& ctx);
static block parse_block(parser_context& ctx, std::string type, source_position start);
static attribute parse_attribute(parser_context& ctx, std::string key, source_position start);
static expression parse_expression(parser_context& ctx);
static expression parse_array(parser_context& ctx);
static expression parse_object(parser_context& ctx);

static ast parse_ast(parser_context& ctx)
{
    ast result;

    while (peek_token(ctx).kind != token_kind::eof)
    {
        if (!looks_like_item_start(ctx))
        {
            error_here(ctx, "Expected block or attribute");
            consume_token(ctx);
            continue;
        }

        result.items.push_back(parse_item(ctx));
    }

    return result;
}

static block_item parse_item(parser_context& ctx)
{
    auto id = expect_token(ctx, token_kind::ident, "Expected identifier");
    const auto pos = id.where.start;

    if (peek_token(ctx).kind == token_kind::equal)
    {
        auto result = parse_attribute(ctx, std::move(std::get<std::string>(id.value)), pos);
        return block_item{
            std::move(result),
            {pos, result.where.end}
        };
    }
    else
    {
        auto result = parse_block(ctx, std::move(std::get<std::string>(id.value)), pos);
        return block_item{
            std::move(result),
            {pos, result.where.end}
        };
    }
}

static attribute parse_attribute(parser_context& ctx, std::string key, source_position start)
{
    attribute result;
    result.key = std::move(key);

    expect_token(ctx, token_kind::equal, "Expected '=' after attribute key");
    result.exp = parse_expression(ctx);
    result.where = {start, result.exp.where.end};
    return result;
}

static block parse_block(parser_context& ctx, std::string type, source_position start)
{
    block result;
    result.type = std::move(type);

    if (ctx.options_.allow_optional_block_labels && peek_token(ctx).kind == token_kind::string)
    {
        auto label = next_token(ctx);

        if (peek_token(ctx).kind == token_kind::lbrace)
        {
            result.label = std::move(std::get<std::string>(label.value));
        }
        else
        {
            error(ctx.diags_, "Expected '{' after block label", label.where);
        }
    }

    expect_token(ctx, token_kind::lbrace, "Expected '{' to start block");

    while (peek_token(ctx).kind != token_kind::rbrace && peek_token(ctx).kind != token_kind::eof)
    {
        if (!looks_like_item_start(ctx))
        {
            error_here(ctx, "Expected block or attribute in block body");
            recover_to_item_or_rbrace(ctx);
            continue;
        }

        result.items.push_back(parse_item(ctx));
    }

    auto rb = expect_token(ctx, token_kind::rbrace, "Expected '}' to end block");
    result.where = {start, rb.where.end};

    return result;
}

static expression parse_expression(parser_context& ctx)
{
    token tok = peek_token(ctx);

    switch (tok.kind)
    {
        case token_kind::string:
            tok = next_token(ctx);
            return expression{std::move(std::get<std::string>(tok.value)), tok.where};

        case token_kind::integer:
            tok = next_token(ctx);
            return expression{std::get<int64_t>(tok.value), tok.where};

        case token_kind::floating:
            next_token(ctx);
            return expression{std::get<double>(tok.value), tok.where};

        case token_kind::ident:
        {
            tok = next_token(ctx);
            const auto& ident = std::get<std::string>(tok.value);
            if (ident == "true")
                return expression{ true, tok.where };
            else if (ident == "false")
                return expression{ false, tok.where };
            else if (ident == "null")
                return expression{ nullptr, tok.where };
            else
            {
                error(ctx.diags_, "Unexpected identifier in expression", tok.where);
                return expression{ nullptr, tok.where };
            }
        }

        case token_kind::lbracket:
            return parse_array(ctx);

        case token_kind::lbrace:
            return parse_object(ctx);

        default:
            error_here(ctx, "Unexpected token in expression");
            consume_token(ctx);
            return expression{nullptr, tok.where};
    }
}

static expression parse_array(parser_context& ctx)
{
    auto lb = expect_token(ctx, token_kind::lbracket, "Expected '[' to start array");
    std::vector<expression> array;

    while (peek_token(ctx).kind != token_kind::rbracket && peek_token(ctx).kind != token_kind::eof)
    {
        array.push_back(parse_expression(ctx));

        if (peek_token(ctx).kind == token_kind::comma)
        {
            consume_token(ctx);

            if (ctx.options_.allow_trailing_commas && peek_token(ctx).kind == token_kind::rbracket)
                break;
        }
        else
            break;
    }

    auto rb = expect_token(ctx, token_kind::rbracket, "Expected ']' to end array");

    return expression{std::move(array), {lb.where.start, rb.where.end}};
}

static expression parse_object(parser_context& ctx)
{
    auto lb = expect_token(ctx, token_kind::lbrace, "Expected '{' to start object");
    std::unordered_map<std::string, expression> object;

    while (peek_token(ctx).kind != token_kind::rbrace && peek_token(ctx).kind != token_kind::eof)
    {
        token key = expect_token(ctx, token_kind::string, "Expected string literal as object key");
        expect_token(ctx, token_kind::equal, "Expected '=' after object key");
        expression value = parse_expression(ctx);

        object.emplace(std::move(std::get<std::string>(key.value)), std::move(value));

        if (peek_token(ctx).kind == token_kind::comma)
        {
            consume_token(ctx);

            if (ctx.options_.allow_trailing_commas && peek_token(ctx).kind == token_kind::rbrace)
                break;
        }
        else
            break;
    }

    auto rb = expect_token(ctx, token_kind::rbrace, "Expected '}' to end object");

    return expression{std::move(object), {lb.where.start, rb.where.end}};
}

parse_result parse(std::span<const std::byte> input, const parse_options& options)
{
    parser_context ctx{input, options};
    ast result = parse_ast(ctx);
    return {std::move(result), std::move(ctx.diags_)};
}

parse_result parse(std::string_view str, const parse_options& options)
{
    return parse(std::as_bytes(std::span{str}), options);
}

parse_result parse(const char *str, std::size_t size, const parse_options& options)
{
    return parse(std::string_view{str, size}, options);
}

parse_result parse_file(std::string_view filename, const parse_options& options)
{
    auto mapped = map_file(filename);
    if (!mapped)
    {
        diagnostics diags;
        error(diags, "Failed to open file", {{0, 0}, {0, 0}});
        return {std::nullopt, std::move(diags)};
    }

    return parse(mapped->data(), options);
}

}