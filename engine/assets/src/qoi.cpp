#include <etude/assets/qoi.h>

#include "file.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <format>
#include <vector>

namespace etude {

    namespace {

        constexpr std::array<std::uint8_t, 4> magic{'q', 'o', 'i', 'f'};
        constexpr std::size_t headerSize = 14;
        constexpr std::size_t endMarkerSize = 8;

        /// @brief Most pixels a header may announce, the same limit as in the reference decoder. Without it, a broken
        /// header could request gigabytes of memory.
        constexpr std::uint64_t maxPixels = 400'000'000;

        constexpr std::uint8_t opRgb = 0xfe;
        constexpr std::uint8_t opRgba = 0xff;
        constexpr std::uint8_t opIndex = 0x00;
        constexpr std::uint8_t opDiff = 0x40;
        constexpr std::uint8_t opLuma = 0x80;
        constexpr std::uint8_t opRun = 0xc0;
        constexpr std::uint8_t tagMask = 0xc0;

        /// @brief Reads a 32-bit number that starts with its most significant byte, as the QOI header stores it.
        std::uint32_t readBigEndian(std::span<const std::uint8_t, 4> bytes) {
            std::uint32_t value = 0;
            for (const std::uint8_t byte : bytes) {
                value = value << 8 | byte;
            }
            return value;
        }

        /// @brief Returns the slot of a pixel in the table of pixels seen so far, as the QOI format defines it.
        std::size_t slotOf(Pixel pixel) {
            return (pixel.r * 3 + pixel.g * 5 + pixel.b * 7 + pixel.a * 11) % 64;
        }

        /// @brief Adds a difference to a channel. Channels wrap around like 8-bit numbers, so 0 - 1 gives 255.
        std::uint8_t addWrapping(std::uint8_t channel, int difference) {
            return static_cast<std::uint8_t>(channel + difference);
        }
    }

    std::expected<Image, std::string> decodeQoi(std::span<const std::uint8_t> bytes) {
        if (bytes.size() < headerSize + endMarkerSize) {
            return std::unexpected("the data is too short for a QOI image");
        }
        if (!std::ranges::equal(bytes.first<4>(), magic)) {
            return std::unexpected("the data does not start with the QOI magic bytes");
        }
        const std::uint32_t width = readBigEndian(bytes.subspan<4, 4>());
        const std::uint32_t height = readBigEndian(bytes.subspan<8, 4>());
        if (width == 0 || height == 0 || std::uint64_t{width} * height > maxPixels) {
            return std::unexpected(std::format("a size of {} x {} pixels is not supported", width, height));
        }

        Image image{
            .width = static_cast<int>(width),
            .height = static_cast<int>(height),
            .pixels = std::vector<Pixel>(std::size_t{width} * height),
        };

        // The chunks lie between header and end marker. A chunk has at most five bytes, so one that starte before the
        // end marker can be read without further checks, at worst it reads into the marker.
        const std::size_t chunksEnd = bytes.size() - endMarkerSize;
        std::size_t position = headerSize;
        std::array<Pixel, 64> seen{};
        Pixel pixel{
            .a = 255,
        };
        int run = 0;
        for (Pixel& decoded : image.pixels) {
            if (run > 0) {
                --run;
            } else if (position < chunksEnd) {
                const std::uint8_t tag = bytes[position++];
                if (tag == opRgb) {
                    pixel.r = bytes[position++];
                    pixel.g = bytes[position++];
                    pixel.b = bytes[position++];
                } else if (tag == opRgba) {
                    pixel.r = bytes[position++];
                    pixel.g = bytes[position++];
                    pixel.b = bytes[position++];
                    pixel.a = bytes[position++];
                } else if ((tag & tagMask) == opIndex) {
                    pixel = seen[tag];
                } else if ((tag & tagMask) == opDiff) {
                    pixel.r = addWrapping(pixel.r, ((tag >> 4) & 0x03) - 2);
                    pixel.g = addWrapping(pixel.g, ((tag >> 2) & 0x03) - 2);
                    pixel.b = addWrapping(pixel.b, (tag & 0x03) - 2);
                } else if ((tag & tagMask) == opLuma) {
                    const std::uint8_t redBlue = bytes[position++];
                    const int green = (tag & 0x3f) - 32;
                    pixel.r = addWrapping(pixel.r, green + (redBlue >> 4) - 8);
                    pixel.g = addWrapping(pixel.g, green);
                    pixel.b = addWrapping(pixel.b, green + (redBlue & 0x0f) - 8);
                } else if ((tag & tagMask) == opRun) {
                    run = tag & 0x3f;
                }
                seen[slotOf(pixel)] = pixel;
            } else {
                return std::unexpected("the pixel data ends early");
            }
            decoded = pixel;
        }
        return image;
    }

    std::expected<Image, std::string> loadQoi(const std::filesystem::path& path) {
        return readFile(path).and_then(decodeQoi).transform_error([&](const std::string& error) {
            return std::format("Cannot load {}: {}.", path.string(), error);
        });
    }
}
