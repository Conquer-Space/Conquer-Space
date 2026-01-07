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

#include <string>

#include "core/universe.h"

namespace cqsp::core::util {
/// <summary>
/// Gets the name of the entity. If that isn't there, then the identifier.
/// If you somehow mess even that up, it returns the identifier in string form.
/// </summary>
/// <param name="universe"></param>
/// <param name="entity"></param>
/// <returns></returns>
std::string GetName(const Universe& universe, entt::entity entity);
std::string GetName(const Universe& universe, components::GoodEntity entity);
std::string GetEntityType(const Universe& universe, entt::entity entity);
}  // namespace cqsp::core::util
