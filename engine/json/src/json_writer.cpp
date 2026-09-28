#include <etude/json/json.h>

#include <algorithm>
#include <array>
#include <charconv>
#include <cmath>
#include <format>
#include <string_view>

namespace etude {

    namespace {

        /// @brief Combines lambdas into one callable with an overload per lambda, so std::visit can take one per
        /// alternative.
        template <typename... Lambdas>
        struct Overloaded : Lambdas... {
            using Lambdas::operator()...;
        };

        void writeValue(std::string& text, const Json& json, int depth);

        void writeIndent(std::string& text, int depth) {
            text.append(4 * static_cast<std::size_t>(depth), ' ');
        }

        /// @brief JSON has no NaN and no infinity, so those become null, like in JavaScript.
        void writeNumber(std::string& text, double number) {
            if (!std::isfinite(number)) {
                text += "null";
                return;
            }
            std::array<char, 32> buffer{};
            const std::to_chars_result result = std::to_chars(buffer.data(), buffer.data() + buffer.size(), number);
            text.append(buffer.data(), result.ptr);
        }

        /// @brief Returns the short escape sequence of a character that a JSON string must not contain as it is, or
        /// an empty view.
        std::string_view shortEscape(char character) {
            switch (character) {
                case '"':
                    return "\\\"";
                case '\\':
                    return "\\\\";
                case '\b':
                    return "\\b";
                case '\f':
                    return "\\f";
                case '\n':
                    return "\\n";
                case '\r':
                    return "\\r";
                case '\t':
                    return "\\t";
                default:
                    return {};
            }
        }

        /// @brief Writes the string in quotes. Bytes beyond ASCII stay as they are, because JSON text is UTF-8.
        void writeString(std::string& text, std::string_view string) {
            text += '"';
            for (const char character : string) {
                if (const std::string_view escape = shortEscape(character); !escape.empty()) {
                    text += escape;
                } else if (static_cast<unsigned char>(character) < 0x20) {
                    text += std::format("\\u{:04x}", static_cast<int>(character));
                } else {
                    text += character;
                }
            }
            text += '"';
        }

        void writeArray(std::string& text, const Json::Array& array, int depth) {
            if (array.empty()) {
                text += "[]";
                return;
            }

            // Plain values such as the two numbers of a position fit on one line, nested values get a line each.
            const bool flat = std::ranges::none_of(array, [](const Json& element) {
                return element.isArray() || element.isObject();
            });
            const char* separator = flat ? "" : "\n";
            text += '[';
            for (const Json& element : array) {
                text += separator;
                separator = flat ? ", " : ",\n";
                if (!flat) {
                    writeIndent(text, depth + 1);
                }
                writeValue(text, element, depth + 1);
            }
            if (!flat) {
                text += '\n';
                writeIndent(text, depth);
            }
            text += ']';
        }

        void writeObject(std::string& text, const Json::Object& object, int depth) {
            if (object.empty()) {
                text += "{}";
                return;
            }

            const char* separator = "\n";
            text += '{';
            for (const auto& [key, member] : object) {
                text += separator;
                separator = ",\n";
                writeIndent(text, depth + 1);
                writeString(text, key);
                text += ": ";
                writeValue(text, member, depth + 1);
            }
            text += '\n';
            writeIndent(text, depth);
            text += '}';
        }

        void writeValue(std::string& text, const Json& json, int depth) {
            json.visit(
                Overloaded{
                    [&](std::nullptr_t) { text += "null"; },
                    [&](bool boolean) { text += boolean ? "true" : "false"; },
                    [&](double number) { writeNumber(text, number); },
                    [&](const std::string& string) { writeString(text, string); },
                    [&](const Json::Array& array) { writeArray(text, array, depth); },
                    [&](const Json::Object& object) { writeObject(text, object, depth); },
                }
            );
        }
    }

    std::string writeJson(const Json& json) {
        std::string text;
        writeValue(text, json, 0);
        return text;
    }
}
