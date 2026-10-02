#ifndef GAME_TRANSFORM2D_HPP
#define GAME_TRANSFORM2D_HPP

#include <cmath>
#include "Global.hpp"
#include "Vector2D.hpp"

/**
 * @file Transform2D.hpp
 * @brief 2D spatial transformation component (position, rotation, and scale).
 *
 * Represents an entity's 2D affine transformation state in continuous world space.
 * Follows the project coordinate convention where the Y axis grows DOWNWARD (screen convention).
 * Rotation is defined in radians, where positive values denote clockwise rotation.
 */
struct Transform2D {
    Vector2D position{0.0f, 0.0f};  ///< World position, in pixels.
    float rotation{0.0f};           ///< Rotation angle, in radians (clockwise, Y-down).
    Vector2D scale{1.0f, 1.0f};     ///< Non-uniform scale factors along X and Y axes.

    constexpr Transform2D() noexcept = default;
    constexpr Transform2D(const Vector2D& in_position, float in_rotation = 0.0f, const Vector2D& in_scale = Vector2D{1.0f, 1.0f}) noexcept
        : position(in_position), rotation(in_rotation), scale(in_scale) {}
    constexpr Transform2D(float in_x, float in_y, float in_rotation = 0.0f, float in_scale_x = 1.0f, float in_scale_y = 1.0f) noexcept
        : position(in_x, in_y), rotation(in_rotation), scale(in_scale_x, in_scale_y) {}

    // --- Transformations ---------------------------------------------------

    /**
     * @brief Translates the position by a displacement vector.
     * @param offset Displacement in pixels.
     */
    void translate(const Vector2D& offset) noexcept;

    /**
     * @brief Translates the position by discrete x and y offsets.
     */
    void translate(float dx, float dy) noexcept;

    /**
     * @brief Increments the rotation angle.
     * @param angle Angle to add, in radians.
     */
    void rotate(float angle) noexcept;

    /**
     * @brief Scales the current scale factors uniformly.
     * @param factor Scalar multiplier.
     */
    void scaleBy(float factor) noexcept;

    /**
     * @brief Scales the current scale factors non-uniformly.
     * @param factors Scale multipliers along X and Y.
     */
    void scaleBy(const Vector2D& factors) noexcept;

    // --- Coordinate conversions --------------------------------------------

    /**
     * @brief Transforms a point from local space to world space.
     * Applies scaling, followed by rotation, then translation.
     * @param localPoint Point in entity's local coordinates.
     * @return World coordinates of the transformed point.
     */
    [[nodiscard]] Vector2D transformPoint(const Vector2D& localPoint) const noexcept;

    /**
     * @brief Transforms a point from world space back to local space.
     * @param worldPoint Point in world coordinates.
     * @pre std::abs(scale.x) > EPSILON && std::abs(scale.y) > EPSILON.
     * @return Local coordinates of the point.
     */
    [[nodiscard]] Vector2D inverseTransformPoint(const Vector2D& worldPoint) const;

    /**
     * @brief Transforms a direction vector from local space to world space (without translation).
     * @param localDirection Direction in local coordinates.
     * @return Transformed direction in world coordinates.
     */
    [[nodiscard]] Vector2D transformDirection(const Vector2D& localDirection) const noexcept;

    // --- Direction vectors -------------------------------------------------

    /// Unit direction vector pointing along the entity's forward heading (local (1, 0)).
    [[nodiscard]] Vector2D forward() const noexcept;

    /// Unit direction vector pointing to the entity's right (local (0, 1) in Y-down convention).
    [[nodiscard]] Vector2D right() const noexcept;

    // --- Comparison --------------------------------------------------------

    /// Approximate equality within a tolerance.
    [[nodiscard]] bool equals(const Transform2D& rhs, float tolerance = EPSILON) const noexcept;
};

#endif // GAME_TRANSFORM2D_HPP
