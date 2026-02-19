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
#include "client/scenes/universe/interface/maintooltip.h"

#include "client/components/clientctx.h"
#include "core/util/nameutil.h"

namespace cqsp::client::systems::rmlui {
ToolTipWindow::~ToolTipWindow() { document->Close(); }

namespace {
template <class... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};
// explicit deduction guide (not needed as of C++20)
template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;
}  // namespace

void ToolTipWindow::ReloadWindow() {
    document = GetApp().ReloadDocument(file_name);
    SetupContent();
}

void ToolTipWindow::SetupContent() {
    tooltip_content = document->GetElementById("tooltip_content");
    SPDLOG_INFO("ASDf {}", tooltip_content->GetOwnerDocument()->GetSourceURL());
}

void ToolTipWindow::Update(double delta_time) {
    // We should move the position into somewhere we can see
    auto& hovering_text = GetUniverse().ctx().at<client::ctx::HoveringItem>();
    // check for right click on the screen
    itemX = GetApp().GetMouseX();
    itemY = GetApp().GetMouseY();
    document->SetProperty("top", fmt::format("{} px", itemY + 10));
    document->SetProperty("left", fmt::format("{} px", itemX + 10));
    // In the future we should probably have a more efficient way of updating this rml
    // Now let's check the value

    // Then if it's low enough then we hide
    bool to_present = true;
    if (last_hover != hovering_text) {
        std::visit(overloaded {[&](std::monostate) {
                                   to_present = false;
                                   SPDLOG_INFO("Showing monostate");
                               },
                               [&](entt::entity entity) {
                                   SPDLOG_INFO("Setting {}", entity);
                                   if (GetUniverse().valid(entity)) {
                                       // Then we set it
                                       tooltip_content->SetInnerRML(core::util::GetName(GetUniverse(), entity));
                                       to_present = true;
                                   } else {
                                       // We show nothing
                                       to_present = false;
                                   }
                               },
                               [&](const std::string& string) {
                                   SPDLOG_INFO("Setting {}", string);
                                   tooltip_content->SetInnerRML(string);
                                   to_present = true;
                               }},
                   last_hover);
        last_tooltip_change = GetApp().GetTime();
        std::visit(overloaded {[&](std::monostate) {
                                   to_present = false;
                                   SPDLOG_INFO("We shouldn't be showing anything");
                               },
                               [&](entt::entity entity) {
                                   SPDLOG_INFO("Setting ent {}", entity);
                                   if (GetUniverse().valid(entity)) {
                                       // Then we set it
                                       tooltip_content->SetInnerRML(core::util::GetName(GetUniverse(), entity));
                                       to_present = true;
                                   } else {
                                       // We show nothing
                                       to_present = false;
                                   }
                               },
                               [&](const std::string& string) {
                                   SPDLOG_INFO("Setting ent {}", string);
                                   tooltip_content->SetInnerRML(string);
                                   to_present = true;
                               }},
                   hovering_text);
    }
    if (std::holds_alternative<std::monostate>(hovering_text)) {
        to_present = false;
    }

    if (GetApp().GetTime() - last_tooltip_change < 0.1) {
        SPDLOG_INFO("We're not showing the tooltip...");
        to_present = false;
    }
    if (to_present && !document->IsVisible()) {
        document->Show();
    } else if (!to_present && document->IsVisible()) {
        SPDLOG_INFO("Hiding");
        document->Hide();
    }
    if (document->IsVisible()) {
        document->PullToFront();
    }
    last_hover = hovering_text;
}

void ToolTipWindow::OpenDocument() {
    document = GetApp().LoadDocument(file_name);
    SetupContent();
}
}  // namespace cqsp::client::systems::rmlui
