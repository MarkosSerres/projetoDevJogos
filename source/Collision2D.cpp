#include "Collision2D.hpp"
#include <cassert>

[[nodiscard]] bool AABB::intersects(const AABB& other) const noexcept{
    assert(min.x <= max.x && min.y <= max.y);
    assert(other.min.x <= other.max.x && other.min.y <= other.max.y);

    return (max.x >= other.min.x && min.x <= other.max.x &&
            max.y >= other.min.y && min.y <= other.max.y);
}

[[nodiscard]] AABB Collision2D::bounds(const Vector2D& position) const noexcept{
    assert(halfExtents.x >= 0.0f && halfExtents.y >= 0.0f);
    return AABB{ position - halfExtents, position + halfExtents };
}
