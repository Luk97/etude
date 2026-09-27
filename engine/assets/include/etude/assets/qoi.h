#pragma once

#include <etude/core/image.h>

#include <cstdint>
#include <expected>
#include <filesystem>
#include <span>
#include <string>

namespace etude {

    /// @brief Decodes an image in the QOI format, the Quite OK Image Format, into pixels with alpha.
    /// Returns the reason instead if the bytes are not a complete QOI image.
    std::expected<Image, std::string> decodeQoi(std::span<const std::uint8_t> bytes);

    /// @brief Reads a QOI file and decodes it. Returns a message with the path instead if that fails.
    std::expected<Image, std::string> loadQoi(const std::filesystem::path& path);
}
