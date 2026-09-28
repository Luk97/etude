#pragma once

#include <concepts>
#include <cstddef>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace etude {

    /// @brief A JSON value: null, a boolean, a number, a string, an array or an object. Objects keep their members in
    /// the order in which they were added, so that a saved file only changes where its content changes.
    class Json {
    public:
        using Array = std::vector<Json>;
        using Object = std::vector<std::pair<std::string, Json>>;

        Json() = default;
        Json(std::nullptr_t) {}
        Json(bool boolean) : value(boolean) {}

        /// @brief Stores every number as a double, just like JavaScript. Integers stay exact up to 2^53.
        template <typename Number>
            requires std::is_arithmetic_v<Number> && (!std::same_as<Number, bool>)
        Json(Number number) : value(static_cast<double>(number)) {}

        Json(std::string string) : value(std::move(string)) {}

        Json(const char* string) : value(std::string(string)) {}

        Json(Array array) : value(std::move(array)) {}
        Json(Object object) : value(std::move(object)) {}

        bool isNull() const {
            return std::holds_alternative<std::nullptr_t>(value);
        }

        bool isBool() const {
            return std::holds_alternative<bool>(value);
        }

        bool isNumber() const {
            return std::holds_alternative<double>(value);
        }

        bool isString() const {
            return std::holds_alternative<std::string>(value);
        }

        bool isArray() const {
            return std::holds_alternative<Array>(value);
        }

        bool isObject() const {
            return std::holds_alternative<Object>(value);
        }

        /// @brief Calls the visitor with whatever the value holds, just like std::visit.
        template <typename Visitor>
        decltype(auto) visit(Visitor&& visitor) const {
            return std::visit(std::forward<Visitor>(visitor), value);
        }

        bool operator==(const Json&) const = default;

    private:
        std::variant<std::nullptr_t, bool, double, std::string, Array, Object> value;
    };

    /// @brief Writes the value as JSON text with one member per line and four spaces per level, so that scene files
    /// stay readable and their diffs small. Arrays of plain values, such as a position, stay on one line. Numbers take
    /// the shortest form that reads back to the same double. The text does not end with a line break.
    std::string writeJson(const Json& json);
}
