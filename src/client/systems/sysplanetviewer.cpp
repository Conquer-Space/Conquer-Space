/*
* Copyright 2021 Conquer Space
*/
#include "client/systems/sysplanetviewer.h"

#include <string>

#include "common/components/area.h"
#include "common/components/bodies.h"
#include "common/components/name.h"
#include "common/components/orbit.h"
#include "common/components/organizations.h"
#include "common/components/player.h"
#include "common/components/population.h"
#include "common/components/resource.h"
#include "common/components/surface.h"
#include "engine/gui.h"

void conquerspace::client::systems::SysPlanetInformation::DisplayPlanet(
    entt::entity& planet, conquerspace::engine::Application& m_app) {
    namespace cqspc = conquerspace::components;
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x * 0.4f,
                                    ImGui::GetIO().DisplaySize.y * 0.7f),
                             ImGuiCond_Appearing);
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.75f,
                                   ImGui::GetIO().DisplaySize.y * 0.6f),
                            ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    std::string planet_name = "Planet";
    if (planet == entt::null) {
        return;
    }
    if (m_app.GetUniverse().registry.all_of<cqspc::Name>(planet)) {
        planet_name = m_app.GetUniverse().registry.get<cqspc::Name>(planet).name;
    }
    ImGui::Begin(planet_name.c_str());
    switch (view_mode) {
        case ViewMode::PLANET_VIEW:
            PlanetInformationPanel(planet, m_app);
            break;
        case ViewMode::CITY_VIEW:
            CityInformationPanel(planet, m_app);
            break;
    }
    ImGui::End();
}

void conquerspace::client::systems::SysPlanetInformation::CityInformationPanel(entt::entity& planet,
                                                        conquerspace::engine::Application& m_app) {
    namespace cqspc = conquerspace::components;
    if (ImGui::Button("<")) {
        view_mode = ViewMode::PLANET_VIEW;
    }
    ImGui::SameLine();
    static bool thing = true;
    ImGui::Checkbox("Macroeconomic/Ownership mode", &thing);

    ImGui::Text(fmt::format("{}", m_app.GetUniverse().registry.
                                            get<cqspc::Name>(selected_city_entity).name).c_str());

    if (m_app.GetUniverse().registry.all_of<cqspc::Settlement>(selected_city_entity)) {
        int size = m_app.GetUniverse().registry
                .get<cqspc::Settlement>(selected_city_entity).population.size();
        for (auto b : m_app.GetUniverse().registry.get<cqspc::Settlement>(
                              selected_city_entity).population) {
            auto& bad_var_name = m_app.GetUniverse()
                                    .registry.get<cqspc::PopulationSegment>(b);
            ImGui::Text(fmt::format("Population: {}", bad_var_name.population).c_str());
        }
    } else {
        ImGui::Text(fmt::format("No population").c_str());
    }

    if (m_app.GetUniverse().registry.all_of<cqspc::Industry>(selected_city_entity)) {
        size_t industries = m_app.GetUniverse().registry
                .get<cqspc::Industry>(selected_city_entity).industries.size();
        if (ImGui::BeginTabBar("CityTabs", ImGuiTabBarFlags_None)) {
            if (ImGui::BeginTabItem("Industries")) {
                ImGui::Text(fmt::format("Factories: {}", industries).c_str());
                ImGui::BeginChild("salepanel",
                                            ImVec2(ImGui::GetWindowContentRegionWidth() * 0.5f
                                            - ImGui::GetStyle().ItemSpacing.y, 260),
                                            true, ImGuiWindowFlags_HorizontalScrollbar);
                ImGui::Text("Services Sector");
                // List all the stuff it produces
                ImGui::Text("GDP:");
                ImGui::EndChild();

                ImGui::SameLine();

                ImGui::BeginChild("ManufacturingPanel",
                                            ImVec2(ImGui::GetWindowContentRegionWidth() * 0.5f
                                            - ImGui::GetStyle().ItemSpacing.y, 260),
                                            true, ImGuiWindowFlags_HorizontalScrollbar);
                ImGui::Text("Manufactuing Sector");
                // List all the stuff it produces
                ImGui::Text("GDP:");
                ImGui::EndChild();

                ImGui::BeginChild("MinePanel", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.5f
                                                - ImGui::GetStyle().ItemSpacing.y, 260),
                                                true, ImGuiWindowFlags_HorizontalScrollbar);
                ImGui::Text("Mining Sector");
                ImGui::Text("GDP:");

                ImGui::EndChild();
                ImGui::SameLine();


                ImGui::BeginChild("AgriPanel", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.5f
                                                        - ImGui::GetStyle().ItemSpacing.y, 260),
                                                        true, ImGuiWindowFlags_HorizontalScrollbar);
                ImGui::Text("Agriculture Sector");
                ImGui::Text("GDP:");

                ImGui::EndChild();
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
    }
}

void conquerspace::client::systems::SysPlanetInformation::PlanetInformationPanel(
                            entt::entity& planet,
                            conquerspace::engine::Application& m_app) {
    namespace cqspc = conquerspace::components;
    if (m_app.GetUniverse().registry.all_of<cqspc::Habitation>(planet)) {
        auto& habit = m_app.GetUniverse().registry.get<cqspc::Habitation>(planet);
        ImGui::Text(fmt::format("Cities: {}", habit.settlements.size()).c_str());
        // List cities
        for (int i = 0; i < habit.settlements.size(); i++) {
            const bool is_selected = (selected_city_index == i);

            entt::entity e = habit.settlements[i];
            std::string name = m_app.GetUniverse().registry.get<cqspc::Name>(e)
                    .name;
            if (ImGui::Selectable(fmt::format("{}", name).c_str(), is_selected)) {
                // Load city
                selected_city_index = i;
                selected_city_entity = habit.settlements[i];
                view_mode = ViewMode::CITY_VIEW;

                if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                    // See city
                    spdlog::info("Mouse clicked");
                }
            }

            if (ImGui::IsItemActive() || ImGui::IsItemHovered()) {
                ImGui::SetTooltip(fmt::format("{}", name).c_str());
            }
        }
    }
}
