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
#include "client/systems/countrywindow.h"

#include <string>
#include <limits>

#include "engine/cqspgui.h"

#include "common/components/organizations.h"
#include "common/components/population.h"
#include "common/components/surface.h"
#include "common/util/utilnumberdisplay.h"
#include "common/components/name.h"
#include "common/components/economy.h"
#include "common/components/infrastructure.h"
#include "common/components/ships.h"
#include "common/systems/actions/shiplaunchaction.h"

#include "client/components/clientctx.h"
#include "client/systems/gui/systooltips.h"
#include "client/systems/views/starsystemview.h"
#include "client/systems/gui/sysstockpileui.h"

namespace cqsp::client::systems {
namespace cqspc = cqsp::common::components;
void SysCountryInformation::Init() {}

void SysCountryInformation::DoUI(int delta_time) {
    // Check the market
    if (market_information_panel) {
        ImGui::Begin("Market", &market_information_panel);
        MarketInformationTooltipContent(current_city);
        ImGui::End();
    }
    entt::entity country =
        GetUniverse().view<cqsp::client::ctx::SelectedProvince>().front();
    if (country != current_country) {
        current_country = country;
        view_mode = ViewMode::COUNTRY_VIEW;
        visible = true;
    }
    if (current_country == entt::null ||
        !GetUniverse().any_of<common::components::Province>(
            current_country)) {
        return;
    }
    if (!visible) {
        return;
    }
    // Get selected country
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x * 0.4f,
                                    ImGui::GetIO().DisplaySize.y * 0.8f),
                             ImGuiCond_Appearing);
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.79f,
                                   ImGui::GetIO().DisplaySize.y * 0.55f),
                            ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::Begin("Country Information", &visible);
    // Then do all countries and compile gdp data
    switch (view_mode) {
        case ViewMode::COUNTRY_VIEW:
            CountryView();
            break;
        case ViewMode::CITY_VIEW:
            CityView();
            break;
    }
    ImGui::End();
}

void SysCountryInformation::DoUpdate(int delta_time) {}

void SysCountryInformation::CountryView() {
    ImGui::TextFmt("{}", gui::GetName(GetUniverse(), current_country));
    // List the cities
    auto& city_list =
        GetUniverse().get<common::components::Province>(current_country);
    int population = 0;
    for (entt::entity entity : city_list.cities) {
        using cqsp::common::components::PopulationSegment;
        using cqsp::common::components::Settlement;
        // Get city population
        auto& settlement = GetUniverse().get<Settlement>(entity);
        for (auto& seg_entity : settlement.population) {
            auto& segment = GetUniverse().get<PopulationSegment>(seg_entity);
            population += segment.population;
        }
    }
    ImGui::TextFmt("Population: {}", util::LongToHumanString(population));
    ImGui::Separator();
    for (entt::entity entity : city_list.cities) {
        if (CQSPGui::DefaultSelectable(
                fmt::format("{}", gui::GetName(GetUniverse(), entity))
                    .c_str())) {
            current_city = entity;
            view_mode = ViewMode::CITY_VIEW;
        }
    }
}

void SysCountryInformation::CityView() {
    // Get city information
    if (CQSPGui::ArrowButton("cityinformationpanelback", ImGuiDir_Left)) {
        view_mode = ViewMode::COUNTRY_VIEW;
    }
    ImGui::SameLine();
    ImGui::TextFmt("{}", GetUniverse().get<cqspc::Name>(current_city).name);
    ImGui::SameLine();
    if (ImGui::Button("Focus on city")) {
        // Focus city
        GetApp().GetUniverse().emplace_or_replace<FocusedCity>(current_city);
    }
    ImGui::Separator();
    if (GetUniverse().all_of<cqspc::Settlement>(current_city)) {
        int size = GetUniverse()
                       .get<cqspc::Settlement>(current_city)
                       .population.size();
        for (auto seg_entity :
             GetUniverse().get<cqspc::Settlement>(current_city).population) {
            auto& pop_segement =
                GetUniverse().get<cqspc::PopulationSegment>(seg_entity);
            ImGui::TextFmt("Population: {}", cqsp::util::LongToHumanString(
                                                 pop_segement.population));
        }
    } else {
        ImGui::TextFmt("No population");
    }

    // Now do time zone
    if (GetUniverse().all_of<cqspc::CityTimeZone>(current_city)) {
        // Set time zone
        auto& tz = GetUniverse().get<cqspc::CityTimeZone>(current_city);
        auto& tz_def = GetUniverse().get<cqspc::TimeZone>(tz.time_zone);
        int time = (int)(GetUniverse().date.GetDate() + tz_def.time_diff) % 24;
        if (time < 0) {
            time = time + 24;
        }

        const std::string& tz_name =
            GetUniverse().get<cqspc::Identifier>(tz.time_zone).identifier;
        ImGui::TextFmt("Time: {} {}:00 ({})",
                       GetUniverse().date.ToString(tz_def.time_diff), time,
                       tz_name);
    }

    // Other industry information
    if (GetUniverse().all_of<cqspc::IndustrialZone>(current_city)) {
        CityIndustryTabs();
    } else {
        ImGui::Text("City has no Industry");
    }
}

void SysCountryInformation::CityIndustryTabs() {
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
        if (GetUniverse().any_of<cqspc::infrastructure::SpacePort>(
                current_city)) {
            if (ImGui::BeginTabItem("Space Port")) {
                SpacePortTab();
                ImGui::EndTabItem();
            }
        }
        ImGui::EndTabBar();
    }
}

void SysCountryInformation::DemographicsTab() {
    using cqsp::common::components::PopulationSegment;
    using cqsp::common::components::Settlement;

    auto& settlement = GetUniverse().get<Settlement>(current_city);
    for (auto& seg_entity : settlement.population) {
        ImGui::TextFmt(
            "Population: {}",
            cqsp::util::LongToHumanString(
                GetUniverse().get<PopulationSegment>(seg_entity).population));
        gui::EntityTooltip(GetUniverse(), seg_entity);
        if (GetUniverse().all_of<cqspc::Hunger>(seg_entity)) {
            ImGui::TextFmt("Hungry");
        }
        if (GetUniverse().any_of<cqspc::Employee>(
                seg_entity)) {
            auto& employee = GetUniverse().get<cqspc::Employee>(seg_entity);
            ImGui::TextFmt(
                "Working Population: {}/{}",
                cqsp::util::LongToHumanString(employee.employed_population),
                cqsp::util::LongToHumanString(employee.working_population));
            if (employee.working_population > 0) {
                ImGui::ProgressBar(
                    static_cast<float>(employee.employed_population) /
                    static_cast<float>(employee.working_population));
            }
        }
        // Get spending for population
        if (GetUniverse().all_of<cqspc::Wallet>(seg_entity)) {
            auto& wallet = GetUniverse().get<cqspc::Wallet>(seg_entity);
            ImGui::TextFmt("Spending: {}", cqsp::util::LongToHumanString(
                                               wallet.GetGDPChange()));
        }
        // Market
        if (GetUniverse().all_of<cqspc::Market>(current_city)) {
            if (ImGui::Button("Market Data")) {
                market_information_panel = true;
                // Set market
            }
            if (ImGui::IsItemHovered()) {
                CQSPGui::SimpleTextTooltip(
                    "Click for detailed market information");
            }
        }
    }
}

void SysCountryInformation::IndustryTab() {
    IndustryListWindow();
    auto& city_industry =
        GetUniverse().get<cqspc::IndustrialZone>(current_city);
    int height = 300;
    ImGui::TextFmt("Factories: {}", city_industry.industries.size());
    if (ImGui::SmallButton("Economy info")) {
        // Put all the economy window information
        city_factory_info = true;
    }
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        for (auto& at : city_industry.industries) {
            ImGui::TextFmt("{}", gui::GetEntityType(GetUniverse(), at));
        }
        ImGui::EndTooltip();
    }

    IndustryTabGenericChild<cqspc::Service>("Service Sector", "Company",
        ImVec2(ImGui::GetContentRegionAvail().x * 0.5f - ImGui::GetStyle().ItemSpacing.y,
        height));

    ImGui::SameLine();

    IndustryTabGenericChild<cqspc::Factory>("Manufacturing Sector", "Factories",
        ImVec2(-1, height));

    IndustryTabGenericChild<cqspc::Mine>("Mining Sector ", " Mines ",
        ImVec2(ImGui::GetContentRegionAvail().x * 0.5f -  ImGui::GetStyle().ItemSpacing.y,
        height));

    ImGui::SameLine();
}

void SysCountryInformation::SpacePortTab() {
    namespace cqspt = cqsp::common::components::types;
    namespace cqsps = cqsp::common::components::ships;
    namespace cqspb = cqsp::common::components::bodies;

    // Set the things
    static float semi_major_axis = 8000;
    static float inclination = 0;
    static float eccentricity = 0;
    static float arg_of_perapsis = 0;
    static float LAN = 0;
    ImGui::SliderFloat("Semi Major Axis", &semi_major_axis, 6000, 100000000);
    ImGui::SliderFloat("Eccentricity", &eccentricity, 0, 0.9999);
    ImGui::SliderAngle("Inclination", &inclination, 0, 180);
    ImGui::SliderAngle("Argument of perapsis", &arg_of_perapsis, 0, 360);
    ImGui::SliderAngle("Longitude of the ascending node", &LAN, 0, 360);
    if (ImGui::Button("Launch!")) {
        // Get reference body
        auto& city_coord =
            GetUniverse().get<cqspc::types::SurfaceCoordinate>(current_city);
        entt::entity reference_body = city_coord.planet;
        // Launch inclination will be the inclination of the thing
        double axial =
            GetUniverse().get<cqspc::bodies::Body>(reference_body).axial;
        double inc = city_coord.r_latitude();
        inc += axial;

        cqspc::types::Orbit orb;
        orb.reference_body = reference_body;
        orb.inclination = inclination;
        orb.semi_major_axis = semi_major_axis;
        orb.eccentricity = eccentricity;
        orb.w = arg_of_perapsis;
        orb.LAN = LAN;
        cqsp::common::systems::actions::LaunchShip(GetUniverse(), orb);
    }
}

void SysCountryInformation::InfrastructureTab() {
    namespace infrastructure = common::components::infrastructure;
    auto& infras =
        GetUniverse().get<infrastructure::CityInfrastructure>(
        current_city);
    ImGui::TextFmt("Default transport cost per m3: {}", infras.default_purchase_cost);
    ImGui::TextFmt("Improvement per m3: {}", infras.improvement);
    // List all the stuff
    // Compile the highway to get the total cost reduction
    if (GetUniverse().any_of<infrastructure::Highway>(current_city)) {
        ImGui::TextFmt("Highway: {}", GetUniverse().get<infrastructure::Highway>(
                                      current_city).extent);
    }
}

void SysCountryInformation::IndustryListWindow() {
    if (!city_factory_info) {
        return;
    }
    ImGui::Begin("Name", &city_factory_info);
    // Loop through market industry
    auto& city_industry =
        GetUniverse().get<cqspc::IndustrialZone>(current_city);

    for (entt::entity industry : city_industry.industries) {
        ImGui::TextFmt("{}",
                       cqsp::client::systems::gui::GetName(GetUniverse(), industry));
        if (ImGui::IsItemHovered()) {
            systems::gui::EntityTooltip(GetUniverse(), industry);
        }
    }
    ImGui::End();
}

template <typename T>
void SysCountryInformation::IndustryTabGenericChild(const std::string& tabname,
                                                    const std::string& industryname,
                                                    const ImVec2& size) {
        ImGui::BeginChild(
            tabname.c_str(), size, true,
            ImGuiWindowFlags_HorizontalScrollbar | window_flags);
        auto& city_industry =
            GetUniverse().get<cqspc::IndustrialZone>(current_city);
        ImGui::TextFmt(tabname);
        // List all the stuff it produces

        cqspc::ResourceLedger input_resources;
        cqspc::ResourceLedger output_resources;
        double GDP_calculation = 0;
        int count = 0;
        for (auto industry : city_industry.industries) {
            if (GetUniverse().all_of<cqspc::Production, T>(industry)) {
                count++;
                const cqspc::Production& generator =
                    GetUniverse().get<cqspc::Production>(industry);
                const cqspc::Recipe& recipe =
                    GetUniverse().get<cqspc::Recipe>(generator.recipe);
                const cqspc::IndustrySize& ratio =
                    GetUniverse().get<cqspc::IndustrySize>(industry);

                input_resources += (recipe.input * ratio.size);
                output_resources[recipe.output.entity] += recipe.output.amount;

                if (GetUniverse().all_of<cqspc::Wallet>(industry)) {
                    GDP_calculation += GetUniverse()
                                            .get<cqspc::Wallet>(industry)
                                            .GetGDPChange();
                }
            }
        }
        ImGui::TextFmt("GDP: {}",
                        cqsp::util::LongToHumanString(GDP_calculation));
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
        DrawLedgerTable(tabname + "output", GetUniverse(),
                        output_resources);

        ImGui::Text("Input");
        DrawLedgerTable(tabname + "input", GetUniverse(), input_resources);
        ImGui::EndChild();
}

void SysCountryInformation::MarketInformationTooltipContent(const entt::entity marketentity) {
}
}  // namespace cqsp::client::systems
