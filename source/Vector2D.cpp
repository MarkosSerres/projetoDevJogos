#include <Vector2D.hpp>
#include <cassert>
#include <iostream>

[[nodiscard]] Vector2D Vector2D::normalized() const{
    float len = length();
    assert(len > EPSILON); 

    return Vector2D(x/len, y/len);
}

void Vector2D::normalize(){
    float len = length();
    assert(len > EPSILON);
    x/=len;
    y/=len;
}

[[nodiscard]] bool Vector2D::equals(const Vector2D& rhs, float tolerance) const noexcept{
    return std::abs(x - rhs.x) <= tolerance && std::abs(y - rhs.y) <= tolerance;
}

[[nodiscard]] Vector2D Vector2D::operator+(const Vector2D& rhs) const noexcept{
    return Vector2D(this->x + rhs.x, this->y + rhs.y);
}

[[nodiscard]] Vector2D Vector2D::operator-(const Vector2D& rhs) const noexcept{
    return Vector2D(this->x - rhs.x, this->y - rhs.y);
}

[[nodiscard]] Vector2D Vector2D::operator*(float scalar) const noexcept{
    return Vector2D(this->x * scalar, this->y * scalar);
}

[[nodiscard]] Vector2D Vector2D::operator/(float scalar) const{
    assert(std::abs(scalar) > EPSILON);
    return Vector2D(this->x / scalar, this->y / scalar);
}

Vector2D& Vector2D::operator+=(const Vector2D& rhs) noexcept{
    x+=rhs.x;
    y+=rhs.y;
    return *this;
}

Vector2D& Vector2D::operator-=(const Vector2D& rhs) noexcept{
    x-=rhs.x;
    y-=rhs.y;
    return *this;
}

Vector2D& Vector2D::operator*=(float scalar) noexcept{
    x*=scalar;
    y*=scalar;
    return *this;
}

Vector2D& Vector2D::operator/=(float scalar){
    assert(std::abs(scalar) > EPSILON);
    x/=scalar;
    y/=scalar;
    return *this;
}

[[nodiscard]]Vector2D operator*(float scalar, const Vector2D& vec) noexcept{
    return vec * scalar;
}
