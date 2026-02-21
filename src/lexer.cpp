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

#include <hclx/diagnostics.h>

#include <charconv>

#include "lexer.h"

namespace hclx
{

inline bool is_space(uint8_t c) noexcept
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

inline bool is_alpha(uint8_t c) noexcept
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

inline bool is_digit(uint8_t c) noexcept
{
    return c >= '0' && c <= '9';
}

inline bool is_ident_cont(uint8_t c) noexcept
{
    return is_alpha(c) || is_digit(c) || c == '_';
}

inline bool at_end(const std::span<const std::byte>& input, std::size_t pos) noexcept
{
    return pos >= input.size();
}

inline bool at_end(const lexer& lx) noexcept
{
    return at_end(lx.input_, lx.index_);
}

inline uint8_t byte_at_unsafe(const std::span<const std::byte>& input, std::size_t pos) noexcept
{
    return static_cast<uint8_t>(input[pos]);
}

inline uint8_t byte_at(const std::span<const std::byte>& input, std::size_t pos) noexcept
{
    return at_end(input, pos) ? 0 : byte_at_unsafe(input, pos);
}

inline uint8_t cur(const lexer& lx) noexcept
{
    return byte_at(lx.input_, lx.index_);
}

inline uint8_t peek(const lexer& lx, std::size_t offset = 1) noexcept
{
    return byte_at(lx.input_, lx.index_ + offset);
}

inline void advance(lexer& lx)
{
    if (at_end(lx))
        return;
    
    auto c = byte_at_unsafe(lx.input_, lx.index_);

    lx.index_++;
    lx.position_.offset++;

    if (c == '\n')
    {
        lx.position_.line++;
        lx.position_.column = 1;
    }
    else
    {
        lx.position_.column++;
    }
}

static void skip_line_comment(lexer& lx)
{
    while (!at_end(lx) && cur(lx) != '\n')
        advance(lx);

    if (!at_end(lx) && cur(lx) == '\n')
        advance(lx);
}

static void skip_block_comment(lexer& lx)
{
    // Skip '//' or '/*'
    advance(lx);
    advance(lx);

    while (!at_end(lx))
    {
        if (cur(lx) == '*' && peek(lx) == '/')
        {
            advance(lx);
            advance(lx);
            return;
        }

        advance(lx);
    }
}

static void skip_trivia(lexer& lx)
{
    while (true)
    {
        while (!at_end(lx) && is_space(cur(lx)))
            advance(lx);

        auto c = cur(lx);

        if (lx.options_.allow_hash_comments && c == '#')
        {
            skip_line_comment(lx);
            continue;
        }
        
        if (lx.options_.allow_slash_comments && c == '/' && peek(lx) == '/')
        {
            advance(lx); // Skip first '/'
            advance(lx); // Skip second '/'
            skip_line_comment(lx);
            continue;
        }
        
        if (lx.options_.allow_slash_comments && c == '/' && peek(lx) == '*')
        {
            auto start = lx.position_;

            skip_block_comment(lx);

            if (at_end(lx))
            {
                error(lx.diags_, "Unterminated block comment", {start, lx.position_});
            }
        }

        break;
    }
}

static token lex_ident(lexer& lx)
{
    const auto pos = lx.position_;
    const auto idx = lx.index_;

    advance(lx); // Skip first character

    while (!at_end(lx) && is_ident_cont(cur(lx)))
        advance(lx);

    return
    {
        token_kind::ident,
        {pos, lx.position_},
        std::string{reinterpret_cast<const char*>(lx.input_.data() + idx), lx.index_ - idx}
    };
}

static token lex_string(lexer& lx)
{
    const auto pos = lx.position_;

    advance(lx); // Skip opening quote

    std::string out;
    while (!at_end(lx))
    {
        auto c = cur(lx);

        if (c == '"')
        {
            advance(lx); // Skip closing quote
            break;
        }

        if (c == '\\')
        {
            advance(lx); // Skip backslash

            if (at_end(lx))
            {
                break;
            }

            c = cur(lx);

            switch (c)
            {
                case '"': out.push_back('"'); break;
                case '\\': out.push_back('\\'); break;
                case 'n': out.push_back('\n'); break;
                case 't': out.push_back('\t'); break;
                case 'r': out.push_back('\r'); break;
                case '{': out.push_back('{'); break;
                case '}': out.push_back('}'); break;
                default:
                    error(lx.diags_, "Invalid escape sequence", {pos, lx.position_});
                    break;
            }

            advance(lx);
            continue;
        }
        
        out.push_back(c);
        advance(lx);
    }

    // TODO: Handle unterminated string literal

    return
    {
        token_kind::string,
        {pos, lx.position_},
        std::move(out)
    };
}

static token lex_number(lexer& lx)
{
    const auto pos = lx.position_;
    auto idx = lx.index_;

    // Optional leading sign
    if (cur(lx) == '-' || cur(lx) == '+')
        advance(lx);

    bool saw_digit = false;
    while (!at_end(lx) && is_digit(cur(lx)))
    {
        advance(lx);
        saw_digit = true;
    }

    bool is_float = false;

    // Optional fractional part
    if (!at_end(lx) && cur(lx) == '.' && is_digit(peek(lx)))
    {
        advance(lx); // Skip '.'

        while (!at_end(lx) && is_digit(cur(lx)))
            advance(lx);

        is_float = true;
    }

    // Optional exponent part
    if (!at_end(lx) && (cur(lx) == 'e' || cur(lx) == 'E'))
    {
        const auto exp_pos = lx.position_;
        const auto exp_idx = lx.index_;

        advance(lx); // Skip 'e' or 'E'

        if (!at_end(lx) && (cur(lx) == '-' || cur(lx) == '+'))
            advance(lx); // Skip optional sign

        bool exp_saw_digit = false;
        while (!at_end(lx) && is_digit(cur(lx)))
        {
            advance(lx);
            exp_saw_digit = true;
        }

        if (exp_saw_digit)
        {
            is_float = true;
        }
        else
        {
            error(lx.diags_, "Invalid exponent in number literal", {pos, lx.position_});

            // Roll back to before the exponent
            lx.index_ = exp_idx;
            lx.position_ = exp_pos;
        }
    }

    if (!saw_digit)
    {
        error(lx.diags_, "Invalid number literal", {pos, lx.position_});
        return {token_kind::integer, {pos, lx.position_}, 0};
    }

    std::string_view num_str(reinterpret_cast<const char*>(lx.input_.data() + idx), lx.index_ - idx);

    if (is_float)
    {
        double value = 0.0;
        auto fc = std::from_chars(num_str.data(), num_str.data() + num_str.size(), value);
        if (fc.ec != std::errc())
        {
            error(lx.diags_, "Floating-point literal out of range", {pos, lx.position_});
        }

        return {token_kind::floating, {pos, lx.position_}, value};
    }
    else
    {
        int64_t value = 0;
        auto fc =  std::from_chars(num_str.data(), num_str.data() + num_str.size(), value);
        if (fc.ec != std::errc())
        {
            error(lx.diags_, "Integer literal out of range", {pos, lx.position_});
        }

        return {token_kind::integer, {pos, lx.position_}, value};
    }
}

static token lex_one(lexer& lx)
{
    skip_trivia(lx);

    const auto pos = lx.position_;

    if (at_end(lx))
    {
        return {token_kind::eof, {pos, pos}, std::monostate{}};
    }

    auto c = cur(lx);

    switch (c)
    {
        case '{':
            advance(lx);
            return {token_kind::lbrace, {pos, lx.position_}, std::monostate{}};
        case '}':
            advance(lx);
            return {token_kind::rbrace, {pos, lx.position_}, std::monostate{}};
        case '[':
            advance(lx);
            return {token_kind::lbracket, {pos, lx.position_}, std::monostate{}};
        case ']':
            advance(lx);
            return {token_kind::rbracket, {pos, lx.position_}, std::monostate{}};
        case '=':
            advance(lx);
            return {token_kind::equal, {pos, lx.position_}, std::monostate{}};
        case ',':
            advance(lx);
            return {token_kind::comma, {pos, lx.position_}, std::monostate{}};
        case '"':
            return lex_string(lx);
        default:
            if (is_alpha(c))
                return lex_ident(lx);
            if (is_digit(c) || (c == '-' && is_digit(peek(lx))) || (c == '+' && is_digit(peek(lx))))
                return lex_number(lx);
    }

    advance(lx);
    error(lx.diags_, "Unexpected character", {pos, lx.position_});

    return {token_kind::eof, {pos, lx.position_}, std::monostate{}};
}

token lexer::next_token()
{
    if (peek_)
    {
        token result = *peek_;
        peek_.reset();
        return result;
    }

    return lex_one(*this);
}


token lexer::peek_token()
{
    if (!peek_)
    {
        peek_ = lex_one(*this);
    }

    return *peek_;
}


void lexer::consume_token() noexcept
{
    peek_.reset();
}

}