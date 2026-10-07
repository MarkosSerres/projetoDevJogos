#include "Transform2D.hpp"
#include <cmath>

Transform2D Transform2D::translation(float tx, float ty) noexcept {
    Transform2D result;
    result.m[2][0] = tx;
    result.m[2][1] = ty;
    return result;
}

Transform2D Transform2D::rotation(float angle_rad) noexcept {
    Transform2D result;
    const float cos_a = std::cos(angle_rad);
    const float sin_a = std::sin(angle_rad);

    result.m[0][0] = cos_a;
    result.m[0][1] = sin_a;
    result.m[1][0] = -sin_a;
    result.m[1][1] = cos_a;
    return result;
}

Transform2D Transform2D::scale(float sx, float sy) noexcept {
    Transform2D result;
    result.m[0][0] = sx;
    result.m[1][1] = sy;
    return result;
}

Transform2D Transform2D::operator*(const Transform2D& rhs) const noexcept {
    Transform2D result;
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            result.m[i][j] = m[i][0] * rhs.m[0][j]
                           + m[i][1] * rhs.m[1][j]
                           + m[i][2] * rhs.m[2][j];
        }
    }
    return result;
}

Transform2D& Transform2D::operator*=(const Transform2D& rhs) noexcept {
    *this = *this * rhs;
    return *this;
}

Vector2D Transform2D::transform_point(const Vector2D& point) const noexcept {
    return Vector2D(
        point.x * m[0][0] + point.y * m[1][0] + m[2][0],
        point.x * m[0][1] + point.y * m[1][1] + m[2][1]
    );
}

Vector2D Transform2D::transform_vector(const Vector2D& direction) const noexcept {
    return Vector2D(
        direction.x * m[0][0] + direction.y * m[1][0],
        direction.x * m[0][1] + direction.y * m[1][1]
    );
}
