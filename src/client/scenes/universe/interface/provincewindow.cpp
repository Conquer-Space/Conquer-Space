/* Conquer Space
 * Copyright (C) 2021-2023 Conquer Space
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
#include "client/scenes/universe/interface/marketwindow.h"
#include "client/scenes/universe/interface/sysstockpileui.h"
#include "client/scenes/universe/interface/systooltips.h"
#include "client/scenes/universe/views/starsystemview.h"
#include "common/components/economy.h"
#include "common/components/infrastructure.h"
#include "common/components/name.h"
#include "common/components/orbit.h"
#include "common/components/organizations.h"
#include "common/components/population.h"
#include "common/components/ships.h"
#include "common/components/surface.h"
#include "common/systems/actions/shiplaunchaction.h"
#include "common/util/nameutil.h"
#include "common/util/utilnumberdisplay.h"
#include "engine/cqspgui.h"

namespace components = cqsp::common::components;
namespace types = cqsp::common::components::types;
namespace ships = cqsp::common::components::ships;
namespace bodies = cqsp::common::components::bodies;
namespace infrastructure = cqsp::common::components::infrastructure;
using cqsp::common::util::LongToHumanString;
using cqsp::common::components::PopulationSegment;
using cqsp::common::components::Settlement;
using cqsp::common::components::Province;
using cqsp::common::components::IndustrialZone;
using cqsp::common::components::Production;
using cqsp::common::components::Factory;
using cqsp::common::components::Market;
using entt::entity;

namespace cqsp::client::systems {
void SysProvinceInformation::Init() {}

void SysProvinceInformation::DoUI(int delta_time) {
    entity country = GetUniverse().view<cqsp::client::ctx::SelectedProvince>().front();
    if (country != current_country) {
        current_country = country;
        view_mode = ViewMode::COUNTRY_VIEW;
        visible = true;
    }
    if (current_country == entt::null || !GetUniverse().any_of<Province>(current_country)) {
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
}

void SysProvinceInformation::DoUpdate(int delta_time) {}

void SysProvinceInformation::ProvinceView() {
    ImGui::TextFmt("{}", common::util::GetName(GetUniverse(), current_country));
    // List the cities
    auto& city_list = GetUniverse().get<Province>(current_country);
    int population = 0;
    for (entity entity : city_list.cities) {
        // Get city population
        auto& settlement = GetUniverse().get<Settlement>(entity);
        for (auto& seg_entity : settlement.population) {
            auto& segment = GetUniverse().get<PopulationSegment>(seg_entity);
            population += segment.population;
        }
    }
    ImGui::TextFmt("Part of {}", common::util::GetName(GetUniverse(), city_list.country));
    ImGui::TextFmt("Population: {}", LongToHumanString(population));
    ImGui::Separator();
    for (entity entity : city_list.cities) {
        if (CQSPGui::DefaultSelectable(fmt::format("{}", common::util::GetName(GetUniverse(), entity)).c_str())) {
            current_city = entity;
            view_mode = ViewMode::CITY_VIEW;
        }
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
        ImGui::TextFmt("Time: {} {}:00 ({})", GetUniverse().date.ToString(tz_def.time_diff), time, tz_name);
    }

    // Other industry information
    if (GetUniverse().all_of<IndustrialZone>(current_city)) {
        CityIndustryTabs();
    } else {
        ImGui::Text("City has no Industry");
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
        if (GetUniverse().any_of<infrastructure::SpacePort>(current_city)) {
            if (ImGui::BeginTabItem("Space Port")) {
                SpacePortTab();
                ImGui::EndTabItem();
            }
        }
        if (ImGui::BeginTabItem("Economy")) {
            // Show economy window

            Market& market = GetUniverse().get<Market>(current_city);
            // List the market's connected cities
            ImGui::TextFmt("Connected cities");
            for (entity entity : market.connected_markets) {
                ImGui::TextFmt("{}", common::util::GetName(GetUniverse(), entity));
            }
            ImGui::Separator();
            MarketInformationTable(GetUniverse(), current_city);
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
}

void SysProvinceInformation::DemographicsTab() {
    auto& settlement = GetUniverse().get<Settlement>(current_city);
    for (auto& seg_entity : settlement.population) {
        ImGui::TextFmt("Population: {}",
                       LongToHumanString(GetUniverse().get<PopulationSegment>(seg_entity).population));
        gui::EntityTooltip(GetUniverse(), seg_entity);
        if (GetUniverse().all_of<components::Hunger>(seg_entity)) {
            ImGui::TextFmt("Hungry");
        }
        ImGui::TextFmt("Labor Force: {}",
                       LongToHumanString(GetUniverse().get<PopulationSegment>(seg_entity).labor_force));
        // Then other labor information

        // Get spending for population
        if (GetUniverse().all_of<components::Wallet>(seg_entity)) {
            auto& wallet = GetUniverse().get<components::Wallet>(seg_entity);
            ImGui::TextFmt("Spending: {}", LongToHumanString(wallet.GetGDPChange()));
        }
        // Market
        if (GetUniverse().all_of<Market>(current_city)) {
            if (ImGui::IsItemHovered()) {
                CQSPGui::SimpleTextTooltip("Click for detailed market information");
            }
        }
    }
}

void SysProvinceInformation::IndustryTab() {
    IndustryListWindow();
    auto& city_industry = GetUniverse().get<IndustrialZone>(current_city);
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
    ImGui::TextFmt("Labor fufillment: {}/{} ({}%%)", LongToHumanString(labor_fufillment),
                   LongToHumanString(labor_demand), percentag);

    IndustryTabGenericChild<components::Service>(
        "Service Sector", "Company",
        ImVec2(ImGui::GetContentRegionAvail().x * 0.5f - ImGui::GetStyle().ItemSpacing.y, height));

    ImGui::SameLine();

    IndustryTabGenericChild<Factory>("Manufacturing Sector", "Factories", ImVec2(-1, height));

    IndustryTabGenericChild<components::Mine>(
        "Mining Sector ", " Mines ",
        ImVec2(ImGui::GetContentRegionAvail().x * 0.5f - ImGui::GetStyle().ItemSpacing.y, height));

    ImGui::SameLine();
}

void SysProvinceInformation::SpacePortTab() {
    // Set the things
    static float semi_major_axis = 8000;
    static float azimuth = 0;
    static float eccentricity = 0;
    static float arg_of_perapsis = 0;
    static float LAN = 0;
    auto& city_coord = GetUniverse().get<components::types::SurfaceCoordinate>(current_city);

    ImGui::SliderFloat("Semi Major Axis", &semi_major_axis, 6000, 100000);
    ImGui::SliderFloat("Eccentricity", &eccentricity, 0, 0.9999);
    ImGui::SliderAngle("Launch Azimuth", &azimuth, 0, 360);
    ImGui::SliderAngle("Argument of perapsis", &arg_of_perapsis, 0, 360);
    ImGui::SliderAngle("Longitude of the ascending node", &LAN, 0, 360);
    if (ImGui::Button("Launch!")) {
        // Get reference body
        entity reference_body = city_coord.planet;
        // Launch inclination will be the inclination of the thing
        double axial = GetUniverse().get<components::bodies::Body>(reference_body).axial;
        double inc = city_coord.r_latitude();
        inc += axial;

        components::types::Orbit orb;
        orb.reference_body = reference_body;
        orb.inclination = components::types::GetLaunchInclination(city_coord.r_latitude(), azimuth);
        orb.semi_major_axis = semi_major_axis;
        orb.eccentricity = eccentricity;
        orb.w = arg_of_perapsis;
        orb.LAN = LAN;
        orb.epoch = GetUniverse().date.ToSecond();
        cqsp::common::systems::actions::LaunchShip(GetUniverse(), orb);
    }
    double periapsis = semi_major_axis * (1 - eccentricity);
    if (GetUniverse().get<bodies::Body>(city_coord.planet).radius > periapsis) {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 1.0f),
                           "Orbit's periapsis is below the planet radius (%f), so it will crash", periapsis);
    }
    ImGui::TextFmt("Launch Inclination: {}", components::types::toDegree(components::types::GetLaunchInclination(
                                                 city_coord.r_latitude(), azimuth)));
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
    ImGui::Begin("Name", &city_factory_info);
    // Loop through market industry
    auto& city_industry = GetUniverse().get<IndustrialZone>(current_city);

    for (entity industry : city_industry.industries) {
        ImGui::TextFmt("{}", common::util::GetName(GetUniverse(), industry));
        if (ImGui::IsItemHovered()) {
            systems::gui::EntityTooltip(GetUniverse(), industry);
        }
    }
    ImGui::End();
}

template <typename T>
void SysProvinceInformation::IndustryTabGenericChild(const std::string& tabname, const std::string& industryname,
                                                     const ImVec2& size) {
    ImGui::BeginChild(tabname.c_str(), size, true, ImGuiWindowFlags_HorizontalScrollbar | window_flags);
    auto& city_industry = GetUniverse().get<IndustrialZone>(current_city);
    ImGui::TextFmt("{}", tabname);
    // List all the stuff it produces

    components::ResourceLedger input_resources;
    components::ResourceLedger output_resources;
    double GDP_calculation = 0;
    int count = 0;
    for (auto industry : city_industry.industries) {
        if (GetUniverse().all_of<Production, T>(industry)) {
            count++;
            const Production& generator = GetUniverse().get<Production>(industry);
            const components::Recipe& recipe = GetUniverse().get<components::Recipe>(generator.recipe);
            const components::IndustrySize& ratio = GetUniverse().get<components::IndustrySize>(industry);

            input_resources += (recipe.input * ratio.size);
            output_resources[recipe.output.entity] += recipe.output.amount;

            if (GetUniverse().all_of<components::Wallet>(industry)) {
                GDP_calculation += GetUniverse().get<components::Wallet>(industry).GetGDPChange();
            }
        }
    }
    ImGui::TextFmt("GDP: {}", LongToHumanString(GDP_calculation));
    ImGui::TextFmt("{} Count: {}", industryname, count);

    ImGui::SameLine();
    if (CQSPGui::SmallDefaultButton("List")) {
        /* factory_list_panel = true;
            industrylist.resize(0);
            auto& city_industry =
                GetUniverse()
                    .get<cqspc::Industry>(selected_city_entity)
                    .industries;
            for (entt::entity production : city_industry) {
                if (GetUniverse().all_of<T>(production))
                    industrylist.push_back(production);
            }*/
    }

    ImGui::Text("Output");
    // Output table
    DrawLedgerTable(tabname + "output", GetUniverse(), output_resources);

    ImGui::Text("Input");
    DrawLedgerTable(tabname + "input", GetUniverse(), input_resources);
    ImGui::EndChild();
}
}  // namespace cqsp::client::systems
