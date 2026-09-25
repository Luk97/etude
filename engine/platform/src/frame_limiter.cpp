#include <etude/platform/frame_limiter.h>

#include <windows.h>

namespace etude {

    namespace {

        /// @brief Windows measures the due time of a timer in ticks of 100 nanoseconds.
        using Ticks = std::chrono::duration<LONGLONG, std::ratio<1, 10'000'000>>;
    }

    FrameLimiter::FrameLimiter(int framesPerSecond)
        : timer(CreateWaitableTimerExW(nullptr, nullptr, CREATE_WAITABLE_TIMER_HIGH_RESOLUTION, TIMER_ALL_ACCESS)),
          nextFrame(std::chrono::steady_clock::now()) {
        setFramesPerSecond(framesPerSecond);
    }

    FrameLimiter::~FrameLimiter() {
        CloseHandle(timer);
    }

    void FrameLimiter::setFramesPerSecond(int framesPerSecond) {
        frameDuration = std::chrono::steady_clock::duration(std::chrono::seconds(1)) / framesPerSecond;
    }

    void FrameLimiter::wait() {
        nextFrame += frameDuration;
        const auto now = std::chrono::steady_clock::now();
        if (nextFrame <= now) {
            nextFrame = now;
            return;
        }

        // A negative due time counts from now instead of naming a point on the calendar.
        LARGE_INTEGER dueTime{};
        dueTime.QuadPart = -std::chrono::duration_cast<Ticks>(nextFrame - now).count();
        SetWaitableTimerEx(timer, &dueTime, 0, nullptr, nullptr, nullptr, 0);
        WaitForSingleObject(timer, INFINITE);
    }
}
