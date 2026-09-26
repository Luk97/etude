#include "file.h"

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
}
