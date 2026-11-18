#include "Minimap.hpp"
#include "TileMap.hpp"
#include <algorithm>

void Minimap::reset(int width, int height) {
    if (width <= 0 || height <= 0) {
        m_width = 0;
        m_height = 0;
        m_explored.clear();
        return;
    }

    m_width = width;
    m_height = height;
    m_explored.assign(static_cast<std::size_t>(m_width * m_height), 0);
}

void Minimap::revealAround(const TileMap& tileMap, const Vector2D& worldPos, int radiusTiles) {
    if (m_width <= 0 || m_height <= 0 || radiusTiles < 0) {
        return;
    }

    int centerCol = 0;
    int centerRow = 0;
    tileMap.toIndices(worldPos, centerCol, centerRow);

    const int minCol = std::max(0, centerCol - radiusTiles);
    const int maxCol = std::min(m_width - 1, centerCol + radiusTiles);
    const int minRow = std::max(0, centerRow - radiusTiles);
    const int maxRow = std::min(m_height - 1, centerRow + radiusTiles);
    const int radiusSq = radiusTiles * radiusTiles;

    for (int row = minRow; row <= maxRow; ++row) {
        const int dy = row - centerRow;
        for (int col = minCol; col <= maxCol; ++col) {
            const int dx = col - centerCol;
            if (dx * dx + dy * dy <= radiusSq) {
                m_explored[static_cast<std::size_t>(row * m_width + col)] = 1;
            }
        }
    }
}

bool Minimap::isExplored(int col, int row) const noexcept {
    if (col < 0 || col >= m_width || row < 0 || row >= m_height) {
        return false;
    }
    return m_explored[static_cast<std::size_t>(row * m_width + col)] != 0;
}

int Minimap::width() const noexcept {
    return m_width;
}

int Minimap::height() const noexcept {
    return m_height;
}
