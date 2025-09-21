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
#include "client/scenes/universe/interface/marketwindow.h"
#include "client/scenes/universe/interface/sysstockpileui.h"
#include "client/scenes/universe/interface/systooltips.h"
#include "client/scenes/universe/views/starsystemview.h"
#include "common/actions/shiplaunchaction.h"
#include "common/components/economy.h"
#include "common/components/infrastructure.h"
#include "common/components/name.h"
#include "common/components/orbit.h"
#include "common/components/organizations.h"
#include "common/components/population.h"
#include "common/components/resource.h"
#include "common/components/ships.h"
#include "common/components/spaceport.h"
#include "common/components/surface.h"
#include "common/util/nameutil.h"
#include "common/util/utilnumberdisplay.h"
#include "engine/cqspgui.h"

namespace cqsp::client::systems {

namespace components = cqsp::common::components;
namespace infrastructure = components::infrastructure;
namespace types = components::types;
namespace ships = components::ships;
namespace bodies = components::bodies;
using common::util::GetName;
using components::PopulationSegment;
using components::Settlement;
using components::Wallet;
using util::LongToHumanString;

void SysProvinceInformation::Init() {}

void SysProvinceInformation::DoUI(int delta_time) {
    entt::entity country = GetUniverse().view<ctx::SelectedProvince>().front();
    if (country != current_province) {
        current_province = country;
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
            CityView();
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
    ImGui::TextFmt("Population: {}", LongToHumanString(population));
    ImGui::Separator();
    if (ImGui::BeginTabBar("ProvinceInformationTabs", ImGuiTabBarFlags_None)) {
        if (ImGui::BeginTabItem("Cities")) {
            for (entt::entity entity : city_list.cities) {
                if (CQSPGui::DefaultSelectable(fmt::format("{}", GetName(GetUniverse(), entity)).c_str())) {
                    current_city = entity;
                    view_mode = ViewMode::CITY_VIEW;
                }
            }
            ImGui::EndTabItem();
        }
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
            ImGui::TextFmt("Population: {}", LongToHumanString(pop_segement.population));
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
        ImGui::TextFmt("GDP Contribution: {}", LongToHumanString(wallet.GetGDPChange()));
        ImGui::TextFmt("Balance: {}", LongToHumanString(wallet.GetBalance()));
        ImGui::TextFmt("Balance change: {}", LongToHumanString(wallet.GetChange()));
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
        if (ImGui::BeginTabItem("Space Port")) {
            SpacePortTab();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Economy")) {
            // Show economy window

            components::Market& market = GetUniverse().get<components::Market>(current_city);
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
        auto& segment_comp = GetUniverse().get<PopulationSegment>(seg_entity);
        ImGui::TextFmt("Population: {}", LongToHumanString(segment_comp.population));

        gui::EntityTooltip(GetUniverse(), seg_entity);
        if (GetUniverse().all_of<components::Hunger>(seg_entity)) {
            ImGui::TextFmt("Hungry");
        }
        ImGui::TextFmt("Labor Force: {}", LongToHumanString(segment_comp.labor_force));
        ImGui::TextFmt("Standard of Living: {}", segment_comp.standard_of_living);

        // Get spending for population
        DisplayWallet(seg_entity);
        if (GetUniverse().all_of<Wallet>(seg_entity)) {
            Wallet& wallet = GetUniverse().get<Wallet>(seg_entity);
            ImGui::TextFmt("GDP per capita: {}", wallet.GetGDPChange() / segment_comp.population);
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
            ImGui::TextFmt("{}", common::util::GetEntityType(GetUniverse(), at));
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
    ImGui::TextFmt("Labor fufillment: {}/{} ({}%)", cqsp::util::LongToHumanString(labor_fufillment),
                   cqsp::util::LongToHumanString(labor_demand), percentag);

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
    if (ImGui::BeginTable("industry_list_table", 7, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("Production Type");
        ImGui::TableSetupColumn("Size");
        ImGui::TableSetupColumn("Utilization");
        ImGui::TableSetupColumn("Utilization Delta");
        ImGui::TableSetupColumn("Hired Workers");
        ImGui::TableSetupColumn("Revenue");
        ImGui::TableSetupColumn("Profit");
        ImGui::TableHeadersRow();
        auto& city_industry = GetUniverse().get<components::IndustrialZone>(current_city);
        for (entt::entity industry : city_industry.industries) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextFmt("{}", common::util::GetName(GetUniverse(), industry));
            if (ImGui::IsItemHovered()) {
                systems::gui::EntityTooltip(GetUniverse(), industry);
            }

            if (GetUniverse().all_of<components::IndustrySize>(industry)) {
                auto& industry_component = GetUniverse().get<components::IndustrySize>(industry);

                ImGui::TableSetColumnIndex(1);
                ImGui::TextFmt("{}", LongToHumanString(static_cast<int64_t>(industry_component.size)));
                ImGui::TableSetColumnIndex(2);
                ImGui::TextFmt("{}", LongToHumanString(static_cast<int64_t>(industry_component.utilization)));
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
            }
            if (GetUniverse().all_of<components::Employer>(industry)) {
                ImGui::TableSetColumnIndex(4);
                auto& employer = GetUniverse().get<components::Employer>(industry);
                ImGui::TextFmt("{}", LongToHumanString(static_cast<int64_t>(employer.population_fufilled)));
            }
            if (GetUniverse().all_of<components::CostBreakdown>(industry)) {
                auto& income_component = GetUniverse().get<components::CostBreakdown>(industry);

                ImGui::TableSetColumnIndex(5);
                ImGui::TextFmt("{}", LongToHumanString(static_cast<int64_t>(income_component.revenue)));
                ImGui::TableSetColumnIndex(6);
                ImGui::TextFmt("{}", LongToHumanString(static_cast<int64_t>(income_component.profit)));
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
    ImGui::TextFmt("GDP: {}", LongToHumanString(GDP_calculation));
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

        types::Orbit orb;
        orb.reference_body = reference_body;
        orb.inclination = components::types::GetLaunchInclination(city_coord.r_latitude(), azimuth);
        orb.semi_major_axis = semi_major_axis;
        orb.eccentricity = eccentricity;
        orb.w = arg_of_perapsis;
        orb.LAN = LAN;
        orb.epoch = GetUniverse().date.ToSecond();
        common::actions::LaunchShip(GetUniverse(), orb);
    }
    double periapsis = semi_major_axis * (1 - eccentricity);
    if (GetUniverse().get<components::bodies::Body>(city_coord.planet).radius > periapsis) {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 1.0f),
                           "Orbit's periapsis is below the planet radius (%f), so it will crash", periapsis);
    }
    ImGui::TextFmt("Launch Inclination: {}",
                   types::toDegree(types::GetLaunchInclination(city_coord.r_latitude(), azimuth)));
}

void SysProvinceInformation::DockedTab() {
    if (!GetUniverse().any_of<components::DockedShips>(current_city)) {
        return;
    }
    auto& docked_ships = GetUniverse().get<components::DockedShips>(current_city);

    for (entt::entity docked : docked_ships.docked_ships) {
        ImGui::Selectable(common::util::GetName(GetUniverse(), docked).c_str());
        if (ImGui::IsItemHovered()) {
            systems::gui::EntityTooltip(GetUniverse(), docked);
        }
    }
}
}  // namespace cqsp::client::systems
