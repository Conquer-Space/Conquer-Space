/*
 * Copyright 2021 Conquer Space
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
    if (universe.all_of<cqspc::ResourceDemand>(entity)) {
        ImGui::Separator();
        ImGui::TextFmt("Demand");
        // Then do demand
        conquerspace::client::systems::DrawLedgerTable(
            "marketdemandtooltip", universe, universe.get<cqspc::ResourceDemand>(entity));
    }
    ImGui::EndTooltip();
}
