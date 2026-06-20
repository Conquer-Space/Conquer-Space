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
#include "client/scenes/universe/interface/economywindow.h"

#include "client/components/clientctx.h"

namespace cqsp::client::systems {
void EconomyWindow::Init() {}
void EconomyWindow::DoUI(int delta_time) {
    bool selected = GetUniverse().ctx().at<ctx::SelectedMenu>() == ctx::SelectedMenu::EconomyMenu;
    if (!selected) {
        return;
    }
    ImGui::Begin("Economy", &selected);
    // Now stuff
    ImGui::Text("Investment");
    // Trigger investment into places or something
    // Now do something

    ImGui::End();
    if (!selected) {
        GetUniverse().ctx().at<ctx::SelectedMenu>() = ctx::SelectedMenu::NoMenu;
    }
}
void EconomyWindow::DoUpdate(int delta_time) {}
}  // namespace cqsp::client::systems
