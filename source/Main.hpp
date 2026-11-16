#ifndef GAME_MAIN_HPP
#define GAME_MAIN_HPP

#include <memory>
#include "Dungeon2D.hpp"
#include "Entity.hpp"
#include "Global.hpp"
#include "TileMap.hpp"

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
class Main {
public:
    /**
     * @brief Constructs the application controller with map dimensions and RNG seed.
     * @param mapWidthTiles  Width of the generated dungeon in tiles (> 0).
     * @param mapHeightTiles Height of the generated dungeon in tiles (> 0).
     * @param seed           Seed passed to Dungeon2D::generate().
     */
    Main(int mapWidthTiles = 40, int mapHeightTiles = 25, unsigned seed = 42);

    ~Main() = default;

    // --- Lifecycle ---------------------------------------------------------

    /**
     * @brief Initializes world subsystems before entering the loop.
     * @details Generates the BSP dungeon, rasterizes it into the TileMap, and
     *          places the player entity at a valid empty cell.
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
    void render() const;

    /**
     * @brief Releases resources and marks the loop as stopped.
     */
    void shutdown() noexcept;

    // --- Queries -----------------------------------------------------------

    /// True while the main loop is active.
    [[nodiscard]] bool isRunning() const noexcept;

    /// Read-only access to the active tile map.
    [[nodiscard]] const TileMap& tileMap() const noexcept;

    /// Read-only access to the player entity.
    [[nodiscard]] const Entity& player() const noexcept;

private:
    Dungeon2D m_dungeon;
    TileMap   m_tileMap;
    Entity    m_player;
    unsigned  m_seed{42};
    bool      m_running{false};
};

#endif // GAME_MAIN_HPP
