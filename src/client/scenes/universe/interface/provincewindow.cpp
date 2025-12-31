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
#include "client/scenes/universe/interface/provincewindow.h"

#include <limits>
#include <string>

#include "client/components/clientctx.h"
#include "client/scenes/universe/interface/markettable.h"
#include "client/scenes/universe/interface/sysstockpileui.h"
#include "client/scenes/universe/interface/systooltips.h"
#include "client/scenes/universe/views/starsystemrenderer.h"
#include "core/actions/maneuver/commands.h"
#include "core/actions/shiplaunchaction.h"
#include "core/components/infrastructure.h"
#include "core/components/market.h"
#include "core/components/name.h"
#include "core/components/orbit.h"
#include "core/components/organizations.h"
#include "core/components/population.h"
#include "core/components/resource.h"
#include "core/components/ships.h"
#include "core/components/spaceport.h"
#include "core/components/surface.h"
#include "core/util/nameutil.h"
#include "core/util/utilnumberdisplay.h"
#include "engine/cqspgui.h"

namespace cqsp::client::systems {

namespace components = cqsp::core::components;
namespace infrastructure = components::infrastructure;
namespace types = components::types;
namespace ships = components::ships;
namespace bodies = components::bodies;
using components::PopulationSegment;
using components::Settlement;
using components::Wallet;
using core::util::GetName;

using util::NumberToHumanString;

void SysProvinceInformation::Init() {}

void SysProvinceInformation::DoUI(int delta_time) {
    const entt::entity selected_country = GetUniverse().view<ctx::SelectedProvince>().front();
    if (selected_country != current_province) {
        current_province = selected_country;
        current_province = current_city;
        view_mode = ViewMode::COUNTRY_VIEW;
        visible = true;
    }
    if (current_province == entt::null || !GetUniverse().any_of<components::Province>(current_province)) {
        return;
    }
    if (!visible) {
        return;
    }
    // Get selected country
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x * 0.4f, ImGui::GetIO().DisplaySize.y * 0.8f),
                             ImGuiCond_Appearing);
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.79f, ImGui::GetIO().DisplaySize.y * 0.55f),
                            ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::Begin("Province Information", &visible);
    // Then do all countries and compile gdp data
    switch (view_mode) {
        case ViewMode::COUNTRY_VIEW:
            ProvinceView();
            break;
        case ViewMode::CITY_VIEW:
            // CityView();
            break;
    }
    ImGui::End();

    IndustryListWindow();
}

void SysProvinceInformation::DoUpdate(int delta_time) {}

void SysProvinceInformation::ProvinceView() {
    ImGui::TextFmt("{}", GetName(GetUniverse(), current_province));
    // List the cities
    auto& city_list = GetUniverse().get<components::Province>(current_province);
    int population = 0;
    for (entt::entity entity : city_list.cities) {
        // Get city population
        auto& settlement = GetUniverse().get<Settlement>(entity);
        for (auto& seg_entity : settlement.population) {
            auto& segment = GetUniverse().get<PopulationSegment>(seg_entity);
            population += segment.population;
        }
    }
    ImGui::TextFmt("Part of {}", GetName(GetUniverse(), city_list.country));
    ImGui::TextFmt("Population: {}", NumberToHumanString(population));
    ImGui::Separator();
    if (ImGui::BeginTabBar("ProvinceInformationTabs", ImGuiTabBarFlags_None)) {
        // if (ImGui::BeginTabItem("Cities")) {
        //     for (entt::entity entity : city_list.cities) {
        //         if (CQSPGui::DefaultSelectable(fmt::format("{}", GetName(GetUniverse(), entity)).c_str())) {
        //             current_city = entity;
        //             changed_city = true;
        //             view_mode = ViewMode::CITY_VIEW;
        //         }
        //     }
        //     ImGui::EndTabItem();
        // }
        // Add all the economic stuff
        CityIndustryTabs();
        if (ImGui::BeginTabItem("Neighbors")) {
            for (entt::entity entity : city_list.neighbors) {
                ImGui::TextFmt("{}", GetName(GetUniverse(), entity));
            }
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
}

void SysProvinceInformation::CityView() {
    // Get city information
    if (CQSPGui::ArrowButton("cityinformationpanelback", ImGuiDir_Left)) {
        view_mode = ViewMode::COUNTRY_VIEW;
    }
    ImGui::SameLine();
    ImGui::TextFmt("{}", GetUniverse().get<components::Name>(current_city).name);
    ImGui::SameLine();
    if (ImGui::Button("Focus on city")) {
        // Focus city
        GetUniverse().emplace_or_replace<FocusedCity>(current_city);
    }
    ImGui::Separator();
    if (GetUniverse().all_of<Settlement>(current_city)) {
        int size = GetUniverse().get<Settlement>(current_city).population.size();
        for (auto seg_entity : GetUniverse().get<Settlement>(current_city).population) {
            auto& pop_segement = GetUniverse().get<PopulationSegment>(seg_entity);
            ImGui::TextFmt("Population: {}", NumberToHumanString(pop_segement.population));
        }
    } else {
        ImGui::TextFmt("No population");
    }

    // Now do time zone
    if (GetUniverse().all_of<components::CityTimeZone>(current_city)) {
        // Set time zone
        auto& tz = GetUniverse().get<components::CityTimeZone>(current_city);
        auto& tz_def = GetUniverse().get<components::TimeZone>(tz.time_zone);
        int time = (int)(GetUniverse().date.GetDate() + tz_def.time_diff) % 24;
        if (time < 0) {
            time = time + 24;
        }

        const std::string& tz_name = GetUniverse().get<components::Identifier>(tz.time_zone).identifier;
        int hour = GetUniverse().date.GetHour(tz_def.time_diff);
        ImGui::TextFmt("Time: {} {}:{} ({})", GetUniverse().date.ToString(tz_def.time_diff), hour,
                       GetUniverse().date.GetMinute(), tz_name);
    }

    // Other industry information
    if (GetUniverse().all_of<components::IndustrialZone>(current_city)) {
        CityIndustryTabs();
    } else {
        ImGui::Text("City has no Industry");
    }
}

void SysProvinceInformation::DisplayWallet(entt::entity entity) {
    if (GetUniverse().all_of<Wallet>(entity)) {
        Wallet& wallet = GetUniverse().get<Wallet>(entity);
        ImGui::TextFmt("GDP Contribution: {}", NumberToHumanString(wallet.GetGDPChange()));
        ImGui::TextFmt("Balance: {}", NumberToHumanString(wallet.GetBalance()));
        ImGui::TextFmt("Balance change: {}", NumberToHumanString(wallet.GetChange()));
    } else {
        ImGui::TextFmt("No wallet");
    }
}

void SysProvinceInformation::CityIndustryTabs() {
    if (ImGui::BeginTabBar("CityTabs", ImGuiTabBarFlags_None)) {
        if (ImGui::BeginTabItem("Demographics")) {
            DemographicsTab();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Industries")) {
            IndustryTab();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Infrastructure")) {
            InfrastructureTab();
            ImGui::EndTabItem();
        }
        const bool has_spaceport = GetUniverse().any_of<components::infrastructure::SpacePort>(current_city);
        if (!has_spaceport) {
            ImGui::BeginDisabled();
        }
        if (ImGui::BeginTabItem("Space Port")) {
            SpacePortTab();
            ImGui::EndTabItem();
        }
        if (!has_spaceport) {
            ImGui::EndDisabled();
        }
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) && !has_spaceport) {
            ImGui::BeginTooltip();
            ImGui::Text("Build a Space Port in this city to unlock this ");
            ImGui::EndTooltip();
        }
        if (ImGui::BeginTabItem("Economy")) {
            // Show economy window
            const components::Market& market = GetUniverse().get<components::Market>(current_city);
            // List the market's connected cities
            ImGui::TextFmt("Connected cities");
            for (entt::entity entity : market.connected_markets) {
                ImGui::TextFmt("{}", GetName(GetUniverse(), entity));
            }
            ImGui::Separator();
            // Now market wallet
            DisplayWallet(current_city);
            MarketInformationTable(GetUniverse(), current_city);
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
}

void SysProvinceInformation::DemographicsTab() {
    auto& settlement = GetUniverse().get<Settlement>(current_city);
    for (auto& seg_entity : settlement.population) {
        auto& pop_segement = GetUniverse().get<PopulationSegment>(seg_entity);
        ImGui::TextFmt("Population: {}", NumberToHumanString(pop_segement.population));

        gui::EntityTooltip(GetUniverse(), seg_entity);
        if (GetUniverse().all_of<components::Hunger>(seg_entity)) {
            ImGui::TextFmt("Hungry");
        }

        ImGui::TextFmt("Spending: {}", NumberToHumanString(static_cast<uint64_t>(pop_segement.spending)));
        ImGui::TextFmt("Spending per capita: {}", pop_segement.spending / pop_segement.population);
        ImGui::TextFmt("Income: {}", NumberToHumanString(static_cast<uint64_t>(pop_segement.income)));
        ImGui::TextFmt("Income per capita: {}", pop_segement.income / pop_segement.population);

        ImGui::TextFmt("Labor Force: {}", NumberToHumanString(pop_segement.labor_force));
        ImGui::TextFmt("Employed: {}", NumberToHumanString(pop_segement.employed_amount));
        ImGui::TextFmt("Unemployment Rate: {:.2f}", (1. - static_cast<double>(pop_segement.employed_amount) /
                                                              static_cast<double>(pop_segement.labor_force)) *
                                                        100.);
        ImGui::TextFmt("Standard of Living: {}", pop_segement.standard_of_living);

        // Get spending for population
        DisplayWallet(seg_entity);
        if (GetUniverse().all_of<Wallet>(seg_entity)) {
            Wallet& wallet = GetUniverse().get<Wallet>(seg_entity);
            ImGui::TextFmt("GDP per capita: {}", wallet.GetGDPChange() / pop_segement.population);
        }

        if (ImGui::CollapsingHeader("Resource Consumption")) {
            if (GetUniverse().all_of<components::ResourceConsumption>(seg_entity)) {
                auto& res_consumption = GetUniverse().get<components::ResourceConsumption>(seg_entity);
                DrawLedgerTable("Resource consumption", GetUniverse(), res_consumption);
            }
        }
        // Display the data
        ImGui::Separator();
    }
}

void SysProvinceInformation::IndustryTab() {
    auto& city_industry = GetUniverse().get<components::IndustrialZone>(current_city);
    int height = 300;
    ImGui::TextFmt("Factories: {}", city_industry.industries.size());
    if (ImGui::SmallButton("Factory list")) {
        // Put all the economy window information
        city_factory_info = true;
    }
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        for (auto& at : city_industry.industries) {
            ImGui::TextFmt("{}", core::util::GetEntityType(GetUniverse(), at));
        }
        ImGui::EndTooltip();
    }

    // Calculate the number of stuff
    uint64_t labor_demand = 0;
    uint64_t labor_fufillment = 0;
    for (auto& factory : city_industry.industries) {
        const auto& employ = GetUniverse().get<components::Employer>(factory);
        labor_demand += employ.population_needed;
        labor_fufillment += employ.population_fufilled;
    }
    double percentag = (double)labor_fufillment / (double)labor_demand * 100.;
    ImGui::TextFmt("Labor fufillment: {}/{} ({}%)", cqsp::util::NumberToHumanString(labor_fufillment),
                   cqsp::util::NumberToHumanString(labor_demand), percentag);

    IndustryTabGenericChild<components::Service>(
        "Service Sector", "Company",
        ImVec2(ImGui::GetContentRegionAvail().x * 0.5f - ImGui::GetStyle().ItemSpacing.y, height));

    ImGui::SameLine();

    IndustryTabGenericChild<components::Factory>("Manufacturing Sector", "Factories", ImVec2(-1, height));

    IndustryTabGenericChild<components::Mine>(
        "Mining Sector ", " Mines ",
        ImVec2(ImGui::GetContentRegionAvail().x * 0.5f - ImGui::GetStyle().ItemSpacing.y, height));

    ImGui::SameLine();
}

void SysProvinceInformation::SpacePortTab() {
    if (ImGui::BeginTabBar("SpacePortTabs", ImGuiTabBarFlags_None)) {
        if (ImGui::BeginTabItem("Launch")) {
            LaunchTab();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Docked Ships")) {
            DockedTab();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Orders")) {
            SpacePortOrdersTab();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Resources")) {
            SpacePortResourceTab();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
}

void SysProvinceInformation::InfrastructureTab() {
    auto& infras = GetUniverse().get<infrastructure::CityInfrastructure>(current_city);
    ImGui::TextFmt("Default transport cost per m3: {}", infras.default_purchase_cost);
    ImGui::TextFmt("Improvement per m3: {}", infras.improvement);
    // List all the stuff
    // Compile the highway to get the total cost reduction
    if (GetUniverse().any_of<infrastructure::Highway>(current_city)) {
        ImGui::TextFmt("Highway: {}", GetUniverse().get<infrastructure::Highway>(current_city).extent);
    }
}

void SysProvinceInformation::IndustryListWindow() {
    if (!city_factory_info) {
        return;
    }
    ImGui::Begin("Name##industry_list_window", &city_factory_info);
    // Loop through market industry
    if (ImGui::BeginTable("industry_list_table", 9, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("Production Type");
        ImGui::TableSetupColumn("Size");
        ImGui::TableSetupColumn("Utilization");
        ImGui::TableSetupColumn("Utilization Delta");
        ImGui::TableSetupColumn("Utilization Delta");
        ImGui::TableSetupColumn("Hired Workers");
        ImGui::TableSetupColumn("Wage");
        ImGui::TableSetupColumn("Revenue");
        ImGui::TableSetupColumn("Profit");
        ImGui::TableHeadersRow();
        auto& city_industry = GetUniverse().get<components::IndustrialZone>(current_city);
        for (entt::entity industry : city_industry.industries) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextFmt("{}", core::util::GetName(GetUniverse(), industry));
            if (ImGui::IsItemHovered()) {
                systems::gui::EntityTooltip(GetUniverse(), industry);
            }

            if (GetUniverse().all_of<components::IndustrySize>(industry)) {
                auto& industry_component = GetUniverse().get<components::IndustrySize>(industry);

                ImGui::TableSetColumnIndex(1);
                if (!industry_component.shortage) {
                    ImGui::TextFmt("{}", NumberToHumanString(static_cast<int64_t>(industry_component.size)));
                } else {
                    ImGui::TextFmtColored(ImVec4(0.75, 0, 0, 1), "{}",
                                          NumberToHumanString(static_cast<int64_t>(industry_component.size)));
                    if (ImGui::IsItemHovered()) {
                        ImGui::BeginTooltip();
                        ImGui::TextFmt("Resource Shortage!");
                        ImGui::EndTooltip();
                    }
                }
                ImGui::TableSetColumnIndex(2);
                ImGui::TextFmt("{}", NumberToHumanString(static_cast<int64_t>(industry_component.utilization)));
                ImGui::TableSetColumnIndex(3);
                double diff = industry_component.diff - 1;
                diff *= 100;
                const char* format_string = "{:.2f}%";
                if (diff > 0) {
                    ImGui::TextFmtColored(ImVec4(0, 0.75, 0, 1), fmt::runtime(format_string), diff);
                } else if (diff < 0) {
                    ImGui::TextFmtColored(ImVec4(0.75, 0, 0, 1), fmt::runtime(format_string), diff);
                } else {
                    ImGui::TextFmt(fmt::runtime(format_string), diff);
                }
                ImGui::TableSetColumnIndex(4);
                ImGui::TextFmt("{}", industry_component.diff_delta);
            }
            if (GetUniverse().all_of<components::Employer>(industry)) {
                ImGui::TableSetColumnIndex(5);
                auto& employer = GetUniverse().get<components::Employer>(industry);
                ImGui::TextFmt("{}", NumberToHumanString(static_cast<int64_t>(employer.population_fufilled)));
            }

            if (GetUniverse().all_of<components::IndustrySize>(industry)) {
                auto& industry_component = GetUniverse().get<components::IndustrySize>(industry);

                ImGui::TableSetColumnIndex(6);
                ImGui::TextFmt("{}", NumberToHumanString(static_cast<int64_t>(industry_component.wages)));
            }
            if (GetUniverse().all_of<components::CostBreakdown>(industry)) {
                auto& income_component = GetUniverse().get<components::CostBreakdown>(industry);

                ImGui::TableSetColumnIndex(7);
                ImGui::TextFmt("{}", NumberToHumanString(static_cast<int64_t>(income_component.revenue)));
                ImGui::TableSetColumnIndex(8);
                ImGui::TextFmt("{}", NumberToHumanString(static_cast<int64_t>(income_component.profit)));
            }
        }
        ImGui::EndTable();
    }
    ImGui::End();
}

template <typename T>
void SysProvinceInformation::IndustryTabGenericChild(const std::string& tabname, const std::string& industryname,
                                                     const ImVec2& size) {
    ImGui::BeginChild(tabname.c_str(), size, true, ImGuiWindowFlags_HorizontalScrollbar | window_flags);
    auto& city_industry = GetUniverse().get<components::IndustrialZone>(current_city);
    ImGui::TextFmt("{}", tabname);
    // List all the stuff it produces

    components::ResourceLedger input_resources;
    components::ResourceLedger output_resources;
    double GDP_calculation = 0;
    int count = 0;
    for (auto industry : city_industry.industries) {
        if (GetUniverse().all_of<components::Production, T>(industry)) {
            count++;
            const components::Production& generator = GetUniverse().get<components::Production>(industry);
            const components::Recipe& recipe = GetUniverse().get<components::Recipe>(generator.recipe);
            const components::IndustrySize& ratio = GetUniverse().get<components::IndustrySize>(industry);

            input_resources += (recipe.input * ratio.utilization) + (recipe.capitalcost * ratio.size);
            output_resources[recipe.output.entity] += recipe.output.amount * ratio.utilization;

            if (GetUniverse().all_of<components::Wallet>(industry)) {
                GDP_calculation += GetUniverse().get<components::Wallet>(industry).GetGDPChange();
            }
        }
    }
    ImGui::TextFmt("GDP: {}", NumberToHumanString(GDP_calculation));
    ImGui::TextFmt("{} Count: {}", industryname, count);

    ImGui::Text("Output");
    // Output table
    DrawLedgerTable(tabname + "output", GetUniverse(), output_resources);

    ImGui::Text("Input");
    DrawLedgerTable(tabname + "input", GetUniverse(), input_resources);
    ImGui::EndChild();
}

void SysProvinceInformation::LaunchTab() {
    // Set the things
    static float semi_major_axis = 8000;
    static float azimuth = 0;
    static float eccentricity = 0;
    static float arg_of_perapsis = 0;
    static float LAN = 0;
    auto& city_coord = GetUniverse().get<types::SurfaceCoordinate>(current_city);

    if (changed_city) {
        // Try to minimize this
        double latitude = city_coord.r_latitude();
        double temp_azimuth = components::types::GetLaunchInclination(city_coord.r_latitude(), 0);
        int idx = 0;
        while (idx < 10) {
            double derivative =
                -latitude * cos((temp_azimuth)) / sqrt(1 - latitude * latitude * sin(temp_azimuth) * sin(temp_azimuth));
            double inclination = components::types::GetLaunchInclination(city_coord.r_latitude(), temp_azimuth);
            temp_azimuth -= derivative / inclination;
            idx++;
        }
        azimuth = temp_azimuth;
    }
    ImGui::SliderFloat("Semi Major Axis", &semi_major_axis, 6000, 100000);
    ImGui::SliderFloat("Eccentricity", &eccentricity, 0, 0.9999);
    ImGui::SliderAngle("Launch Azimuth", &azimuth, 0, 360);
    ImGui::SliderAngle("Argument of perapsis", &arg_of_perapsis, 0, 360);
    ImGui::SliderAngle("Longitude of the ascending node", &LAN, 0, 360);
    if (ImGui::Button("Launch!")) {
        // Get reference body
        entt::entity reference_body = city_coord.planet;
        // Launch inclination will be the inclination of the thing
        double axial = GetUniverse().get<components::bodies::Body>(reference_body).axial;
        double inc = city_coord.r_latitude();
        inc += axial;

        types::Orbit orb(semi_major_axis, eccentricity,
                         components::types::GetLaunchInclination(city_coord.r_latitude(), (azimuth)), LAN,
                         arg_of_perapsis, 0, reference_body);
        entt::entity ship = core::actions::LaunchShip(GetUniverse(), orb);
        // Also compute the value
        GetUniverse().emplace<ctx::VisibleOrbit>(ship);

        core::systems::commands::LeaveSOI(GetUniverse(), ship, 1000);
        // Add maneuver like 1000 seconds in the future
        core::systems::commands::PushManeuver(GetUniverse(), ship,
                                              core::systems::commands::MakeManeuver(glm::dvec3(0, 0, 0), 1000));

        // Also self destruct after leaving soi
        entt::entity escape_action = GetUniverse().create();
        GetUniverse().emplace<core::components::Trigger>(escape_action, core::components::Trigger::OnExitSOI);
        GetUniverse().emplace<core::components::Command>(escape_action, core::components::Command::SelfDestruct);

        auto& command_queue = GetUniverse().get_or_emplace<components::CommandQueue>(ship);
        command_queue.commands.push_back(escape_action);
    }
    double periapsis = semi_major_axis * (1 - eccentricity);
    if (GetUniverse().get<components::bodies::Body>(city_coord.planet).radius > periapsis) {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 1.0f),
                           "Orbit's periapsis is below the planet radius (%f), so it will crash", periapsis);
    }
    ImGui::TextFmt("Launch Inclination: {}",
                   types::toDegree(types::GetLaunchInclination(city_coord.r_latitude(), (azimuth))));
}

void SysProvinceInformation::DockedTab() {
    if (!GetUniverse().any_of<components::DockedShips>(current_city)) {
        return;
    }
    auto& docked_ships = GetUniverse().get<components::DockedShips>(current_city);

    for (entt::entity docked : docked_ships.docked_ships) {
        ImGui::Selectable(core::util::GetName(GetUniverse(), docked).c_str());
        if (ImGui::IsItemHovered()) {
            systems::gui::EntityTooltip(GetUniverse(), docked);
        }
    }
}

void SysProvinceInformation::SpacePortOrdersTab() {
    // Just list it for a basic UI
    auto& space_port = GetUniverse().get<components::infrastructure::SpacePort>(current_city);
    for (auto& [target, queue] : space_port.deliveries) {
        for (auto& transport : queue) {
            ImGui::TextFmt("To {}: {} {}/{}", core::util::GetName(GetUniverse(), target),
                           core::util::GetName(GetUniverse(), transport.good), transport.fulfilled, transport.amount);
        }
    }
}

void SysProvinceInformation::SpacePortResourceTab() {
    auto& space_port = GetUniverse().get<components::infrastructure::SpacePort>(current_city);
    if (ImGui::BeginTable("space_port_resource_tables", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("Good");
        ImGui::TableSetupColumn("Output Resources");

        ImGui::TableHeadersRow();
        auto goodsview = GetUniverse().view<components::Price>();

        for (entt::entity good_entity : goodsview) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextFmt("{}", GetName(GetUniverse(), good_entity));
            ImGui::TableSetColumnIndex(1);
            ImGui::TextFmt("{}", util::NumberToHumanString(space_port.output_resources[good_entity]));
        }
        ImGui::EndTable();
    }
}
}  // namespace cqsp::client::systems
