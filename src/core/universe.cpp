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
#include "core/universe.h"

#include <memory>
#include <ranges>
#include <utility>

#include "core/components/player.h"
#include "core/util/random/stdrandom.h"
#include "core/util/uuid.h"

namespace cqsp::core {
Universe::Universe() : Universe(util::random_id()) {}

Universe::Universe(std::string uuid) : uuid(std::move(uuid)) {
    random = std::make_unique<util::StdRandom>(42);
    nodeFactory = [this](entt::entity entity) { return Node(*this, entity); };
}

std::set<Node> Universe::Convert(const std::set<entt::entity>& entities) const {
    std::set<Node> nodes;
    for (const auto& entity : entities) {
        nodes.insert(Node(*this, entity));
    }
    return nodes;
}

entt::entity Universe::GetPlayer() const { return view<components::Player>().front(); }

Node::Node(const Universe& universe, const entt::entity entity) : entt::handle((entt::registry&)(universe), entity) {}
Node::Node(const entt::handle handle, const entt::entity entity) : entt::handle(*handle.registry(), entity) {}
Node::Node(Universe& universe) : entt::handle(universe, universe.create()) {}
Universe& Node::universe() const { return static_cast<Universe&>(*this->registry()); }

std::set<Node> Node::Convert(const std::set<entt::entity>& entities) const {
    return this->universe().Convert(entities);
}
Node Node::Convert(const entt::entity entity) const { return Node(*this, entity); }

}  // namespace cqsp::core
