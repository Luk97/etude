#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <etude/assets/qoi.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <vector>

using etude::Image;
using etude::Pixel;

namespace {

    /// @brief Wraps chunks into a complete QOI stream: the header with the size, the chunks, then the end marker.
    std::vector<std::uint8_t> qoiStream(
        std::uint32_t width,
        std::uint32_t height,
        const std::vector<std::uint8_t>& chunks
    ) {
        std::vector<std::uint8_t> bytes{'q', 'o', 'i', 'f'};
        for (const std::uint32_t value : {width, height}) {
            for (int shift = 24; shift >= 0; shift -= 8) {
                bytes.push_back(static_cast<std::uint8_t>(value >> shift));
            }
        }
        // Four channels in sRGB. Both fields only describe the source image, the decoder ignores them.
        bytes.insert(bytes.end(), {4, 0});
        bytes.insert(bytes.end(), chunks.begin(), chunks.end());
        bytes.insert(bytes.end(), {0, 0, 0, 0, 0, 0, 0, 1});
        return bytes;
    }

    /// @brief Encodes an image like the reference encoder of the QOI format. Only the tests need an encoder, to check
    /// the decoder on streams with every chunk type.
    std::vector<std::uint8_t> encodeQoi(const Image& image) {
        std::vector<std::uint8_t> chunks;
        std::array<Pixel, 64> seen{};
        Pixel previous{
            .a = 255,
        };
        int run = 0;
        for (const Pixel pixel : image.pixels) {
            if (pixel == previous) {
                // A run holds at most 62 pixels, the two remaining values of the tag mean QOI_OP_RGB and QOI_OP_RGBA.
                if (++run == 62) {
                    chunks.push_back(static_cast<std::uint8_t>(0xc0 | (run - 1)));
                    run = 0;
                }
                continue;
            }
            if (run > 0) {
                chunks.push_back(static_cast<std::uint8_t>(0xc0 | (run - 1)));
                run = 0;
            }

            const std::size_t slot = (pixel.r * 3 + pixel.g * 5 + pixel.b * 7 + pixel.a * 11) % 64;
            const int dr = static_cast<std::int8_t>(pixel.r - previous.r);
            const int dg = static_cast<std::int8_t>(pixel.g - previous.g);
            const int db = static_cast<std::int8_t>(pixel.b - previous.b);
            if (seen[slot] == pixel) {
                chunks.push_back(static_cast<std::uint8_t>(slot));
            } else if (pixel.a != previous.a) {
                chunks.insert(chunks.end(), {0xff, pixel.r, pixel.g, pixel.b, pixel.a});
            } else if (dr >= -2 && dr <= 1 && dg >= -2 && dg <= 1 && db >= -2 && db <= 1) {
                chunks.push_back(static_cast<std::uint8_t>(0x40 | ((dr + 2) << 4) | ((dg + 2) << 2) | (db + 2)));
            } else if (dg >= -32 && dg <= 31 && dr - dg >= -8 && dr - dg <= 7 && db - dg >= -8 && db - dg <= 7) {
                chunks.push_back(static_cast<std::uint8_t>(0x80 | (dg + 32)));
                chunks.push_back(static_cast<std::uint8_t>(((dr - dg + 8) << 4) | (db - dg + 8)));
            } else {
                chunks.insert(chunks.end(), {0xfe, pixel.r, pixel.g, pixel.b});
            }
            seen[slot] = pixel;
            previous = pixel;
        }
        if (run > 0) {
            chunks.push_back(static_cast<std::uint8_t>(0xc0 | (run - 1)));
        }
        return qoiStream(static_cast<std::uint32_t>(image.width), static_cast<std::uint32_t>(image.height), chunks);
    }
}

TEST_CASE("decodeQoi reads the size and starts from opaque black") {
    // A run of six, stored with a bias of -1.
    const auto image = etude::decodeQoi(qoiStream(3, 2, {0b11'000101}));
    REQUIRE(image);
    CHECK(image->width == 3);
    CHECK(image->height == 2);
    CHECK(image->pixels == std::vector<Pixel>(6, {0, 0, 0, 255}));
}

TEST_CASE("decodeQoi sets all channels with QOI_OP_RGBA and keeps the alpha with QOI_OP_RGB") {
    const auto image = etude::decodeQoi(qoiStream(2, 1, {0xff, 1, 2, 3, 128, 0xfe, 10, 20, 30}));
    REQUIRE(image);
    CHECK(image->pixels == std::vector<Pixel>{{1, 2, 3, 128}, {10, 20, 30, 128}});
}

TEST_CASE("decodeQoi adds small differences with QOI_OP_DIFF and wraps around") {
    // Red -1, green 0 and blue +1, each stored with a bias of 2.
    const auto image = etude::decodeQoi(qoiStream(1, 1, {0b01'01'10'11}));
    REQUIRE(image);
    CHECK(image->pixels[0] == Pixel{255, 0, 1, 255});
}

TEST_CASE("decodeQoi derives red and blue from the green difference with QOI_OP_LUMA") {
    // Green +10 with a bias of 32, then red +2 and blue -5 relative to green, each with a bias of 8.
    const auto image = etude::decodeQoi(qoiStream(1, 1, {0b10'101010, 0b1010'0011}));
    REQUIRE(image);
    CHECK(image->pixels[0] == Pixel{12, 10, 5, 255});
}

TEST_CASE("decodeQoi repeats the previous pixel with QOI_OP_RUN") {
    // A run of three, stored with a bias of -1, then a new color.
    const auto image = etude::decodeQoi(qoiStream(5, 1, {0xfe, 10, 20, 30, 0b11'000010, 0xfe, 40, 50, 60}));
    REQUIRE(image);
    const Pixel repeated{10, 20, 30, 255};
    const Pixel next{40, 50, 60, 255};
    CHECK(image->pixels == std::vector<Pixel>{repeated, repeated, repeated, repeated, next});
}

TEST_CASE("decodeQoi looks up a pixel seen before with QOI_OP_INDEX") {
    // Slot of (10, 20, 30, 255): (10 * 3 + 20 * 5 + 30 * 7 + 255 * 11) % 64 = 9.
    const auto image = etude::decodeQoi(qoiStream(3, 1, {0xfe, 10, 20, 30, 0xfe, 40, 50, 60, 0b00'001001}));
    REQUIRE(image);
    CHECK(image->pixels[2] == Pixel{10, 20, 30, 255});
}

TEST_CASE("decodeQoi finds transparent black in the untouched index") {
    // The index starts zeroed, so encoders use slot 0 for a first pixel of (0, 0, 0, 0).
    const auto image = etude::decodeQoi(qoiStream(1, 1, {0b00'000000}));
    REQUIRE(image);
    CHECK(image->pixels[0] == Pixel{0, 0, 0, 0});
}

TEST_CASE("decodeQoi rejects data without the QOI magic bytes") {
    std::vector<std::uint8_t> bytes = qoiStream(1, 1, {0b11'000000});
    bytes[0] = 'x';
    CHECK_FALSE(etude::decodeQoi(bytes));
}

TEST_CASE("decodeQoi rejects data shorter than header and end marker") {
    CHECK_FALSE(etude::decodeQoi(std::vector<std::uint8_t>{'q', 'o', 'i', 'f'}));
}

TEST_CASE("decodeQoi rejects an image without pixels") {
    CHECK_FALSE(etude::decodeQoi(qoiStream(0, 1, {})));
}

TEST_CASE("decodeQoi rejects pixel data that ends early") {
    CHECK_FALSE(etude::decodeQoi(qoiStream(2, 1, {0xfe, 10, 20, 30})));
}

TEST_CASE("decodeQoi restores an image from the reference encoding") {
    Image image{
        .width = 70,
        .height = 1,
        .pixels = {
            {0, 0, 0, 0},        // QOI_OP_INDEX: slot 0 starts as transparent black
            {10, 20, 30, 128},   // QOI_OP_RGBA: the alpha changes
            {11, 19, 30, 128},   // QOI_OP_DIFF: every channel changes by at most 2
            {21, 27, 35, 128},   // QOI_OP_LUMA: red and blue change about as much as green
            {200, 100, 50, 128}, // QOI_OP_RGB: a large change with the same alpha
            {10, 20, 30, 128},   // QOI_OP_INDEX: seen before
        },
    };
    // QOI_OP_RUN twice, as a run holds at most 62 pixels.
    image.pixels.insert(image.pixels.end(), 64, {10, 20, 30, 128});

    const auto decoded = etude::decodeQoi(encodeQoi(image));
    REQUIRE(decoded);
    CHECK(*decoded == image);
}

TEST_CASE("loadQoi reads the file in binary mode") {
    // Red and green form a Windows line break, which a file read in text mode would shorten to one byte.
    const Image image{
        .width = 1,
        .height = 1,
        .pixels = {{13, 10, 0, 255}},
    };
    const std::vector<std::uint8_t> bytes = encodeQoi(image);
    const std::filesystem::path path = std::filesystem::temp_directory_path() / "etude_load_qoi_test.qoi";
    std::ofstream(path, std::ios::binary)
        .write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));

    const auto loaded = etude::loadQoi(path);
    std::filesystem::remove(path);
    REQUIRE(loaded);
    CHECK(*loaded == image);
}

TEST_CASE("loadQoi names the file it cannot read") {
    const auto image = etude::loadQoi("missing.qoi");
    REQUIRE_FALSE(image);
    CHECK_THAT(image.error(), Catch::Matchers::ContainsSubstring("missing.qoi"));
}
