#include "RigidBody2D.hpp"
#include <cassert>

void RigidBody2D::integrate(float dt) noexcept{
    assert(dt>0);
    
    velocity+=acceleration * dt;
    position+=velocity * dt;
}