#include "Menu.hpp"

#include <algorithm>

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
    }

    return MenuAction::None;
}

void Menu::setVolume(int volume) noexcept {
    m_volume = clampVolume(volume);
}

int Menu::clampVolume(int volume) noexcept {
    return std::clamp(volume, MIN_VOLUME, MAX_VOLUME);
}

std::size_t Menu::itemCount() const noexcept {
    switch (m_screen) {
        case MenuScreen::Main:
            return static_cast<std::size_t>(MainOption::Count);
        case MenuScreen::Options:
            return static_cast<std::size_t>(OptionsItem::Count);
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
