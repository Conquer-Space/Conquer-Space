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

#define CHECK_DEFINED(x, entity) if (!x.defined()) {\
                                    universe.destroy(entity);\
                                    continue;\
                                 }

void cqsp::common::systems::loading::LoadGoods(cqsp::common::Universe& universe, Hjson::Value& goods) {
    namespace cqspc = cqsp::common::components;
    for (int i = 0; i < goods.size(); i++) {
        Hjson::Value val = goods[i];
        // Create good
        entt::entity good = universe.create();
        universe.emplace<cqspc::Good>(good);

        CHECK_DEFINED(val["identifier"], good);
        CHECK_DEFINED(val["name"], good);

        universe.emplace<cqspc::Name>(good, val["name"].to_string());
        universe.emplace<cqspc::Identifier>(good, val["identifier"].to_string());

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
                universe.emplace_or_replace<cqspc::Mineral>(good);
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

void cqsp::common::systems::loading::LoadRecipes(cqsp::common::Universe& universe, Hjson::Value& recipes) {
    namespace cqspc = cqsp::common::components;
    for (int i = 0; i < recipes.size(); i++) {
        Hjson::Value& val = recipes[i];

        entt::entity recipe = universe.create();
        auto& recipe_component = universe.emplace<cqspc::Recipe>(recipe);
        Hjson::Value input_value = val["input"];
        for (auto input_good : input_value) {
            recipe_component.input[universe.goods[input_good.first]] = input_good.second;
        }

        Hjson::Value output_value = val["output"];
        for (auto output_good : output_value) {
            recipe_component.output[universe.goods[output_good.first]] = output_good.second;
        }

        auto &name_object = universe.emplace<cqspc::Identifier>(recipe);
        name_object.identifier = val["identifier"].to_string();
        universe.recipes[name_object] = recipe;
    }
}

void cqsp::common::systems::loading::LoadTerrainData(cqsp::common::Universe& universe, Hjson::Value& value) {
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
