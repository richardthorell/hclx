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

#include "mapped_file.h"

#if defined(_WIN32)
#include <windows.h>
#else
// TODO: Add includes
#endif

namespace hclx
{
mapped_file::~mapped_file()
{
#if defined(_WIN32)
    if (handle_)
    {
        UnmapViewOfFile(data_.data());
        CloseHandle(mapping_);
        CloseHandle(handle_);
    }
#else
    if (fd_ != -1)
    {
        munmap(const_cast<std::byte*>(data_.data()), data_.size());
        close(fd_);
    }
#endif
}

std::optional<mapped_file> map_file(std::string_view filename)
{
    mapped_file result;
#if defined(_WIN32)
    result.handle_ = CreateFileA(filename.data(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (result.handle_ == INVALID_HANDLE_VALUE)    {
        return std::nullopt;
    }
    
    LARGE_INTEGER file_size;
    if (!GetFileSizeEx(result.handle_, &file_size))
    {
        CloseHandle(result.handle_);
        return std::nullopt;
    }

    result.mapping_ = CreateFileMappingA(result.handle_, nullptr, PAGE_READONLY, 0, 0, nullptr);
    if (!result.mapping_)
    {
        CloseHandle(result.handle_);
        return std::nullopt;
    }

    void* data = MapViewOfFile(result.mapping_, FILE_MAP_READ, 0, 0, 0);
    if (!data)
    {
        CloseHandle(result.mapping_);
        CloseHandle(result.handle_);
        return std::nullopt;
    }

    result.data_ = std::span<const std::byte>(static_cast<const std::byte*>(data), file_size.QuadPart);
#else
    result.fd_ = open(filename.data(), O_RDONLY);
    if (result.fd_ == -1)
    {
        return std::nullopt;
    }

    struct stat st;
    if (fstat(result.fd_, &st) == -1)
    {
        close(result.fd_);
        return std::nullopt;
    }

    void* data = mmap(nullptr, st.st_size, PROT_READ, MAP_PRIVATE, result.fd_, 0);
    if (data == MAP_FAILED)
    {
        close(result.fd_);
        return std::nullopt;
    }

    result.data_ = std::span<const std::byte>(static_cast<const std::byte*>(data), st.st_size);
#endif

    return result;
}

}