#ifndef GAME_MENU_HPP
#define GAME_MENU_HPP

#include <SFML/Graphics.hpp>
#include <cstddef>
#include <string>
#include <vector>

enum class MenuScreen {
    Main,
    Options,
    Credits
};

enum class MainOption {
    Play = 0,
    Options,
    Credits,
    Exit,
    Count
};

enum class OptionsItem {
    MasterVolume = 0,
    Back,
    Count
};

enum class MenuAction {
    None,
    StartGame,
    ExitGame
};

class Renderer;

/** Stores the current menu screen and applies mouse-click actions. */
class Menu {
public:
    static constexpr int MIN_VOLUME = 0;
    static constexpr int MAX_VOLUME = 100;

    Menu() = default;

    MenuAction handleMenuClick(const sf::Vector2i& pixelPos, Renderer& renderer) ;
    [[nodiscard]] MenuAction click(std::size_t index) noexcept;
    void setVolume(int volume) noexcept;

    [[nodiscard]] MenuScreen currentScreen() const noexcept;
    [[nodiscard]] std::size_t itemCount() const noexcept;
    [[nodiscard]] std::vector<std::string> creditsLines() const;
    [[nodiscard]] int volume() const noexcept;
    

private:
    static int clampVolume(int volume) noexcept;

    MenuScreen m_screen{MenuScreen::Main};
    int m_volume{MAX_VOLUME};
};

#endif // GAME_MENU_HPP
