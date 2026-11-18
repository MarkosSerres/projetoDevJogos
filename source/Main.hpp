#ifndef GAME_MAIN_HPP
#define GAME_MAIN_HPP

#include "Dungeon2D.hpp"
#include "Global.hpp"
#include "Minimap.hpp"
#include "Player.hpp"
#include "TileMap.hpp"
#include "Renderer.hpp"
#include "Menu.hpp"

/**
 * @file Main.hpp
 * @brief Application entry point and top-level game loop orchestrator.
 *
 * Main owns the core world state (dungeon geometry, tile map, and entities)
 * and drives the lifecycle of a game session: initialization, fixed/variable
 * frame updates, rendering, and shutdown.
 *
 * Following the project's coordinate conventions (Global.hpp), world positions
 * are stored in continuous pixels (Y-down) while the discrete grid lives in
 * TileMap.
 */

enum class AppState {
    Menu,
    Playing
};

class Main {
public:
    /**
     * @brief Constructs the application controller with map dimensions and RNG seed.
     * @param mapWidthTiles  Width of the generated dungeon in tiles (> 0).
     * @param mapHeightTiles Height of the generated dungeon in tiles (> 0).
     * @param seed           Seed passed to Dungeon2D::generate().
     */
    Main(sf::RenderWindow& win, int mapWidthTiles = MAP_WIDTH, int mapHeightTiles = MAP_HEIGHT, unsigned seed = SEED);

    // --- Lifecycle ---------------------------------------------------------

    /**
     * @brief Loads resources and prepares the menu before entering the loop.
     * @return true if initialization succeeded.
     */
    [[nodiscard]] bool init();

    /**
     * @brief Runs the main application loop until shutdown is requested.
     * @return Exit status code (0 on normal termination).
     */
    int run();

    /**
     * @brief Advances simulation state by one frame.
     * @param dt Elapsed time since the previous frame, in seconds.
     */
    void update(float dt);

    /**
     * @brief Renders the current world state.
     */
    void render();

    /** Marks the application loop as stopped. */
    void shutdown() noexcept;

    // --- Queries -----------------------------------------------------------

    /// True while the main loop is active.
    [[nodiscard]] bool isRunning() const noexcept;

    /// Read-only access to the active tile map.
    [[nodiscard]] const TileMap& tileMap() const noexcept;

    /// Read-only access to the player entity.
    [[nodiscard]] const Player& player() const noexcept;

private:
    void startNewGame();
    Vector2D checkMovement() const;

    Dungeon2D m_dungeon;
    TileMap   m_tileMap;
    Minimap   m_minimap;
    Player    m_player;
    unsigned  m_seed;
    bool      m_running{false};
    Renderer  m_renderer;
    Menu      m_menu;
    AppState  m_state{AppState::Menu};
};

#endif // GAME_MAIN_HPP
