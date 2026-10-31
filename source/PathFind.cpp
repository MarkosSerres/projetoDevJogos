#include "PathFind.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <limits>
#include <queue>


namespace {

    constexpr double ORTHOGONAL_COST = 1.0;
    constexpr double DIAGONAL_COST   = 1.4142135623730951;   // sqrt(2)

    constexpr std::size_t NO_PARENT = std::numeric_limits<std::size_t>::max();

    struct Step {
        int    dCol;
        int    dRow;
        double cost;
    };

    constexpr Step STEPS[8] = {
        { 1,  0, ORTHOGONAL_COST}, {-1,  0, ORTHOGONAL_COST},
        { 0,  1, ORTHOGONAL_COST}, { 0, -1, ORTHOGONAL_COST},
        { 1,  1, DIAGONAL_COST},   { 1, -1, DIAGONAL_COST},
        {-1,  1, DIAGONAL_COST},   {-1, -1, DIAGONAL_COST},
    };

    /// One entry of the open list. The same cell may be queued several times (a
    /// cheaper route was found later); stale copies are skipped when popped.
    struct OpenEntry {
        double      f;       ///< g + h
        double      h;       ///< heuristic, used only to break ties on f
        std::size_t index;   ///< row * width + col
    };

    /// std::priority_queue is a max-heap, so "less" here means "worse": a larger f
    /// is worse, then a larger h, then a larger index.
    struct OpenEntryWorse {
        bool operator()(const OpenEntry& a, const OpenEntry& b) const {
            if (a.f != b.f) return a.f > b.f;
            if (a.h != b.h) return a.h > b.h;
            return a.index > b.index;
        }
    };

    /// Octile distance between two cells.
    double heuristic(int col, int row, int goalCol, int goalRow) {
        const int dx = std::abs(col - goalCol);
        const int dy = std::abs(row - goalRow);
        const int diagonalSteps = std::min(dx, dy);
        const int straightSteps = std::max(dx, dy) - diagonalSteps;
        return DIAGONAL_COST * diagonalSteps + ORTHOGONAL_COST * straightSteps;
    }

    /// World position -> cell, using col = floor(x / TILE_SIZE), row = floor(y / TILE_SIZE).
    /// Returns false when the cell lies outside the matrix. The range test is written
    /// so that NaN fails it, and it runs BEFORE the conversion to int, so infinities
    /// and huge values never reach an out-of-range float-to-int cast.
    bool worldToCell(const Vector2D& world, int width, int height, int& col, int& row) {
        const float fx = std::floor(world.x / TILE_SIZE);
        const float fy = std::floor(world.y / TILE_SIZE);

        if (!(fx >= 0.0f && fx < static_cast<float>(width)))  return false;
        if (!(fy >= 0.0f && fy < static_cast<float>(height))) return false;

        col = static_cast<int>(fx);
        row = static_cast<int>(fy);
        return true;
    }

    /// Centre of a cell in world coordinates (Global.hpp convention).
    Vector2D cellCentre(int col, int row) {
        return Vector2D{
            (static_cast<float>(col) + 0.5f) * TILE_SIZE,
            (static_cast<float>(row) + 0.5f) * TILE_SIZE
        };
    }

}  // namespace

PathFind::PathFind(const int* grid, int width, int height)
    : m_grid(grid), m_width(width), m_height(height) {
    assert(grid != nullptr);
    assert(width > 0 && height > 0);
}

std::vector<Vector2D> PathFind::findPath(const Vector2D& startWorld,
                                         const Vector2D& goalWorld) const {
    std::vector<Vector2D> path;

    // Only reachable if the constructor's preconditions were broken in a build
    // where assert is disabled; answer "no path" instead of reading bad memory.
    if (m_grid == nullptr || m_width <= 0 || m_height <= 0) {
        return path;
    }

    int startCol = 0, startRow = 0, goalCol = 0, goalRow = 0;
    if (!worldToCell(startWorld, m_width, m_height, startCol, startRow) ||
        !worldToCell(goalWorld,  m_width, m_height, goalCol,  goalRow)) {
        return path;
    }

    const auto indexOf = [this](int col, int row) {
        return static_cast<std::size_t>(row) * static_cast<std::size_t>(m_width) + static_cast<std::size_t>(col);
    };

    // Out-of-range cells count as not walkable, which also covers the neighbours
    // of cells on the border.
    const auto walkable = [this, &indexOf](int col, int row) {
        return col >= 0 && col < m_width && row >= 0 && row < m_height &&
               m_grid[indexOf(col, row)] == EMPTY_TILE;
    };

    if (!walkable(startCol, startRow) || !walkable(goalCol, goalRow)) {
        return path;
    }

    const std::size_t startIndex = indexOf(startCol, startRow);
    const std::size_t goalIndex  = indexOf(goalCol, goalRow);

    if (startIndex == goalIndex) {
        path.push_back(cellCentre(startCol, startRow));
        return path;
    }

    const std::size_t cellCount = static_cast<std::size_t>(m_width) * static_cast<std::size_t>(m_height);
    std::vector<double>      gScore(cellCount, std::numeric_limits<double>::infinity());
    std::vector<std::size_t> cameFrom(cellCount, NO_PARENT);
    std::vector<char>        closed(cellCount, 0);

    std::priority_queue<OpenEntry, std::vector<OpenEntry>, OpenEntryWorse> open;

    gScore[startIndex] = 0.0;
    const double startH = heuristic(startCol, startRow, goalCol, goalRow);
    open.push(OpenEntry{startH, startH, startIndex});

    while (!open.empty()) {
        const OpenEntry current = open.top();
        open.pop();

        if (closed[current.index]) {
            continue;   // stale duplicate of a cell that was already expanded
        }
        closed[current.index] = 1;

        if (current.index == goalIndex) {
            // Walk the parent links back to the start, then flip the order.
            for (std::size_t i = goalIndex; i != NO_PARENT; i = cameFrom[i]) {
                path.push_back(cellCentre(static_cast<int>(i % static_cast<std::size_t>(m_width)),
                                          static_cast<int>(i / static_cast<std::size_t>(m_width))));
            }
            std::reverse(path.begin(), path.end());
            return path;
        }

        const int col = static_cast<int>(current.index % static_cast<std::size_t>(m_width));
        const int row = static_cast<int>(current.index / static_cast<std::size_t>(m_width));

        for (const Step& step : STEPS) {
            const int nextCol = col + step.dCol;
            const int nextRow = row + step.dRow;

            if (!walkable(nextCol, nextRow)) {
                continue;
            }

            const bool diagonal = (step.dCol != 0 && step.dRow != 0);
            if (diagonal && !(walkable(col + step.dCol, row) && walkable(col, row + step.dRow))) {
                continue;   // would clip the corner of a wall
            }

            const std::size_t nextIndex = indexOf(nextCol, nextRow);
            if (closed[nextIndex]) {
                continue;
            }

            const double tentative = gScore[current.index] + step.cost;
            if (tentative < gScore[nextIndex]) {
                gScore[nextIndex]  = tentative;
                cameFrom[nextIndex] = current.index;

                const double h = heuristic(nextCol, nextRow, goalCol, goalRow);
                open.push(OpenEntry{tentative + h, h, nextIndex});
            }
        }
    }

    return path;   // open list exhausted: the goal cannot be reached
}