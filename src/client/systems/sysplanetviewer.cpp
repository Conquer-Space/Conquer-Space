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
#include "client/systems/sysplanetviewer.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <noiseutils.h>
#include <noise/noise.h>

#include <string>
#include <map>

#include "client/systems/sysstarsystemrenderer.h"
#include "client/systems/gui/sysstockpileui.h"
#include "client/scenes/universescene.h"
#include "client/systems/gui/systooltips.h"

#include "common/components/area.h"
#include "common/components/bodies.h"
#include "common/components/name.h"
#include "common/components/movement.h"
#include "common/components/organizations.h"
#include "common/components/player.h"
#include "common/components/population.h"
#include "common/components/resource.h"
#include "common/components/surface.h"
#include "common/components/economy.h"
#include "common/util/utilnumberdisplay.h"
#include "common/systems/actions/factoryconstructaction.h"
#include "common/systems/economy/markethelpers.h"

#include "engine/gui.h"
#include "engine/cqspgui.h"

void conquerspace::client::systems::SysPlanetInformation::DisplayPlanet() {
    namespace cqspc = conquerspace::common::components;
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
    if (selected_planet == entt::null) {
        return;
    }
    if (GetApp().GetUniverse().all_of<cqspc::Name>(selected_planet)) {
        planet_name = GetApp().GetUniverse().get<cqspc::Name>(selected_planet);
    }
    ImGui::Begin(planet_name.c_str(), &to_see, window_flags);
    switch (view_mode) {
        case ViewMode::PLANET_VIEW:
            PlanetInformationPanel();
            break;
        case ViewMode::CITY_VIEW:
            CityInformationPanel();
            MineInformationPanel();
            FactoryInformationPanel();
            break;
    }
    ImGui::End();
}

void conquerspace::client::systems::SysPlanetInformation::Init() {}

void conquerspace::client::systems::SysPlanetInformation::DoUI(int delta_time) {
    DisplayPlanet();
}

void conquerspace::client::systems::SysPlanetInformation::DoUpdate(int delta_time) {
    // If clicked on a planet, go to the planet
    // Get the thing
    namespace cqspb = conquerspace::common::components::bodies;
    selected_planet = conquerspace::scene::GetCurrentViewingPlanet(GetApp());
    entt::entity mouse_over = GetApp().GetUniverse().
                        view<conquerspace::client::systems::MouseOverEntity>().front();
    if (!ImGui::GetIO().WantCaptureMouse &&
                GetApp().MouseButtonIsReleased(GLFW_MOUSE_BUTTON_LEFT) &&
                mouse_over == selected_planet && !conquerspace::scene::IsGameHalted() &&
        !GetApp().MouseDragged()) {
        to_see = true;
        SPDLOG_INFO("Switched entity");
    }
    if (!GetApp().GetUniverse().all_of<cqspb::Body>(selected_planet)) {
        to_see = false;
    }
}

void conquerspace::client::systems::SysPlanetInformation::CityInformationPanel() {
    namespace cqspc = conquerspace::common::components;
    if (CQSPGui::ArrowButton("cityinformationpanel", ImGuiDir_Left)) {
        view_mode = ViewMode::PLANET_VIEW;
    }
    ImGui::SameLine();
    static bool thing = true;
    CQSPGui::DefaultCheckbox("Macroeconomic/Ownership mode", &thing);

    ImGui::TextFmt("{}", GetApp().GetUniverse().get<cqspc::Name>(selected_city_entity));

    if (GetApp().GetUniverse().all_of<cqspc::Settlement>(selected_city_entity)) {
        int size = GetApp().GetUniverse().get<cqspc::Settlement>(selected_city_entity).population.size();
        for (auto b : GetApp().GetUniverse().get<cqspc::Settlement>(selected_city_entity).population) {
            auto& bad_var_name = GetApp().GetUniverse().get<cqspc::PopulationSegment>(b);
            ImGui::TextFmt("Population: {}",conquerspace::util::LongToHumanString(bad_var_name.population));
        }
    } else {
        ImGui::TextFmt("No population");
    }

    if (GetApp().GetUniverse().all_of<cqspc::Industry>(selected_city_entity)) {
        if (ImGui::BeginTabBar("CityTabs", ImGuiTabBarFlags_None)) {
            if (ImGui::BeginTabItem("Demographics")) {
                DemographicsTab();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Industries")) {
                IndustryTab();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Resources")) {
                ResourcesTab();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Construction")) {
                ConstructionTab();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Space Port")) {
                SpacePortTab();
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
    }
}

void conquerspace::client::systems::SysPlanetInformation::PlanetInformationPanel() {
    namespace cqspc = conquerspace::common::components;
    if (!GetApp().GetUniverse().all_of<cqspc::Habitation>(selected_planet)) {
        return;
    }
    auto& habit = GetApp().GetUniverse().get<cqspc::Habitation>(selected_planet);
    ImGui::TextFmt("Cities: {}", habit.settlements.size());

    ImGui::BeginChild("citylist", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar |
                                        window_flags);
    // Market
    if (GetApp().GetUniverse().all_of<cqspc::MarketCenter>(selected_planet)) {
        auto& center = GetApp().GetUniverse().get<cqspc::MarketCenter>(selected_planet);
        auto& market = GetApp().GetUniverse().get<cqspc::Market>(center.market);
        ImGui::Text("Is market center");
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::Text(fmt::format("Has {} entities attached to it", market.participants.size()).c_str());
            // Get resource stockpile
            auto& stockpile = GetApp().GetUniverse().get<cqspc::ResourceStockpile>(center.market);
            DrawLedgerTable("marketstockpile", GetApp().GetUniverse(), stockpile);

            // Market prices
            ImGui::Separator();
            ImGui::Text("Market prices");
            if (ImGui::BeginTable("goodpricetable", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
                ImGui::TableSetupColumn("Good");
                ImGui::TableSetupColumn("Prices");
                for (auto& price : market.prices) {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextFmt("{}", GetApp().GetUniverse().get<cqspc::Identifier>(price.first));
                    ImGui::TableSetColumnIndex(1);
                    ImGui::TextFmt("{}", price.second);
                }
                ImGui::EndTable();
            }
            ImGui::Text("Market demands");
            DrawLedgerTable("marketdemand", GetApp().GetUniverse(), market.demand);
            ImGui::Text("Market supply");
            DrawLedgerTable("marketsupply", GetApp().GetUniverse(), market.supply);
            ImGui::EndTooltip();
        }
    }

    // Get population
    uint64_t pop_size = 0;
    for (entt::entity settlement : habit.settlements) {
        for (entt::entity population : GetApp().GetUniverse().get<cqspc::Settlement>(settlement).population) {
            pop_size += GetApp().GetUniverse().get<cqspc::PopulationSegment>(population).population;
        }
    }
    ImGui::TextFmt("Population: {} ({})", conquerspace::util::LongToHumanString(pop_size), pop_size);
    ImGui::Separator();

    // List cities
    for (int i = 0; i < habit.settlements.size(); i++) {
        const bool is_selected = (selected_city_index == i);

        entt::entity e = habit.settlements[i];
        std::string name = GetApp().GetUniverse().get<cqspc::Name>(e);
        if (CQSPGui::DefaultSelectable(fmt::format("{}", name).c_str(), is_selected)) {
            // Load city
            selected_city_index = i;
            selected_city_entity = habit.settlements[i];
            view_mode = ViewMode::CITY_VIEW;

            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                // See city
                spdlog::info("Mouse clicked");
            }
        }
        gui::EntityTooltip(GetApp().GetUniverse(), e);
    }
    ImGui::EndChild();
}

void conquerspace::client::systems::SysPlanetInformation::ResourcesTab() {
    namespace cqspc = conquerspace::common::components;
    // Consolidate resources
    auto &city_industry = GetApp().GetUniverse().get<cqspc::Industry>(selected_city_entity);
    cqspc::ResourceLedger resources;
    for (auto area : city_industry.industries) {
        if (GetApp().GetUniverse().all_of<cqspc::ResourceStockpile>(area)) {
            // Add resources
            auto& stockpile = GetApp().GetUniverse().get<cqspc::ResourceStockpile>(area);
            resources += stockpile;
        }
    }

    DrawLedgerTable("cityresources", GetApp().GetUniverse(), resources);
}

void conquerspace::client::systems::SysPlanetInformation::IndustryTab() {
    namespace cqspc = conquerspace::common::components;
    auto& city_industry =
        GetApp().GetUniverse().get<cqspc::Industry>(selected_city_entity);

    int height = 300;
    ImGui::TextFmt("Factories: {}", city_industry.industries.size());
    ImGui::BeginChild("salepanel", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.5f -
                                 ImGui::GetStyle().ItemSpacing.y, height), true,
                      ImGuiWindowFlags_HorizontalScrollbar | window_flags);
    IndustryTabServicesChild();
    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginChild("ManufacturingPanel", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.5f -
                                 ImGui::GetStyle().ItemSpacing.y, height), true,
                      ImGuiWindowFlags_HorizontalScrollbar | window_flags);
    IndustryTabManufacturingChild();
    ImGui::EndChild();

    ImGui::BeginChild("MinePanel", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.5f -
                                 ImGui::GetStyle().ItemSpacing.y, height), true,
                      ImGuiWindowFlags_HorizontalScrollbar | window_flags);
    IndustryTabMiningChild();
    ImGui::EndChild();
    ImGui::SameLine();

    ImGui::BeginChild("AgriPanel", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.5f -
                                 ImGui::GetStyle().ItemSpacing.y, height), true,
                      ImGuiWindowFlags_HorizontalScrollbar | window_flags);
    IndustryTabAgricultureChild();
    ImGui::EndChild();
}

void conquerspace::client::systems::SysPlanetInformation::IndustryTabServicesChild() {
    ImGui::Text("Services Sector");
    // List all the stuff it produces
    ImGui::Text("GDP:");
}

void conquerspace::client::systems::SysPlanetInformation::IndustryTabManufacturingChild() {
    namespace cqspc = conquerspace::common::components;
    auto& city_industry =
        GetApp().GetUniverse().get<cqspc::Industry>(selected_city_entity);
    ImGui::Text("Manufactuing Sector");
    // List all the stuff it produces
    ImGui::Text("GDP:");

    cqspc::ResourceLedger input_resources;
    cqspc::ResourceLedger output_resources;
    int count = 0;
    for (auto industry : city_industry.industries) {
        if (GetApp().GetUniverse().all_of<cqspc::ResourceConverter, cqspc::Factory>(industry)) {
            count++;
            auto& generator = GetApp().GetUniverse().get<cqspc::ResourceConverter>(industry);
            auto& recipe = GetApp().GetUniverse().get<cqspc::Recipe>(generator.recipe);
            input_resources += recipe.input;
            output_resources += recipe.output;
        }
    }
    ImGui::TextFmt("Factories: {}", count);

    ImGui::SameLine();
    if (CQSPGui::SmallDefaultButton("Factory List")) {
        factory_list_panel = true;
    }

    ImGui::Text("Output");
    // Output table
    DrawLedgerTable("industryoutput", GetApp().GetUniverse(), output_resources);

    ImGui::Text("Input");
    DrawLedgerTable("industryinput", GetApp().GetUniverse(), input_resources);
}

void conquerspace::client::systems::SysPlanetInformation::IndustryTabMiningChild() {
    namespace cqspc = conquerspace::common::components;
    auto& city_industry = GetApp().GetUniverse().get<cqspc::Industry>(selected_city_entity);
    ImGui::Text("Mining Sector");
    ImGui::Text("GDP:");
    // Get what resources they are making
    cqspc::ResourceLedger resources;
    int mine_count = 0;
    for (auto mine : city_industry.industries) {
        if (GetApp().GetUniverse().all_of<cqspc::ResourceGenerator, cqspc::Mine>(mine)) {
            auto& generator = GetApp().GetUniverse().get<cqspc::ResourceGenerator>(mine);
            resources += generator;
            mine_count++;
        }
    }
    ImGui::TextFmt("Mines: {}", mine_count);

    ImGui::SameLine();
    if (CQSPGui::SmallDefaultButton("Mine List")) {
        mine_list_panel = true;
    }

    // Draw on table
    DrawLedgerTable("mineproduction", GetApp().GetUniverse(), resources);
}

void conquerspace::client::systems::SysPlanetInformation::IndustryTabAgricultureChild() {
    ImGui::Text("Agriculture Sector");
    ImGui::Text("GDP:");
}

void conquerspace::client::systems::SysPlanetInformation::DemographicsTab() {
    namespace cqspc = conquerspace::common::components;
    using conquerspace::common::components::Settlement;
    using conquerspace::common::components::PopulationSegment;

    auto& settlement = GetApp().GetUniverse().get<Settlement>(selected_city_entity);
    for (auto &b : settlement.population) {
        ImGui::TextFmt("Population: {}", GetApp().GetUniverse().get<PopulationSegment>(b).population);
        if (GetApp().GetUniverse().all_of<cqspc::Hunger>(b)) {
            ImGui::TextFmt("Hungry");
        }
    }
    // Then do demand and other things.
}

void conquerspace::client::systems::SysPlanetInformation::ConstructionTab() {
    namespace cqspc = conquerspace::common::components;
    ImGui::Text("Construction");
    ImGui::Text("Construct factories");

    if (ImGui::BeginTabBar("constructiontab")) {
        if (ImGui::BeginTabItem("Factories")) {
            FactoryConstruction();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Mines")) {
            MineConstruction();
            ImGui::EndTabItem();
        }
        // TODO(EhWhoAmI): Add other things like labs, infrastructure, etc.
        ImGui::EndTabBar();
    }
}

void conquerspace::client::systems::SysPlanetInformation::FactoryConstruction() {
    namespace cqspc = conquerspace::common::components;
    auto recipes = GetApp().GetUniverse().view<cqspc::Recipe>();
    static int selected_recipe_index = -1;
    static entt::entity selected_recipe = entt::null;
    int index = 0;
    ImGui::BeginChild("constructionlist", ImVec2(0, 150), true, window_flags);
    for (entt::entity entity : recipes) {
        if (selected_recipe_index == -1) {
            selected_recipe_index = 0;
            selected_recipe = entity;
        }
        const bool selected = selected_recipe_index == index;
        std::string name = GetApp().GetUniverse().all_of<cqspc::Identifier>(entity) ?
            GetApp().GetUniverse().get<cqspc::Identifier>(entity) : fmt::format("{}", entity);
        if (CQSPGui::DefaultSelectable(fmt::format("{}", name).c_str(), selected)) {
            selected_recipe_index = index;
            selected_recipe = entity;
        }
        index++;
    }
    ImGui::EndChild();

    static int prod = 1;
    ImGui::PushItemWidth(-1);
    CQSPGui::DragInt("label", &prod, 1, 1, INT_MAX);
    ImGui::PopItemWidth();
    if (CQSPGui::DefaultButton("Construct!")) {
        // Construct things
        SPDLOG_INFO("Constructing factory with recipe {}", selected_recipe);
        // Add demand to the market for the amount of resources
        // When construction takes time in the future, then do the costs.
        // So first charge it to the market
        entt::entity city_market = GetApp().GetUniverse().get<cqspc::MarketCenter>(selected_planet).market;
        auto cost = conquerspace::common::systems::actions::GetFactoryCost(
            GetApp().GetUniverse(), selected_city_entity, selected_recipe, prod);
        GetApp().GetUniverse().get<cqspc::Market>(city_market).demand += cost;
        GetApp().GetUniverse().get<cqspc::ResourceStockpile>(city_market) -= cost;
        // Buy things on the market
        entt::entity factory = conquerspace::common::systems::actions::CreateFactory(
            GetApp().GetUniverse(), selected_city_entity, selected_recipe, prod);
        conquerspace::common::systems::economy::AddParticipant(
                                                    GetApp().GetUniverse(), city_market, factory);
        // Enable confirmation window
    }

    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        DrawLedgerTable("building_cost_tooltip", GetApp().GetUniverse(),
                    conquerspace::common::systems::actions::GetFactoryCost(
                            GetApp().GetUniverse(), selected_city_entity, selected_recipe, prod));
        ImGui::EndTooltip();
    }
}

void conquerspace::client::systems::SysPlanetInformation::MineConstruction() {
    namespace cqspc = conquerspace::common::components;
    ImGui::BeginChild("mineconstructionlist", ImVec2(0, 150), true, window_flags);
    auto recipes = GetApp().GetUniverse().view<cqspc::Good, cqspc::Mineral>();
    static int selected_good_index = -1;
    static entt::entity selected_good = entt::null;
    int index = 0;
    for (entt::entity entity : recipes) {
        if (selected_good_index == -1) {
            selected_good_index = 0;
            selected_good = entity;
        }
        const bool selected = selected_good_index == index;
        std::string name = GetApp().GetUniverse().all_of<cqspc::Identifier>(entity) ?
            GetApp().GetUniverse().get<cqspc::Identifier>(entity) : fmt::format("{}", entity);
        if (CQSPGui::DefaultSelectable(fmt::format("{}", name).c_str(), selected)) {
            selected_good_index = index;
            selected_good = entity;
        }
        index++;
    }
    ImGui::EndChild();

    static int prod = 1;
    ImGui::PushItemWidth(-1);
    ImGui::Text("Production");
    ImGui::SameLine();
    CQSPGui::DragInt("label", &prod, 1, 1, INT_MAX);
    ImGui::PopItemWidth();
    if (CQSPGui::DefaultButton("Construct!")) {
        // Construct things
        SPDLOG_INFO("Constructing mine with good {}", selected_good);
        // Add demand to the market for the amount of resources
        // When construction takes time in the future, then do the costs.
        // So first charge it to the market
        entt::entity city_market = GetApp().GetUniverse().get<cqspc::MarketCenter>(selected_planet).market;
        auto cost = conquerspace::common::systems::actions::GetFactoryCost(
            GetApp().GetUniverse(), selected_city_entity, selected_good, prod);
        GetApp().GetUniverse().get<cqspc::Market>(city_market).demand += cost;
        GetApp().GetUniverse().get<cqspc::ResourceStockpile>(city_market) -= cost;
        // Buy things on the market
        entt::entity factory = conquerspace::common::systems::actions::CreateMine(
            GetApp().GetUniverse(), selected_city_entity, selected_good, prod);
        conquerspace::common::systems::economy::AddParticipant(GetApp().GetUniverse(), city_market, factory);
    }

    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        DrawLedgerTable("building_cost_tooltip", GetApp().GetUniverse(),
                    conquerspace::common::systems::actions::GetMineCost(
                            GetApp().GetUniverse(), selected_city_entity, selected_good, prod));
        ImGui::EndTooltip();
    }
}

void conquerspace::client::systems::SysPlanetInformation::MineInformationPanel() {
    namespace cqspc = conquerspace::common::components;
    if(mine_list_panel) {
        auto &city_industry = GetApp().GetUniverse().get<cqspc::Industry>(selected_city_entity);
        ImGui::Begin(fmt::format("Mines of {}", selected_city_entity).c_str(), &mine_list_panel);
        // List mines
        static int selected_mine = 0;
        int mine_index = 0;
        for (int i = 0; i < city_industry.industries.size(); i++) {
            entt::entity e = city_industry.industries[i];
            if (GetApp().GetUniverse().all_of<cqspc::Mine>(e)) {
                // Then do the things
                mine_index++;
            } else {
                continue;
            }

            const bool is_selected = (selected_mine == mine_index);
            std::string name = fmt::format("{}", e);
            if (GetApp().GetUniverse().all_of<cqspc::Name>(e)) {
                name = GetApp().GetUniverse().get<cqspc::Name>(e);
            }
            if (CQSPGui::DefaultSelectable(fmt::format("{}", name).c_str(), is_selected)) {
                // Load 
                selected_mine = mine_index;
            }
            gui::EntityTooltip(GetApp().GetUniverse(), e);
        }
        ImGui::End();
    }
}

void conquerspace::client::systems::SysPlanetInformation::FactoryInformationPanel() {
    namespace cqspc = conquerspace::common::components;
    if(factory_list_panel) {
        auto &city_industry = GetApp().GetUniverse().get<cqspc::Industry>(selected_city_entity);
        ImGui::Begin(fmt::format("Factories of {}", selected_city_entity).c_str(), &factory_list_panel);
        // List mines
        static int selected_factory = 0;
        int factory_index = 0;
        for (int i = 0; i < city_industry.industries.size(); i++) {
            entt::entity e = city_industry.industries[i];
            if (GetApp().GetUniverse().all_of<cqspc::Factory>(e)) {
                // Then do the things
                factory_index++;
            } else {
                continue;
            }

            const bool is_selected = (selected_factory == factory_index);
            std::string name = fmt::format("{}", e);
            if (GetApp().GetUniverse().all_of<cqspc::Name>(e)) {
                name = GetApp().GetUniverse().get<cqspc::Name>(e);
            }
            if (CQSPGui::DefaultSelectable(fmt::format("{}", name).c_str(), is_selected)) {
                // Load 
                selected_factory = factory_index;
            }
            gui::EntityTooltip(GetApp().GetUniverse(), e);
        }
        ImGui::End();
    }
}

void conquerspace::client::systems::SysPlanetInformation::SpacePortTab() {}
