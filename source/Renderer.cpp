#include "Renderer.hpp"
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace {
constexpr float MENU_START_Y = 220.0f;
constexpr float MENU_TITLE_Y = 110.0f;
constexpr float MENU_BUTTON_SPACING = 84.0f;
constexpr float CREDIT_LINE_SPACING = 50.0f;
constexpr float CREDIT_BUTTON_GAP = 40.0f;
constexpr float OPTIONS_LABEL_Y = 230.0f;
constexpr float OPTIONS_VALUE_Y = 275.0f;
constexpr float OPTIONS_BAR_Y = 335.0f;
constexpr float OPTIONS_BAR_WIDTH = 360.0f;
constexpr float OPTIONS_BAR_HEIGHT = 28.0f;
constexpr float OPTIONS_BACK_Y = 500.0f;
}

Renderer::Renderer(sf::RenderWindow& win)
    : m_window(win), m_tileSprite(m_floorTex), m_playerSprite(m_playerTex){
    m_window.setFramerateLimit(FRAME_RATE);
}

bool Renderer::loadResources() {
    using Option = MainOption;

    const bool loaded =
                        m_floorTex.loadFromFile(FLOOR_TEX) &&
                        m_wallTex.loadFromFile(WALL_TEX) &&
                        m_playerTex.loadFromFile(PLAYER_TEX) &&
                        m_mainBtnTex[static_cast<std::size_t>(Option::Play)].loadFromFile(PLAY_BTN_TEX) &&
                        m_mainBtnTex[static_cast<std::size_t>(Option::Options)].loadFromFile(OPTIONS_BTN_TEX) &&
                        m_mainBtnTex[static_cast<std::size_t>(Option::Credits)].loadFromFile(CREDITS_BTN_TEX) &&
                        m_mainBtnTex[static_cast<std::size_t>(Option::Exit)].loadFromFile(EXIT_BTN_TEX) &&
                        m_backBtnTex.loadFromFile(BACK_BTN_TEX) &&
                        m_font.openFromFile(MENU_FONT);

    if (!loaded) {
        return false;
    }
    m_playerSprite.setTexture(m_playerTex, true);
    const sf::FloatRect bounds = m_playerSprite.getLocalBounds();
    m_playerSprite.setOrigin({ bounds.size.x * 0.5f, bounds.size.y * 0.5f });
    
    // Build a 256x256 radial alpha mask: transparent center -> solid black edge.
    constexpr unsigned TEX_SIZE = 256;
    sf::Image darknessImg({ TEX_SIZE, TEX_SIZE }, sf::Color::Black);
    const float half = static_cast<float>(TEX_SIZE) * 0.5f;

    for (unsigned y = 0; y < TEX_SIZE; ++y) {
        for (unsigned x = 0; x < TEX_SIZE; ++x) {
            const float dx = (static_cast<float>(x) + 0.5f - half) / half;
            const float dy = (static_cast<float>(y) + 0.5f - half) / half;
            const float dist = std::clamp(std::sqrt(dx * dx + dy * dy), 0.0f, 1.0f);
            // Smoothstep fade-out from inner lit core to total darkness at dist = 1.
            const float t = std::clamp((dist - 0.15f) / 0.85f, 0.0f, 1.0f);
            const float alphaF = t * t * (3.0f - 2.0f * t);
            const auto alpha = static_cast<std::uint8_t>(alphaF * 255.0f + 0.5f);
            darknessImg.setPixel({ x, y }, sf::Color(0, 0, 0, alpha));
        }
    }

    if (!m_darknessTex.loadFromImage(darknessImg)) {
        return false;
    }
    m_darknessTex.setSmooth(true);
    return true;
}

const sf::Texture& Renderer::menuButtonTexture(const Menu& menu, std::size_t index) const {
    if (menu.currentScreen() == MenuScreen::Main) {
        return m_mainBtnTex[index];
    }

    return m_backBtnTex;
}

sf::Vector2f Renderer::menuButtonPosition(const Menu& menu, std::size_t index) const {
    const float centerX = static_cast<float>(m_window.getSize().x) * 0.5f;

    if (menu.currentScreen() == MenuScreen::Options) {
        return { centerX, OPTIONS_BACK_Y };
    }

    float startY = MENU_START_Y;
    if (menu.currentScreen() == MenuScreen::Credits) {
        startY += static_cast<float>(menu.creditsLines().size()) * CREDIT_LINE_SPACING;
        startY += CREDIT_BUTTON_GAP;
    }

    return {
        centerX,
        startY + static_cast<float>(index) * MENU_BUTTON_SPACING
    };
}

sf::FloatRect Renderer::volumeControlBounds() const {
    const float centerX = static_cast<float>(m_window.getSize().x) * 0.5f;
    return {
        { centerX - OPTIONS_BAR_WIDTH * 0.5f, OPTIONS_BAR_Y - OPTIONS_BAR_HEIGHT * 0.5f },
        { OPTIONS_BAR_WIDTH, OPTIONS_BAR_HEIGHT }
    };
}

sf::FloatRect Renderer::getMenuButtonBounds(const Menu& menu, std::size_t index) const {
    if (menu.currentScreen() == MenuScreen::Options &&
        index == static_cast<std::size_t>(OptionsItem::MasterVolume)) {
        return volumeControlBounds();
    }

    const sf::Texture& texture = menuButtonTexture(menu, index);
    const sf::FloatRect localBounds = sf::Sprite(texture).getLocalBounds();
    const sf::Vector2f position = menuButtonPosition(menu, index);
    const sf::Vector2f origin{
        localBounds.position.x + localBounds.size.x * 0.5f,
        localBounds.position.y + localBounds.size.y * 0.5f
    };

    return {
        { position.x - origin.x, position.y - origin.y },
        localBounds.size
    };
}

void Renderer::drawSpriteButton(const sf::Texture& texture, sf::Vector2f position) {
    sf::Sprite sprite(texture);
    const sf::FloatRect bounds = sprite.getLocalBounds();
    sprite.setOrigin({
        bounds.position.x + bounds.size.x * 0.5f,
        bounds.position.y + bounds.size.y * 0.5f
    });
    sprite.setPosition(position);
    m_window.draw(sprite);
}

void Renderer::drawVolumeControl(const Menu& menu) {
    const sf::FloatRect bounds = volumeControlBounds();

    sf::RectangleShape background(bounds.size);
    background.setPosition(bounds.position);
    background.setFillColor(sf::Color(70, 70, 70));
    m_window.draw(background);

    const float volumeRatio = static_cast<float>(menu.volume()) /
                              static_cast<float>(Menu::MAX_VOLUME);
    sf::RectangleShape fill({ bounds.size.x * volumeRatio, bounds.size.y });
    fill.setPosition(bounds.position);
    fill.setFillColor(sf::Color(235, 200, 70));
    m_window.draw(fill);
}

void Renderer::setupMapView(float zoomFactor) {
    const sf::Vector2u windowSize = m_window.getSize();
    m_mapView.setSize({
        static_cast<float>(windowSize.x) / zoomFactor,
        static_cast<float>(windowSize.y) / zoomFactor
    });
    m_window.setView(m_mapView);
}

void Renderer::updateCamera(const Player& player, const TileMap& tileMap) {
    float mapWidth  = static_cast<float>(tileMap.width())  * TILE_SIZE;
    float mapHeight = static_cast<float>(tileMap.height()) * TILE_SIZE;

    sf::Vector2f viewSize = m_mapView.getSize();
    float halfW = viewSize.x * 0.5f;
    float halfH = viewSize.y * 0.5f;

    float camX = player.body.position.x + TILE_SIZE * 0.5f;
    float camY = player.body.position.y + TILE_SIZE * 0.5f;

    if(mapWidth > viewSize.x)
        camX = std::clamp(camX, halfW, mapWidth - halfW);
    else
        camX = mapWidth * 0.5f;

    
    if(mapHeight > viewSize.y)
        camY = std::clamp(camY, halfH, mapHeight - halfH);
    else
        camY = mapHeight * 0.5f;

    m_mapView.setCenter({ camX, camY });
    m_window.setView(m_mapView);
}

void Renderer::drawMenu(const Menu& menu) {
    m_window.setView(m_window.getDefaultView());

    const float centerX = static_cast<float>(m_window.getSize().x) * 0.5f;

    sf::Text title(m_font, "Little Game", 48);
    title.setFillColor(sf::Color::White);
    const sf::FloatRect titleBounds = title.getLocalBounds();
    title.setOrigin({
        titleBounds.position.x + titleBounds.size.x * 0.5f,
        titleBounds.position.y + titleBounds.size.y * 0.5f
    });
    title.setPosition({ centerX, MENU_TITLE_Y });
    m_window.draw(title);

    if (menu.currentScreen() == MenuScreen::Options) {
        sf::Text label(m_font, "Game Volume", 30);
        label.setFillColor(sf::Color::White);
        const sf::FloatRect labelBounds = label.getLocalBounds();
        label.setOrigin({
            labelBounds.position.x + labelBounds.size.x * 0.5f,
            labelBounds.position.y + labelBounds.size.y * 0.5f
        });
        label.setPosition({ centerX, OPTIONS_LABEL_Y });
        m_window.draw(label);

        sf::Text value(m_font, "Sound: " + std::to_string(menu.volume()) + "%", 24);
        value.setFillColor(sf::Color(200, 200, 200));
        const sf::FloatRect valueBounds = value.getLocalBounds();
        value.setOrigin({
            valueBounds.position.x + valueBounds.size.x * 0.5f,
            valueBounds.position.y + valueBounds.size.y * 0.5f
        });
        value.setPosition({ centerX, OPTIONS_VALUE_Y });
        m_window.draw(value);

        drawVolumeControl(menu);
        drawSpriteButton(
            m_backBtnTex,
            menuButtonPosition(menu, static_cast<std::size_t>(OptionsItem::Back))
        );
        return;
    }

    float creditY = MENU_START_Y;
    if (menu.currentScreen() == MenuScreen::Credits) {
        for (const std::string& line : menu.creditsLines()) {
            sf::Text creditText(m_font, line, 26);
            creditText.setFillColor(sf::Color(200, 200, 200));
            const sf::FloatRect bounds = creditText.getLocalBounds();
            creditText.setOrigin({
                bounds.position.x + bounds.size.x * 0.5f,
                bounds.position.y + bounds.size.y * 0.5f
            });
            creditText.setPosition({ centerX, creditY });
            m_window.draw(creditText);
            creditY += CREDIT_LINE_SPACING;
        }
    }

    for (std::size_t index = 0; index < menu.itemCount(); ++index) {
        drawSpriteButton(
            menuButtonTexture(menu, index),
            menuButtonPosition(menu, index)
        );
    }
}

sf::RenderWindow& Renderer::getWindow() {
    return m_window;
}

void Renderer::drawMap(const TileMap& tileMap) 
{
    m_window.setView(m_mapView);
    for (int row = 0; row < tileMap.height(); ++row) {
        for (int col = 0; col < tileMap.width(); ++col) {
            m_tileSprite.setTexture(tileMap.isWall(col, row) ? m_wallTex : m_floorTex, true);
            m_tileSprite.setPosition({
                static_cast<float>(col) * TILE_SIZE,
                static_cast<float>(row) * TILE_SIZE
            });
            m_window.draw(m_tileSprite);
        }
    }
}

void Renderer::drawPlayer(const Player& player) 
{
    if (!player.active) {
        return;
    }
    m_playerSprite.setPosition({ player.body.position.x, player.body.position.y });
    m_window.draw(m_playerSprite);
}

void Renderer::drawDarkness(const Vector2D& lightCenter, float lightRadius) {
    if (lightRadius <= 0.0f) {
        return;
    }

    const sf::Vector2f viewCenter = m_mapView.getCenter();
    const sf::Vector2f viewSize   = m_mapView.getSize();

    const float viewLeft   = viewCenter.x - viewSize.x * 0.5f;
    const float viewRight  = viewCenter.x + viewSize.x * 0.5f;
    const float viewTop    = viewCenter.y - viewSize.y * 0.5f;
    const float viewBottom = viewCenter.y + viewSize.y * 0.5f;

    const float boxLeft   = lightCenter.x - lightRadius;
    const float boxRight  = lightCenter.x + lightRadius;
    const float boxTop    = lightCenter.y - lightRadius;
    const float boxBottom = lightCenter.y + lightRadius;

    // 1. Radial fade-out circle centered on the player.
    sf::Sprite lightMask(m_darknessTex);
    const sf::Vector2u texSize = m_darknessTex.getSize();
    const float diameter = lightRadius * 2.0f;
    lightMask.setScale({
        diameter / static_cast<float>(texSize.x),
        diameter / static_cast<float>(texSize.y)
    });
    lightMask.setPosition({ boxLeft, boxTop });
    m_window.draw(lightMask);

    // 2. Solid black rectangles covering the remainder of the viewport.
    auto drawBlackRect = [this](float left, float top, float right, float bottom) {
        if (right <= left || bottom <= top) {
            return;
        }
        sf::RectangleShape rect({ right - left, bottom - top });
        rect.setPosition({ left, top });
        rect.setFillColor(sf::Color::Black);
        m_window.draw(rect);
    };

    drawBlackRect(viewLeft, viewTop, viewRight, std::min(viewBottom, boxTop));
    drawBlackRect(viewLeft, std::max(viewTop, boxBottom), viewRight, viewBottom);

    const float midTop    = std::max(viewTop, boxTop);
    const float midBottom = std::min(viewBottom, boxBottom);
    drawBlackRect(viewLeft, midTop, std::min(viewRight, boxLeft), midBottom);
    drawBlackRect(std::max(viewLeft, boxRight), midTop, viewRight, midBottom);
}

void Renderer::beginFrame() {
    m_window.clear(sf::Color(100, 100, 100));
}

void Renderer::endFrame() {
    m_window.display();
}

const bool Renderer::windowHasFocus() const { 
    return m_window.hasFocus(); 
}