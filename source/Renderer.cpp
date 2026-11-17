#include "Renderer.hpp"

Renderer::Renderer(sf::RenderWindow& win) : m_window(win), m_tileSprite(m_floorTex){}

bool Renderer::loadResources(){
    return m_floorTex.loadFromFile(FLOOR_TEX) && m_wallTex.loadFromFile(WALL_TEX);
}

void Renderer::setupMapView(float zoomFactor){
    sf::Vector2u winSize = m_window.getSize();
    m_mapView.setSize({
        static_cast<float>(winSize.x) / 0.2F,
        static_cast<float>(winSize.y) / 0.2F
    });
    m_mapView.setCenter({
        (MAP_WIDTH * TILE_SIZE) * 0.5f,
        (MAP_HEIGHT * TILE_SIZE) * 0.5f
    });
    m_window.setView(m_mapView);
}

 sf::RenderWindow& Renderer::getWindow(){
     return m_window; 
}

 void Renderer::beginFrame(){
    m_window.clear(sf::Color::Black);
 }

 void Renderer::drawMap(const TileMap& tileMap){
    for(int row = 0 ; row < tileMap.height() ; row++){
        for(int col = 0 ; col < tileMap.width() ; col++){
             m_tileSprite.setTexture(tileMap.isWall(col, row) ? m_wallTex : m_floorTex, true);
             m_tileSprite.setPosition({ static_cast<float>(col) * TILE_SIZE,
                                        static_cast<float>(row) * TILE_SIZE
            });
            m_window.draw(m_tileSprite);
        }
    }
 }

 void Renderer::endFrame(){
    m_window.display();
 }
