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
#include "core/loading/recipeloader.h"

#include <spdlog/spdlog.h>

#include <string>
#include <tuple>

#include "core/components/area.h"
#include "core/components/bodies.h"
#include "core/components/market.h"
#include "core/components/name.h"
#include "core/components/resource.h"
#include "core/loading/loadutil.h"

namespace cqsp::core::loading {

RecipeLoader::RecipeLoader(Universe& universe) : HjsonLoader(universe) {
    default_val["input"] = Hjson::Type::Vector;
    default_val["output"] = Hjson::Type::Vector;
}

bool RecipeLoader::LoadValue(const Hjson::Value& values, Node& node) {
    auto& recipe_component = node.emplace<components::Recipe>();

    Hjson::Value input_value = values["input"];
    recipe_component.input = HjsonToLedger(universe, input_value);

    Hjson::Value output_value = values["output"];
    // Just get the first value
    if (output_value.size() == 1) {
        // Get the values
        auto beg = output_value.begin();
        recipe_component.output.entity = universe.good_map[universe.goods[beg->first]];
        recipe_component.output.amount = beg->second.to_double();

    } else {
        SPDLOG_WARN("Output({}) is not equal to 1, so there will have some issues", output_value.size());
    }

    // Check if it has cost
    if (values["cost"].defined()) {
        Hjson::Value cost_map = values["cost"];
        auto& recipe_cost = node.emplace<components::RecipeCost>();

        if (cost_map["capital"].defined()) {
            Hjson::Value capital = cost_map["capital"];
            recipe_component.capitalcost = HjsonToLedger(universe, capital);
        }

        if (cost_map["labor"].defined()) {
            Hjson::Value labor = cost_map["labor"];
            // TODO(EhWhoAmI): Add different types of labor that is needed for this
            recipe_component.workers = labor["worker"].to_double();
        }

        if (cost_map["fixed"].defined()) {
            Hjson::Value fixed = cost_map["fixed"];
            recipe_cost.fixed = HjsonToLedger(universe, fixed);
        }

        if (cost_map["scaling"].defined()) {
            Hjson::Value scaling = cost_map["scaling"];
            recipe_cost.scaling = HjsonToLedger(universe, scaling);
        }
    }

    if (values["construction"].defined() && values["construction"].type() == Hjson::Type::Map) {
        // Then we get our time
        // Maybe in the future we will have different stages for what resources we need
        // for each construction stage and stuff
        const Hjson::Value& construction = values["construction"];
        auto& construction_cost = node.emplace<components::ConstructionCost>();
        bool time_correct;
        double time = ReadUnit(construction["time"].to_string(), components::types::UnitType::Time, &time_correct);
        if (!time_correct) {
            // Then we don't have a proper time...
            time = 100;
        }
        construction_cost.time = static_cast<int>(time);
        // Then get cost
        const Hjson::Value& cost_map = construction["cost"];
        construction_cost.cost = HjsonToLedger(universe, cost_map) / time;
    }

    for (int i = 0; i < values["tags"].size(); i++) {
        if (values["tags"][i] == "raw") {
            recipe_component.type = components::ProductionType::mine;
        } else if (values["tags"][i] == "service") {
            recipe_component.type = components::ProductionType::service;
        } else if (values["tags"][i] == "factory") {
            recipe_component.type = components::ProductionType::factory;
        }
    }

    auto& name_object = node.get<components::Identifier>();
    universe.recipes[name_object] = node;
    return true;
}
}  // namespace cqsp::core::loading
