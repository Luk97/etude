#pragma once

#include <cstdint>
#include <utility>

namespace etude {

    /// @brief Names an entity of a world with 20 bits of index and 12 bits of generation. The index is the place of
    /// the entity in the storages of its world. When an entity is destroyed, a later entity gets its index with the
    /// next generation, so handles to the old one no longer count as alive.
    enum class Entity : std::uint32_t {
    };

    inline constexpr std::uint32_t entityIndexBits = 20;
    inline constexpr std::uint32_t entityIndexMask = (1u << entityIndexBits) - 1;

    /// @brief Generations count up to 4095 and then start again at 0. After 4096 reuses of the same index, a very old
    /// handle can therefore look alive again, which is rare enough to accept.
    inline constexpr std::uint32_t entityGenerationMask = (1u << (32 - entityIndexBits)) - 1;

    constexpr Entity makeEntity(std::uint32_t index, std::uint32_t generation) {
        return static_cast<Entity>(generation << entityIndexBits | index);
    }

    constexpr std::uint32_t indexOf(Entity entity) {
        return std::to_underlying(entity) & entityIndexMask;
    }

    constexpr std::uint32_t generationOf(Entity entity) {
        return std::to_underlying(entity) >> entityIndexBits;
    }
}
