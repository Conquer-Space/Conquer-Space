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
#include "common/systems/loading/loadcountries.h"

#include "common/components/economy.h"
#include "common/components/name.h"
#include "common/components/organizations.h"

namespace cqsp::common::systems::loading {
bool CountryLoader::LoadValue(const Hjson::Value& values, entt::entity entity) {
    // Just make the country
    universe.emplace<components::Country>(entity);
    universe.countries[universe.get<components::Identifier>(entity).identifier] = entity;

    // A country will be it's own market
    universe.emplace<components::Market>(entity);

    return true;
}
}  // namespace cqsp::common::systems::loading
