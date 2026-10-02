#include "Transform2D.hpp"
#include <cassert>
#include <cmath>

void Transform2D::translate(const Vector2D& offset) noexcept{
    position += offset;
}

void Transform2D::translate(float dx, float dy) noexcept{
    position.x += dx;
    position.y += dy;
}

void Transform2D::rotate(float angle) noexcept{
    rotation += angle;
}

void Transform2D::scaleBy(float factor) noexcept{
    scale *= factor;
}

void Transform2D::scaleBy(const Vector2D& factors) noexcept{
    scale.x *= factors.x;
    scale.y *= factors.y;
}

[[nodiscard]] Vector2D Transform2D::transformPoint(const Vector2D& localPoint) const noexcept{
    float sx = localPoint.x * scale.x;
    float sy = localPoint.y * scale.y;

    float cosR = std::cos(rotation);
    float sinR = std::sin(rotation);

    float rx = sx * cosR - sy * sinR;
    float ry = sx * sinR + sy * cosR;

    return Vector2D(position.x + rx, position.y + ry);
}

[[nodiscard]] Vector2D Transform2D::inverseTransformPoint(const Vector2D& worldPoint) const{
    assert(std::abs(scale.x) > EPSILON);
    assert(std::abs(scale.y) > EPSILON);

    float dx = worldPoint.x - position.x;
    float dy = worldPoint.y - position.y;

    float cosR = std::cos(rotation);
    float sinR = std::sin(rotation);

    float rx = dx * cosR + dy * sinR;
    float ry = -dx * sinR + dy * cosR;

    return Vector2D(rx / scale.x, ry / scale.y);
}

[[nodiscard]] Vector2D Transform2D::transformDirection(const Vector2D& localDirection) const noexcept{
    float cosR = std::cos(rotation);
    float sinR = std::sin(rotation);

    return Vector2D(localDirection.x * cosR - localDirection.y * sinR,
                    localDirection.x * sinR + localDirection.y * cosR);
}

[[nodiscard]] Vector2D Transform2D::forward() const noexcept{
    return Vector2D(std::cos(rotation), std::sin(rotation));
}

[[nodiscard]] Vector2D Transform2D::right() const noexcept{
    return Vector2D(-std::sin(rotation), std::cos(rotation));
}

[[nodiscard]] bool Transform2D::equals(const Transform2D& rhs, float tolerance) const noexcept{
    return position.equals(rhs.position, tolerance) &&
           std::abs(rotation - rhs.rotation) <= tolerance &&
           scale.equals(rhs.scale, tolerance);
}
