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
#include "common/systems/loading/loadutil.h"

#include "common/components/name.h"

namespace cqsp::common::systems::loading {
bool LoadName(Universe& universe, const entt::entity &entity, const Hjson::Value& value) {
    if (value["name"].type() != Hjson::Type::String) {
        return false;
    }
    std::string identifier = value["name"].to_string();
    auto& name = universe.emplace<components::Name>(entity);
    name.name = identifier;
    return true;
}

bool LoadIdentifier(Universe& universe, const entt::entity &entity, const Hjson::Value& value) {
    if (value["identifier"].type() != Hjson::Type::String) {
        return false;
    }
    std::string identifier = value["identifier"].to_string();
    auto& identifier_comp = universe.emplace<components::Identifier>(entity);
    identifier_comp.identifier = identifier;
    return true;
}

bool LoadDescription(Universe& universe, const entt::entity& entity, const Hjson::Value& value) {
    if (value["description"].type() != Hjson::Type::String) {
        return false;
    }
    std::string identifier = value["description"].to_string();
    auto& identifier_comp = universe.emplace<components::Description>(entity);
    identifier_comp.description = identifier;
    return true;
}

bool LoadInitialValues(Universe& universe, const entt::entity& entity, const Hjson::Value& value) {
    LoadName(universe, entity, value);
    LoadDescription(universe, entity, value);
    return LoadIdentifier(universe, entity, value);
}
}  // namespace cqsp::common::systems::loading
