#ifndef GAME_CHEST_HPP
#define GAME_CHEST_HPP

#include "Global.hpp"
#include "Vector2D.hpp"

/**
 * @file Chest.hpp
 * @brief Interactive loot chest spanning 2x1 tiles on the map.
 */
struct Chest {
    static constexpr float WIDTH_PX            = TILE_SIZE * 2.0f;
    static constexpr float HEIGHT_PX           = TILE_SIZE;
    static constexpr float INTERACT_DISTANCE   = TILE_SIZE * 1.8f;
    static constexpr float HEAL_AMOUNT         = 30.0f;
    static constexpr float STAMINA_BONUS       = 20.0f;
    static constexpr int   MIMIC_CHANCE_MODULO = 4; ///< 1 in 4 (25%) chance to spawn an enemy.

    Vector2D position; ///< Top-left world position in pixels.
    bool     opened{false};

    [[nodiscard]] Vector2D center() const noexcept {
        return { position.x + WIDTH_PX * 0.5f, position.y + HEIGHT_PX * 0.5f };
    }
};

#endif // GAME_CHEST_HPP
