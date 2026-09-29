#include <catch2/catch_test_macros.hpp>

#include <etude/core/file.h>

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace {

    std::filesystem::path temporaryFile(const char* name) {
        return std::filesystem::temp_directory_path() / name;
    }
}

TEST_CASE("writeFile writes the text byte for byte, so that line breaks stay as they are") {
    const std::filesystem::path path = temporaryFile("etude_write_file_test.txt");
    REQUIRE(etude::writeFile(path, "a\nb"));
    const auto bytes = etude::readFile(path);
    std::filesystem::remove(path);
    REQUIRE(bytes);
    CHECK(*bytes == std::vector<std::uint8_t>{'a', '\n', 'b'});
}

TEST_CASE("writeFile replaces what the file held before") {
    const std::filesystem::path path = temporaryFile("etude_replace_file_test.txt");
    REQUIRE(etude::writeFile(path, "a longer text"));
    REQUIRE(etude::writeFile(path, "x"));
    const auto bytes = etude::readFile(path);
    std::filesystem::remove(path);
    REQUIRE(bytes);
    CHECK(*bytes == std::vector<std::uint8_t>{'x'});
}

TEST_CASE("readFile and writeFile say why they fail") {
    const auto read = etude::readFile(temporaryFile("etude_missing_file.txt"));
    REQUIRE_FALSE(read);
    CHECK(read.error() == "the file cannot be opened");

    const auto written = etude::writeFile(temporaryFile("etude_missing_folder") / "file.txt", "text");
    REQUIRE_FALSE(written);
    CHECK(written.error() == "the file cannot be written");
}

TEST_CASE("Paths from UTF-8 strings find files with umlauts") {
    // Windows reads the narrow strings of a path in the code page of the program, which the manifest of the platform
    // module sets to UTF-8.
    const std::filesystem::path folder = std::filesystem::temp_directory_path();
    const std::filesystem::path path = folder / std::filesystem::path(u8"etude_häschen.txt");
    REQUIRE(etude::writeFile(path, "x"));
    const bool found = std::filesystem::exists(folder / std::string("etude_h\xc3\xa4schen.txt"));
    std::filesystem::remove(path);
    CHECK(found);
}
