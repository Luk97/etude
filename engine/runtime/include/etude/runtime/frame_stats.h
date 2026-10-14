#pragma once

namespace etude {

    /// @brief How fast the game loop ran over the last full second: frames and simulation steps per second, and how
    /// long a frame took on average, on the CPU from the game up to handing the frame over, and on the GPU.
    struct FrameStats {
        double framesPerSecond = 0.0;
        double stepsPerSecond = 0.0;
        double cpuMilliseconds = 0.0;
        double gpuMilliseconds = 0.0;
    };
}
