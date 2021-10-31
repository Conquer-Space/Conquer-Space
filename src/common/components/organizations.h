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
#pragma once

#include <entt/entt.hpp>

namespace cqsp {
namespace common {
namespace components {
struct Organization {};

struct Civilization {
    entt::entity starting_planet;
    // NOTE(EhWhoAmI): This will not make sense to have in the future, as we transition to have
    // empires form and destroy on their own, and as galaxies diversify in terms of population.
    // However, we need it for the intial population generation
    entt::entity founding_species;
};
}  // namespace components
}  // namespace common
}  // namespace cqsp
