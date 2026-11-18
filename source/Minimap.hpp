#ifndef GAME_MINIMAP_HPP
#define GAME_MINIMAP_HPP

#include <cstdint>
#include <vector>
#include "Vector2D.hpp"

class TileMap;

/**
 * @file Minimap.hpp
 * @brief Fog-of-war exploration tracker for the minimap HUD.
 *
 * Minimap records which cells of a TileMap have been discovered by the player
 * as they move through the labyrinth. It stores only exploration state
 * (row-major, matching TileMap) and has no rendering dependencies.
 */
class Minimap {
public:
    static constexpr int DEFAULT_REVEAL_RADIUS_TILES = 3;

    Minimap() = default;

    /**
     * @brief Clears exploration data and resizes the grid to @p width x @p height.
     */
    void reset(int width, int height);

    /**
     * @brief Marks walkable/wall cells within @p radiusTiles of @p worldPos as explored.
     */
    void revealAround(const TileMap& tileMap,
                      const Vector2D& worldPos,
                      int radiusTiles = DEFAULT_REVEAL_RADIUS_TILES);

    // --- Queries -----------------------------------------------------------

    [[nodiscard]] bool isExplored(int col, int row) const noexcept;
    [[nodiscard]] int  width() const noexcept;
    [[nodiscard]] int  height() const noexcept;

private:
    int                        m_width{0};
    int                        m_height{0};
    std::vector<std::uint8_t>  m_explored; ///< Row-major, 1 = discovered, 0 = hidden.
};

#endif // GAME_MINIMAP_HPP
