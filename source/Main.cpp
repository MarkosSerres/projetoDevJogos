#include "Main.hpp"
#include <algorithm>
#include <iostream>

Main::Main(sf::RenderWindow& win, int mapWidthTiles, int mapHeightTiles, unsigned seed)
    : m_dungeon(mapWidthTiles, mapHeightTiles)
    , m_seed(seed)
    , m_renderer(win) 
{
    m_player.id = PLAYER_ID;
}

bool Main::init() {
    if (!m_renderer.loadResources()) {
        return false;
    }
    m_state = AppState::Menu;
    m_running = true;
    return true;
}

void Main::startNewGame() {
    m_dungeon.generate(m_seed);
    m_tileMap.loadFromDungeon(m_dungeon);

    int startCol = 0, startRow = 0;
    m_tileMap.randomEmptyCell(startCol, startRow);
    m_player.body.position = m_tileMap.toWorld(startCol, startRow);
    m_player.active = true;

    m_renderer.setupMapView();
    m_state = AppState::Playing;
    m_running = true;
}

int Main::run() {
    if (!init()) {
        std::cerr << "Failed to initialize game.\n";
        return 1;
    }

    sf::Clock clock;
    
    while(m_renderer.getWindow().isOpen()){
        while (const std::optional event = m_renderer.getWindow().pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                m_renderer.getWindow().close();
            } else if (m_state == AppState::Menu) {
                if (const auto* mouse = event->getIf<sf::Event::MouseButtonPressed>()) {
                    if (mouse->button == sf::Mouse::Button::Left) {
                        handleMenuClick(mouse->position);
                    }
                }
            }
        }
        float dt = std::min(clock.restart().asSeconds(), FIXED_DT);

        if(dt > 0){
            update(dt);
        }

        render();
    }

    shutdown();
    return 0;
}

void Main::update(float dt) {
    if (!m_running || m_state != AppState::Playing) {
        return;
    }

    // TODO: integrate RigidBody2D, Collision2D, PathFind and BehaviorTree ticks.
    (void)dt;
}

void Main::render() {
    if (!m_running) {
        return;
    }

    m_renderer.beginFrame();

        if (m_state == AppState::Menu) {
        m_renderer.drawMenu(m_menu);
        } else {
        m_renderer.drawMap(m_tileMap);
        }

    m_renderer.endFrame();
}

void Main::handleMenuClick(const sf::Vector2i& pixelPos) {
    const sf::Vector2f worldPos = m_renderer.getWindow().mapPixelToCoords(
        pixelPos, m_renderer.getWindow().getDefaultView()
    );

    const std::size_t count = m_menu.itemCount();
    for (std::size_t i = 0; i < count; ++i) {
        const sf::FloatRect bounds = m_renderer.getMenuButtonBounds(m_menu, i);
        if (!bounds.contains(worldPos)) {
            continue;
        }

        if (m_menu.currentScreen() == MenuScreen::Options &&
            i == static_cast<std::size_t>(OptionsItem::MasterVolume)) {
            const float relativePosition = std::clamp(
                (worldPos.x - bounds.position.x) / bounds.size.x,
                0.0f,
                1.0f
            );
            m_menu.setVolume(static_cast<int>(relativePosition * 100.0f + 0.5f));
            break;
        }

        const MenuAction action = m_menu.click(i);

        switch (action) {
            case MenuAction::StartGame:
                startNewGame();
                break;
            case MenuAction::ExitGame:
                m_renderer.getWindow().close();
                break;
            default:
                break;
        }

        break;
    }
}

void Main::shutdown() noexcept {
    m_running = false;
}

bool Main::isRunning() const noexcept {
    return m_running;
}

const TileMap& Main::tileMap() const noexcept {
    return m_tileMap;
}

const Entity& Main::player() const noexcept {
    return m_player;
}

int main() {
    sf::RenderWindow window(sf::VideoMode({1200, 900}), "Joguinho");
    Main app(window);

    return app.run();
}
