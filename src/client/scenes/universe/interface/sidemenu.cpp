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
#include "client/scenes/universe/interface/sidemenu.h"

#include "client/components/clientctx.h"

namespace cqsp::client::systems::rmlui {
SideMenu::~SideMenu() {
    Rml::ElementList list;
    document->GetElementsByClassName(list, "active_button");
    for (Rml::Element* element : list) {
        element->RemoveEventListener(Rml::EventId::Click, &right_click_listener);
    }
    document->Close();
}

void SideMenu::Update(double delta_time) {}

void SideMenu::OpenDocument() {
    document = GetApp().LoadDocument(file_name);
    SetupDocument();
    document->Show();
}

void SideMenu::ClickEventListener::ProcessEvent(Rml::Event& event) {
    // Now we should get our event id or something
    // Next time
    Rml::Element* element = event.GetCurrentElement();
    const std::string& id = element->GetId();
    if (id == "projects") {
        universe.ctx().at<ctx::SelectedMenu>() = ctx::SelectedMenu::ProjectsMenu;
    } else if (id == "space") {
        universe.ctx().at<ctx::SelectedMenu>() = ctx::SelectedMenu::SpaceMenu;
    } else if (id == "economy") {
        universe.ctx().at<ctx::SelectedMenu>() = ctx::SelectedMenu::EconomyMenu;
    }
}

void SideMenu::ReloadWindow() {
    document = GetApp().ReloadDocument(file_name);
    SetupDocument();
    document->Show();
}

void SideMenu::SetupDocument() {
    Rml::ElementList list;
    document->GetElementsByClassName(list, "active_button");
    for (Rml::Element* element : list) {
        element->AddEventListener(Rml::EventId::Click, &right_click_listener);
    }
}
}  // namespace cqsp::client::systems::rmlui
