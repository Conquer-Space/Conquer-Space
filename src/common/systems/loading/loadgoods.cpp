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

#include <tuple>

#include "common/components/name.h"
#include "common/components/economy.h"
#include "common/components/resource.h"
#include "common/components/bodies.h"
#include "common/systems/loading/loadutil.h"

#define CHECK_DEFINED(x, entity) if (!x.defined()) {\
                                    universe.destroy(entity);\
                                    continue;\
                                 }

namespace cqsp::common::systems::loading {
void LoadGoods(cqsp::common::Universe& universe, Hjson::Value& goods) {
    namespace cqspc = cqsp::common::components;
    for (int i = 0; i < goods.size(); i++) {
        Hjson::Value val = goods[i];
        // Create good
        entt::entity good = universe.create();
        universe.emplace<cqspc::Good>(good);

        if (!LoadInitialValues(universe, good, val)) {
            universe.destroy(good);
            continue;
        }

        if (val["mass"].defined() && val["volume"].defined()) {
            // Then it's matter and physical
            auto& matter = universe.emplace<cqspc::Matter>(good);
            matter.mass = val["mass"];
            matter.volume = val["volume"];
        }

        if (val["energy"].defined()) {
            double t = val["energy"];
            universe.emplace<cqspc::Energy>(good, t);
        }

        for (int i = 0; i < val["tags"].size(); i++) {
            if (val["tags"][i] == "mineral") {
                universe.get_or_emplace<cqspc::Mineral>(good);
            }
        }

        if (val["price"].defined()) {
            universe.emplace<cqspc::Price>(good, val["price"].to_double());
        } else {
            universe.emplace<cqspc::Price>(good, 1.f);
        }

        if (val["unit"].defined()) {
            universe.emplace<cqspc::Unit>(good, val["unit"].to_string());
        }

        // Basically if it fails at any point, we'll remove the component
        universe.goods[val["identifier"].to_string()] = good;
    }
}

void LoadRecipes(cqsp::common::Universe& universe, Hjson::Value& recipes) {
    namespace cqspc = cqsp::common::components;
    for (int i = 0; i < recipes.size(); i++) {
        Hjson::Value& val = recipes[i];

        entt::entity recipe = universe.create();
        auto& recipe_component = universe.emplace<cqspc::Recipe>(recipe);

        LoadInitialValues(universe, recipe, val);

        Hjson::Value input_value = val["input"];
        recipe_component.input = HjsonToLedger(universe, input_value);

        Hjson::Value output_value = val["output"];
        recipe_component.output = HjsonToLedger(universe, output_value);

        auto &name_object = universe.get<cqspc::Identifier>(recipe);
        universe.recipes[name_object] = recipe;

        // Check if it has cost
        if (val["cost"].defined()) {
            Hjson::Value cost_map = val["cost"];
            auto& recipe_cost = universe.emplace<cqspc::RecipeCost>(recipe);

            Hjson::Value fixed = cost_map["fixed"];
            recipe_cost.fixed = HjsonToLedger(universe, fixed);
            Hjson::Value scaling = cost_map["scaling"];
            recipe_cost.scaling = HjsonToLedger(universe, scaling);
        }
    }
}

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

    if (values["mass"].defined() && values["volume"].defined()) {
        // Then it's matter and physical
        auto& matter = universe.emplace<cqspc::Matter>(entity);
        matter.mass = values["mass"];
        matter.volume = values["volume"];
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
    universe.goods[values["identifier"].to_string()] = entity;
    return true;
}
}  // namespace cqsp::common::systems::loading
