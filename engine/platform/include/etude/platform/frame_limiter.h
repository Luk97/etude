#pragma once

#include <chrono>

namespace etude {

    /// @brief Limits the frame rate by waiting until the next frame is due.
    /// It waits on a high-resolution waitable timer, because Sleep and std::this_thread::sleep_until only wake up
    /// in steps of about 15.6 ms on Windows, which turns 60 frames per second into about 32.
    class FrameLimiter {
    public:
        explicit FrameLimiter(int framesPerSecond);
        ~FrameLimiter();

        FrameLimiter(const FrameLimiter&) = delete;
        FrameLimiter& operator=(const FrameLimiter&) = delete;

        /// @brief Changes the frame rate, starting with the next frame.
        void setFramesPerSecond(int framesPerSecond);

        /// @brief Waits until the next frame is due. If the frame took longer than planned, it returns at once
        /// and plans the following frames from now, instead of rushing through frames to catch up.
        void wait();

    private:
        /// @brief Win32 handle of the timer. HANDLE is a void pointer, so this header does not need windows.h.
        void* timer;
        std::chrono::steady_clock::duration frameDuration;
        std::chrono::steady_clock::time_point nextFrame;
    };
}
