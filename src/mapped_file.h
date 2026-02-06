#pragma once

#include <byte>
#include <span>

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