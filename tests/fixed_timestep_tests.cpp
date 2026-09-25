#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <etude/core/fixed_timestep.h>

#include <chrono>

using namespace std::chrono_literals;
using etude::FixedTimestep;

TEST_CASE("FixedTimestep turns frame time into whole steps") {
    FixedTimestep timestep(100);
    CHECK(timestep.step() == 10ms);
    CHECK(timestep.advance(30ms) == 3);
}

TEST_CASE("FixedTimestep keeps the rest of a frame for the next one") {
    FixedTimestep timestep(100);
    CHECK(timestep.advance(25ms) == 2);
    CHECK(timestep.advance(5ms) == 1);
    CHECK(timestep.advance(9ms) == 0);
}

TEST_CASE("FixedTimestep takes the same number of steps per second at any frame rate") {
    // These frame rates split a second into whole nanoseconds, so the count has to match exactly.
    const int framesPerSecond = GENERATE(20, 50, 125, 200);
    CAPTURE(framesPerSecond);
    FixedTimestep timestep(100);
    const auto frame = FixedTimestep::Duration(1s) / framesPerSecond;
    int steps = 0;
    for (int i = 0; i < framesPerSecond; ++i) {
        steps += timestep.advance(frame);
    }
    CHECK(steps == 100);
}

TEST_CASE("FixedTimestep counts a long stall only up to the longest frame") {
    FixedTimestep timestep(100);
    CHECK(timestep.advance(5s) == 25);
}
