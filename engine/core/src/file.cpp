#include <etude/core/file.h>

#include <fstream>
#include <system_error>

namespace etude {

    std::expected<std::vector<std::uint8_t>, std::string> readFile(const std::filesystem::path& path) {
        std::ifstream file(path, std::ios::binary);
        std::error_code error;
        const std::uintmax_t size = std::filesystem::file_size(path, error);
        if (!file || error) {
            return std::unexpected("the file cannot be opened");
        }

        std::vector<std::uint8_t> bytes(size);
        if (!file.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(size))) {
            return std::unexpected("the file cannot be read");
        }
        return bytes;
    }

    std::expected<void, std::string> writeFile(const std::filesystem::path& path, std::string_view text) {
        // The text goes into a second file first, which replaces the old one only once it is complete. A write that
        // fails halfway therefore keeps the old content.
        std::filesystem::path temporary = path;
        temporary += ".tmp";
        std::ofstream file(temporary, std::ios::binary);
        if (!file) {
            return std::unexpected("the file cannot be written");
        }
        file.write(text.data(), static_cast<std::streamsize>(text.size()));

        // Closing flushes the buffer, so it also reports a write that only fails at the end.
        file.close();
        std::error_code error;
        if (file) {
            std::filesystem::rename(temporary, path, error);
        }
        if (!file || error) {
            std::filesystem::remove(temporary, error);
            return std::unexpected("the file cannot be written");
        }
        return {};
    }
}
