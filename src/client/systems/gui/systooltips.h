/* Conquer Space
 * Copyright (C) 2021-2023 Conquer Space
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

#include <string>

#include <entt/entt.hpp>

#include "common/universe.h"

namespace cqsp::client::systems::gui {
/// <summary>
/// This actually creates the tooltip for the entity
/// </summary>
/// <param name=""></param>
/// <param name=""></param>
void EntityTooltip(const cqsp::common::Universe &, entt::entity);
/// <summary>
/// In case you want the tooltip content for debug displaying the information of an entity.
/// </summary>
void EntityTooltipContent(const cqsp::common::Universe &, entt::entity);
}  // namespace cqsp::client::systems::gui
