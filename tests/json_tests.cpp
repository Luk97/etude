#include <catch2/catch_test_macros.hpp>

#include <etude/json/json.h>

#include <limits>

using etude::Json;

TEST_CASE("Json keeps a string literal as a string instead of a boolean") {
    CHECK(Json("text").isString());
    CHECK_FALSE(Json("text").isBool());
}

TEST_CASE("Json compares by value, so 1 equals 1.0 but not the string 1") {
    CHECK(Json(Json::Array{1, "two"}) == Json(Json::Array{1.0, "two"}));
    CHECK(Json(1) != Json("1"));
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
