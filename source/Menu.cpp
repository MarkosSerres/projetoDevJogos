#include "Menu.hpp"
#include "Renderer.hpp"
#include <algorithm>

MenuAction Menu::handleMenuClick(const sf::Vector2i& pixelPos, Renderer& renderer) 
{
    const sf::Vector2f worldPos = renderer.getWindow().mapPixelToCoords(pixelPos, renderer.getWindow().getDefaultView());

    const std::size_t count = itemCount();
    for (std::size_t i = 0; i < count; ++i) {
        const sf::FloatRect bounds = renderer.getMenuButtonBounds(*this, i);
        if (!bounds.contains(worldPos)) {
            continue;
        }

        if ((currentScreen() == MenuScreen::Options || currentScreen() == MenuScreen::PauseOptions) &&
            i == static_cast<std::size_t>(OptionsItem::MasterVolume)) {
            const float relativePosition = std::clamp((worldPos.x - bounds.position.x) / bounds.size.x, 0.0f, 1.0f);
            setVolume(static_cast<int>(relativePosition * 100.0f + 0.5f));
            return MenuAction::None;
        }

        return click(i);
    }

    return MenuAction::None;
}


MenuAction Menu::click(std::size_t index) noexcept {
    if (index >= itemCount()) {
        return MenuAction::None;
    }

    switch (m_screen) {
        case MenuScreen::Main:
            switch (static_cast<MainOption>(index)) {
                case MainOption::Play:
                    return MenuAction::StartGame;
                case MainOption::Options:
                    m_screen = MenuScreen::Options;
                    return MenuAction::None;
                case MainOption::Credits:
                    m_screen = MenuScreen::Credits;
                    return MenuAction::None;
                case MainOption::Exit:
                    return MenuAction::ExitGame;
                case MainOption::Count:
                    return MenuAction::None;
            }
            break;

        case MenuScreen::Options:
            if (index == static_cast<std::size_t>(OptionsItem::Back)) {
                m_screen = MenuScreen::Main;
            }
            return MenuAction::None;

        case MenuScreen::Credits:
            if (index == 0) {
                m_screen = MenuScreen::Main;
            }
            return MenuAction::None;

        case MenuScreen::Pause:
            switch (static_cast<PauseOption>(index)) {
                case PauseOption::Resume:
                    return MenuAction::ResumeGame;
                case PauseOption::Options:
                    m_screen = MenuScreen::PauseOptions;
                    return MenuAction::None;
                case PauseOption::MainMenu:
                    m_screen = MenuScreen::Main;
                    return MenuAction::ReturnToMenu;
                case PauseOption::Count:
                    return MenuAction::None;
            }
            break;

        case MenuScreen::PauseOptions:
            if (index == static_cast<std::size_t>(OptionsItem::Back)) {
                m_screen = MenuScreen::Pause;
            }
            return MenuAction::None;
    }

    return MenuAction::None;
}

void Menu::setVolume(int volume) noexcept {
    m_volume = clampVolume(volume);
}

void Menu::openPause() noexcept {
    m_screen = MenuScreen::Pause;
}

void Menu::openMain() noexcept {
    m_screen = MenuScreen::Main;
}

int Menu::clampVolume(int volume) noexcept {
    return std::clamp(volume, MIN_VOLUME, MAX_VOLUME);
}

std::size_t Menu::itemCount() const noexcept {
    switch (m_screen) {
        case MenuScreen::Main:
            return static_cast<std::size_t>(MainOption::Count);
        case MenuScreen::Options:
        case MenuScreen::PauseOptions:
            return static_cast<std::size_t>(OptionsItem::Count);
        case MenuScreen::Pause:
            return static_cast<std::size_t>(PauseOption::Count);
        case MenuScreen::Credits:
            return 1;
    }
    return 0;
}

MenuScreen Menu::currentScreen() const noexcept {
    return m_screen;
}

int Menu::volume() const noexcept {
    return m_volume;
}

std::vector<std::string> Menu::creditsLines() const {
    return {
        "Game Dev",
        "Team Project",
        "Markos And Samuel"
    };
}
