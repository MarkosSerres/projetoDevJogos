#include "Dungeon2D.hpp"
#include <stdlib.h>
#include <random>

Dungeon2D::Dungeon2D(int widthTiles, int heightTiles) 
                : width{widthTiles}, height{heightTiles}, root(nullptr){}

namespace{
    constexpr int MIN_NODE_SIZE = 7;

    using Rng = std::mt19937;

    int randInt(Rng& rng, int lo, int hi) { 
        return lo + static_cast<int>(rng() % static_cast<unsigned>(hi - lo + 1));
    }

    void splitNode(std::unique_ptr<BSPNode>& node, Rng& rng)
    {
        int w = node->region.right - node->region.left;
        int h = node->region.bottom - node->region.top;

        if(w < MIN_NODE_SIZE * 2 && h < MIN_NODE_SIZE * 2){
            constexpr int MIN_ROOM_SIZE = 5;
            constexpr int ROOM_MARGIN   = 1;

            const int usableW = w - 2 * ROOM_MARGIN;
            const int usableH = h - 2 * ROOM_MARGIN;

            const int roomW = MIN_ROOM_SIZE + randInt(rng, 0, usableW - MIN_ROOM_SIZE);
            const int roomH = MIN_ROOM_SIZE + randInt(rng, 0, usableH - MIN_ROOM_SIZE);

            const int rx = node->region.left + ROOM_MARGIN + randInt(rng, 0, usableW - roomW);
            const int ry = node->region.top  + ROOM_MARGIN + randInt(rng, 0, usableH - roomH);

            node->room = Rect{rx, ry, rx + roomW, ry + roomH};
            return;
        }

        bool splitHorizontal;
        if(w > h && w >= MIN_NODE_SIZE * 2){
            splitHorizontal = false;
        }else if(h > w && h >= MIN_NODE_SIZE * 2){
            splitHorizontal = true;
        }else{
            splitHorizontal = (randInt(rng, 0, 1) == 0);
        }

        node->left  = std::make_unique<BSPNode>();
        node->right = std::make_unique<BSPNode>();

        if(splitHorizontal){
            const int splitY = node->region.top + randInt(rng, MIN_NODE_SIZE, h - MIN_NODE_SIZE);

            node->left->region  = Rect{node->region.left,
                                       node->region.top,
                                       node->region.right,
                                       splitY};

            node->right->region = Rect{node->region.left,
                                       splitY,
                                       node->region.right,
                                       node->region.bottom};
        }
        else{
            const int splitX = node->region.left + randInt(rng, MIN_NODE_SIZE, w - MIN_NODE_SIZE);

            node->left->region  = Rect{node->region.left, 
                                       node->region.top, 
                                       splitX, 
                                       node->region.bottom};

            node->right->region = Rect{splitX, 
                                       node->region.top, 
                                       node->region.right,
                                       node->region.bottom};
        }

        splitNode(node->left, rng);
        splitNode(node->right, rng);
    }

    void getLeafCenter(const std::unique_ptr<BSPNode>& node, int& cx, int& cy, Rng& rng){
        if(node->isLeaf()){
            cx = node->room.left + (node->room.right  - node->room.left) / 2;
            cy = node->room.top  + (node->room.bottom - node->room.top)  / 2;
            return;
        }
        const auto& child = (randInt(rng, 0, 1) == 0) ? node->left : node->right;
        getLeafCenter(child, cx, cy, rng);
    }

    void createCorridors(const std::unique_ptr<BSPNode>& node, std::vector<Corridor>& corridors, Rng& rng){
        if(node->isLeaf()) return;

        createCorridors(node->left , corridors, rng);
        createCorridors(node->right, corridors, rng);

        int x1, y1, x2, y2;
        getLeafCenter(node->left , x1, y1, rng);
        getLeafCenter(node->right, x2, y2, rng);

        if(randInt(rng, 0, 1) == 0){
            corridors.push_back(Corridor{(x1 < x2) ? x1 : x2,
                                         y1 - 1,
                                         (x1 > x2) ? x1 : x2,
                                         y1 + 1});

            corridors.push_back(Corridor{x2 - 1, 
                                         (y1 < y2) ? y1 : y2,
                                         x2 + 1,
                                         (y1 > y2) ? y1 : y2});
        } else {
            corridors.push_back(Corridor{x1 - 1,
                                         (y1 < y2) ? y1 : y2,
                                         x1 + 1,
                                         (y1 > y2) ? y1 : y2});

            corridors.push_back(Corridor{(x1 < x2) ? x1 : x2,
                                         y2 - 1,
                                         (x1 > x2) ? x1 : x2,
                                         y2 + 1});
        }
    }
}

void Dungeon2D::generate(unsigned seed){
    Rng rng(seed);
    
    corridors.clear();
    root.reset();

    if(width < MIN_NODE_SIZE || height < MIN_NODE_SIZE) return;

    root = std::make_unique<BSPNode>();
    root->region = Rect{0, 0, width, height};
    
    splitNode(root, rng);
    createCorridors(root, corridors, rng);
}

