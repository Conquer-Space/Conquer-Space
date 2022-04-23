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

#include <iostream>
#include "common/components/name.h"

bool cqsp::common::systems::loading::LoadName(Universe& universe,
                                              const entt::entity &entity,
                                              const Hjson::Value& value) {
    if (value["name"].type() != Hjson::Type::String) {
        return false;
    }
    std::cout << "Loading name" << std::endl;
    std::string identifier = value["name"].to_string();
    std::cout << "Loading name2" << std::endl;

    universe.emplace_or_replace<components::Name>(entity, identifier);
}

bool cqsp::common::systems::loading::LoadIdentifier(Universe& universe,
                                                    const entt::entity &entity,
                                                    const Hjson::Value& value) {
       if (value["identifier"].type() != Hjson::Type::String) {
        return false;
    }
    std::string identifier = value["identifier"].to_string();
    universe.emplace_or_replace<components::Identifier>(entity, identifier);
}
