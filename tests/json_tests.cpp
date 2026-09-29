#include <catch2/catch_test_macros.hpp>

#include <etude/json/json.h>

#include <limits>
#include <string>
#include <string_view>

using etude::Json;

namespace {

    bool refuses(std::string_view text) {
        return !etude::parseJson(text).has_value();
    }
}

TEST_CASE("Json keeps a string literal as a string instead of a boolean") {
    CHECK(Json("text").isString());
    CHECK_FALSE(Json("text").isBool());
}

TEST_CASE("Json compares by value, so 1 equals 1.0 but not the string 1") {
    CHECK(Json(Json::Array{1, "two"}) == Json(Json::Array{1.0, "two"}));
    CHECK(Json(1) != Json("1"));
}

TEST_CASE("Json hands out what it holds, but only as the right type") {
    const Json number = 2.5;
    REQUIRE(number.tryGet<double>() != nullptr);
    CHECK(*number.tryGet<double>() == 2.5);
    CHECK(number.tryGet<std::string>() == nullptr);
}

TEST_CASE("Json finds the members of an object by their key") {
    const Json object = Json::Object{{"a", 1}, {"b", "two"}};
    REQUIRE(object.find("b") != nullptr);
    CHECK(*object.find("b") == Json("two"));
    CHECK(object.find("c") == nullptr);
    CHECK(Json(1).find("a") == nullptr);
}

TEST_CASE("writeJson writes null, booleans and strings") {
    CHECK(etude::writeJson(nullptr) == "null");
    CHECK(etude::writeJson(true) == "true");
    CHECK(etude::writeJson(false) == "false");
    CHECK(etude::writeJson("ÉTUDE") == "\"ÉTUDE\"");
}

TEST_CASE("writeJson writes numbers in the shortest form that reads back exactly") {
    CHECK(etude::writeJson(640) == "640");
    CHECK(etude::writeJson(0.1) == "0.1");
    CHECK(etude::writeJson(-2.5f) == "-2.5");
    CHECK(etude::writeJson(1e21) == "1e+21");
}

TEST_CASE("writeJson writes numbers that JSON cannot express as null") {
    CHECK(etude::writeJson(std::numeric_limits<double>::quiet_NaN()) == "null");
    CHECK(etude::writeJson(std::numeric_limits<double>::infinity()) == "null");
}

TEST_CASE("writeJson escapes quotes, backslashes and control characters") {
    CHECK(etude::writeJson("say \"hi\"\\\n\t\x01") == R"("say \"hi\"\\\n\t\u0001")");
}

TEST_CASE("writeJson writes empty arrays and objects on one line") {
    CHECK(etude::writeJson(Json::Array{}) == "[]");
    CHECK(etude::writeJson(Json::Object{}) == "{}");
}

TEST_CASE("writeJson keeps arrays of plain values on one line") {
    CHECK(etude::writeJson(Json::Array{1, 2.5, "three", nullptr}) == R"([1, 2.5, "three", null])");
}

TEST_CASE("writeJson indents nested values and keeps the order of the members") {
    const Json player = Json::Object{
        {"name", "Player"},
        {"position", Json::Array{640, 360}},
    };
    const Json scene = Json::Object{
        {"version", 1},
        {"entities", Json::Array{player}},
    };
    CHECK(etude::writeJson(scene) == R"({
    "version": 1,
    "entities": [
        {
            "name": "Player",
            "position": [640, 360]
        }
    ]
})");
}

TEST_CASE("parseJson reads null, booleans and numbers") {
    CHECK(etude::parseJson("null") == Json(nullptr));
    CHECK(etude::parseJson("true") == Json(true));
    CHECK(etude::parseJson("false") == Json(false));
    CHECK(etude::parseJson("-0.5") == Json(-0.5));
    CHECK(etude::parseJson("1e3") == Json(1000));
}

TEST_CASE("parseJson resolves escapes and \\u sequences into UTF-8") {
    CHECK(etude::parseJson(R"("a\"b\\c\/d\n")") == Json("a\"b\\c/d\n"));
    CHECK(etude::parseJson(R"("\u00c9TUDE")") == Json("ÉTUDE"));
    CHECK(etude::parseJson(R"("\ud83d\ude00")") == Json("\xf0\x9f\x98\x80"));
}

TEST_CASE("parseJson reads nested values and keeps the order of the members") {
    const Json expected = Json::Object{
        {"version", 1},
        {"entities", Json::Array{Json::Object{{"name", "Player"}}}},
        {"empty", Json::Object{}},
    };
    CHECK(etude::parseJson(" {\"version\": 1, \"entities\": [{\"name\": \"Player\"}], \"empty\": {}}\n") == expected);
}

TEST_CASE("parseJson reads back exactly what writeJson wrote") {
    const Json original = Json::Object{
        {"numbers", Json::Array{0.1, 1e21, -2.5, 123456789.125, 5e-324}},
        {"text", "line\nbreak \"quoted\" \x01 ÉTUDE"},
        {"nested", Json::Array{Json::Array{}, Json::Object{{"flag", false}}, nullptr}},
    };
    CHECK(etude::parseJson(etude::writeJson(original)) == original);
}

TEST_CASE("parseJson reports the line and column of an error") {
    const auto result = etude::parseJson("{\n    \"a\": 1,\n    \"b\" 2\n}");
    REQUIRE_FALSE(result);
    CHECK(result.error().line == 3);
    CHECK(result.error().column == 9);
}

TEST_CASE("parseJson refuses a key that appears twice") {
    const auto result = etude::parseJson(R"({"a": 1, "a": 2})");
    REQUIRE_FALSE(result);
    CHECK(result.error().column == 10);
}

TEST_CASE("parseJson refuses numbers that JSON does not allow") {
    CHECK(refuses("01"));
    CHECK(refuses("+1"));
    CHECK(refuses(".5"));
    CHECK(refuses("1."));
    CHECK(refuses("1e"));
    CHECK(refuses("-"));
    CHECK(refuses("0x10"));
    CHECK(refuses("NaN"));
    CHECK(refuses("1e400"));
}

TEST_CASE("parseJson names numbers beyond the range of a double, too large or too small") {
    for (const std::string_view text : {"1e400", "-1e400", "1e-400"}) {
        const auto result = etude::parseJson(text);
        REQUIRE_FALSE(result);
        CHECK(result.error().message == "the number is beyond the range of a double");
    }
}

TEST_CASE("parseJson refuses broken strings") {
    CHECK(refuses("\"no end"));
    CHECK(refuses("\"raw\ttab\""));
    CHECK(refuses(R"("\q")"));
    CHECK(refuses(R"("\u12")"));
    CHECK(refuses(R"("\ud83d")"));
    CHECK(refuses(R"("\ude00")"));
}

TEST_CASE("parseJson refuses trailing commas, missing ends and text after the value") {
    CHECK(refuses("[1, 2,]"));
    CHECK(refuses(R"({"a": 1,})"));
    CHECK(refuses("[1, 2"));
    CHECK(refuses(R"({"a": 1)"));
    CHECK(refuses("1 2"));
    CHECK(refuses(""));
}

TEST_CASE("parseJson refuses values nested too deeply instead of overflowing the stack") {
    CHECK_FALSE(refuses(std::string(100, '[') + std::string(100, ']')));
    CHECK(refuses(std::string(10000, '[') + std::string(10000, ']')));
}
