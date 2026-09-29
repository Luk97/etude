#pragma once

#include <cstdint>

namespace etude {

    /// @brief Names a texture that the renderer keeps on the GPU. As its own type it cannot be mixed up with other
    /// numbers, and it stays small enough to be copied everywhere. TextureId{} names a white texture that every
    /// renderer provides, so that a sprite without a texture of its own shows a plain color.
    enum class TextureId : std::uint32_t {
    };
}
