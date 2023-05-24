/* Conquer Space
 * Copyright (C) 2021-2023 Conquer Space
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#pragma once

#include <RmlUi/Core/EventListener.h>
#include <RmlUi/Core/Types.h>
#include <engine/application.h>

#include <string>
#include <vector>

namespace cqsp::client {
class SettingsWindow : public Rml::EventListener {
 public:
    explicit SettingsWindow(cqsp::engine::Application& app);

    void ProcessEvent(Rml::Event& event);

    void AddEventListeners();
    void InitializeOptionVariables();
    void RemoveEventListeners();
    void InitializeDataModel();

    void Show();
    void Hide();

    void Close();

    void LoadDocument();
    void ReloadDocument();

    // Various functions for hiding the window.
    float GetOpacity();
    void PushToBack();

 private:
    const std::string document_name = "../data/core/gui/options.rml";

    cqsp::engine::Application& GetApp() { return app; }

    float ui_volume;
    float music_volume;
    bool full_screen;

    std::vector<Rml::Vector2i> window_sizes = {{1024, 768},  {1280, 1024}, {1280, 720},  {1280, 800},  {1360, 768},
                                               {1366, 768},  {1440, 900},  {1600, 900},  {1680, 1050}, {1920, 1200},
                                               {1920, 1080}, {2560, 1440}, {2560, 1080}, {3440, 1440}, {3840, 2160}};

    Rml::ElementDocument* options_menu;
    Rml::DataModelHandle model_handle;

    cqsp::engine::Application& app;
};
}  // namespace cqsp::client
