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
void ToolTipWindow::Update(double delta_time) {
    // We should move the position into somewhere we can see
    auto& hovering_text = GetUniverse().ctx().at<client::ctx::HoveringItem>();
    document->SetProperty("top", fmt::format("{} px", GetApp().GetMouseY() + 5));
    document->SetProperty("left", fmt::format("{} px", GetApp().GetMouseX() + 5));
    // In the future we should probably have a more efficient way of updating this rml
    // Now let's check the value

    // Then if it's low enough then we hide
    bool to_present = true;
    if (hovering_text.Set()) {
        last_tooltip_change = GetApp().GetTime();
    }

    if (hovering_text.Set()) {
        std::visit(overloaded {[&](std::monostate) { to_present = false; },
                               [&](entt::entity entity) {
                                   if (GetUniverse().valid(entity)) {
                                       // Then we set it
                                       document->SetInnerRML(core::util::GetName(GetUniverse(), entity));
                                   } else {
                                       // We show nothing
                                       to_present = false;
                                   }
                               },
                               [&](const std::string& string) { document->SetInnerRML(string); }},
                   hovering_text);
    }

    if (GetApp().GetTime() - last_tooltip_change < 0.1) {
        to_present = false;
    } else {
        to_present = to_present & true;
    }
    if (to_present && !document->IsVisible()) {
        document->Show();
    } else if (!to_present && document->IsVisible()) {
        document->Hide();
    }
    hovering_text.Reset();
}

void ToolTipWindow::OpenDocument() { document = GetApp().LoadDocument(file_name); }
}  // namespace cqsp::client::systems::rmlui
