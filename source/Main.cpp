#include "Main.hpp"
#include <iostream>

Main::Main(sf::RenderWindow& win, int mapWidthTiles, int mapHeightTiles, unsigned seed)
    : m_dungeon(mapWidthTiles, mapHeightTiles)
    , m_seed(seed)
    , m_renderer(win) 
{
    m_player.id = PLAYER_ID;
}

bool Main::init() {
    m_dungeon.generate(m_seed);
    m_tileMap.loadFromDungeon(m_dungeon);

    if (m_tileMap.width() <= 0 || m_tileMap.height() <= 0) {
        return false;
    }

    int startCol = 0;
    int startRow = 0;
    m_tileMap.randomEmptyCell(startCol, startRow);
    m_player.body.position = m_tileMap.toWorld(startCol, startRow);
    m_player.active = true;
    
    if(!m_renderer.loadResources()){ 
        
        return false;
    }
    m_renderer.setupMapView();
    m_running = true;
    return m_running = true;
}

int Main::run() {
    if (!init()) {
        std::cerr << "Failed to initialize game.\n";
        return 1;
    }

    // Minimal single-step execution until the real-time loop and renderer
    // (e.g., SFML window + clock) are wired in.
    if (m_running) {
        update(FIXED_DT);
        render();
        shutdown();
    }

    sf::Clock clock;
    
    while(m_renderer.getWindow().isOpen()){
        while(const std::optional event = m_renderer.getWindow().pollEvent()){
            if(event->is<sf::Event::Closed>()){
                m_renderer.getWindow().close();
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
    if (!m_running) {
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
    m_renderer.drawMap(m_tileMap);
    m_renderer.endFrame();
    // Placeholder console output until SFML rendering is attached.
    std::cout << "Dungeon ready: "
              << m_tileMap.width() << "x" << m_tileMap.height() << " tiles | "
              << "Player (" << m_player.id << ") at ("
              << m_player.body.position.x << ", "
              << m_player.body.position.y << ")\n";
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
