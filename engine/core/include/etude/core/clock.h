#pragma once

#include <chrono>

namespace etude {

    /// @brief Monotonic stopwatch that measures the time since its construction or the last reset.
    /// Changes of the system time do not affect it.
    class Clock {
    public:
        Clock() : start(std::chrono::steady_clock::now()) {}

        double elapsedSeconds() const {
            return std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
        }

        void reset() {
            start = std::chrono::steady_clock::now();
        }

        /// @brief Returns the time since construction or the last restart and start measuring again from now.
        /// Measuring and restarting in one call loses no time in between, which suits measuring frame times.
        std::chrono::steady_clock::duration restart() {
            const auto now = std::chrono::steady_clock::now();
            const auto elapsed = now - start;
            start = now;
            return elapsed;
        }

    private:
        std::chrono::steady_clock::time_point start;
    };
}
