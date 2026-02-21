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

#include <RmlUi/Core/DataModelHandle.h>

#include "client/components/clientctx.h"
#include "client/components/rightclick.h"
#include "core/components/bodies.h"
#include "core/util/nameutil.h"

namespace cqsp::client::systems::rmlui {
RightClickWindow::~RightClickWindow() {
    document->RemoveEventListener(Rml::EventId::Mouseover, &listener);
    document->RemoveEventListener(Rml::EventId::Click, &right_click_listener);
    document->Close();
}

void RightClickWindow::ReloadWindow() {
    document = GetApp().ReloadDocument(file_name);
    SetupContent();
}

void RightClickWindow::SetupContent() {
    header_element = document->GetElementById("header");
    document->AddEventListener(Rml::EventId::Mouseover, &listener);
    document->AddEventListener(Rml::EventId::Click, &right_click_listener);
}

void RightClickWindow::SetupDataModels() {
    // Setup model
    Rml::DataModelConstructor constructor = GetApp().GetRmlUiContext()->CreateDataModel("right_click_buttons");

    if (auto invader_handle = constructor.RegisterStruct<RightClickMenuItem>()) {
        invader_handle.RegisterMember("name", &RightClickMenuItem::name);
        invader_handle.RegisterMember("action", &RightClickMenuItem::action);
    }

    constructor.RegisterArray<std::vector<RightClickMenuItem>>();
    constructor.Bind("button_list", &buttons);
    handle = constructor.GetModelHandle();
    handle.DirtyAllVariables();
}

void RightClickWindow::DetermineButtons(entt::entity entity) {
    buttons.clear();
    if (GetUniverse().any_of<core::components::bodies::Body>(entity)) {
        buttons.push_back({"Send to orbit", "somerandomaction"});
    }
    handle.DirtyAllVariables();
}

void RightClickWindow::EventListener::ProcessEvent(Rml::Event& event) {}

void RightClickWindow::Update(double delta_time) {
    bool mouse_over_this = MouseOverDocument();
    if ((GetApp().MouseButtonIsReleased(GLFW_MOUSE_BUTTON_RIGHT) ||
         GetApp().MouseButtonIsReleased(GLFW_MOUSE_BUTTON_LEFT)) &&
        !mouse_over_this) {
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
        if (std::holds_alternative<entt::entity>(hovering_text.world_space)) {
            // Later determine what the actual right click value should be?
            right_click_item = std::get<entt::entity>(hovering_text.world_space);
            header_element->SetInnerRML(core::util::GetName(GetUniverse(), right_click_item));
            // Then we should determine what kind of stuff we should show as wel and then do stuff
            DetermineButtons(right_click_item);
            if (buttons.empty()) {
                document->Hide();
            }
        } else {
            right_click_item = entt::null;
            document->Hide();
        }
    }
}

void RightClickWindow::OpenDocument() {
    SetupDataModels();
    document = GetApp().LoadDocument(file_name);
    SetupContent();
}

void RightClickWindow::ClickEventListener::ProcessEvent(Rml::Event& event) {
    // Check if we're gonna be clicking anything important...
    // Then delete this
    if (event.GetTargetElement() == nullptr) {
        return;
    }
    const Rml::Variant* value = event.GetTargetElement()->GetAttribute("onclick");
    if (value == nullptr && value->GetType() != Rml::Variant::STRING) {
        return;
    }
    SPDLOG_INFO("{}", value->Get<std::string>());
    window.document->Hide();
    window.right_click_item = entt::null;
}
}  // namespace cqsp::client::systems::rmlui
