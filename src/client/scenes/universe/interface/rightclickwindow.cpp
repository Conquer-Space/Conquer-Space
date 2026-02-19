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
#include "client/scenes/universe/interface/rightclickwindow.h"

#include "client/components/clientctx.h"
#include "core/util/nameutil.h"

namespace cqsp::client::systems::rmlui {
RightClickWindow::~RightClickWindow() { document->Close(); }

void RightClickWindow::ReloadWindow() {
    document = GetApp().ReloadDocument(file_name);
    SetupContent();
}

void RightClickWindow::SetupContent() { header_element = document->GetElementById("header"); }

void RightClickWindow::Update(double delta_time) {
    // Now let's display the value
    bool mouse_over_this = MouseOverDocument();
    if ((GetApp().MouseButtonIsReleased(GLFW_MOUSE_BUTTON_RIGHT) ||
         GetApp().MouseButtonIsReleased(GLFW_MOUSE_BUTTON_LEFT)) &&
        !GetApp().MouseDragged() && !mouse_over_this) {
        // Now let's turn it on
        to_right_click = false;
        document->Hide();
    }

    if (GetApp().MouseButtonIsReleased(GLFW_MOUSE_BUTTON_RIGHT) && !GetApp().MouseDragged()) {
        // Now let's turn it on
        to_right_click = true;
        itemX = GetApp().GetMouseX();
        itemY = GetApp().GetMouseY();
        document->Show();
        document->SetProperty("top", fmt::format("{} px", itemY + 5));
        document->SetProperty("left", fmt::format("{} px", itemX + 5));
        auto& hovering_text = GetUniverse().ctx().at<client::ctx::HoveringItem>();
        if (std::holds_alternative<entt::entity>(hovering_text)) {
            right_click_item = std::get<entt::entity>(hovering_text);
            header_element->SetInnerRML(core::util::GetName(GetUniverse(), right_click_item));
        }
    }
}

void RightClickWindow::OpenDocument() {
    document = GetApp().LoadDocument(file_name);
    SetupContent();
}
}  // namespace cqsp::client::systems::rmlui
