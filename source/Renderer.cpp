#include "Renderer.hpp"

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
    : m_window(win), m_tileSprite(m_floorTex) {
    m_window.setFramerateLimit(FRAME_RATE);
}

bool Renderer::loadResources() {
    using Option = MainOption;

    return m_floorTex.loadFromFile(FLOOR_TEX) &&
           m_wallTex.loadFromFile(WALL_TEX) &&
           m_mainBtnTex[static_cast<std::size_t>(Option::Play)].loadFromFile(PLAY_BTN_TEX) &&
           m_mainBtnTex[static_cast<std::size_t>(Option::Options)].loadFromFile(OPTIONS_BTN_TEX) &&
           m_mainBtnTex[static_cast<std::size_t>(Option::Credits)].loadFromFile(CREDITS_BTN_TEX) &&
           m_mainBtnTex[static_cast<std::size_t>(Option::Exit)].loadFromFile(EXIT_BTN_TEX) &&
           m_backBtnTex.loadFromFile(BACK_BTN_TEX) &&
           m_font.openFromFile(MENU_FONT);
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
    m_mapView.setCenter({
        (MAP_WIDTH * TILE_SIZE) * 0.5f,
        (MAP_HEIGHT * TILE_SIZE) * 0.5f
    });
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

void Renderer::drawMap(const TileMap& tileMap) {
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

void Renderer::beginFrame() {
    m_window.clear(sf::Color(100, 100, 100));
}

void Renderer::endFrame() {
    m_window.display();
}
