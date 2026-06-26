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
#include "client/scenes/universe/interface/economywindow.h"

#include "client/components/clientctx.h"
#include "client/scenes/universe/interface/ledgertable.h"
#include "core/actions/economy/subsidyhelper.h"
#include "core/components/organizations.h"
#include "core/components/resource.h"
#include "core/components/surface.h"
#include "core/util/nameutil.h"

namespace cqsp::client::systems {
namespace components = cqsp::core::components;
void EconomyWindow::Init() {}
void EconomyWindow::DoUI(int delta_time) {
    bool selected = GetUniverse().ctx().at<ctx::SelectedMenu>() == ctx::SelectedMenu::EconomyMenu;
    if (!selected) {
        show_add_subsidy_window = false;
        return;
    }

    SubsidyWindow();

    ImGui::Begin("Economy", &selected);
    // ImGui::TextFmt("Place a dollar amount to invest in specific industry");
    // ImGui::TextFmt("Inject money into a specific factory/buy money");

    // List out the recipes and then subsidize them...
    if (ImGui::BeginTabBar("###economy_tabbar")) {
        if (ImGui::BeginTabItem("Subsidize Industry")) {
            SubsidyTab();
            ImGui::EndTabItem();
        } else {
            show_add_subsidy_window = false;
        }
        if (ImGui::BeginTabItem("Labor")) {
            LaborTab();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::End();
    if (!selected) {
        GetUniverse().ctx().at<ctx::SelectedMenu>() = ctx::SelectedMenu::NoMenu;
    }
}

void EconomyWindow::DoUpdate(int delta_time) {}

void EconomyWindow::SubsidyTab() {
    ImGui::TextFmt("Subsidize output/input for something");
    // Get player subsidies
    entt::entity player = GetUniverse().GetPlayer();

    ImGui::Text("Subsidies");
    ImGui::SameLine();
    if (ImGui::Button("Add Subsidy")) {
        show_add_subsidy_window = !show_add_subsidy_window;
    }
    // List out all the subsidies
    // then set if you're able to cancel them as well
    // List recipes
    // List subsidies
    auto& subsidies = GetUniverse().get<core::components::Subsidies>(player);
    // Display subsidies
    if (ImGui::BeginTable("###subsidy_table", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("Good");
        ImGui::TableSetupColumn("Subsidy Amount");
        ImGui::TableSetupColumn("##del", ImGuiTableColumnFlags_WidthFixed, 24.f);
        ImGui::TableHeadersRow();
        // Then loop through subsidies
        entt::entity to_delete = entt::null;
        for (auto& [entity, subsidy_amount] : subsidies.global_subsidy) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextFmt("{}", core::util::GetName(GetUniverse(), entity));
            ImGui::TableSetColumnIndex(1);
            ImGui::TextFmt("{}", subsidy_amount * 100);
            ImGui::TableSetColumnIndex(2);
            if (ImGui::Button("x")) {
                // Remove that
                to_delete = entity;
            }
        }
        if (to_delete != entt::null) {
            // Then delete from the subsidy list
            subsidies.global_subsidy.erase(subsidies.global_subsidy.find(to_delete));
        }
        ImGui::EndTable();
    }
}

void EconomyWindow::SubsidyWindow() {
    if (!show_add_subsidy_window) {
        return;
    }

    ImGui::SetNextWindowSize(ImVec2(800, 700), ImGuiCond_FirstUseEver);
    ImGui::Begin("Subsidy", &show_add_subsidy_window);
    // Then
    ImGui::Text("Add Subsidy");
    ImGui::BeginChild("subsidy_viewer_left", ImVec2(300, -1));
    for (auto&& [entity, recipe] : GetUniverse().view<core::components::Recipe>().each()) {
        // Now then we can set our investment and stuff
        bool selected = (selected_subsidy == entity);
        if (ImGui::Selectable(fmt::format("{}", core::util::GetName(GetUniverse(), entity)).c_str(), selected)) {
            selected_subsidy = entity;
        }
    }
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("subsidy_viewer_right", ImVec2(-1, -1));
    // Add subsidy
    if (full_subsidy) {
        ImGui::BeginDisabled();
    }
    ImGui::SliderFloat("Subsidy amount", &subsidy_amount, 0, 1);
    if (full_subsidy) {
        ImGui::EndDisabled();
    }
    ImGui::Checkbox("Full subsidy", &full_subsidy);
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Subsidize all losses");
    }
    if (ImGui::Button("Apply Subsidy")) {
        float apply_subsidy_amount = subsidy_amount;
        if (full_subsidy) {
            apply_subsidy_amount = 1.;
        }
        if (selected_subsidy != entt::null) {
            core::actions::ApplySubsidy(GetUniverse(), GetUniverse().GetPlayer(), selected_subsidy,
                                        apply_subsidy_amount);
        }
        show_add_subsidy_window = false;
    }
    ImGui::EndChild();
    ImGui::End();
}

void EconomyWindow::LaborTab() {
    // Now then display various labor stats
    // Display what we need, and what we might need in the future
    // Sum together all the jobs and values that we have...
    // We should just display the job demands and what we are lacking...
    ImGui::TextFmt("Labor market");
    entt::entity player = GetUniverse().GetPlayer();
    auto& city_list = GetUniverse().get<components::CountryCityList>(player);
    // Get all the summaries of what we want
    std::map<entt::entity, double> demand_list;
    std::map<entt::entity, double> supply_list;
    for (entt::entity province : city_list.province_list) {
        auto& province_comp = GetUniverse().get<components::Province>(province);
        // Get market
        auto& market = GetUniverse().get<components::Market>(province);
        for (entt::entity labor : GetUniverse().view<components::LaborGood>()) {
            // If the demand + supply is too high then we should handle that as well
            demand_list[labor] += market.demand[GetUniverse().good_map[labor]];
            supply_list[labor] += market.supply[GetUniverse().good_map[labor]];
        }
    }

    EntityLedgerTable(GetUniverse(), demand_list, "demand");
    EntityLedgerTable(GetUniverse(), supply_list, "supply");
}
}  // namespace cqsp::client::systems
