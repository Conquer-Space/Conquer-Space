/* Conquer Space
* Copyright (C) 2021 Conquer Space
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
#include "client/systems/loadgamewindow.h"

#include <filesystem>

#include "common/util/paths.h"
#include "common/util/uuid.h"

struct SaveGame {
    SaveGame(std::string country, int date) : country(country), date(date) {}
    std::string country;
    int date;
};

static std::vector<SaveGame> saves;

cqsp::client::LoadGameWindow::LoadGameWindow(cqsp::engine::Application& app) : app(app) { InitializeDataModel(); }

void cqsp::client::LoadGameWindow::ProcessEvent(Rml::Event& event) {
    if (event.GetId() == Rml::EventId::Keydown) {
        Rml::Input::KeyIdentifier key_identifier =
            (Rml::Input::KeyIdentifier)event.GetParameter<int>("key_identifier", 0);
        if (key_identifier == Rml::Input::KI_ESCAPE) {
            Hide();
        }
    }
}

void cqsp::client::LoadGameWindow::AddEventListeners() {
    options_menu->AddEventListener(Rml::EventId::Click, this);
    options_menu->AddEventListener(Rml::EventId::Keydown, this);
    options_menu->AddEventListener(Rml::EventId::Submit, this);
}

void cqsp::client::LoadGameWindow::RemoveEventListeners() {
    options_menu->RemoveEventListener(Rml::EventId::Click, this);
    options_menu->RemoveEventListener(Rml::EventId::Keydown, this);
    options_menu->RemoveEventListener(Rml::EventId::Submit, this);
}

void cqsp::client::LoadGameWindow::InitializeDataModel() {
    // Load saves
    // List the files in the save directory
    std::filesystem::path save_path = common::util::GetCqspSavePath();
    std::filesystem::directory_iterator it(save_path);
    for (auto entry : it) {
        // Check if it has the meta file name
        if (!entry.is_directory()) {
            continue;
        }
        auto meta_file = entry.path() / "meta.hjson";
        if (!std::filesystem::exists(meta_file)) {
            continue;
        }
        Hjson::Value value = Hjson::UnmarshalFromFile(meta_file.string());
        Hjson::Value def;
        def["date"] = -1;
        value = Hjson::Merge(def, value);
        SaveGame save(entry.path().filename().string(), value["date"]);
        saves.push_back(save);
    }
    // Process all other information
    Rml::DataModelConstructor constructor = GetApp().GetRmlUiContext()->CreateDataModel("load_game");
    if (auto save_handle = constructor.RegisterStruct<SaveGame>()) {
        save_handle.RegisterMember("country", &SaveGame::country);
        save_handle.RegisterMember("date", &SaveGame::date);
    }
    constructor.RegisterArray<decltype(saves)>();
    constructor.Bind("save_list", &saves);
}

void cqsp::client::LoadGameWindow::Show() {
    options_menu->Show();
    options_menu->PullToFront();
    options_menu->Focus();
    options_menu->SetClass("visible", true);
    options_menu->RemoveProperty("display");
}

void cqsp::client::LoadGameWindow::Hide() { options_menu->SetProperty("display", "none"); }

void cqsp::client::LoadGameWindow::Close() {
    options_menu->RemoveEventListener(Rml::EventId::Click, this);
    options_menu->RemoveEventListener(Rml::EventId::Keydown, this);
    options_menu->RemoveEventListener(Rml::EventId::Submit, this);
    options_menu->Close();
}

void cqsp::client::LoadGameWindow::LoadDocument() {
    options_menu = GetApp().LoadDocument(document_name);
    AddEventListeners();
    options_menu->PushToBack();
}

void cqsp::client::LoadGameWindow::ReloadDocument() {
    options_menu = GetApp().ReloadDocument(document_name);
    AddEventListeners();
}

float cqsp::client::LoadGameWindow::GetOpacity() { return 0.0f; }

void cqsp::client::LoadGameWindow::PushToBack() {}

void cqsp::client::LoadGameWindow::GetAllGames() {}
