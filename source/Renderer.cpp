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
constexpr int   WALL_TILE_SPACING = 1;
constexpr int   WALL_TILE_STRIDE  = static_cast<int>(TILE_SIZE) + WALL_TILE_SPACING;

unsigned tileHash(int col, int row) noexcept {
    return static_cast<unsigned>(col) * 73856093u ^
           static_cast<unsigned>(row) * 19349663u;
}

bool isSolidCell(const TileMap& tileMap, int col, int row) {
    if (col < 0 || col >= tileMap.width()) {
        return false;
    }
    if (row < 0 || row >= tileMap.height()) {
        return true; // Top (-1) and bottom (height) border rows are solid.
    }
    return tileMap.isWall(col, row);
}

bool isRoofCell(const TileMap& tileMap, int col, int row) {
    if (!isSolidCell(tileMap, col, row)) {
        return false;
    }
    // A solid cell is a roof iff the cell directly below it is also solid.
    // (Solid cells directly above floor are drawn as front-facing walls.)
    return isSolidCell(tileMap, col, row + 1);
}

int selectWallOrRoofTile(const TileMap& tileMap, int col, int row) {
    if (!isRoofCell(tileMap, col, row)) {
        // Front-facing wall (tiles 0..3).
        const bool leftEmpty  = !isSolidCell(tileMap, col - 1, row);
        const bool rightEmpty = !isSolidCell(tileMap, col + 1, row);

        if (leftEmpty && !rightEmpty) {
            return 2; // Left corner wall
        }
        if (rightEmpty && !leftEmpty) {
            return 3; // Right corner wall
        }
        return (tileHash(col, row) % 4u == 0u) ? 1 : 0; // Base or variation
    }

    // Roof tile (tiles 4..10) based on bordering non-roof neighbors.
    const bool borderTop    = !isRoofCell(tileMap, col, row - 1);
    const bool borderBottom = !isRoofCell(tileMap, col, row + 1);
    const bool borderLeft   = !isRoofCell(tileMap, col - 1, row);
    const bool borderRight  = !isRoofCell(tileMap, col + 1, row);

    if (borderTop && borderBottom && borderLeft && borderRight) {
        return 4; // All sides
    }
    if (borderBottom && borderLeft && borderRight) {
        return 5; // Bottom + left + right
    }
    if (borderTop && borderLeft && borderRight) {
        return 10; // Top + left + right
    }
    if (borderLeft && borderRight) {
        return 6; // Left + right
    }
    if (borderRight) {
        return 7; // Right only
    }
    if (borderLeft) {
        return 9; // Left only
    }
    return 8; // No borders
}
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
    const int tilePx = static_cast<int>(TILE_SIZE);

    for (int row = -1; row <= tileMap.height(); ++row) {
        for (int col = 0; col < tileMap.width(); ++col) {
            int tileCol = 0;
            int tileRow = 0;

            if (row < 0 || row >= tileMap.height() || tileMap.isWall(col, row)) {
                tileRow = 0;
                tileCol = selectWallOrRoofTile(tileMap, col, row);
            } else {
                tileRow = 1;
                tileCol = (tileHash(col, row) % 9u == 0u) ? 1 : 0; // 8 padrão : 1 variante
            }

            m_tileSprite.setTexture(m_wallTex);
            m_tileSprite.setTextureRect(sf::IntRect(
                { tileCol * WALL_TILE_STRIDE, tileRow * WALL_TILE_STRIDE },
                { tilePx, tilePx }
            ));

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

void Renderer::drawMinimap(const TileMap& tileMap, const Minimap& minimap, const Vector2D& playerPos) {
    if (tileMap.width() <= 0 || tileMap.height() <= 0) {
        return;
    }

    constexpr float MINIMAP_RADIUS  = 95.0f;
    constexpr float MINIMAP_MARGIN  = 20.0f;
    constexpr float MINIMAP_TILE_PX = 6.0f;
    constexpr float FADE_INNER_FRAC = 0.55f;
    constexpr int   CIRCLE_SEGMENTS = 48;

    m_window.setView(m_window.getDefaultView());
    const sf::Vector2u winSize = m_window.getSize();
    const sf::Vector2f center{
        static_cast<float>(winSize.x) - MINIMAP_MARGIN - MINIMAP_RADIUS,
        MINIMAP_MARGIN + MINIMAP_RADIUS
    };

    // 1. Soft circular backdrop with no hard outline (fades to transparent at edge).
    sf::VertexArray backdrop(sf::PrimitiveType::TriangleFan, CIRCLE_SEGMENTS + 2);
    backdrop[0].position = center;
    backdrop[0].color    = sf::Color(0, 0, 0, 150);

    constexpr float TWO_PI = 6.28318530718f;
    for (int i = 0; i <= CIRCLE_SEGMENTS; ++i) {
        const float angle = (static_cast<float>(i) / static_cast<float>(CIRCLE_SEGMENTS)) * TWO_PI;
        backdrop[static_cast<std::size_t>(i + 1)].position = {
            center.x + std::cos(angle) * MINIMAP_RADIUS,
            center.y + std::sin(angle) * MINIMAP_RADIUS
        };
        backdrop[static_cast<std::size_t>(i + 1)].color = sf::Color(0, 0, 0, 0);
    }
    m_window.draw(backdrop);

    // 2. White path/wall outlines for explored cells, fading out near the circular edge.
    auto edgeAlpha = [&](sf::Vector2f pt) -> std::uint8_t {
        const float dx = pt.x - center.x;
        const float dy = pt.y - center.y;
        const float dist = std::sqrt(dx * dx + dy * dy);
        if (dist >= MINIMAP_RADIUS) {
            return 0;
        }
        const float innerR = MINIMAP_RADIUS * FADE_INNER_FRAC;
        if (dist <= innerR) {
            return 255;
        }
        const float t = (dist - innerR) / (MINIMAP_RADIUS - innerR);
        const float fade = 1.0f - (t * t * (3.0f - 2.0f * t));
        return static_cast<std::uint8_t>(fade * 255.0f + 0.5f);
    };

    const float playerTileX = playerPos.x / TILE_SIZE;
    const float playerTileY = playerPos.y / TILE_SIZE;
    const int tileRange = static_cast<int>(MINIMAP_RADIUS / MINIMAP_TILE_PX) + 2;

    const int minCol = std::max(0, static_cast<int>(std::floor(playerTileX)) - tileRange);
    const int maxCol = std::min(tileMap.width() - 1, static_cast<int>(std::floor(playerTileX)) + tileRange);
    const int minRow = std::max(0, static_cast<int>(std::floor(playerTileY)) - tileRange);
    const int maxRow = std::min(tileMap.height() - 1, static_cast<int>(std::floor(playerTileY)) + tileRange);

    sf::VertexArray lines(sf::PrimitiveType::Lines);
    auto pushEdge = [&](sf::Vector2f a, sf::Vector2f b) {
        const std::uint8_t alphaA = edgeAlpha(a);
        const std::uint8_t alphaB = edgeAlpha(b);
        if (alphaA == 0 && alphaB == 0) {
            return;
        }
        lines.append(sf::Vertex{ a, sf::Color(255, 255, 255, alphaA) });
        lines.append(sf::Vertex{ b, sf::Color(255, 255, 255, alphaB) });
    };

    auto isBlocked = [&](int c, int r) -> bool {
        return c < 0 || c >= tileMap.width() || r < 0 || r >= tileMap.height() || tileMap.isWall(c, r);
    };

    for (int row = minRow; row <= maxRow; ++row) {
        for (int col = minCol; col <= maxCol; ++col) {
            if (!minimap.isExplored(col, row) || tileMap.isWall(col, row)) {
                continue;
            }

            const float left   = center.x + (static_cast<float>(col) - playerTileX) * MINIMAP_TILE_PX;
            const float right  = left + MINIMAP_TILE_PX;
            const float top    = center.y + (static_cast<float>(row) - playerTileY) * MINIMAP_TILE_PX;
            const float bottom = top + MINIMAP_TILE_PX;

            if (isBlocked(col, row - 1)) pushEdge({ left, top }, { right, top });
            if (isBlocked(col, row + 1)) pushEdge({ left, bottom }, { right, bottom });
            if (isBlocked(col - 1, row)) pushEdge({ left, top }, { left, bottom });
            if (isBlocked(col + 1, row)) pushEdge({ right, top }, { right, bottom });
        }
    }
    m_window.draw(lines);

    // 3. Red dot representing the player at the center of the minimap.
    constexpr float DOT_RADIUS = 3.5f;
    sf::CircleShape playerDot(DOT_RADIUS);
    playerDot.setOrigin({ DOT_RADIUS, DOT_RADIUS });
    playerDot.setPosition(center);
    playerDot.setFillColor(sf::Color(235, 55, 55));
    m_window.draw(playerDot);
}

void Renderer::drawHud(const Player& player) {
    m_window.setView(m_window.getDefaultView());

    auto drawBar = [this](sf::Vector2f pos, sf::Vector2f size, float ratio, sf::Color fillColor) {
        const float clamped = std::clamp(ratio, 0.0f, 1.0f);

        sf::RectangleShape background(size);
        background.setPosition(pos);
        background.setFillColor(sf::Color(25, 25, 25, 200));
        background.setOutlineThickness(1.5f);
        background.setOutlineColor(sf::Color(60, 60, 60, 220));
        m_window.draw(background);

        if (clamped > 0.0f) {
            sf::RectangleShape fill({ size.x * clamped, size.y });
            fill.setPosition(pos);
            fill.setFillColor(fillColor);
            m_window.draw(fill);
        }
    };

    const float hpRatio = (player.maxHp > 0.0f) ? (player.hp / player.maxHp) : 0.0f;
    const float staminaRatio = (player.maxStamina > 0.0f) ? (player.stamina / player.maxStamina) : 0.0f;

    drawBar({ 20.0f, 20.0f }, { 220.0f, 18.0f }, hpRatio, sf::Color(220, 45, 45));
    drawBar({ 20.0f, 46.0f }, { 220.0f, 14.0f }, staminaRatio, sf::Color(0, 210, 235));
}

void Renderer::beginFrame() {
    m_window.clear(sf::Color::Black);
}

void Renderer::endFrame() {
    m_window.display();
}

const bool Renderer::windowHasFocus() const { 
    return m_window.hasFocus(); 
}