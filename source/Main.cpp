#include "Main.hpp"
#include <iostream>

Main::Main(int mapWidthTiles, int mapHeightTiles, unsigned seed)
    : m_dungeon(mapWidthTiles, mapHeightTiles)
    , m_seed(seed) {
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

    m_running = true;
    return true;
}

int Main::run() {
    if (!init()) {
        std::cerr << "Failed to initialize game.\n";
        return 1;
    }

    // Minimal single-step execution until the real-time loop and renderer
    // (e.g., SFML window + clock) are wired in.
    constexpr float fixedDt = 1.0f / 60.0f;
    if (m_running) {
        update(fixedDt);
        render();
        shutdown();
    }

    return 0;
}

void Main::update(float dt) {
    if (!m_running) {
        return;
    }

    // TODO: integrate RigidBody2D, Collision2D, PathFind and BehaviorTree ticks.
    (void)dt;
}

void Main::render() const {
    if (!m_running) {
        return;
    }

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
    Main app;
    return app.run();
}
