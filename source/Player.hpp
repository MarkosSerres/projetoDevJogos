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
    static constexpr float DEFAULT_VISION_RADIUS = 500.0f;
    static constexpr float INITIAL_HP            = 100.0F;
    static constexpr float WALL_SLIDE_FACTOR     = 0.65f;

    float speed{DEFAULT_SPEED};                ///< Movement speed in pixels per second.
    float visionRadius{DEFAULT_VISION_RADIUS}; ///< Light radius around the player in pixels.
    float hp{INITIAL_HP};

    Player();

    /**
     * @brief Moves the player along @p direction for @p dt seconds, resolving
     *        collisions against solid tiles in @p tileMap.
     * @param direction Desired movement vector (normalized internally if non-zero).
     * @param dt        Elapsed frame time in seconds.
     * @param tileMap   Discrete world grid used for wall collision checks.
     */
    void move(const Vector2D& direction, float dt, const TileMap& tileMap);

private:
    bool hasWallInCells(const TileMap& tileMap, int minCol, int minRow, int maxCol, int maxRow) const ;
};

#endif // GAME_PLAYER_HPP
