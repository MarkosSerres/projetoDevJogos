#ifndef GAME_RENDERER_HPP
#define GAME_RENDERER_HPP

#include <SFML/Graphics.hpp>
#include "Entity.hpp" 
#include "TileMap.hpp"

class Renderer{
public:
    explicit Renderer(sf::RenderWindow& win);

    bool loadResources();
    void setupMapView(float zoomFactor = 1.0f);

    sf::RenderWindow& getWindow();

    void beginFrame();
    void drawMap(const TileMap& tileMap);
    void endFrame();

private:
    sf::RenderWindow& m_window;
    sf::View m_mapView;

    sf::Texture m_floorTex;
    sf::Texture m_wallTex;
    sf::Texture m_playerTex;

    sf::Sprite m_tileSprite;
};

#endif // GAME_RENDERER_HPP