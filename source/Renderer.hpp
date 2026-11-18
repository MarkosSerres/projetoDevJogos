#ifndef GAME_RENDERER_HPP
#define GAME_RENDERER_HPP

#include <array>
#include <SFML/Graphics.hpp>

#include "TileMap.hpp"
#include "Menu.hpp"
#include "Minimap.hpp"
#include "Player.hpp"

class Renderer{
public:
    explicit Renderer(sf::RenderWindow& win);

    bool loadResources();
    void setupMapView(float zoomFactor = 1.0f);
    void updateCamera(const Player& player, const TileMap& tileMap);

    sf::RenderWindow& getWindow();
    sf::FloatRect getMenuButtonBounds(const Menu& menu, std::size_t index) const;
    
    const bool windowHasFocus() const;

    void beginFrame();

    void drawMap(const TileMap& tileMap);
    void drawPlayer(const Player& player);
    void drawDarkness(const Vector2D& lightCenter, float lightRadius);
    void drawMinimap(const TileMap& tileMap, const Minimap& minimap, const Vector2D& playerPos);
    void drawMenu(const Menu& menu);

    void endFrame();

private:
    [[nodiscard]] const sf::Texture& menuButtonTexture(const Menu& menu, std::size_t index) const;
    [[nodiscard]] sf::Vector2f menuButtonPosition(const Menu& menu, std::size_t index) const;
    [[nodiscard]] sf::FloatRect volumeControlBounds() const;
    void drawVolumeControl(const Menu& menu);
    void drawSpriteButton(const sf::Texture& tex, sf::Vector2f pos);
    
    sf::RenderWindow& m_window;
    sf::View m_mapView;

    sf::Texture m_floorTex;
    sf::Texture m_wallTex;
    sf::Texture m_darknessTex;
    sf::Texture m_playerTex;

    std::array<sf::Texture, static_cast<std::size_t>(MainOption::Count)> m_mainBtnTex;
    sf::Texture m_backBtnTex;
    sf::Font m_font;

    sf::Sprite m_tileSprite;
    sf::Sprite m_playerSprite;
};

#endif // GAME_RENDERER_HPP
