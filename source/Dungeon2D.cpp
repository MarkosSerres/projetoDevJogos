#include "Dungeon2D.hpp"
#include <stdlib.h>

Dungeon2D::Dungeon2D(int widthTiles, int heightTiles) 
                : width{widthTiles}, height{heightTiles}, root(nullptr){}

namespace{
    constexpr int MIN_NODE_SIZE = 7;
    
    void splitNode(std::unique_ptr<BSPNode>& node)
    {
        int w = node->region.right - node->region.left;
        int h = node->region.bottom - node->region.top;

        if(w < MIN_NODE_SIZE * 2 && h < MIN_NODE_SIZE * 2){
            constexpr int MIN_ROOM_SIZE = 5;

            int max_w = w - MIN_ROOM_SIZE + 1;
            int max_h = h - MIN_ROOM_SIZE + 1;

            int roomW = MIN_ROOM_SIZE + (rand() % max_w);
            int roomH = MIN_ROOM_SIZE + (rand() % max_h);

            int rx = node->region.left + (rand() % (w - roomW + 1));
            int ry = node->region.top  + (rand() % (h - roomH + 1));

            node->room = Rect{rx, ry, rx + roomW, ry + roomH};
            return;
        }

        bool splitHorizontal;
        if(w > h && w >= MIN_NODE_SIZE * 2){
            splitHorizontal = false;
        }else if(h > w && h >= MIN_NODE_SIZE * 2){
            splitHorizontal = true;
        }else{
            splitHorizontal = (rand() % 2) == 0;
        }

        node->left  = std::make_unique<BSPNode>();
        node->right = std::make_unique<BSPNode>();

        if(splitHorizontal){
            int max_split = h - MIN_NODE_SIZE;
            int    splitY = node->region.top + MIN_NODE_SIZE + (rand() % (max_split - MIN_NODE_SIZE + 1));

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
            int max_split = w - MIN_NODE_SIZE;
            int    splitX = node->region.left + MIN_NODE_SIZE + (rand() % (max_split - MIN_NODE_SIZE + 1));

            node->left->region  = Rect{node->region.left, 
                                       node->region.top, 
                                       splitX, 
                                       node->region.bottom};

            node->right->region = Rect{splitX, 
                                       node->region.top, 
                                       node->region.right,
                                       node->region.bottom};
        }

        splitNode(node->left);
        splitNode(node->right);
    }

    void getLeafCenter(const std::unique_ptr<BSPNode>& node, int& cx, int& cy){
        if(node->isLeaf()){
            cx = node->room.left + (node->room.right  - node->room.left) / 2;
            cy = node->room.top  + (node->room.bottom - node->room.top)  / 2;
            return;
        }
        getLeafCenter(node->left, cx, cy);
    }

    void createCorridors(const std::unique_ptr<BSPNode>& node, std::vector<Corridor>& corridors){
        if(node->isLeaf()) return;

        createCorridors(node->left, corridors);
        createCorridors(node->right, corridors);

        int x1, y1, x2, y2;
        getLeafCenter(node->left, x1, y1);
        getLeafCenter(node->right, x2, y2);

        if((rand() % 2) == 0){
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
    srand(seed);
    
    root = std::make_unique<BSPNode>();
    root->region = Rect{0, 0, width, height};
    
    splitNode(root);
    createCorridors(root, corridors);
}

