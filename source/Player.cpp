#include "Player.hpp"
#include "TileMap.hpp"
#include <cmath>

Player::Player() {
    id = PLAYER_ID;
    // Slightly smaller than a full 64x64 tile so the player glides through corridors.
    collision.halfExtents = { TILE_SIZE * 0.5f, TILE_SIZE * 0.5f };
}

bool Player::collidesWithWall(const TileMap& tileMap) const {
    const AABB box = bounds();

    int minCol, minRow, maxCol, maxRow;
    tileMap.toIndices(box.min, minCol, minRow);
    tileMap.toIndices({ box.max.x - EPSILON, box.max.y - EPSILON }, maxCol, maxRow);

    if (minCol < 0 || minRow < 0 || maxCol >= tileMap.width() || maxRow >= tileMap.height())
        return true;

    for (int row = minRow; row <= maxRow; ++row) {
        for (int col = minCol; col <= maxCol; ++col) {
            if (tileMap.isWall(col, row)) return true;
        }
    }
    return false;
}

void Player::move(const Vector2D& direction, float dt, const TileMap& tileMap) {
    if (!active || dt <= 0.0f) {
        return;
    }

    if (direction.x * direction.x + direction.y * direction.y <= EPSILON) {
        body.velocity = { 0.0f, 0.0f };
        return;
    }

    body.velocity = { direction.x * speed, direction.y * speed };

    bool hitX = false;
    body.position.x += body.velocity.x * dt;
    if (collidesWithWall(tileMap)) {
        hitX = true;
        const AABB box = bounds();
        if (body.velocity.x > 0.0f) {
            const float wallCol = std::floor((box.max.x - EPSILON) / TILE_SIZE);
            body.position.x = wallCol * TILE_SIZE - collision.halfExtents.x;
        } else if (body.velocity.x < 0.0f) {
            const float wallCol = std::floor(box.min.x / TILE_SIZE);
            body.position.x = (wallCol + 1.0f) * TILE_SIZE + collision.halfExtents.x;
        }
        body.velocity.x = 0.0f;
        body.velocity.y *= WALL_SLIDE_FACTOR;
    }

    body.position.y += body.velocity.y * dt;
    if (collidesWithWall(tileMap)) {
        const AABB box = bounds();
        if (body.velocity.y > 0.0f) {
            const float wallRow = std::floor((box.max.y - EPSILON) / TILE_SIZE);
            body.position.y = wallRow * TILE_SIZE - collision.halfExtents.y;
        } else if (body.velocity.y < 0.0f) {
            const float wallRow = std::floor(box.min.y / TILE_SIZE);
            body.position.y = (wallRow + 1.0f) * TILE_SIZE + collision.halfExtents.y;
        }
        body.velocity.y = 0.0f;

        if (!hitX && body.velocity.x != 0.0f) {
            body.position.x -= body.velocity.x * (1.0f - WALL_SLIDE_FACTOR) * dt;
            body.velocity.x *= WALL_SLIDE_FACTOR;
        }
    }
}
