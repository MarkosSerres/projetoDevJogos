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
    m_renderer.setupMapView(0.9);
    m_state = AppState::Menu;
    m_running = true;
    return true;
}

void Main::startNewGame() {
    m_dungeon.generate(m_seed);
    m_tileMap.loadFromDungeon(m_dungeon);
    m_minimap.reset(m_tileMap.width(), m_tileMap.height());

    int startCol = 0, startRow = 0;
    m_tileMap.randomEmptyCell(startCol, startRow);
    m_player.body.position = m_tileMap.toWorld(startCol, startRow);
    m_player.active = true;
    m_minimap.revealAround(m_tileMap, m_player.body.position);

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
                    if (mouse->button == sf::Mouse::Button::Left) 
                    {
                        const MenuAction action = m_menu.handleMenuClick(mouse->position, m_renderer);
                        if (action == MenuAction::StartGame) {
                            startNewGame();
                        } else if (action == MenuAction::ExitGame) {
                            m_renderer.getWindow().close();
                        }
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

    m_player.move(checkMovement(), dt, m_tileMap);
    m_minimap.revealAround(m_tileMap, m_player.body.position);
    if (m_player.active){
        m_renderer.updateCamera(m_player, m_tileMap);
    }
}

 Vector2D Main::checkMovement() const{
    if(!m_renderer.windowHasFocus())
        return {0.0f,0.0f};
    
    Vector2D moveDir{0.0f, 0.0f};

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
        moveDir.y -= 1.0f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
        moveDir.y += 1.0f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
        moveDir.x -= 1.0f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
        moveDir.x += 1.0f;

    if(moveDir.x != 0.f || moveDir.y != 0.f)
        moveDir = moveDir.normalized();

    return moveDir;
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
        m_renderer.drawPlayer(m_player);
        m_renderer.drawDarkness(m_player.body.position, m_player.visionRadius);
        m_renderer.drawMinimap(m_tileMap, m_minimap, m_player.body.position);
    }

    m_renderer.endFrame();
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

const Player& Main::player() const noexcept {
    return m_player;
}

int main() {
    sf::RenderWindow window(sf::VideoMode({1100, 800}), "Joguinho");
    Main app(window);

    return app.run();
}
