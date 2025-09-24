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
#pragma once

#include <utility>

#include <entt/entt.hpp>

#include "common/components/resource.h"
#include "common/universe.h"

namespace cqsp::common::actions {
/// <summary>
/// Will charge the entity builder the amount needed to construct the factory/building. You'll have to find a
/// organization that can construct it though.
/// </summary>
/// <param name="universe"></param>
/// <param name="city"></param>
/// <param name="recipe"></param>
/// <param name="productivity"></param>
/// <param name="builder"></param>
/// <returns>The factory entity created, entt::null if it's not possible.</returns>
entt::entity OrderConstructionFactory(Universe& universe, entt::entity city, entt::entity market,
                                      entt::entity recipe, int productivity, entt::entity builder);

/// <summary>
/// Creates a factory from thin air
/// </summary>
/// <param name="universe"></param>
/// <param name="city"></param>
/// <param name="recipe"></param>
/// <param name="productivity"></param>
/// <returns>The factory entity created</returns>
entt::entity CreateFactory(Universe& universe, entt::entity city, entt::entity recipe, int productivity, double wages);

components::ResourceLedger GetFactoryCost(Universe& universe, entt::entity city, entt::entity recipe, int productivity);

entt::entity CreateCommercialArea(Universe& universe, entt::entity city);

}  // namespace cqsp::common::systems::actions
