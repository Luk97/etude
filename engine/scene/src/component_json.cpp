#include <etude/scene/component_json.h>

#include <array>
#include <charconv>
#include <cmath>
#include <limits>

namespace etude {

    Json toJson(float value) {
        std::array<char, 32> digits{};
        const char* end = std::to_chars(digits.data(), digits.data() + digits.size(), value).ptr;
        double number = 0.0;
        std::from_chars(digits.data(), end, number);

        // For a few floats such as 7.038531e-26, the shortest digits lie so close to the middle between two floats
        // that their double turns into the other one. Those keep the exact value of the float instead.
        return static_cast<float>(number) == value ? number : static_cast<double>(value);
    }

    Json toJson(int value) {
        return value;
    }

    Json toJson(const std::string& value) {
        return value;
    }

    Json toJson(Vec2 value) {
        return Json::Array{toJson(value.x), toJson(value.y)};
    }

    std::expected<void, std::string> fromJson(const Json& json, float& value) {
        const double* number = json.tryGet<double>();

        // Numbers just above the largest float still round down to it, only larger ones become infinite.
        if (number == nullptr || std::isinf(static_cast<float>(*number))) {
            return std::unexpected("expected a number in the range of a float");
        }
        value = static_cast<float>(*number);
        return {};
    }

    std::expected<void, std::string> fromJson(const Json& json, int& value) {
        const double* number = json.tryGet<double>();
        if (number == nullptr || *number != std::trunc(*number) || *number < std::numeric_limits<int>::min() ||
            *number > std::numeric_limits<int>::max()) {
            return std::unexpected("expected a whole number in the range of an int");
        }
        value = static_cast<int>(*number);
        return {};
    }

    std::expected<void, std::string> fromJson(const Json& json, std::string& value) {
        const std::string* string = json.tryGet<std::string>();
        if (string == nullptr) {
            return std::unexpected("expected a string");
        }
        value = *string;
        return {};
    }

    std::expected<void, std::string> fromJson(const Json& json, Vec2& value) {
        const Json::Array* array = json.tryGet<Json::Array>();
        if (array == nullptr || array->size() != 2) {
            return std::unexpected("expected an array of two numbers");
        }
        return fromJson((*array)[0], value.x).and_then([&] { return fromJson((*array)[1], value.y); });
    }
}
