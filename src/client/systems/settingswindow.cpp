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
#include "client/systems/settingswindow.h"

cqsp::client::SettingsWindow::SettingsWindow(cqsp::engine::Application& app) : app(app) {
    InitializeDataModel();
}

void cqsp::client::SettingsWindow::ProcessEvent(Rml::Event& event) {
    std::string id_pressed = event.GetTargetElement()->GetId();
    if (event.GetId() == Rml::EventId::Keydown) {
        Rml::Input::KeyIdentifier key_identifier =
                    (Rml::Input::KeyIdentifier) event.GetParameter<int>("key_identifier", 0);
        if (key_identifier == Rml::Input::KI_ESCAPE) {
            Hide();
        }
    } else if (event.GetId() == Rml::EventId::Click) {
        if (id_pressed == "close") {
            Hide();
        }
    } else if (event.GetId() == Rml::EventId::Submit) {
        const auto& p = event.GetParameters();
        // Get the values
        GetApp().GetAudioInterface().SetMusicVolume(music_volume);
        GetApp().GetAudioInterface().SetChannelVolume(1, ui_volume);
        GetApp().SetFullScreen(full_screen);
        // Get selected index
        // Set all the options
        int selection =
                ((Rml::ElementFormControlSelect*)options_menu->GetElementById("window_size_select"))
                    ->GetSelection();

        const Rml::Vector2i& size = window_sizes[selection];
        GetApp().SetWindowDimensions(size.x, size.y);
        // Save settings and write to file
        GetApp().GetClientOptions().GetOptions()["full_screen"] = full_screen;
        GetApp().GetClientOptions().GetOptions()["audio"]["music"] = music_volume;
        GetApp().GetClientOptions().GetOptions()["audio"]["ui"] = ui_volume;
        GetApp().GetClientOptions().GetOptions()["window"]["width"] = size.x;
        GetApp().GetClientOptions().GetOptions()["window"]["height"] = size.y;
        GetApp().GetClientOptions().WriteOptions();
    }
}

void cqsp::client::SettingsWindow::AddEventListeners() {
    options_menu->AddEventListener(Rml::EventId::Click, this);
    options_menu->AddEventListener(Rml::EventId::Keydown, this);
    options_menu->AddEventListener(Rml::EventId::Submit, this);
}

void cqsp::client::SettingsWindow::InitializeOptionVariables() {
    music_volume = GetApp().GetClientOptions().GetOptions()["audio"]["music"];
    ui_volume = GetApp().GetClientOptions().GetOptions()["audio"]["ui"];
    full_screen = static_cast<bool>(GetApp().GetClientOptions().GetOptions()["full_screen"]);

    // Refresh page, I guess
    // Set selected thing
    const int height = GetApp().GetClientOptions().GetOptions()["window"]["height"];
    const int width =  GetApp().GetClientOptions().GetOptions()["window"]["width"];
    // find the width and height that fits it
    int selected_index = 0;
    for (auto size : window_sizes) {
        if (size.x == width && size.y == height) {
            // Set the value
            ((Rml::ElementFormControlSelect*)options_menu->GetElementById("window_size_select"))
                                                ->SetSelection(selected_index);
        }
        selected_index++;
    }
    if (full_screen) {
        options_menu->GetElementById("fullscreen")->SetAttribute("checked", true);
    }
    // Mark all as dirty
    model_handle.MarkAllDirty();
}

void cqsp::client::SettingsWindow::RemoveEventListeners() {
    options_menu->RemoveEventListener(Rml::EventId::Click, this);
    options_menu->RemoveEventListener(Rml::EventId::Keydown, this);
    options_menu->RemoveEventListener(Rml::EventId::Submit, this);
}

void cqsp::client::SettingsWindow::InitializeDataModel() {
    Rml::DataModelConstructor constructor = GetApp().GetRmlUiContext()->CreateDataModel("settings");
    constructor.Bind("music_volume", &music_volume);
    constructor.Bind("ui_volume", &ui_volume);
    constructor.Bind("fullscreen", &full_screen);

    if (auto vec_handle = constructor.RegisterStruct<Rml::Vector2i>()) {
        vec_handle.RegisterMember("x", &Rml::Vector2i::x);
        vec_handle.RegisterMember("y", &Rml::Vector2i::y);
    }
    constructor.RegisterArray<Rml::Vector<Rml::Vector2i>>();
    constructor.Bind("window_sizes", &window_sizes);

    model_handle = constructor.GetModelHandle();
}

void cqsp::client::SettingsWindow::Show() {
    options_menu->PullToFront();
    options_menu->Focus();
    options_menu->SetClass("visible", true);
    InitializeOptionVariables();
}

void cqsp::client::SettingsWindow::Hide() {
     options_menu->SetClass("visible", false);
}

void cqsp::client::SettingsWindow::Close() {
    options_menu->RemoveEventListener(Rml::EventId::Click, this);
    options_menu->RemoveEventListener(Rml::EventId::Keydown, this);
    options_menu->RemoveEventListener(Rml::EventId::Submit, this);
    options_menu->Close();
}

void cqsp::client::SettingsWindow::LoadDocument() {
    options_menu = GetApp().LoadDocument(document_name);
    AddEventListeners();
    InitializeOptionVariables();
    options_menu->Show();
    options_menu->PushToBack();
}

void cqsp::client::SettingsWindow::ReloadDocument() {
    options_menu = GetApp().ReloadDocument(document_name);
    AddEventListeners();
    InitializeOptionVariables();
}

float cqsp::client::SettingsWindow::GetOpacity() {
    return options_menu->GetProperty(Rml::PropertyId::Opacity)->Get<float>();
}

void cqsp::client::SettingsWindow::PushToBack() { options_menu->PushToBack(); }
