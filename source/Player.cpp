#include "Player.hpp"
#include "TileMap.hpp"
#include <cmath>

Player::Player() {
    id = PLAYER_ID;
    // Slightly smaller than a full 64x64 tile so the player glides through corridors.
    collision.halfExtents = { TILE_SIZE * 0.5f, TILE_SIZE * 0.5f };
}

bool Player::hasWallInCells(const TileMap& tileMap, int minCol, int minRow, int maxCol, int maxRow) const {
    if (minCol < 0 || minRow < 0 || maxCol >= tileMap.width() || maxRow >= tileMap.height()) {
        return true;
    }

    for (int row = minRow; row <= maxRow; ++row) {
        for (int col = minCol; col <= maxCol; ++col) {
            if (tileMap.isWall(col, row)) {
                return true;
            }
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

    constexpr float SKIN = 0.01f;
    body.velocity = { direction.x * speed, direction.y * speed };

    bool hitX = false;
    if (body.velocity.x != 0.0f) {
            body.position.x += body.velocity.x * dt;
            const AABB box = bounds();

            int minCol, minRow, maxCol, maxRow;
            tileMap.toIndices({ box.min.x + SKIN, box.min.y + SKIN }, minCol, minRow);
            tileMap.toIndices({ box.max.x - SKIN, box.max.y - SKIN }, maxCol, maxRow);

            if (body.velocity.x > 0.0f && hasWallInCells(tileMap, maxCol, minRow, maxCol, maxRow)) {
                hitX = true;
                body.position.x = static_cast<float>(maxCol) * TILE_SIZE - collision.halfExtents.x;
                body.velocity.x = 0.0f;
                body.velocity.y *= WALL_SLIDE_FACTOR;
            } else if (body.velocity.x < 0.0f && hasWallInCells(tileMap, minCol, minRow, minCol, maxRow)) {
                hitX = true;
                body.position.x = static_cast<float>(minCol + 1) * TILE_SIZE + collision.halfExtents.x;
                body.velocity.x = 0.0f;
                body.velocity.y *= WALL_SLIDE_FACTOR;
            }
    }

    if (body.velocity.y != 0.0f) {
        body.position.y += body.velocity.y * dt;
        const AABB box = bounds();

        int minCol, minRow, maxCol, maxRow;
        tileMap.toIndices({ box.min.x + SKIN, box.min.y + SKIN }, minCol, minRow);
        tileMap.toIndices({ box.max.x - SKIN, box.max.y - SKIN }, maxCol, maxRow);

        if (body.velocity.y > 0.0f && hasWallInCells(tileMap, minCol, maxRow, maxCol, maxRow)) {
            body.position.y = static_cast<float>(maxRow) * TILE_SIZE - collision.halfExtents.y;
            body.velocity.y = 0.0f;
            if (!hitX && body.velocity.x != 0.0f) {
                body.position.x -= body.velocity.x * (1.0f - WALL_SLIDE_FACTOR) * dt;
                body.velocity.x *= WALL_SLIDE_FACTOR;
            }
        } else if (body.velocity.y < 0.0f && hasWallInCells(tileMap, minCol, minRow, maxCol, minRow)) {
            body.position.y = static_cast<float>(minRow + 1) * TILE_SIZE + collision.halfExtents.y;
            body.velocity.y = 0.0f;
            if (!hitX && body.velocity.x != 0.0f) {
                body.position.x -= body.velocity.x * (1.0f - WALL_SLIDE_FACTOR) * dt;
                body.velocity.x *= WALL_SLIDE_FACTOR;
            }
        }
    }
}
