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
#include "common/systems/loading/loadgoods.h"

#include <spdlog/spdlog.h>

#include <string>
#include <tuple>

#include "common/components/name.h"
#include "common/components/economy.h"
#include "common/components/resource.h"
#include "common/components/bodies.h"
#include "common/systems/loading/loadutil.h"
#include "common/components/area.h"

#define CHECK_DEFINED(x, entity) if (!x.defined()) {\
                                    universe.destroy(entity);\
                                    continue;\
                                 }

namespace cqsp::common::systems::loading {
void LoadTerrainData(cqsp::common::Universe& universe, Hjson::Value& value) {
    for (auto it = value.begin(); it != value.end(); it++) {
        entt::entity entity = universe.create();

        using cqsp::common::components::bodies::TerrainData;
        TerrainData& data = universe.get_or_emplace<TerrainData>(entity);

        data.sea_level = it->second["sealevel"];
        auto terrain_colors = it->second["terrain"];
        for (int i = 0; i < terrain_colors.size(); i++) {
            float place = terrain_colors[i][0];
            Hjson::Value color = terrain_colors[i][1];
            if (color.size() == 4) {
                int r = color[0];
                int g = color[1];
                int b = color[2];
                int a = color[3];
                std::tuple<int, int, int, int> tuple = std::make_tuple(r, g, b, a);
                data.data[place] = tuple;
            } else if (color.size() == 3) {
                int r = color[0];
                int g = color[1];
                int b = color[2];
                // Now add the tuple
                std::tuple<int, int, int, int> tuple = std::make_tuple(r, g, b, 255);
                data.data[place] = tuple;
            }
        }
        universe.terrain_data[it->first] = entity;
    }
}

GoodLoader::GoodLoader() {
    default_val["price"] = 1.f;
    default_val["tags"] = Hjson::Type::Vector;
}

bool GoodLoader::LoadValue(const Hjson::Value& values, Universe& universe,
                           entt::entity entity) {
    namespace cqspc = cqsp::common::components;
    namespace cqspt = cqsp::common::components::types;

    std::string identifier = values["identifier"].to_string();
    if (values["mass"].defined() && values["volume"].defined()) {
        // Then it's matter and physical
        auto& matter = universe.emplace<cqspc::Matter>(entity);
        bool mass_correct;
        matter.mass = ReadUnit(values["mass"].to_string(), cqspt::Mass, &mass_correct);
        if (!mass_correct) {
            SPDLOG_WARN("Mass is formatted incorrectly for {}: {}", identifier, values["mass"].to_string());
            return false;
        }

        bool volume_correct;
        matter.volume = ReadUnit(values["volume"].to_string(), cqspt::Volume, &volume_correct);
        if (!volume_correct) {
            SPDLOG_WARN("Volume is formatted incorrectly for {}: {}", identifier, values["volume"].to_string());
            return false;
        }
    }

    if (values["energy"].defined()) {
        double t = values["energy"];
        universe.emplace<cqspc::Energy>(entity, t);
    }

    for (int i = 0; i < values["tags"].size(); i++) {
        if (values["tags"][i] == "mineral") {
            universe.get_or_emplace<cqspc::Mineral>(entity);
        }
    }

    universe.emplace<cqspc::Price>(entity, values["price"].to_double());

    if (values["unit"].type() == Hjson::Type::String) {
        universe.emplace<cqspc::Unit>(entity, values["unit"].to_string());
    }

    // Basically if it fails at any point, we'll remove the component
    universe.goods[identifier] = entity;
    return true;
}

RecipeLoader::RecipeLoader() {
    default_val["input"] = Hjson::Type::Vector;
    default_val["output"] = Hjson::Type::Vector;
}

bool RecipeLoader::LoadValue(const Hjson::Value& values, Universe& universe,
                             entt::entity entity) {
    namespace cqspc = cqsp::common::components;

    auto& recipe_component = universe.emplace<cqspc::Recipe>(entity);

    Hjson::Value input_value = values["input"];
    recipe_component.input = HjsonToLedger(universe, input_value);

    Hjson::Value output_value = values["output"];
    recipe_component.output = HjsonToLedger(universe, output_value);

    // Check if it has cost
    if (values["cost"].defined()) {
        Hjson::Value cost_map = values["cost"];
        auto& recipe_cost = universe.emplace<cqspc::RecipeCost>(entity);

        if (cost_map["fixed"].defined()) {
            Hjson::Value fixed = cost_map["fixed"];
            recipe_cost.fixed = HjsonToLedger(universe, fixed);
        }

        if (cost_map["scaling"].defined()) {
            Hjson::Value scaling = cost_map["scaling"];
            recipe_cost.scaling = HjsonToLedger(universe, scaling);
        }
    }

    for (int i = 0; i < values["tags"].size(); i++) {
        if (values["tags"][i] == "raw") {
            recipe_component.type = cqspc::mine;
        }
        else if (values["tags"][i] == "service") {
            recipe_component.type = cqspc::service;
        } 
        else if (values["tags"][i] == "service") {
            recipe_component.type = cqspc::factory;
        }
    }

    auto &name_object = universe.get<cqspc::Identifier>(entity);
    universe.recipes[name_object] = entity;
    return true;
}
}  // namespace cqsp::common::systems::loading
