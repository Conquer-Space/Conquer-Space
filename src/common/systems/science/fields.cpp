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
#include "common/systems/science/fields.h"

#include <spdlog/spdlog.h>

#include <map>
#include <iostream>
#include <string>
#include <vector>

#include "common/components/name.h"
#include "common/components/science.h"
#include "common/systems/loading/loadutil.h"

struct FieldTemplate {
    std::vector<std::string> parent;
    std::vector<std::string> adjacent;
};
#include <iostream>
void cqsp::common::systems::science::LoadFields(Universe& universe,
                                                Hjson::Value& hjson) {
    for (int i = 0; i < hjson.size(); i++) {
        Hjson::Value val = hjson[i];
        if (val["name"].type() != Hjson::Type::String) {
            continue;
        }
        // Get the name
        entt::entity field = universe.create();
        auto &field_comp = universe.emplace<components::science::Field>(field);
        std::cout << "load make field" << std::endl;
        loading::LoadName(universe, field, val);
        std::cout << "Loaded name" << std::endl;
        if (!loading::LoadIdentifier(universe, field, val)) {
            std::cout << "load identifier" << std::endl;
            universe.destroy(field);
            SPDLOG_INFO("No field with {}", Hjson::Marshal(val));
            continue;
        }

        std::string identifier = universe.get<components::Identifier>(field);
        std::cout << "Get identifier" << std::endl;
        auto& field_template = universe.emplace<FieldTemplate>(field);

        if (val["parent"].type() == Hjson::Type::Vector) {
            for (int j = 0; j < val["parent"].size(); j++) {
                std::string parent_name = val["parent"][j];
                if (parent_name == identifier) {
                    SPDLOG_WARN("Parent for {} cannot be itself!", identifier);
                    continue;
                }
                field_template.parent.push_back(parent_name);
           }
        }
        if (val["adjacent"].type() == Hjson::Type::Vector) {
            for (int j = 0; j < val["adjacent"].size(); j++) {
                std::string parent_name = val["adjacent"][j];
                // Adjacent cannot be itself
                if (parent_name == identifier) {
                    SPDLOG_WARN("Adjacent for {} cannot be itself!", identifier);
                    continue;
                }
                field_template.adjacent.push_back(parent_name);
           }
        }
        if (universe.fields.find(identifier) != universe.fields.end()) {
            SPDLOG_WARN("Field {} already exists, overwriting", identifier);
        }

        // Description
        if (val["description"].type() == Hjson::Type::String) {
            universe.emplace<components::Description>(field, val["description"].to_string());
        }
        universe.fields[identifier] = field;
    }

    auto view = universe.view<FieldTemplate>();
    for (entt::entity entity : view) {
        auto& field_template = universe.get<FieldTemplate>(entity);
        auto& field = universe.get<components::science::Field>(entity);
        for (const auto& parent_name : field_template.parent) {
            // If it does not contain, warn
            if (universe.fields.find(parent_name) == universe.fields.end()) {
                SPDLOG_WARN("No field {} exists", parent_name);
                continue;
            }
            field.parents.push_back(universe.fields[parent_name]);
        }
        for (const auto& parent_name : field_template.adjacent) {
            if (universe.fields.find(parent_name) == universe.fields.end()) {
                SPDLOG_WARN("No field {} exists", parent_name);
                continue;
            }
            field.adjacent.push_back(universe.fields[parent_name]);
        }
    }
    universe.clear<FieldTemplate>();
}
