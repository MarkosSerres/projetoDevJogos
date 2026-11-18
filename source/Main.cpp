#include "Main.hpp"
#include <algorithm>
#include <cmath>
#include <cstdlib>
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
    m_player.hp = m_player.maxHp;
    m_player.maxStamina = Player::INITIAL_STAMINA;
    m_player.stamina = m_player.maxStamina;
    m_enemies.clear();
    m_nextEnemyId = INITIAL_ENTITY_ID;
    spawnChests(28);
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
            } else if (m_state == AppState::Menu || m_state == AppState::Paused) {
                if (m_state == AppState::Paused) {
                    if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
                        if (key->code == sf::Keyboard::Key::Escape) {
                            if (m_menu.currentScreen() == MenuScreen::PauseOptions) {
                                m_menu.openPause();
                            } else {
                                m_state = AppState::Playing;
                            }
                            continue;
                        }
                    }
                }
                if (const auto* mouse = event->getIf<sf::Event::MouseButtonPressed>()) {
                    if (mouse->button == sf::Mouse::Button::Left) 
                    {
                        const MenuAction action = m_menu.handleMenuClick(mouse->position, m_renderer);
                        if (action == MenuAction::StartGame) {
                            startNewGame();
                        } else if (action == MenuAction::ResumeGame) {
                            m_state = AppState::Playing;
                        } else if (action == MenuAction::ReturnToMenu) {
                            m_state = AppState::Menu;
                            m_menu.openMain();
                            m_renderer.getWindow().setView(m_renderer.getWindow().getDefaultView());
                        } else if (action == MenuAction::ExitGame) {
                            m_renderer.getWindow().close();
                        }
                    }
                }
            } else if (m_state == AppState::Playing) {
                if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
                    if (key->code == sf::Keyboard::Key::E) {
                        interactWithChest();
                    } else if (key->code == sf::Keyboard::Key::Escape) {
                        m_state = AppState::Paused;
                        m_menu.openPause();
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

    const bool sprinting = m_renderer.windowHasFocus() &&
                           (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift) ||
                            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RShift));

    m_player.move(checkMovement(), dt, m_tileMap, sprinting);
    for (Enemy& enemy : m_enemies) {
        enemy.update(dt, m_tileMap, m_player);
    }

    if (m_player.hp <= 0.0f) {
        m_state = AppState::Menu;
        m_renderer.getWindow().setView(m_renderer.getWindow().getDefaultView());
        return;
    }

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
        m_renderer.drawChests(m_chests);
        m_renderer.drawEnemies(m_enemies);
        const bool nearChest = (findNearestUnopenedChest() < m_chests.size());
        m_renderer.drawPlayer(m_player, nearChest);
        m_renderer.drawDarkness(m_player.body.position, m_player.visionRadius);
        m_renderer.drawMinimap(m_tileMap, m_minimap, m_player.body.position);
        m_renderer.drawHud(m_player);

        if (m_state == AppState::Paused) {
            m_renderer.drawMenu(m_menu);
        }
    }

    m_renderer.endFrame();
}

namespace {
void collectLeafRooms(const std::unique_ptr<BSPNode>& node, std::vector<Rect>& rooms) {
    if (!node) {
        return;
    }
    if (node->isLeaf()) {
        rooms.push_back(node->room);
        return;
    }
    collectLeafRooms(node->left, rooms);
    collectLeafRooms(node->right, rooms);
}

int countRoomEntrances(const TileMap& tileMap, const Rect& room) {
    auto isOpen = [&](int col, int row) -> bool {
        if (col < 0 || col >= tileMap.width() || row < 0 || row >= tileMap.height()) {
            return false;
        }
        return !tileMap.isWall(col, row);
    };

    std::vector<bool> ring;
    for (int c = room.left; c < room.right; ++c) ring.push_back(isOpen(c, room.top - 1));
    for (int r = room.top; r < room.bottom; ++r) ring.push_back(isOpen(room.right, r));
    for (int c = room.right - 1; c >= room.left; --c) ring.push_back(isOpen(c, room.bottom));
    for (int r = room.bottom - 1; r >= room.top; --r) ring.push_back(isOpen(room.left - 1, r));

    if (ring.empty()) {
        return 0;
    }

    int entrances = 0;
    const std::size_t n = ring.size();
    for (std::size_t i = 0; i < n; ++i) {
        if (ring[i] && !ring[(i + n - 1) % n]) {
            ++entrances;
        }
    }
    return entrances;
}
}

std::size_t Main::findNearestUnopenedChest() const {
    std::size_t bestIndex = m_chests.size();
    float bestDistSq = Chest::INTERACT_DISTANCE * Chest::INTERACT_DISTANCE;

    for (std::size_t i = 0; i < m_chests.size(); ++i) {
        if (m_chests[i].opened) {
            continue;
        }
        const Vector2D c = m_chests[i].center();
        const float dx = c.x - m_player.body.position.x;
        const float dy = c.y - m_player.body.position.y;
        const float distSq = dx * dx + dy * dy;
        if (distSq <= bestDistSq) {
            bestDistSq = distSq;
            bestIndex = i;
        }
    }
    return bestIndex;
}

void Main::spawnChests(int count) {
    m_chests.clear();
    if (count <= 0 || m_tileMap.width() < 2 || m_tileMap.height() < 1) {
        return;
    }

    int playerCol = 0;
    int playerRow = 0;
    m_tileMap.toIndices(m_player.body.position, playerCol, playerRow);

    std::vector<Rect> rooms;
    collectLeafRooms(m_dungeon.root, rooms);

    std::vector<Rect> singleEntranceRooms;
    std::vector<Rect> multiEntranceRooms;
    for (const Rect& r : rooms) {
        if (countRoomEntrances(m_tileMap, r) == 1) {
            singleEntranceRooms.push_back(r);
        } else {
            multiEntranceRooms.push_back(r);
        }
    }

    auto tryPlaceInArea = [&](int minCol, int maxCol, int minRow, int maxRow, float minSpacingTiles, int maxAttempts) -> bool {
        if (maxCol - minCol < 2 || maxRow - minRow < 1) {
            return false;
        }
        const float minSepSq = (minSpacingTiles * TILE_SIZE) * (minSpacingTiles * TILE_SIZE);

        for (int a = 0; a < maxAttempts; ++a) {
            const int col = minCol + (std::rand() % (maxCol - minCol - 1));
            const int row = minRow + (std::rand() % (maxRow - minRow));

            if (col < 0 || col + 1 >= m_tileMap.width() || row < 0 || row >= m_tileMap.height()) {
                continue;
            }
            if (m_tileMap.isWall(col, row) || m_tileMap.isWall(col + 1, row)) {
                continue;
            }
            if (row == playerRow && (col == playerCol || col + 1 == playerCol)) {
                continue;
            }

            Chest candidate;
            candidate.position = { static_cast<float>(col) * TILE_SIZE, static_cast<float>(row) * TILE_SIZE };
            const Vector2D c = candidate.center();

            bool tooClose = false;
            for (const Chest& existing : m_chests) {
                const Vector2D ec = existing.center();
                const float dx = c.x - ec.x;
                const float dy = c.y - ec.y;
                if (dx * dx + dy * dy < minSepSq) {
                    tooClose = true;
                    break;
                }
            }
            if (tooClose) {
                continue;
            }

            m_chests.push_back(candidate);
            return true;
        }
        return false;
    };

    // 1. Prioritize single-entrance rooms (high chance: try every single-entrance room).
    for (const Rect& r : singleEntranceRooms) {
        if (static_cast<int>(m_chests.size()) >= count) {
            break;
        }
        if (std::rand() % 10 < 9) { // 90% chance per single-entrance room
            tryPlaceInArea(r.left, r.right, r.top, r.bottom, 4.0f, 50);
        }
    }

    // 2. Fill remaining chests in multi-entrance rooms (well separated).
    for (int pass = 0; pass < 3 && static_cast<int>(m_chests.size()) < count; ++pass) {
        const float minSep = (pass == 0) ? 7.0f : 5.0f;
        for (const Rect& r : multiEntranceRooms) {
            if (static_cast<int>(m_chests.size()) >= count) {
                break;
            }
            if (pass > 0 || std::rand() % 10 < 6) {
                tryPlaceInArea(r.left, r.right, r.top, r.bottom, minSep, 50);
            }
        }
    }

    // 3. Fallback: if rooms were still not enough, pick empty cells with wide spacing.
    int fallbackAttempts = 0;
    while (static_cast<int>(m_chests.size()) < count && fallbackAttempts < 400) {
        ++fallbackAttempts;
        int col = 0;
        int row = 0;
        m_tileMap.randomEmptyCell(col, row);
        tryPlaceInArea(col, col + 2, row, row + 1, 8.0f, 1);
    }
}

void Main::interactWithChest() {
    const std::size_t bestIndex = findNearestUnopenedChest();

    if (bestIndex >= m_chests.size()) {
        return;
    }

    if (std::rand() % Chest::MIMIC_CHANCE_MODULO == 0) {
        Enemy mimic;
        mimic.id = m_nextEnemyId++;
        mimic.body.position = m_chests[bestIndex].center();
        m_enemies.push_back(std::move(mimic));
        m_chests.erase(m_chests.begin() + static_cast<std::ptrdiff_t>(bestIndex));
        return;
    }

    m_chests[bestIndex].opened = true;
    if (std::rand() % 2 == 0) {
        m_player.hp = std::min(m_player.maxHp, m_player.hp + Chest::HEAL_AMOUNT);
    } else {
        m_player.maxStamina += Chest::STAMINA_BONUS;
        m_player.stamina = std::min(m_player.maxStamina, m_player.stamina + Chest::STAMINA_BONUS);
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

const Player& Main::player() const noexcept {
    return m_player;
}

int main() {
    sf::RenderWindow window(sf::VideoMode({1100, 800}), "Joguinho");
    Main app(window);

    return app.run();
}
