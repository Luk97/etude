#include <catch2/catch_test_macros.hpp>

#include <etude/core/utf8.h>

#include <string>
#include <string_view>

namespace {

    std::string toUtf8(char32_t character) {
        std::string text;
        etude::appendUtf8(text, character);
        return text;
    }

    std::u32string takeAll(std::string_view text) {
        std::u32string characters;
        while (!text.empty()) {
            characters += etude::takeUtf8(text);
        }
        return characters;
    }
}

TEST_CASE("appendUtf8 writes one to four bytes, depending on the character") {
    CHECK(toUtf8(U'A') == "A");
    CHECK(toUtf8(U'ä') == "\xC3\xA4");
    CHECK(toUtf8(U'€') == "\xE2\x82\xAC");
    CHECK(toUtf8(U'😀') == "\xF0\x9F\x98\x80");
}

TEST_CASE("appendUtf8 keeps the text that is already there") {
    std::string text = "Hase";
    etude::appendUtf8(text, U'!');
    CHECK(text == "Hase!");
}

TEST_CASE("takeUtf8 reads the characters of a text one after the other") {
    CHECK(takeAll("Aä€😀") == U"Aä€😀");
}

TEST_CASE("takeUtf8 replaces every byte that does not start a valid character") {
    // A continuation byte without a lead byte, and a euro sign cut short before the z and at the end.
    CHECK(takeAll("\x80z") == U"\uFFFDz");
    CHECK(takeAll("\xE2\x82z") == U"\uFFFD\uFFFDz");
    CHECK(takeAll("\xE2\x82") == U"\uFFFD\uFFFD");
    // A slash in two bytes instead of one, a surrogate and a number beyond U+10FFFF.
    CHECK(takeAll("\xC0\xAF") == U"\uFFFD\uFFFD");
    CHECK(takeAll("\xED\xA0\x80") == U"\uFFFD\uFFFD\uFFFD");
    CHECK(takeAll("\xF4\x90\x80\x80") == U"\uFFFD\uFFFD\uFFFD\uFFFD");
}
