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

struct mapped_file
{
    ~mapped_file();

    [[nodiscard]] std::span<const std::byte> data() const noexcept
    {
        return data_;
    }

private:
    friend std::optional<mapped_file> map_file(std::string_view filename);

    std::span<const std::byte> data_;
#if defined(_WIN32)
    void* handle_ = nullptr;
    void* mapping_ = nullptr;
#else
    int fd_ = -1;
#endif
};

/**
 * @brief Maps a file into memory and returns a `mapped_file` object containing the file data. If the file cannot be opened or mapped, returns `std::nullopt`.
 * 
 * @param filename The path to the file to be mapped.
 * 
 * @return An optional `mapped_file` object containing the file data, or `std::nullopt` if the file cannot be opened or mapped.
 */
std::optional<mapped_file> map_file(std::string_view filename);

}