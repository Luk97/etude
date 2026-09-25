#pragma once

#include <algorithm>
#include <chrono>

namespace etude {

    /// @brief Turns frame times of any length into a whole number of fixed simulation steps.
    /// Every frame adds its duration to an account, and each full step duration in the account is one step.
    /// The reset stays in the account for the next frame, so the simulation keeps its rate at any frame rate.
    class FixedTimestep {
    public:
        using Duration = std::chrono::nanoseconds;

        /// @brief Longest frame that counts in full. A longer stall, for example while the window is dragged, would
        /// otherwise demand so many catch-up steps that the next frame stalls as well, the spiral of death.
        static constexpr Duration maxFrame = std::chrono::milliseconds(250);

        explicit FixedTimestep(int stepsPerSecond) : stepDuration(Duration(std::chrono::seconds(1)) / stepsPerSecond) {}

        /// @brief Adds the duration of one frame and returns how many simulations steps are due now.
        int advance(Duration frame) {
            account += std::min(frame, maxFrame);
            const auto steps = account / stepDuration;
            account %= stepDuration;
            return static_cast<int>(steps);
        }

        Duration step() const {
            return stepDuration;
        }

    private:
        Duration stepDuration;
        Duration account{};
    };
}
