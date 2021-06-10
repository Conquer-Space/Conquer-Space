/*
* Copyright 2021 Conquer Space
*/
#include "client/systems/sysplanetviewer.h"

#include <string>
#include <map>

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
#include "util/utilnumberdisplay.h"

void conquerspace::client::systems::SysPlanetInformation::DisplayPlanet(
    entt::entity& planet, conquerspace::engine::Application& m_app) {
    namespace cqspc = conquerspace::components;
    if (!to_see) {
        return;
    }

    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x * 0.4f,
                                    ImGui::GetIO().DisplaySize.y * 0.8f),
                             ImGuiCond_Appearing);
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.79f,
                                   ImGui::GetIO().DisplaySize.y * 0.55f),
                            ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    std::string planet_name = "Planet";
    if (planet == entt::null) {
        return;
    }
    if (m_app.GetUniverse().all_of<cqspc::Name>(planet)) {
        planet_name = m_app.GetUniverse().get<cqspc::Name>(planet).name;
    }
    ImGui::Begin(planet_name.c_str(), &to_see);
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

void conquerspace::client::systems::SysPlanetInformation::CityInformationPanel(
                                                        entt::entity& planet,
                                                        conquerspace::engine::Application& m_app) {
    namespace cqspc = conquerspace::components;
    if (ImGui::Button("<")) {
        view_mode = ViewMode::PLANET_VIEW;
    }
    ImGui::SameLine();
    static bool thing = true;
    ImGui::Checkbox("Macroeconomic/Ownership mode", &thing);

    ImGui::Text(fmt::format("{}", m_app.GetUniverse().
                                            get<cqspc::Name>(selected_city_entity).name).c_str());

    if (m_app.GetUniverse().all_of<cqspc::Settlement>(selected_city_entity)) {
        int size = m_app.GetUniverse()
                .get<cqspc::Settlement>(selected_city_entity).population.size();
        for (auto b : m_app.GetUniverse().get<cqspc::Settlement>(
                              selected_city_entity).population) {
            auto& bad_var_name = m_app.GetUniverse()
                                    .get<cqspc::PopulationSegment>(b);
            ImGui::Text(fmt::format("Population: {}",
                        conquerspace::util::LongToHumanString(bad_var_name.population)).c_str());
        }
    } else {
        ImGui::Text(fmt::format("No population").c_str());
    }

    if (m_app.GetUniverse().all_of<cqspc::Industry>(selected_city_entity)) {
        size_t industries = m_app.GetUniverse()
                .get<cqspc::Industry>(selected_city_entity).industries.size();
        if (ImGui::BeginTabBar("CityTabs", ImGuiTabBarFlags_None)) {
            if (ImGui::BeginTabItem("Industries")) {
                auto &city_industry = m_app.GetUniverse()
                    .get<cqspc::Industry>(selected_city_entity);

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

                std::map<entt::entity, int> input_resources;
                std::map<entt::entity, int> output_resources;
                for (auto thingies : city_industry.industries) {
                    if (m_app.GetUniverse()
                        .all_of<cqspc::ResourceConverter, cqspc::Factory>(thingies)) {
                        auto& generator =
                            m_app.GetUniverse().get<cqspc::ResourceConverter>(thingies);
                        auto& recipe =
                            m_app.GetUniverse().get<cqspc::Recipe>(
                                generator.recipe);
                        for (auto iterator = recipe.input.begin();
                             iterator != recipe.input.end(); iterator++) {
                             if (input_resources.find(iterator->first) == input_resources.end()) {
                                 input_resources[iterator->first] = 0;
                             }
                             input_resources[iterator->first] = input_resources[iterator->first]
                                                                                + iterator->second;
                        }
                        for (auto iterator = recipe.output.begin();
                             iterator != recipe.output.end(); iterator++) {
                             if (output_resources.find(iterator->first)
                                                                    == output_resources.end()) {
                                 output_resources[iterator->first] = 0;
                             }
                             output_resources[iterator->first] = output_resources[iterator->first]
                                                                                + iterator->second;
                        }
                    }
                }

                ImGui::Text("Output");
                // Output table
                if (ImGui::BeginTable("output_tabke", 2, ImGuiTableFlags_Borders |
                                                                        ImGuiTableFlags_RowBg)) {
                    ImGui::TableSetupColumn("Good");
                    ImGui::TableSetupColumn("Amount");
                    ImGui::TableHeadersRow();
                    for (auto iterator = output_resources.begin();
                                                iterator != output_resources.end(); iterator++) {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text(
                            fmt::format("{}", m_app.GetUniverse()
                                    .get<cqspc::Identifier>(iterator->first).identifier).c_str());

                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text(fmt::format("{}",
                                conquerspace::util::LongToHumanString(iterator->second)).c_str());
                    }
                    ImGui::EndTable();
                }

                ImGui::Text("Input");
                if (ImGui::BeginTable("input_tabke", 2,
                                                ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
                    ImGui::TableSetupColumn("Good");
                    ImGui::TableSetupColumn("Amount");
                    ImGui::TableHeadersRow();
                    for (auto iterator = input_resources.begin();
                                                iterator != input_resources.end(); iterator++) {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text(fmt::format("{}",
                                                m_app.GetUniverse().
                                                get<cqspc::Identifier>(iterator->first)
                                                .identifier).c_str());

                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text(fmt::format("{}",
                                conquerspace::util::LongToHumanString(iterator->second)).c_str());
                    }
                    ImGui::EndTable();
                }
                ImGui::EndChild();

                ImGui::BeginChild("MinePanel", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.5f
                                                - ImGui::GetStyle().ItemSpacing.y, 260),
                                                true, ImGuiWindowFlags_HorizontalScrollbar);
                ImGui::Text("Mining Sector");
                ImGui::Text("GDP:");
                // Get what resources they are making
                std::map<entt::entity, int> resources;
                for (auto thingies : city_industry.industries) {
                    if (m_app.GetUniverse()
                        .all_of<cqspc::ResourceGenerator, cqspc::Mine>(thingies)) {
                        auto& generator =
                            m_app.GetUniverse().get<cqspc::ResourceGenerator>(thingies);

                        for (auto iterator = generator.begin();
                            iterator != generator.end(); iterator++) {
                            if (resources.find(iterator->first) == resources.end()) {
                                resources[iterator->first] = 0;
                            }
                            resources[iterator->first] = resources[iterator->first]
                                                                                + iterator->second;
                        }
                    }
                }

                // Draw on table
                if (ImGui::BeginTable("table1", 2,
                                            ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
                    ImGui::TableSetupColumn("Good");
                    ImGui::TableSetupColumn("Amount");
                    ImGui::TableHeadersRow();
                    for (auto iterator = resources.begin();
                                                        iterator != resources.end(); iterator++) {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);

                        ImGui::Text(fmt::format("{}", m_app.GetUniverse().
                                                        get<cqspc::Identifier>(iterator->first)
                                                        .identifier).c_str());

                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text(fmt::format("{}", conquerspace::util::
                                                    LongToHumanString(iterator->second)).c_str());
                    }
                    ImGui::EndTable();
                }

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
            if (ImGui::BeginTabItem("Resources")) {
                // Consolidate resources
                auto &city_industry = m_app.GetUniverse()
                    .get<cqspc::Industry>(selected_city_entity);
                std::map<entt::entity, int> resources;
                for (auto area : city_industry.industries) {
                    if (m_app.GetUniverse().all_of<cqspc::ResourceStockpile>(area)) {
                        // Add resources
                        auto& stockpile = m_app.GetUniverse().get<cqspc::ResourceStockpile>(area);
                        for (auto iterator = stockpile.begin();
                            iterator != stockpile.end(); iterator++) {
                            if (resources.find(iterator->first) == resources.end()) {
                                resources[iterator->first] = 0;
                            }
                            resources[iterator->first] = resources[iterator->first]
                                                                                + iterator->second;
                        }
                    }
                }

                if (ImGui::BeginTable("table1", 2,
                                            ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
                    ImGui::TableSetupColumn("Good");
                    ImGui::TableSetupColumn("Amount");
                    ImGui::TableHeadersRow();
                    for (auto iterator = resources.begin();
                                                        iterator != resources.end(); iterator++) {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);

                        ImGui::Text(fmt::format("{}", m_app.GetUniverse().
                                                        get<cqspc::Identifier>(iterator->first)
                                                        .identifier).c_str());

                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text(fmt::format("{}", conquerspace::util::
                                                    LongToHumanString(iterator->second)).c_str());
                    }
                    ImGui::EndTable();
                }
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
    if (m_app.GetUniverse().all_of<cqspc::Habitation>(planet)) {
        auto& habit = m_app.GetUniverse().get<cqspc::Habitation>(planet);
        ImGui::Text(fmt::format("Cities: {}", habit.settlements.size()).c_str());
        // List cities
        for (int i = 0; i < habit.settlements.size(); i++) {
            const bool is_selected = (selected_city_index == i);

            entt::entity e = habit.settlements[i];
            std::string name = m_app.GetUniverse().get<cqspc::Name>(e)
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
