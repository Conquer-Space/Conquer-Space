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
#include "common/components/area.h"
#include "common/components/resource.h"
#include "client/systems/gui/sysstockpileui.h"
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
    } else if (universe.any_of<cqspc::Mine>(entity)) {
        ImGui::TextFmt("Mine");
        std::string production = "";
        auto& generator = universe.get<cqspc::ResourceGenerator>(entity);
        for (auto it = generator.begin();
             it != generator.end(); ++it) {
            production += universe.get<cqspc::Name>(it->first).name + ", ";
        }
        // Remove last comma
        if (!production.empty()) {
            production = production.substr(0, production.size() - 2);
        }
        ImGui::TextFmt("{} Mine", production);
    } else if (universe.any_of<cqspc::Factory>(entity)) {
        ImGui::TextFmt("Factory");
        std::string production = "";
        auto& generator = universe.get<cqspc::ResourceConverter>(entity);
        ImGui::TextFmt("{} Factory", universe.get<cqspc::Identifier>(generator.recipe).identifier);
    } else {
        ImGui::TextFmt("Nothing");
    }

    if (universe.all_of<cqspc::ResourceStockpile>(entity)) {
        ImGui::Separator();
        ImGui::TextFmt("Resources");
        // Then do demand
        conquerspace::client::systems::DrawLedgerTable(
            "resourcesstockpiletooltip", universe, universe.get<cqspc::ResourceStockpile>(entity));
    }
    if (universe.all_of<cqspc::Production>(entity)) {
        ImGui::Text("Producing next tick");
    }
    if (universe.all_of<cqspc::Production>(entity)) {
        ImGui::Text("Producing next tick");
    }
    if (universe.all_of<cqspc::ResourceGenerator>(entity)) {
        ImGui::Separator();
        ImGui::TextFmt("Generating");
        // Then do demand
        conquerspace::client::systems::DrawLedgerTable(
            "factorygentooltip", universe, universe.get<cqspc::ResourceGenerator>(entity));
    }
    ImGui::EndTooltip();
}
