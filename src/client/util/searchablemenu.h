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

#include <array>
#include <vector>

#include <entt/entity/entity.hpp>

#include "core/universe.h"

namespace cqsp::client::systems {
class SearchableMenu {
 public:
    SearchableMenu() { search_text.fill(0); }

    void Display(const std::string& name, core::Universe& universe, const std::vector<entt::entity>& list);

    entt::entity GetSelected() const { return selected_item; }

    // Right menu is just put as normal lol
 private:
    // We want it to be immediate mode so we should just send it through a lambda or something?
    void LeftMenu(const std::string& name, core::Universe& universe, const std::vector<entt::entity>& list);

    std::array<char, 255> search_text;
    entt::entity selected_item = entt::null;
};
}  // namespace cqsp::client::systems
