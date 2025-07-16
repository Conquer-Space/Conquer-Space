/* Conquer Space
 * Copyright (C) 2021-2025 Conquer Space
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

#include <RmlUi/Core/DataModelHandle.h>

#include <string>
#include <vector>

#include "RmlUi/Core/EventListener.h"
#include "RmlUi/Core/Types.h"
#include "engine/application.h"

namespace cqsp::client {
class LoadGameWindow : public Rml::EventListener {
 public:
    explicit LoadGameWindow(engine::Application& app);

    void ProcessEvent(Rml::Event& event);

    void AddEventListeners();
    void RemoveEventListeners();
    void InitializeDataModel();

    void Show();
    void Hide();

    void Close();

    void LoadDocument();
    void ReloadDocument();
    bool Update();

    // Various functions for hiding the window.
    float GetOpacity();
    void PushToBack();

    void GetAllGames();
    std::string GetSaveDir();

    struct SaveGame {
        SaveGame(std::string country, int date) : country(country), date(date) {}
        std::string country;
        std::string path;
        int date;
    };

    std::vector<SaveGame> saves;

 private:
    const std::string document_name = "../data/core/gui/loadsave.rml";

    engine::Application& GetApp() { return app; }

    Rml::ElementDocument* options_menu;
    Rml::DataModelHandle model_handle;

    engine::Application& app;

    std::string load_path;
    bool to_load = false;
};
}  // namespace cqsp::client
