#ifndef GAME_ENEMY_HPP
#define GAME_ENEMY_HPP

#include <vector>
#include "Entity.hpp"
#include "Global.hpp"
#include "Vector2D.hpp"

class TileMap;
struct Player;

/**
 * @file Enemy.hpp
 * @brief Chasing enemy (e.g., chest mimic) that navigates the tile map via PathFind.
 */
struct Enemy : Entity {
    static constexpr float DEFAULT_SPEED        = 190.0f;
    static constexpr float CONTACT_DAMAGE       = 20.0f;
    static constexpr float ATTACK_COOLDOWN_TIME = 1.0f;
    static constexpr float REPATH_INTERVAL      = 0.35f;

    float speed{DEFAULT_SPEED};
    float attackCooldown{0.0f};
    float repathTimer{0.0f};
    std::vector<Vector2D> path;

    Enemy();

    /**
     * @brief Updates pathfinding, movement toward @p player, and contact damage.
     */
    void update(float dt, const TileMap& tileMap, Player& player);
};

#endif // GAME_ENEMY_HPP
