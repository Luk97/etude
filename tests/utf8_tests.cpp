#include <catch2/catch_test_macros.hpp>

#include <etude/core/utf8.h>

#include <string>

namespace {

    std::string toUtf8(char32_t character) {
        std::string text;
        etude::appendUtf8(text, character);
        return text;
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
