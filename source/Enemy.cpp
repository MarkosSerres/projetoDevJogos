#include "Enemy.hpp"
#include "PathFind.hpp"
#include "Player.hpp"
#include "TileMap.hpp"
#include <algorithm>
#include <cmath>

Enemy::Enemy() {
    collision.halfExtents = { TILE_SIZE * 0.4f, TILE_SIZE * 0.4f };
}

void Enemy::update(float dt, const TileMap& tileMap, Player& player) {
    if (!active || !player.active || dt <= 0.0f) {
        return;
    }

    if (attackCooldown > 0.0f) {
        attackCooldown = std::max(0.0f, attackCooldown - dt);
    }

    repathTimer -= dt;
    if (repathTimer <= 0.0f) {
        repathTimer = REPATH_INTERVAL;
        PathFind finder(tileMap.data(), tileMap.width(), tileMap.height());
        path = finder.findPath(body.position, player.body.position);
    }

    // Pick the next waypoint ahead of our current cell.
    Vector2D target = player.body.position;
    if (path.size() >= 2) {
        target = path[1];
        const Vector2D toWp = { target.x - body.position.x, target.y - body.position.y };
        if (toWp.x * toWp.x + toWp.y * toWp.y < (TILE_SIZE * 0.25f) * (TILE_SIZE * 0.25f) &&
            path.size() >= 3) {
            target = path[2];
        }
    }

    const Vector2D delta{ target.x - body.position.x, target.y - body.position.y };
    const float distSq = delta.x * delta.x + delta.y * delta.y;
    if (distSq > EPSILON) {
        const float invDist = 1.0f / std::sqrt(distSq);
        body.velocity = { delta.x * invDist * speed, delta.y * invDist * speed };
        body.position.x += body.velocity.x * dt;
        body.position.y += body.velocity.y * dt;
    } else {
        body.velocity = { 0.0f, 0.0f };
    }

    if (attackCooldown <= 0.0f && bounds().intersects(player.bounds())) {
        player.hp = std::max(0.0f, player.hp - CONTACT_DAMAGE);
        attackCooldown = ATTACK_COOLDOWN_TIME;
    }
}
