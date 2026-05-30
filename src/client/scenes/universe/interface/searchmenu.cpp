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
#include "client/scenes/universe/interface/searchmenu.h"

#include "RmlUi/Core/Elements/ElementFormControlInput.h"

namespace cqsp::client::systems::rmlui {
SearchMenu::~SearchMenu() {
    Rml::Element* element = document->GetElementById("search_input");
    element->RemoveEventListener(Rml::EventId::Change, &search_listener);
}
void SearchMenu::Update(double delta_time) {
    if (GetApp().ButtonIsHeld(engine::KeyInput::KEY_LEFT_CONTROL) && GetApp().ButtonIsHeld(engine::KeyInput::KEY_P)) {
        document->Show();
    }
    if (GetApp().ButtonIsHeld(engine::KeyInput::KEY_ESCAPE)) {
        document->Hide();
    }
}

void SearchMenu::OpenDocument() {
    document = GetApp().LoadDocument(file_name);
    document->Show();
    SetupDocument();
}

void SearchMenu::ReloadWindow() {
    document = GetApp().ReloadDocument(file_name);
    document->Show();
    SetupDocument();
}

void SearchMenu::SetupDocument() {
    Rml::Element* element = document->GetElementById("search_input");
    element->AddEventListener(Rml::EventId::Change, &search_listener);
}

void SearchMenu::SearchEventListener::ProcessEvent(Rml::Event& event) {
    auto* input = static_cast<Rml::ElementFormControlInput*>(event.GetTargetElement());
    Rml::String value = input->GetValue();
    SPDLOG_INFO("{}", value);
}
}  // namespace cqsp::client::systems::rmlui
