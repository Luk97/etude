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

    private:
        std::chrono::steady_clock::time_point start;
    };
}
