#include "PathFind.hpp"
#include <cmath>
#include <algorithm>

constexpr int ORTHOGONAL_COST = 10;
constexpr int DIAGONAL_COST   = 14;

namespace {
    struct Node {
        int x{0};
        int y{0};
        int g{0};
        int h{0};
        int f{0};
        Node* parent{nullptr};
        bool closed{false};
        bool inOpen{false};
    };
}

PathFind::PathFind(const int* grid, int width, int height)
    : m_grid(grid), m_width(width), m_height(height){}

std::vector<Vector2D> PathFind::findPath(const Vector2D& startWorld, const Vector2D& goalWorld) const {
    std::vector<Vector2D> path;

    if(m_grid == nullptr || m_width <= 0 || m_height <= 0)
        return path;

    const float fx = std::floor(startWorld.x / TILE_SIZE), fy = std::floor(startWorld.y / TILE_SIZE);
    const float gx = std::floor(goalWorld.x  / TILE_SIZE), gy = std::floor(goalWorld.y  / TILE_SIZE);
    const float W = static_cast<float>(m_width), H = static_cast<float>(m_height);
    
    if (!(fx >= 0 && fx < W && fy >= 0 && fy < H && gx >= 0 && gx < W && gy >= 0 && gy < H)) 
        return path;
    int startX = static_cast<int>(fx), startY = static_cast<int>(fy);
    int goalX  = static_cast<int>(gx), goalY  = static_cast<int>(gy);;

    auto isValid = [this](int x, int y) {
        return x >= 0 && x < m_width && y >= 0 && y < m_height;
    };

    auto isWalkable = [this](int x, int y) {
        return m_grid[y * m_width + x] == EMPTY_TILE;
    };

    if(!isValid(startX, startY) || !isValid(goalX, goalY)) 
        return path;
    if(!isWalkable(startX, startY) || !isWalkable(goalX, goalY)) 
        return path;

    std::vector<Node> nodes(m_width * m_height);
    for(int i = 0; i < m_width * m_height; ++i) {
        nodes[i].x = i % m_width;
        nodes[i].y = i / m_width;
    }

    std::vector<Node*> openList;
    Node* startNode = &nodes[startY * m_width + startX];
    Node* goalNode  = &nodes[goalY * m_width + goalX];

    openList.push_back(startNode);
    startNode->inOpen = true;

    bool found = false;

    while (!openList.empty()) {
        Node* q = openList[0];
        size_t qIndex = 0;

        for (size_t i = 1; i < openList.size(); ++i) {
            if (openList[i]->f < q->f) {
                q = openList[i];
                qIndex = i;
            }
        }

        openList[qIndex] = openList.back();
        openList.pop_back();
        q->inOpen = false;
        q->closed = true;

        if(q == goalNode){
            found = true;
            break;
        }

        int dx[8] = {-1,  0,  1,  -1, 1,  -1, 0, 1};
        int dy[8] = {-1, -1, -1,   0, 0,   1, 1, 1};

        for(int i = 0; i < 8; ++i) {
            int nx = q->x + dx[i];
            int ny = q->y + dy[i];

            if(!isValid(nx, ny) || !isWalkable(nx, ny))
                continue;

            if(dx[i] != 0 && dy[i] != 0) {
                if (!isWalkable(q->x + dx[i], q->y) || !isWalkable(q->x, q->y + dy[i]))
                    continue;
            }

            Node* successorNode = &nodes[ny * m_width + nx];
            if(successorNode->closed)
                continue;

            int cost = (dx[i] != 0 && dy[i] != 0) ? DIAGONAL_COST : ORTHOGONAL_COST;
            int newg = q->g + cost;

            int diffX = std::abs(successorNode->x - goalNode->x);
            int diffY = std::abs(successorNode->y - goalNode->y);
            int max = std::max(diffX, diffY);
            int min = std::min(diffX, diffY);
            int newh = (ORTHOGONAL_COST * (max - min)) + (DIAGONAL_COST * min);
            
            int newf = newg + newh;

            if(!successorNode->inOpen || newf < successorNode->f) {
                successorNode->g = newg;
                successorNode->h = newh;
                successorNode->f = newf;
                successorNode->parent = q;

                if(!successorNode->inOpen) {
                    openList.push_back(successorNode);
                    successorNode->inOpen = true;
                }
            }
        }
    }

    if(found){
        Node* trace = goalNode;
        while(trace != nullptr){
            float px = (trace->x * TILE_SIZE) + (TILE_SIZE / 2.0f);
            float py = (trace->y * TILE_SIZE) + (TILE_SIZE / 2.0f);
            path.push_back(Vector2D(px, py));
            trace = trace->parent;
        }
        std::reverse(path.begin(), path.end());
    }

    return path;
}