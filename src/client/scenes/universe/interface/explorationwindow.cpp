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
#include "client/scenes/universe/interface/explorationwindow.h"

#include "client/components/clientctx.h"
#include "core/util/nameutil.h"

namespace cqsp::client::systems::rmlui {
ExplorationWindow::~ExplorationWindow() { document->Close(); }

void ExplorationWindow::ReloadWindow() {
    document = GetApp().ReloadDocument(file_name);
    SetupContent();
}

void ExplorationWindow::SetupContent() {}

void ExplorationWindow::Update(double delta_time) {
    if (GetUniverse().ctx().at<ctx::SelectedMenu>() != ctx::SelectedMenu::ExplorationMenu) {
        return;
    }
}

void ExplorationWindow::OpenDocument() {
    document = GetApp().LoadDocument(file_name);
    SetupContent();
}

void ExplorationWindow::EventListener::ProcessEvent(Rml::Event& event) {
    // Now let's do stuff
    const Rml::Variant* value = event.GetTargetElement()->GetAttribute("onclick");
    if (value == nullptr || value->GetType() != Rml::Variant::STRING) {
        return;
    }
    std::string action = value->Get<std::string>();
    if (action == "colonize") {
        // then we should set our hover text
        universe.ctx().at<ctx::HoveringItem>().ui_space = "Start the process of setting up a colony on this province";
    }
}
}  // namespace cqsp::client::systems::rmlui
