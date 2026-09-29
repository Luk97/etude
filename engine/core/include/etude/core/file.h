#pragma once

#include <cstdint>
#include <expected>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace etude {

    /// @brief Reads a whole file into memory. Returns the reason instead if the file cannot be opened or read.
    std::expected<std::vector<std::uint8_t>, std::string> readFile(const std::filesystem::path& path);

    /// @brief Writes the text into the file, which it creates or replaces, byte for byte and without turning line
    /// breaks into those of Windows. Returns the reason instead if that fails, which keeps the old content.
    std::expected<void, std::string> writeFile(const std::filesystem::path& path, std::string_view text);
}
