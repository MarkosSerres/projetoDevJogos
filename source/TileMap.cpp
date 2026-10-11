#include "TileMap.hpp"
#include "Dungeon2D.hpp"
#include <algorithm>
#include <cassert>
#include <charconv>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <string_view>

namespace {
    void carveRoomsFromLeaves(const std::unique_ptr<BSPNode>& node, int width, int height, std::vector<int>& cells) {
        if (!node) {
            return;
        }

        if (node->isLeaf()) {
            int startCol = std::max(0, node->room.left);
            int endCol   = std::min(width, node->room.right);
            int startRow = std::max(0, node->room.top);
            int endRow   = std::min(height, node->room.bottom);

            for (int r = startRow; r < endRow; ++r) {
                for (int c = startCol; c < endCol; ++c) {
                    cells[r * width + c] = EMPTY_TILE;
                }
            }
            return;
        }

        carveRoomsFromLeaves(node->left, width, height, cells);
        carveRoomsFromLeaves(node->right, width, height, cells);
    }
}

void TileMap::loadFromDungeon(const Dungeon2D& dungeon) {
    if (dungeon.width <= 0 || dungeon.height <= 0) {
        m_width = 0;
        m_height = 0;
        m_cells.clear();
        return;
    }

    m_width = dungeon.width;
    m_height = dungeon.height;
    m_cells.assign(m_width * m_height, WALL_TILE);

    if (dungeon.root) {
        carveRoomsFromLeaves(dungeon.root, m_width, m_height, m_cells);
    }

    for (const auto& corridor : dungeon.corridors) {
        int minCol = std::min(corridor.left, corridor.right);
        int maxCol = std::max(corridor.left, corridor.right);
        int minRow = std::min(corridor.top, corridor.bottom);
        int maxRow = std::max(corridor.top, corridor.bottom);

        int startCol = std::max(0, minCol);
        int endCol   = std::min(m_width - 1, maxCol);
        int startRow = std::max(0, minRow);
        int endRow   = std::min(m_height - 1, maxRow);

        for (int r = startRow; r <= endRow; ++r) {
            for (int c = startCol; c <= endCol; ++c) {
                m_cells[r * m_width + c] = EMPTY_TILE;
            }
        }
    }
}

bool TileMap::saveCSV(const std::string& path) const {
    std::ofstream file(path);
    if (!file.is_open()) {
        return false;
    }

    for (int r = 0; r < m_height; ++r) {
        for (int c = 0; c < m_width; ++c) {
            file << m_cells[r * m_width + c];
            if (c + 1 < m_width) {
                file << ',';
            }
        }
        file << '\n';
    }

    return file.good();
}

bool TileMap::loadCSV(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return false;
    }

    std::vector<int> tempCells;
    int tempWidth = -1;
    int tempHeight = 0;
    std::string line;

    while (std::getline(file, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        if (line.empty()) {
            return false;
        }

        // Trailing comma check: row must not end with a trailing comma
        if (line.back() == ',') {
            return false;
        }

        int rowCols = 0;
        size_t start = 0;

        while (start <= line.size()) {
            size_t commaPos = line.find(',', start);
            size_t len = (commaPos == std::string::npos) ? (line.size() - start) : (commaPos - start);

            if (len == 0) {
                return false;
            }

            std::string_view token(line.data() + start, len);
            int value = 0;
            auto [ptr, ec] = std::from_chars(token.data(), token.data() + token.size(), value);
            if (ec != std::errc{} || ptr != token.data() + token.size()) {
                return false;
            }

            tempCells.push_back(value);
            rowCols++;

            if (commaPos == std::string::npos) {
                break;
            }
            start = commaPos + 1;
        }

        if (tempHeight == 0) {
            tempWidth = rowCols;
            if (tempWidth <= 0) {
                return false;
            }
        } else if (rowCols != tempWidth) {
            return false;
        }

        tempHeight++;
    }

    if (tempHeight <= 0 || tempWidth <= 0) {
        return false;
    }

    if (tempCells.size() != static_cast<size_t>(tempWidth * tempHeight)) {
        return false;
    }

    m_width = tempWidth;
    m_height = tempHeight;
    m_cells = std::move(tempCells);
    return true;
}

int TileMap::width() const noexcept {
    return m_width;
}

int TileMap::height() const noexcept {
    return m_height;
}

int TileMap::at(int col, int row) const {
    assert(col >= 0 && col < m_width);
    assert(row >= 0 && row < m_height);
    return m_cells[row * m_width + col];
}

bool TileMap::isWall(int col, int row) const {
    assert(col >= 0 && col < m_width);
    assert(row >= 0 && row < m_height);
    return at(col, row) == WALL_TILE;
}

const int* TileMap::data() const noexcept {
    return m_cells.data();
}

void TileMap::toIndices(const Vector2D& world, int& col, int& row) const noexcept {
    col = static_cast<int>(std::floor(world.x / TILE_SIZE));
    row = static_cast<int>(std::floor(world.y / TILE_SIZE));
}

Vector2D TileMap::toWorld(int col, int row) const noexcept {
    return Vector2D{
        (static_cast<float>(col) + 0.5f) * TILE_SIZE,
        (static_cast<float>(row) + 0.5f) * TILE_SIZE
    };
}

void TileMap::randomEmptyCell(int& col, int& row) const {
    std::vector<int> emptyIndices;
    emptyIndices.reserve(m_cells.size());

    for (int i = 0; i < static_cast<int>(m_cells.size()); ++i) {
        if (m_cells[i] == EMPTY_TILE) {
            emptyIndices.push_back(i);
        }
    }

    assert(!emptyIndices.empty());

    if (emptyIndices.empty()) {
        col = 0;
        row = 0;
        return;
    }

    int chosen = emptyIndices[std::rand() % emptyIndices.size()];
    col = chosen % m_width;
    row = chosen / m_width;
}
