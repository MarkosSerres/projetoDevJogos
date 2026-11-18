#ifndef GAME_PLAYER_HPP
#define GAME_PLAYER_HPP

#include "Entity.hpp"
#include "Vector2D.hpp"


class TileMap;

/**
 * @file Player.hpp
 * @brief Playable character entity with grid-aware movement and vision metrics.
 *
 * Player extends Entity with movement speed, vision radius (used by the
 * darkness/lighting pass), and tile-map collision resolution so the character
 * slides smoothly along walls without clipping into solid tiles.
 */
struct Player : Entity {
    static constexpr float DEFAULT_SPEED         = 350.0f;
    static constexpr float SPRINT_MULTIPLIER     = 1.6f;
    static constexpr float DEFAULT_VISION_RADIUS = 500.0f;
    static constexpr float INITIAL_HP            = 100.0f;
    static constexpr float INITIAL_STAMINA       = 100.0f;
    static constexpr float STAMINA_DRAIN_RATE    = 35.0f;
    static constexpr float STAMINA_REGEN_RATE    = 20.0f;
    static constexpr float WALL_SLIDE_FACTOR     = 0.65f;

    float speed{DEFAULT_SPEED};                ///< Movement speed in pixels per second.
    float visionRadius{DEFAULT_VISION_RADIUS}; ///< Light radius around the player in pixels.
    float hp{INITIAL_HP};
    float maxHp{INITIAL_HP};
    float stamina{INITIAL_STAMINA};
    float maxStamina{INITIAL_STAMINA};

    Player();

    /**
     * @brief Moves the player along @p direction for @p dt seconds, resolving
     *        collisions against solid tiles in @p tileMap.
     * @param direction Desired movement vector (normalized internally if non-zero).
     * @param dt        Elapsed frame time in seconds.
     * @param tileMap   Discrete world grid used for wall collision checks.
     * @param sprinting Whether the player is holding the sprint key.
     */
    void move(const Vector2D& direction, float dt, const TileMap& tileMap, bool sprinting = false);

private:
    bool hasWallInCells(const TileMap& tileMap, int minCol, int minRow, int maxCol, int maxRow) const ;
};

#endif // GAME_PLAYER_HPP
