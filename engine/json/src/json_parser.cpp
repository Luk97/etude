#include <etude/json/json.h>

#include <algorithm>
#include <charconv>
#include <cstdint>
#include <format>
#include <optional>
#include <system_error>
#include <utility>

namespace etude {

    namespace {

        /// @brief Deeper nesting is refused, so that a hostile file cannot overflow the stack of the parser.
        constexpr int maxDepth = 128;

        constexpr std::string_view whitespace = " \t\n\r";

        bool isDigit(char character) {
            return character >= '0' && character <= '9';
        }

        /// @brief Appends the character in UTF-8: one byte for ASCII, up to four for the rest of Unicode.
        void appendUtf8(std::string& text, char32_t character) {
            if (character < 0x80) {
                text += static_cast<char>(character);
            } else if (character < 0x800) {
                text += static_cast<char>(0xc0 | (character >> 6));
                text += static_cast<char>(0x80 | (character & 0x3f));
            } else if (character < 0x10000) {
                text += static_cast<char>(0xe0 | (character >> 12));
                text += static_cast<char>(0x80 | ((character >> 6) & 0x3f));
                text += static_cast<char>(0x80 | (character & 0x3f));
            } else {
                text += static_cast<char>(0xf0 | (character >> 18));
                text += static_cast<char>(0x80 | ((character >> 12) & 0x3f));
                text += static_cast<char>(0x80 | ((character >> 6) & 0x3f));
                text += static_cast<char>(0x80 | (character & 0x3f));
            }
        }

        /// @brief Reads JSON by recursive descent, with one function per rule of the grammar. The first error stops
        /// the parse: fail keeps it with its position and returns nothing, which every caller passes on.
        class Parser {
        public:
            explicit Parser(std::string_view text) : text(text) {}

            std::expected<Json, JsonError> parse() {
                skipWhitespace();
                std::optional<Json> value = parseValue(0);
                skipWhitespace();
                if (value && position < text.size()) {
                    fail("expected the end of the text");
                    value.reset();
                }
                if (!value) {
                    return std::unexpected(std::move(*error));
                }
                return std::move(*value);
            }

        private:
            std::optional<Json> parseValue(int depth) {
                if (position == text.size()) {
                    return fail("expected a value, but the text ends");
                }
                switch (text[position]) {
                    case '{':
                        return parseObject(depth);
                    case '[':
                        return parseArray(depth);
                    case '"':
                        return parseString().transform([](std::string string) { return Json(std::move(string)); });
                    case 't':
                        return parseWord("true", true);
                    case 'f':
                        return parseWord("false", false);
                    case 'n':
                        return parseWord("null", nullptr);
                    default:
                        return parseNumber();
                }
            }

            std::optional<Json> parseObject(int depth) {
                if (depth == maxDepth) {
                    return fail("the values are nested too deeply");
                }
                ++position;
                skipWhitespace();

                Json::Object object;
                if (accept('}')) {
                    return object;
                }
                while (true) {
                    if (position == text.size() || text[position] != '"') {
                        return fail("expected a key in quotes");
                    }
                    const std::size_t keyStart = position;
                    std::optional<std::string> key = parseString();
                    if (!key) {
                        return std::nullopt;
                    }

                    // Objects in scene files are small, so a linear search for duplicates is fast enough.
                    if (std::ranges::any_of(object, [&](const auto& member) { return member.first == *key; })) {
                        position = keyStart;
                        return fail(std::format("the key \"{}\" appears twice", *key));
                    }

                    skipWhitespace();
                    if (!accept(':')) {
                        return fail("expected ':' after the key");
                    }
                    skipWhitespace();
                    std::optional<Json> value = parseValue(depth + 1);
                    if (!value) {
                        return std::nullopt;
                    }
                    object.emplace_back(std::move(*key), std::move(*value));

                    skipWhitespace();
                    if (accept('}')) {
                        return object;
                    }
                    if (!accept(',')) {
                        return fail("expected ',' or '}'");
                    }
                    skipWhitespace();
                }
            }

            std::optional<Json> parseArray(int depth) {
                if (depth == maxDepth) {
                    return fail("the values are nested too deeply");
                }
                ++position;
                skipWhitespace();

                Json::Array array;
                if (accept(']')) {
                    return array;
                }
                while (true) {
                    std::optional<Json> element = parseValue(depth + 1);
                    if (!element) {
                        return std::nullopt;
                    }
                    array.push_back(std::move(*element));

                    skipWhitespace();
                    if (accept(']')) {
                        return array;
                    }
                    if (!accept(',')) {
                        return fail("expected ',' or ']'");
                    }
                    skipWhitespace();
                }
            }

            /// @brief Reads a string in quotes and resolves its escapes. Bytes beyond ASCII stay as they are, because
            /// JSON text is UTF-8.
            std::optional<std::string> parseString() {
                ++position;
                std::string string;
                while (true) {
                    if (position == text.size()) {
                        return fail("the string has no closing quote");
                    }
                    const char character = text[position];
                    if (character == '"') {
                        ++position;
                        return string;
                    }
                    if (static_cast<unsigned char>(character) < 0x20) {
                        return fail("control characters in strings have to be escaped");
                    }
                    ++position;
                    if (character != '\\') {
                        string += character;
                        continue;
                    }
                    const std::optional<char32_t> escaped = parseEscape();
                    if (!escaped) {
                        return std::nullopt;
                    }
                    appendUtf8(string, *escaped);
                }
            }

            /// @brief Reads the rest of an escape sequence after its backslash and returns the character it stands for.
            std::optional<char32_t> parseEscape() {
                if (position == text.size()) {
                    return fail("the string has no closing quote");
                }
                switch (text[position++]) {
                    case '"':
                        return U'"';
                    case '\\':
                        return U'\\';
                    case '/':
                        return U'/';
                    case 'b':
                        return U'\b';
                    case 'f':
                        return U'\f';
                    case 'n':
                        return U'\n';
                    case 'r':
                        return U'\r';
                    case 't':
                        return U'\t';
                    case 'u':
                        return parseUnicodeEscape();
                    default:
                        --position;
                        return fail("unknown escape sequence");
                }
            }

            /// @brief Reads the hex digits of a \u escape. Characters beyond the first 65536 take two escapes, a high
            /// surrogate followed by a low one, which only together name the character.
            std::optional<char32_t> parseUnicodeEscape() {
                const std::optional<char32_t> unit = parseHexDigits();
                if (!unit) {
                    return std::nullopt;
                }
                if (*unit >= 0xdc00 && *unit <= 0xdfff) {
                    return fail("a low surrogate needs a high surrogate before it");
                }
                if (*unit < 0xd800 || *unit > 0xdbff) {
                    return unit;
                }

                if (!accept('\\') || !accept('u')) {
                    return fail("a high surrogate needs a low surrogate after it");
                }
                const std::optional<char32_t> low = parseHexDigits();
                if (!low) {
                    return std::nullopt;
                }
                if (*low < 0xdc00 || *low > 0xdfff) {
                    return fail("a high surrogate needs a low surrogate after it");
                }
                return 0x10000 + ((*unit - 0xd800) << 10) + (*low - 0xdc00);
            }

            std::optional<char32_t> parseHexDigits() {
                const char* begin = text.data() + position;
                const char* end = text.data() + std::min(position + 4, text.size());
                std::uint32_t value = 0;
                if (std::from_chars(begin, end, value, 16).ptr != begin + 4) {
                    return fail("expected four hex digits");
                }
                position += 4;
                return static_cast<char32_t>(value);
            }

            /// @brief Checks the number against the grammar of JSON first, which is stricter than std::from_chars: no
            /// plus sign, no leading zeros, digits on both sides of the point, and no words like NaN.
            std::optional<Json> parseNumber() {
                const std::size_t start = position;
                const bool negative = accept('-');
                if (!accept('0') && !acceptDigits()) {
                    return fail(negative ? "expected a digit after the minus sign" : "expected a value");
                }
                if (accept('.') && !acceptDigits()) {
                    return fail("expected a digit after the decimal point");
                }
                if (accept('e') || accept('E')) {
                    if (!accept('+')) {
                        accept('-');
                    }
                    if (!acceptDigits()) {
                        return fail("expected a digit in the exponent");
                    }
                }

                double number = 0.0;
                const std::from_chars_result result =
                    std::from_chars(text.data() + start, text.data() + position, number);
                if (result.ec == std::errc::result_out_of_range) {
                    position = start;
                    return fail("the number is beyond the range of a double");
                }
                return number;
            }

            std::optional<Json> parseWord(std::string_view word, Json value) {
                if (!text.substr(position).starts_with(word)) {
                    return fail("expected a value");
                }
                position += word.size();
                return value;
            }

            void skipWhitespace() {
                while (position < text.size() && whitespace.contains(text[position])) {
                    ++position;
                }
            }

            bool accept(char expected) {
                if (position == text.size() || text[position] != expected) {
                    return false;
                }
                ++position;
                return true;
            }

            /// @brief Skips all digits at the current position and says whether there was at least one.
            bool acceptDigits() {
                const std::size_t start = position;
                while (position < text.size() && isDigit(text[position])) {
                    ++position;
                }
                return position > start;
            }

            /// @brief Keeps the error together with the line and column of the current position. Returns nothing, so a
            /// parse function can end with return fail(...).
            std::nullopt_t fail(std::string message) {
                const std::string_view before = text.substr(0, position);
                const std::size_t lineStart = before.rfind('\n');
                const std::size_t column = lineStart == std::string_view::npos ? position + 1 : position - lineStart;
                error = JsonError{
                    .line = static_cast<int>(std::ranges::count(before, '\n')) + 1,
                    .column = static_cast<int>(column),
                    .message = std::move(message),
                };
                return std::nullopt;
            }

            std::string_view text;
            std::size_t position = 0;
            std::optional<JsonError> error;
        };
    }

    std::expected<Json, JsonError> parseJson(std::string_view text) {
        return Parser(text).parse();
    }
}
