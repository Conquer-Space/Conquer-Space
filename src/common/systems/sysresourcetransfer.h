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


namespace conquerspace {
namespace common {
namespace systems {
namespace resource {
/// <summary>
/// Transfers resources from the first entity to the second
/// </summary>
/// <param name="registry">Registry to transfer</param>
/// <param name="from">Needs to be a resource stockpile</param>
/// <param name="to">Needs to be a resource stockpile</param>
/// <param name="good">good to transfer</param>
/// <param name="amount">amount of resources to transfer</param>
/// <returns>If transfering resources failed.</returns>
bool TransferResources(entt::registry &registry, entt::entity from, entt::entity to,
                                                        entt::entity good, double amount);
}
}  // namespace systems
}  // namespace common
}  // namespace conquerspace
