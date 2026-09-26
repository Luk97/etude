#pragma once

#include <cstdint>
#include <expected>
#include <filesystem>
#include <string>
#include <vector>

namespace etude {

    /// @brief Reads a whole file into memory. Returns the reason instead if the file cannot be opened or read.
    std::expected<std::vector<std::uint8_t>, std::string> readFile(const std::filesystem::path& path);
}
