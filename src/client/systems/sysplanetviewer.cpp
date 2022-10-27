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

#include <GLFW/glfw3.h>

#include <noiseutils.h>
#include <noise/noise.h>

#include <string>
#include <map>
#include <vector>
#include <limits>

#include "client/systems/views/starsystemview.h"
#include "client/systems/gui/sysstockpileui.h"
#include "client/scenes/universescene.h"
#include "client/systems/gui/systooltips.h"
#include "client/components/planetrendering.h"

#include "common/components/area.h"
#include "common/components/bodies.h"
#include "common/components/name.h"
#include "common/components/coordinates.h"
#include "common/components/organizations.h"
#include "common/components/player.h"
#include "common/components/population.h"
#include "common/components/resource.h"
#include "common/components/surface.h"
#include "common/components/economy.h"
#include "common/components/ships.h"
#include "common/components/infrastructure.h"
#include "common/components/history.h"
#include "common/components/science.h"

#include "common/util/utilnumberdisplay.h"
#include "common/systems/actions/factoryconstructaction.h"
#include "common/systems/actions/shiplaunchaction.h"
#include "common/systems/economy/markethelpers.h"

#include "engine/gui.h"
#include "engine/cqspgui.h"

namespace cqspc = cqsp::common::components;

namespace cqsp::client::systems {
void SysPlanetInformation::DisplayPlanet() {
    to_see = false;
    if (!to_see) {
        return;
    }

    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x * 0.4f,
                                    ImGui::GetIO().DisplaySize.y * 0.8f),
                             ImGuiCond_Appearing);
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.79f,
                                   ImGui::GetIO().DisplaySize.y * 0.55f),
                            ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (SysStarSystemRenderer::IsFoundingCity(GetUniverse())) {
        ImGui::SetNextWindowCollapsed(true, ImGuiCond_Always);
    } else {
        ImGui::SetNextWindowCollapsed(false, ImGuiCond_Always);
    }
    std::string viewname;
    if (selected_planet == entt::null) {
        return;
    }
    switch (view_mode) {
        case ViewMode::PLANET_VIEW:
            viewname = gui::GetName(GetUniverse(), selected_planet);
            break;
        case ViewMode::CITY_VIEW:
            viewname = gui::GetName(GetUniverse(), selected_city_entity);
            break;
    }

    std::string windowname = "Economic data: " + viewname;
    ImGui::Begin(windowname.c_str(), &to_see,
                 window_flags | ImGuiWindowFlags_NoCollapse);
    switch (view_mode) {
        case ViewMode::PLANET_VIEW:
            PlanetInformationPanel();
            break;
        case ViewMode::CITY_VIEW:
            CityInformationPanel();
            DetailedProductionPanel();
            break;
    }
    ImGui::End();
}

void SysPlanetInformation::Init() {}

void SysPlanetInformation::DoUI(int delta_time) {
    DisplayPlanet();

    //ConstructionConfirmationPanel();

    if (renaming_city) {
        ImGui::SetNextWindowSize(ImVec2(300, -1));
        ImGui::Begin("Rename City", &renaming_city);

        ImGui::PushItemWidth(-1);
        ImGui::InputText("Rename City", &city_founding_name);
        ImGui::PopItemWidth();
        if (ImGui::Button("Rename")) {
            renaming_city = false;
            using cqsp::common::components::Name;
            GetUniverse().get<Name>(selected_city_entity).name = city_founding_name;
        }
        ImGui::End();
    }
}

void SysPlanetInformation::DoUpdate(int delta_time) {
    // If clicked on a planet, go to the planet
    // Get the thing
    to_see = true;
    namespace cqspb = cqsp::common::components::bodies;
    selected_planet = cqsp::scene::GetCurrentViewingPlanet(GetUniverse());
    entt::entity mouse_over = GetUniverse().view<MouseOverEntity>().front();
    if (!ImGui::GetIO().WantCaptureMouse &&
                GetApp().MouseButtonIsReleased(GLFW_MOUSE_BUTTON_LEFT) &&
                mouse_over == selected_planet && !cqsp::scene::IsGameHalted() &&
        !GetApp().MouseDragged()) {
        to_see = true;

        if (GetUniverse().valid(selected_planet)) {
            SPDLOG_INFO("Switched entity: {}",
                        GetUniverse()
                            .get<cqspc::Identifier>(selected_planet)
                            .identifier);
        } else {
            SPDLOG_INFO("Switched entity is not valid");
        }
    }
    if (!GetUniverse().valid(selected_planet) || !GetUniverse().all_of<cqspb::Body>(selected_planet)) {
        to_see = false;
    }
}

void SysPlanetInformation::CityInformationPanel() {
    if (CQSPGui::ArrowButton("cityinformationpanel", ImGuiDir_Left)) {
        view_mode = ViewMode::PLANET_VIEW;
    }
    ImGui::SameLine();

    ImGui::TextFmt("{}", GetUniverse().get<cqspc::Name>(selected_city_entity).name);
    if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
        // Then rename the city
        renaming_city = true;
        city_founding_name = GetUniverse().get<cqspc::Name>(selected_city_entity).name;
    }
    ImGui::SameLine();
    if (ImGui::Button("Focus on city")) {
        // Focus city
        GetApp().GetUniverse().emplace_or_replace<FocusedCity>(selected_city_entity);
    }
    if (market_information_panel) {
        ImGui::Begin("Market Information", &market_information_panel,
                     window_flags);
        MarketInformationTooltipContent(selected_city_entity);
        ImGui::End();
    }

    if (GetUniverse().all_of<cqspc::Settlement>(selected_city_entity)) {
        int size = GetUniverse().get<cqspc::Settlement>(selected_city_entity).population.size();
        for (auto seg_entity : GetUniverse().get<cqspc::Settlement>(selected_city_entity).population) {
            auto& pop_segement = GetUniverse().get<cqspc::PopulationSegment>(seg_entity);
            ImGui::TextFmt("Population: {}", cqsp::util::LongToHumanString(pop_segement.population));
        }
    } else {
        ImGui::TextFmt("No population");
    }

    if (GetUniverse().all_of<cqspc::CityTimeZone>(selected_city_entity)) {
        // Set time zone
        auto& tz = GetUniverse().get<cqspc::CityTimeZone>(selected_city_entity);
        auto& tz_def = GetUniverse().get<cqspc::TimeZone>(tz.time_zone);
        int time = (int) (GetUniverse().date.GetDate() + tz_def.time_diff) % 24;
        if (time < 0) {
            time = time + 24;
        }
        const std::string& tz_name =
            GetUniverse().get<cqspc::Identifier>(tz.time_zone).identifier;
        ImGui::TextFmt(
            "{} {}:00 ({})", GetUniverse().date.ToString(tz_def.time_diff),
            time, tz_name);
    }

    if (GetUniverse().all_of<cqspc::IndustrialZone>(selected_city_entity)) {
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
            /*
            if (ImGui::BeginTabItem("Construction")) {
                ConstructionTab();
                ImGui::EndTabItem();
            }
            */
            if (ImGui::BeginTabItem("Infrastructure")) {
                InfrastructureTab();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Science")) {
                ScienceTab();
                ImGui::EndTabItem();
            }
            if (GetUniverse().any_of<cqspc::infrastructure::SpacePort>(selected_city_entity)) {
                if (ImGui::BeginTabItem("Space Port")) {
                    SpacePortTab();
                    ImGui::EndTabItem();
                }
            }
            ImGui::EndTabBar();
        }
    }
}

void SysPlanetInformation::PlanetInformationPanel() {
    if (!GetUniverse().all_of<cqspc::Habitation>(selected_planet)) {
        ImGui::TextFmt("Planet is uninhabited");
        return;
    }
    auto& habit = GetUniverse().get<cqspc::Habitation>(selected_planet);
    ImGui::TextFmt("Cities: {}", habit.settlements.size());

    ImGui::BeginChild("citylist", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar |
                                        window_flags);


    if (ImGui::Button("Found City")) {
        // Enable city founding
        entt::entity ent = GetUniverse().create();
        GetUniverse().emplace<CityFounding>(ent);
        is_founding_city = true;
    }

    // Get population
    uint64_t pop_size = 0;
    for (entt::entity settlement : habit.settlements) {
        for (entt::entity population : GetUniverse().get<cqspc::Settlement>(settlement).population) {
            pop_size += GetUniverse().get<cqspc::PopulationSegment>(population).population;
        }
    }
    ImGui::TextFmt("Population: {} ({})", cqsp::util::LongToHumanString(pop_size), pop_size);
    ImGui::Separator();

    // Show resources
    /*
    if (GetUniverse().all_of<cqspc::ResourceDistribution>(selected_planet)) {
        auto& dist = GetUniverse().get<cqspc::ResourceDistribution>(selected_planet);
        using cqsp::client::components::PlanetTerrainRender;
        // Show the resources on it
        ImGui::Text("Resources");
        if (ImGui::Button("Default")) {
            GetUniverse().remove_if_exists<PlanetTerrainRender>(selected_planet);
        }
        for (auto it = dist.begin(); it != dist.end(); it++) {
            if (ImGui::Button(gui::GetName(GetUniverse(), it->first).c_str())) {
                // Set rendering thing
                GetUniverse().emplace_or_replace<PlanetTerrainRender>(
                        selected_planet, it->first);
            }
        }
    }*/
    // List cities
    for (int i = 0; i < habit.settlements.size(); i++) {
        const bool is_selected = (selected_city_index == i);

        entt::entity e = habit.settlements[i];
        std::string name = "No name";
        if (GetUniverse().any_of<cqspc::Name>(e)) {
            name = GetUniverse().get<cqspc::Name>(e);
        }
        if (CQSPGui::DefaultSelectable(fmt::format("{}", name).c_str(), is_selected)) {
            // Load city
            selected_city_index = i;
            selected_city_entity = habit.settlements[i];
            view_mode = ViewMode::CITY_VIEW;
        }
        gui::EntityTooltip(GetUniverse(), e);
    }
    ImGui::EndChild();
}

void SysPlanetInformation::ResourcesTab() {
    // Consolidate resources
    auto& city_industry =
        GetUniverse().get<cqspc::IndustrialZone>(selected_city_entity);
    cqspc::ResourceLedger resources =
        GetUniverse().get<cqspc::ResourceStockpile>(selected_city_entity);
    DrawLedgerTable("cityresources", GetUniverse(), resources);
    for (auto area : city_industry.industries) {
        if (GetUniverse().all_of<cqspc::ResourceStockpile>(area)) {
            // Add resources
            auto& stockpile = GetUniverse().get<cqspc::ResourceStockpile>(area);
            resources += stockpile;
        }
    }

    DrawLedgerTable("cityresources2", GetUniverse(), resources);
}

void SysPlanetInformation::IndustryTab() {
    auto& city_industry =
        GetUniverse().get<cqspc::IndustrialZone>(selected_city_entity);

    int height = 300;
    ImGui::TextFmt("Factories: {}", city_industry.industries.size());
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        for (auto& at : city_industry.industries) {
            ImGui::TextFmt("{}", gui::GetEntityType(GetUniverse(), at));
        }
        ImGui::EndTooltip();
    }

    IndustryTabGenericChild<cqspc::Service>(
        "Service Sector", "Company",
        ImVec2(ImGui::GetContentRegionAvail().x * 0.5f -
                   ImGui::GetStyle().ItemSpacing.y,
               height));

    ImGui::SameLine();

    IndustryTabGenericChild<cqspc::Factory>("Manufacturing Sector", "Factories",
                                            ImVec2(-1, height));
    IndustryTabGenericChild<cqspc::Mine>("Mining Sector", "Mines",
                                            ImVec2(ImGui::GetContentRegionAvail().x * 0.5f -
                                 ImGui::GetStyle().ItemSpacing.y, height));

    ImGui::SameLine();

    IndustryTabFinanceChild(ImVec2(-1, height));
}

template <typename inddustrytype>
void SysPlanetInformation::IndustryTabGenericChild(
    const std::string& tabname, const std::string & industryname,
                                                  const ImVec2 & size) {
    ImGui::BeginChild(tabname.c_str(), size, true,
                      ImGuiWindowFlags_HorizontalScrollbar | window_flags);
    auto& city_industry =
        GetUniverse().get<cqspc::IndustrialZone>(selected_city_entity);
    ImGui::TextFmt(tabname);
    // List all the stuff it produces

    cqspc::ResourceLedger input_resources;
    cqspc::ResourceLedger output_resources;
    double GDP_calculation = 0;
    int count = 0;
    for (auto industry : city_industry.industries) {
        if (GetUniverse()
                .all_of<cqspc::Production, inddustrytype>(
                industry)) {
            count++;
            const cqspc::Production& generator =
                GetUniverse().get<cqspc::Production>(industry);
            const cqspc::Recipe& recipe =
                GetUniverse().get<cqspc::Recipe>(generator.recipe);
            const cqspc::IndustrySize& ratio =
                GetUniverse().get<cqspc::IndustrySize>(industry);

            input_resources  += (recipe.input * ratio.size);
            output_resources[recipe.output.entity] += recipe.output.amount;

            if (GetUniverse().all_of<cqspc::Wallet>(industry)) {
                GDP_calculation +=
                    GetUniverse().get<cqspc::Wallet>(industry).GetGDPChange();
            }
        }
    }
    ImGui::TextFmt("GDP: {}", cqsp::util::LongToHumanString(GDP_calculation));
    ImGui::TextFmt("{} Count: {}", industryname, count);

    ImGui::SameLine();
    if (CQSPGui::SmallDefaultButton("List")) {
        factory_list_panel = true;
        industrylist.resize(0);
        auto& city_industry =
            GetUniverse()
                .get<cqspc::IndustrialZone>(selected_city_entity)
                .industries;
        for (entt::entity production : city_industry) {
            if (GetUniverse().all_of<inddustrytype>(production))
                industrylist.push_back(production);
        }
    }

    ImGui::Text("Output");
    // Output table
    DrawLedgerTable(tabname + "output", GetUniverse(), output_resources);

    ImGui::Text("Input");
    DrawLedgerTable(tabname + "input", GetUniverse(), input_resources);
    ImGui::EndChild();
}

void SysPlanetInformation::DetailedProductionPanel() {
    if (factory_list_panel && industrylist.size() > 0) {
        ImGui::Begin(
            fmt::format("Factories of {}", selected_city_entity).c_str(),
            &factory_list_panel);

        for (entt::entity e : industrylist) {
            const bool is_selected = (selected_factory == e);
            std::string name = fmt::format("{}", e);
            if (GetUniverse().all_of<cqspc::Name>(e)) {
                name = GetUniverse().get<cqspc::Name>(e);
            }
            if (CQSPGui::DefaultSelectable(fmt::format("{}", name).c_str(),
                                           is_selected)) {
                // Load
                e = selected_factory;
            }
            gui::EntityTooltip(GetUniverse(), e);
        }
        ImGui::End();
    }
}

void SysPlanetInformation::IndustryTabFinanceChild(const ImVec2& size) {
    ImGui::BeginChild("FinancePan", size, true,
                      ImGuiWindowFlags_HorizontalScrollbar | window_flags);

    ImGui::Text("Finance Sector");
    // List all the stuff it produces
    ImGui::Text("GDP:");
    ImGui::EndChild();
}

void SysPlanetInformation::DemographicsTab() {
    using cqsp::common::components::Settlement;
    using cqsp::common::components::PopulationSegment;

    auto& settlement = GetUniverse().get<Settlement>(selected_city_entity);
    for (auto &seg_entity : settlement.population) {
        ImGui::TextFmt("Population: {}",
            cqsp::util::LongToHumanString(GetUniverse().get<PopulationSegment>(seg_entity).population));
        gui::EntityTooltip(GetUniverse(), seg_entity);
        if (GetUniverse().all_of<cqspc::Hunger>(seg_entity)) {
            ImGui::TextFmt("Hungry");
        }
        if (GetUniverse().any_of<cqsp::common::components::Employee>(seg_entity)) {
            auto& employee = GetUniverse().get<cqspc::Employee>(seg_entity);
            ImGui::TextFmt("Working Population: {}/{}", cqsp::util::LongToHumanString(employee.employed_population),
                                                        cqsp::util::LongToHumanString(employee.working_population));
            if (employee.working_population > 0) {
                ImGui::ProgressBar(static_cast<float>(employee.employed_population) /
                                    static_cast<float>(employee.working_population));
            }
        }
        // Get spending for population
        if (GetUniverse().all_of<cqspc::Wallet>(seg_entity)) {
            auto& wallet = GetUniverse().get<cqspc::Wallet>(seg_entity);
            ImGui::TextFmt("Spending: {}", cqsp::util::LongToHumanString(wallet.GetGDPChange()));
        }
        // Market
        if (GetUniverse().all_of<cqspc::Market>(selected_city_entity)) {
            if (ImGui::Button("Market Data")) {
                market_information_panel = true;
            }
            if (ImGui::IsItemHovered()) {
                CQSPGui::SimpleTextTooltip(
                    "Click for detailed market information");
            }
        }
    }
    // Then do demand and other things.
}

void SysPlanetInformation::ConstructionTab() {
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
        if (ImGui::BeginTabItem("Power Plant")) {
            ImGui::EndTabItem();
        }
        if (!GetUniverse().any_of<cqspc::infrastructure::SpacePort>(selected_city_entity)) {
            if (ImGui::BeginTabItem("Space Port##Construction")) {
                if (ImGui::Button("Construct Spaceport")) {
                    GetUniverse().emplace<cqspc::infrastructure::SpacePort>(selected_city_entity);
                }
                ImGui::EndTabItem();
            }
        }
        // TODO(EhWhoAmI): Add other things like labs, infrastructure, etc.
        ImGui::EndTabBar();
    }
}

void SysPlanetInformation::FactoryConstruction() {
    auto recipes = GetUniverse().view<cqspc::Recipe>();
    static int selected_recipe_index = -1;
    static entt::entity selected_recipe = entt::null;
    int index = 0;

    entt::entity player = GetUniverse().view<common::components::Player>().front();
    auto& tech_progress =
        GetUniverse().get<common::components::science::TechnologicalProgress>(player);

    // Check for tech and stuff, I guess
    ImGui::BeginChild("constructionlist", ImVec2(0, 150), true, window_flags);
    // Check if the civilization has the factory recipe
    for (entt::entity entity : tech_progress.researched_recipes) {
        if (selected_recipe_index == -1) {
            selected_recipe_index = 0;
            selected_recipe = entity;
        }
        const bool selected = selected_recipe_index == index;
        std::string name = gui::GetName(GetUniverse(), entity);

        if (CQSPGui::DefaultSelectable(fmt::format("{}", name).c_str(), selected)) {
            selected_recipe_index = index;
            selected_recipe = entity;
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
    if (tech_progress.researched_recipes.size() > 0) {
        auto cost =
                common::systems::actions::GetFactoryCost(GetUniverse(), selected_city_entity, selected_recipe, prod);

        RecipeConstructionCostPanel(selected_recipe, prod, cost);
        RecipeConstructionConstructButton(selected_recipe, prod, cost);

        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            DrawLedgerTable("building_cost_tooltip", GetUniverse(), cost);
            ImGui::EndTooltip();
        }
    }
}

void SysPlanetInformation::MineConstruction() {
    ImGui::BeginChild("mineconstructionlist", ImVec2(0, 150), true, window_flags);
    auto recipes = GetUniverse().view<cqspc::Good, cqspc::Mineral>();
    static int selected_good_index = -1;
    static entt::entity selected_good = entt::null;
    int index = 0;
    entt::entity player = GetUniverse().view<common::components::Player>().front();
    auto& tech_progress =
        GetUniverse().get<common::components::science::TechnologicalProgress>(player);

    for (entt::entity entity : tech_progress.researched_mining) {
        if (selected_good_index == -1) {
            selected_good_index = 0;
            selected_good = entity;
        }
        const bool selected = selected_good_index == index;
        std::string name = gui::GetName(GetUniverse(), entity);
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
    if (tech_progress.researched_mining.size() > 0) {
        if (CQSPGui::DefaultButton("Construct!")) {
            // Construct things
            SPDLOG_INFO("Constructing mine with good {}", selected_good);
            // Add demand to the market for the amount of resources
            // When construction takes time in the future, then do the costs.
            // So first charge it to the market
            entt::entity city_market = GetUniverse().get<cqspc::MarketCenter>(selected_planet).market;
            /*auto cost = cqsp::common::systems::actions::GetFactoryCost(
                GetUniverse(), selected_city_entity, selected_good, prod);
            GetUniverse().get<cqspc::Market>(city_market).demand += cost;
            GetUniverse().get<cqspc::ResourceStockpile>(city_market) -= cost;
            */
            // Buy things on the market
            //entt::entity factory = cqsp::common::systems::actions::CreateMine(
            //    GetUniverse(), selected_city_entity, selected_good, 1, prod);
            //cqsp::common::systems::economy::AddParticipant(GetUniverse(), city_market, factory);
        }

        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            DrawLedgerTable("building_cost_tooltip", GetUniverse(),
                        cqsp::common::systems::actions::GetFactoryCost(
                                GetUniverse(), selected_city_entity, selected_good, prod));
            ImGui::EndTooltip();
        }
    }
}

void SysPlanetInformation::SpacePortTab() {
    namespace cqspt = cqsp::common::components::types;
    namespace cqsps = cqsp::common::components::ships;
    namespace cqspb = cqsp::common::components::bodies;

    // Set the things
    static float semi_major_axis = 8000;
    static float inclination = 0;
    static float eccentricity = 0;
    static float arg_of_perapsis = 0;
    static float LAN = 0;
    ImGui::SliderFloat("Semi Major Axis", &semi_major_axis, 6000, 5000000);
    ImGui::SliderFloat("Eccentricity", &eccentricity, 0, 0.9999);
    ImGui::SliderAngle("Inclination", &inclination, 0, 180);
    ImGui::SliderAngle("Argument of perapsis", &arg_of_perapsis, 0, 360);
    ImGui::SliderAngle("Longitude of the ascending node", &LAN, 0, 360);
    if (ImGui::Button("Launch!")) {
        // Get reference body
        entt::entity reference_body = selected_planet;
        // Launch inclination will be the inclination of the thing
        double axial = GetUniverse().get<cqspc::bodies::Body>(selected_planet).axial;
        double inc = GetUniverse().get<cqspc::types::SurfaceCoordinate>(selected_city_entity)
                .r_latitude();
        inc += axial;
        double sma = 0;
        // Currently selected city
        //entt::entity star_system = GetUniverse().get<cqspc::bodies::Body>(selected_planet);
        // Launch
        cqspc::types::Orbit orb;
        orb.reference_body = selected_planet;
        orb.inclination = inclination;
        orb.semi_major_axis = semi_major_axis;
        orb.eccentricity = eccentricity;
        orb.w = arg_of_perapsis;
        orb.LAN = LAN;
        cqsp::common::systems::actions::LaunchShip(GetUniverse(), orb);
        //cqsp::common::systems::actions::CreateShip(
        //GetUniverse(), entt::null, selected_planet, star_system);
    }
}

void SysPlanetInformation::InfrastructureTab() {
    if (power_plant_output_panel) {
        ImGui::Begin("Power Plant", &power_plant_output_panel);
        double& prod_d = GetUniverse().get<cqspc::infrastructure::PowerPlant>(power_plant_changing).production;
        float prod = static_cast<float>(prod_d);
        ImGui::PushItemWidth(-1);
        CQSPGui::DragFloat("power_plant_supply", &prod, 1, 1, INT_MAX);
        prod_d = prod;
        ImGui::PopItemWidth();
        ImGui::End();
    }
    ImGui::Text("Infrastructure");
    // Get the areas that generate power
    ImGui::Separator();
    ImGui::Text("Power");
    auto& city_industry =
        GetUniverse().get<cqspc::IndustrialZone>(selected_city_entity);
    double power_production = 0;
    double power_demand = 0;
    if (GetUniverse().any_of<cqspc::infrastructure::CityPower>(selected_city_entity)) {
        auto& power = GetUniverse().get<cqspc::infrastructure::CityPower>(selected_city_entity);
        power_production = power.total_power_prod;
        power_demand = power.total_power_consumption;
    }
    std::vector<entt::entity> power_plants;
    for (int i = 0; i < city_industry.industries.size(); i++) {
        entt::entity industry = city_industry.industries[i];
        if (GetUniverse().any_of<cqspc::infrastructure::PowerPlant>(industry)) {
            power_plants.push_back(industry);
        }
    }
    ImGui::TextFmt("Power Production: {}/{} MW", power_demand, power_production);
    if (GetUniverse().any_of<cqspc::infrastructure::BrownOut>(selected_city_entity)) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255, 0, 0, 1));
        // Get seriousness, then do random other things
        ImGui::TextFmt("Brown Out!");
        ImGui::PopStyleColor(1);
    }

    for (entt::entity plant : power_plants) {
        double prod = GetUniverse().get<cqspc::infrastructure::PowerPlant>(plant).production;
        ImGui::TextFmt("Power Plant: {} MW", prod);
        ImGui::SameLine();
        if (ImGui::Button("Change Power plant output")) {
            power_plant_output_panel = true;
            power_plant_changing = plant;
        }
    }
}

void SysPlanetInformation::ScienceTab() {
    if (!GetUniverse().valid(selected_planet)) {
        return;
    }
    auto& city_industry =
        GetUniverse().get<cqspc::IndustrialZone>(selected_city_entity);
    ImGui::Text("Science");
    // Get the science labs
    cqspc::ResourceLedger led;
    for (int i = 0; i < city_industry.industries.size(); i++) {
        entt::entity industry = city_industry.industries[i];
        if (GetUniverse().any_of<cqspc::science::Lab>(industry)) {
            ImGui::Text("Lab %d", i);
            auto& lab = GetUniverse().get<cqspc::science::Lab>(industry);
            led += lab.science_contribution;
        }
    }
    // Get all the combined science
    ImGui::Text("Science Contribution");
    systems::DrawLedgerTable("science_contrib_table", GetUniverse(), led);
}

void SysPlanetInformation::MarketInformationTooltipContent(
    const entt::entity marketentity) {
}

void SysPlanetInformation::ConstructionConfirmationPanel() {
    if (!enable_construction_confirmation_panel) {
        return;
    }
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f,
                                   ImGui::GetIO().DisplaySize.y * 0.5f),
                            ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::Begin("Construction Complete", &enable_construction_confirmation_panel, ImGuiWindowFlags_NoDecoration);
    ImGui::Text("Constructed factory");
    if (ImGui::Button("Ok", ImVec2(-1, 0))) {
        enable_construction_confirmation_panel = false;
    }
    ImGui::End();
}

void SysPlanetInformation::RecipeConstructionCostPanel(entt::entity selected_recipe, double prod,
                                                       const common::components::ResourceLedger& cost) {
    entt::entity city_market = GetUniverse().get<cqspc::MarketCenter>(selected_planet).market;
    // Cost table
    ImGui::TextFmt("Estimated Cost: {}", common::systems::economy::GetCost(GetUniverse(), city_market, cost));
    CQSPGui::SimpleTextTooltip("Estimated cost at current market prices");

    ImGui::Text("Resources Needed");
    DrawLedgerTable("factory_cost", GetUniverse(), cost);
}

void
SysPlanetInformation::RecipeConstructionConstructButton(entt::entity selected_recipe, double prod,
                                                        const common::components::ResourceLedger& cost) {
    // Get the cost, and display it
    // Calculate the cost
    if (!CQSPGui::DefaultButton("Construct!")) {
        return;
    }
    // Construct things
    SPDLOG_INFO("Constructing factory with recipe {}", selected_recipe);
    // Create construction site and do the cost
    entt::entity player = GetUniverse().view<cqspc::Player>().front();

    entt::entity city_market = GetUniverse().get<cqspc::MarketCenter>(selected_planet).market;
    entt::entity factory = common::systems::actions::OrderConstructionFactory(
            GetUniverse(), selected_city_entity, city_market, selected_recipe, prod,
            player);
    if (factory == entt::null) {
        return;
    }
    enable_construction_confirmation_panel = true;
}
}  // namespace cqsp::client::systems
