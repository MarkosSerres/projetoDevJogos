#ifndef GAME_MENU_HPP
#define GAME_MENU_HPP

#include <cstddef>
#include <string>
#include <vector>

/**
 * @file Menu.hpp
 * @brief Pure logic and state management for the game's menu system.
 *
 * Menu handles navigation, active screen state (Main, Options, Credits), and
 * configurable settings (screen resolutions, audio volume, music volume)
 * without any rendering or windowing dependencies. The graphics layer (e.g.,
 * SFML) queries Menu's state and forwards input events to it.
 */

/// Active screen within the menu state machine.
enum class MenuScreen {
    Main,
    Options,
    Credits
};

/// Top-level items available on the Main screen.
enum class MainOption {
    Play = 0,
    Options,
    Credits,
    Exit,
    Count
};

/// Items available inside the Options submenu.
enum class OptionsItem {
    Resolution = 0,
    Audio,
    Music,
    Back,
    Count
};

/// High-level action emitted when the user confirms or cancels a selection.
enum class MenuAction {
    None,
    StartGame,
    OpenOptions,
    OpenCredits,
    BackToMain,
    ExitGame
};

/// A display resolution preset in pixels.
struct Resolution {
    int width{1280};
    int height{720};
};

/// Persistent user-configurable settings managed by the Options screen.
struct MenuSettings {
    std::vector<Resolution> availableResolutions{
        {800, 600},
        {1280, 720},
        {1600, 900},
        {1920, 1080}
    };
    std::size_t resolutionIndex{1};   ///< Defaults to 1280x720.
    int         audioVolume{100};     ///< Master/SFX volume in [0, 100].
    int         musicVolume{100};     ///< Music volume in [0, 100].
};

/**
 * @brief Stateful menu controller decoupled from rendering.
 *
 * Navigation wraps around at list boundaries. On the Options screen,
 * adjustLeft() / adjustRight() modify the currently highlighted setting
 * (resolution preset, audio volume, or music volume).
 */
class Menu {
public:
    static constexpr int MIN_VOLUME  = 0;
    static constexpr int MAX_VOLUME  = 100;
    static constexpr int VOLUME_STEP = 10;

    Menu() = default;
    explicit Menu(MenuSettings initialSettings);

    // --- Navigation --------------------------------------------------------

    /// Moves selection one item up (wraps to the last item).
    void moveUp() noexcept;

    /// Moves selection one item down (wraps to the first item).
    void moveDown() noexcept;

    /// Decrements the highlighted setting on the Options screen.
    void adjustLeft() noexcept;

    /// Increments the highlighted setting on the Options screen.
    void adjustRight() noexcept;

    /**
     * @brief Activates the currently selected item.
     * @return Action describing any screen transition or game command.
     */
    [[nodiscard]] MenuAction confirm() noexcept;

    /**
     * @brief Returns from a submenu (Options / Credits) to the Main screen.
     * @return MenuAction::BackToMain if a transition occurred, else None.
     */
    [[nodiscard]] MenuAction back() noexcept;

    // --- Settings mutators -------------------------------------------------

    void setResolutionIndex(std::size_t index) noexcept;
    void setAudioVolume(int volume) noexcept;
    void setMusicVolume(int volume) noexcept;

    // --- Queries (for game loop & renderer) --------------------------------

    [[nodiscard]] MenuScreen                currentScreen() const noexcept;
    [[nodiscard]] std::size_t               selectedIndex() const noexcept;
    [[nodiscard]] std::size_t               itemCount() const noexcept;
    [[nodiscard]] std::vector<std::string>  currentItems() const;
    [[nodiscard]] std::vector<std::string>  creditsLines() const;

    [[nodiscard]] const MenuSettings&       settings() const noexcept;
    [[nodiscard]] Resolution                currentResolution() const noexcept;
    [[nodiscard]] int                       audioVolume() const noexcept;
    [[nodiscard]] int                       musicVolume() const noexcept;

private:
    static int clampVolume(int volume) noexcept;
    void adjustCurrentOption(int delta) noexcept;

    MenuScreen   m_screen{MenuScreen::Main};
    std::size_t  m_selectedIndex{0};
    MenuSettings m_settings{};
};

#endif // GAME_MENU_HPP
