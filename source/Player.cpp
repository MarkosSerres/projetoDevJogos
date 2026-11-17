#include "Player.hpp"
#include "TileMap.hpp"
#include <algorithm>
#include <cmath>

Player::Player() {
    id = PLAYER_ID;
    // Slightly smaller than a full 64x64 tile so the player glides through corridors.
    collision.halfExtents = { TILE_SIZE * 0.35f, TILE_SIZE * 0.35f };
}

bool Player::collidesWithWall(const TileMap& tileMap) const {
    const AABB box = bounds();

    int minCol = 0;
    int minRow = 0;
    int maxCol = 0;
    int maxRow = 0;
    tileMap.toIndices(box.min, minCol, minRow);
    tileMap.toIndices({ box.max.x - EPSILON, box.max.y - EPSILON }, maxCol, maxRow);

    if (minCol < 0 || minRow < 0 ||
        maxCol >= tileMap.width() || maxRow >= tileMap.height()) {
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

    Vector2D dir = direction;
    const float lenSq = dir.x * dir.x + dir.y * dir.y;
    if (lenSq > EPSILON) {
        const float invLen = 1.0f / std::sqrt(lenSq);
        dir.x *= invLen;
        dir.y *= invLen;
    } else {
        body.velocity = { 0.0f, 0.0f };
        return;
    }

    body.velocity = { dir.x * speed, dir.y * speed };

    // Resolve X and Y separately so the player slides smoothly along walls.
    const float prevX = body.position.x;
    body.position.x += body.velocity.x * dt;
    if (collidesWithWall(tileMap)) {
        body.position.x = prevX;
        body.velocity.x = 0.0f;
    }

    const float prevY = body.position.y;
    body.position.y += body.velocity.y * dt;
    if (collidesWithWall(tileMap)) {
        body.position.y = prevY;
        body.velocity.y = 0.0f;
    }
}
