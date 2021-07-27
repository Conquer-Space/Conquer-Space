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
#include "client/systems/gui/systooltips.h"

#include "common/components/name.h"
#include "common/components/surface.h"
#include "common/components/bodies.h"
#include "engine/gui.h"

void conquerspace::client::systems::gui::EntityTooltip(
    entt::entity entity, conquerspace::common::components::Universe &universe) {
    if (!ImGui::IsItemHovered()) {
        return;
    }
    namespace cqspc = conquerspace::common::components;
    ImGui::BeginTooltip();
    if (universe.all_of<cqspc::Name>(entity)) {
        ImGui::TextFmt("{}", universe.get<cqspc::Name>(entity).name);
    } else {
        ImGui::TextFmt("{}", entity);
    }
    // Then get type of entity
    // And then tooltip, I guess
    if (universe.all_of<cqspc::bodies::Star>(entity)) {
        ImGui::TextFmt("Star");
    } else if (universe.all_of<cqspc::bodies::Planet>(entity)) {
        ImGui::TextFmt("Planet");
    } else if (universe.any_of<cqspc::Settlement, cqspc::Habitation>(entity)) {
        ImGui::TextFmt("City");
    } else {
        ImGui::TextFmt("Nothing");
    }
    ImGui::EndTooltip();
}
