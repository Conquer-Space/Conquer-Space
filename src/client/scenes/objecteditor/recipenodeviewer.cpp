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
#include "client/scenes/objecteditor/recipenodeviewer.h"

#include <imgui_node_editor.h>

#include "client/scenes/universe/interface/ledgertable.h"
#include "client/scenes/universe/interface/systooltips.h"
#include "core/components/market.h"
#include "core/components/name.h"
#include "core/components/resource.h"
#include "core/util/nameutil.h"
#include "core/util/utilnumberdisplay.h"

namespace cqsp::client::systems {
namespace components = core::components;
namespace ed = ax::NodeEditor;

void RecipeNodeViewer::Init() {
    SPDLOG_INFO("logging");
    auto recipes = GetUniverse().view<components::Recipe>();
    for (auto&& [entity, recipe_comp] : recipes.each()) {
        // Then let's make a new node and we should also position them later
        // Let's make a graph
        // Then find the goods that are capital goods
        // They should go to the leftmost
        // Then everything goes to the right after that
        // So all the recipes that
        if (GetUniverse().any_of<components::RawGood>(recipe_comp.output.entity)) {
            // Now then we set it's level to 0 ig
            // If it contains then we should
            recipe_tier[entity] = 0;
        } else if (GetUniverse().any_of<components::CapitalGood>(recipe_comp.output.entity)) {
            recipe_tier[entity] = 1;
        } else {
            recipe_tier[entity] = 2;
        }
    }
}

void RecipeNodeViewer::DoUI(int delta_time) {
    ed::PushStyleVar(ed::StyleVar::StyleVar_LinkStrength, 1000);
    ImGui::SetNextWindowSize(ImVec2(800, 700), ImGuiCond_FirstUseEver);
    ImGui::Begin("Recipe Node Viewer");
    // List out all the stuff
    auto recipes = GetUniverse().view<components::Recipe>();
    ed::Begin("Recipe Node viewer");
    // This is for the output -> input map?
    std::map<components::GoodEntity, std::vector<int>> input_map;
    std::map<components::GoodEntity, std::vector<int>> output_map;
    std::map<int, int> level_map;
    int unique_id = 0;
    for (auto&& [entity, recipe_comp] : recipes.each()) {
        // Then let's make a new node and we should also position them later
        int node_id = unique_id++;
        ed::BeginNode(node_id);
        ImGui::TextFmt("{}", GetUniverse().get<components::Name>(entity).name);
        // Get the other stuff
        ImGui::TextFmt("Workers: {}", recipe_comp.workers);
        for (auto& [input, amount] : recipe_comp.input) {
            input_map[input].push_back(unique_id);
            ed::BeginPin(unique_id++, ed::PinKind::Input);
            ax::Drawing::Icon(ImVec2(16, 16), ax::Drawing::IconType::Diamond, true, ImColor(0, 0, 255, 255),
                              ImColor(32, 32, 32, 255));
            ImGui::SameLine();
            std::string fmt_size = fmt::format("##{}{}", core::util::GetName(GetUniverse(), input),
                                               GetUniverse().get<components::Name>(entity).name);
            float tmp = amount;
            ImGui::CalcTextSize(fmt_size.c_str());
            ImGui::PushItemWidth(50);
            ImGui::DragFloat(fmt::format("##{}{}", core::util::GetName(GetUniverse(), input),
                                         GetUniverse().get<components::Name>(entity).name)
                                 .c_str(),
                             &tmp, 1, 0, 10000,
                             fmt::format("{} %.3f", core::util::GetName(GetUniverse(), input)).c_str());
            amount = static_cast<double>(tmp);
            ImGui::PopItemWidth();
            // Then if it's clicked we change to a thingy?
            if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                // Then we set that to true?
                // Then something something we let go or something
                SPDLOG_INFO("Dobule clickjed!");
            }
            ed::EndPin();
        }

        for (const auto [input, amount] : recipe_comp.capitalcost) {
            input_map[input].push_back(unique_id);
            ed::BeginPin(unique_id++, ed::PinKind::Input);
            ax::Drawing::Icon(ImVec2(16, 16), ax::Drawing::IconType::RoundSquare, true, ImColor(0, 255, 0, 255),
                              ImColor(32, 32, 32, 255));
            ImGui::SameLine();
            ImGui::TextFmt("{} {}", core::util::GetName(GetUniverse(), input), amount);
            ed::EndPin();
        }
        ImGui::SameLine();

        output_map[recipe_comp.output.entity].push_back(unique_id);
        ed::BeginPin(unique_id++, ed::PinKind::Output);
        ImGui::SameLine();
        ImGui::TextFmt("{} {}", core::util::GetName(GetUniverse(), recipe_comp.output.entity),
                       recipe_comp.output.amount);
        ImGui::SameLine();
        ax::Drawing::Icon(ImVec2(16, 16), ax::Drawing::IconType::Diamond, true, ImColor(255, 0, 0, 255),
                          ImColor(32, 32, 32, 255));
        ed::EndPin();
        ed::EndNode();
        if (first_setup) {
            ed::SetNodePosition(node_id, ImVec2(recipe_tier[entity] * 350, level_map[recipe_tier[entity]]++ * 200));
        }
    }
    first_setup = false;

    // Then we should do our connections
    for (auto good : GetUniverse().GoodIterator()) {
        if (!input_map.contains(good) || !output_map.contains(good)) {
            continue;
        }
        // Otherwise let's go from a to b
        for (auto input_node : input_map[good]) {
            for (auto output_node : output_map[good]) {
                ed::Link(unique_id++, input_node, output_node, ImVec4(1, 1, 1, 1));
            }
        }
    }
    ed::End();
    ImGui::End();
    ed::PopStyleVar();
}

void RecipeNodeViewer::DoUpdate(int delta_time) {}

namespace {
double GetLedgerCost(core::Universe& universe, const components::ResourceVector& ledger) {
    double input_cost = 0;
    for (auto& [entity, amount] : ledger) {
        input_cost = universe.get<components::Price>(entity) * amount;
    }
    return input_cost;
}
}  // namespace
}  // namespace cqsp::client::systems
