#include "Menu.hpp"
#include <algorithm>

Menu::Menu(MenuSettings initialSettings)
    : m_settings(std::move(initialSettings)) {
    if (m_settings.availableResolutions.empty()) {
        m_settings.availableResolutions.push_back({1280, 720});
    }
    if (m_settings.resolutionIndex >= m_settings.availableResolutions.size()) {
        m_settings.resolutionIndex = 0;
    }
    m_settings.audioVolume = clampVolume(m_settings.audioVolume);
    m_settings.musicVolume = clampVolume(m_settings.musicVolume);
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
            return 1; // "Voltar"
    }
    return 0;
}

void Menu::moveUp() noexcept {
    const std::size_t count = itemCount();
    if (count == 0) {
        return;
    }
    m_selectedIndex = (m_selectedIndex == 0) ? (count - 1) : (m_selectedIndex - 1);
}

void Menu::moveDown() noexcept {
    const std::size_t count = itemCount();
    if (count == 0) {
        return;
    }
    m_selectedIndex = (m_selectedIndex + 1) % count;
}

void Menu::adjustLeft() noexcept {
    adjustCurrentOption(-1);
}

void Menu::adjustRight() noexcept {
    adjustCurrentOption(1);
}

void Menu::adjustCurrentOption(int delta) noexcept {
    if (m_screen != MenuScreen::Options) {
        return;
    }

    switch (static_cast<OptionsItem>(m_selectedIndex)) {
        case OptionsItem::Resolution: {
            const std::size_t count = m_settings.availableResolutions.size();
            if (count == 0) {
                return;
            }
            if (delta < 0) {
                m_settings.resolutionIndex =
                    (m_settings.resolutionIndex == 0) ? (count - 1) : (m_settings.resolutionIndex - 1);
            } else {
                m_settings.resolutionIndex = (m_settings.resolutionIndex + 1) % count;
            }
            break;
        }
        case OptionsItem::Audio:
            setAudioVolume(m_settings.audioVolume + delta * VOLUME_STEP);
            break;
        case OptionsItem::Music:
            setMusicVolume(m_settings.musicVolume + delta * VOLUME_STEP);
            break;
        case OptionsItem::Back:
        case OptionsItem::Count:
            break;
    }
}

MenuAction Menu::confirm() noexcept {
    switch (m_screen) {
        case MenuScreen::Main:
            switch (static_cast<MainOption>(m_selectedIndex)) {
                case MainOption::Play:
                    return MenuAction::StartGame;
                case MainOption::Options:
                    m_screen = MenuScreen::Options;
                    m_selectedIndex = 0;
                    return MenuAction::OpenOptions;
                case MainOption::Credits:
                    m_screen = MenuScreen::Credits;
                    m_selectedIndex = 0;
                    return MenuAction::OpenCredits;
                case MainOption::Exit:
                    return MenuAction::ExitGame;
                case MainOption::Count:
                    return MenuAction::None;
            }
            break;

        case MenuScreen::Options:
            if (static_cast<OptionsItem>(m_selectedIndex) == OptionsItem::Back) {
                return back();
            }
            adjustRight();
            return MenuAction::None;

        case MenuScreen::Credits:
            return back();
    }
    return MenuAction::None;
}

MenuAction Menu::back() noexcept {
    if (m_screen == MenuScreen::Main) {
        return MenuAction::None;
    }
    m_screen = MenuScreen::Main;
    m_selectedIndex = 0;
    return MenuAction::BackToMain;
}

void Menu::setResolutionIndex(std::size_t index) noexcept {
    if (index < m_settings.availableResolutions.size()) {
        m_settings.resolutionIndex = index;
    }
}

void Menu::setAudioVolume(int volume) noexcept {
    m_settings.audioVolume = clampVolume(volume);
}

void Menu::setMusicVolume(int volume) noexcept {
    m_settings.musicVolume = clampVolume(volume);
}

MenuScreen Menu::currentScreen() const noexcept {
    return m_screen;
}

std::size_t Menu::selectedIndex() const noexcept {
    return m_selectedIndex;
}

std::vector<std::string> Menu::currentItems() const {
    switch (m_screen) {
        case MenuScreen::Main:
            return {"Jogar", "Opcoes", "Creditos", "Sair"};

        case MenuScreen::Options: {
            const Resolution res = currentResolution();
            return {
                "Resolucao: " + std::to_string(res.width) + "x" + std::to_string(res.height),
                "Audio: " + std::to_string(m_settings.audioVolume) + "%",
                "Musica: " + std::to_string(m_settings.musicVolume) + "%",
                "Voltar"
            };
        }

        case MenuScreen::Credits:
            return {"Voltar"};
    }
    return {};
}

std::vector<std::string> Menu::creditsLines() const {
    return {
        "Desenvolvimento de Jogos",
        "Equipe do Projeto"
    };
}

const MenuSettings& Menu::settings() const noexcept {
    return m_settings;
}

Resolution Menu::currentResolution() const noexcept {
    if (m_settings.availableResolutions.empty()) {
        return Resolution{};
    }
    return m_settings.availableResolutions[m_settings.resolutionIndex];
}

int Menu::audioVolume() const noexcept {
    return m_settings.audioVolume;
}

int Menu::musicVolume() const noexcept {
    return m_settings.musicVolume;
}
