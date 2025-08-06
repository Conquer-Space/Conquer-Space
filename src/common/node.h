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


#include <map>
#include <memory>
#include <string>
#include <vector>

#include <entt/entt.hpp>


namespace cqsp::common {
class Node {
public:
    entt::entity entity;
    entt::registry& current_registry;
    //current_registry.create(); creates a node
    entt::registry& GetUniverse() { return current_registry; }


    Node(entt::entity new_entity, entt::registry& parent_registry)
        : entity(new_entity), current_registry(parent_registry) {
    }
    Node(entt::entity new_entity, Node& parent) : entity(new_entity), current_registry(parent.current_registry) {
    }

    Node(const Node& parent) : current_registry(parent.current_registry) { 
        this->entity = current_registry.create(); 
    }


    Node(entt::registry& parent_registry) : current_registry(parent_registry) { 
        this->entity = current_registry.create(); 
    }


    template <typename... Component>
    [[nodiscard]] decltype(auto) get() {
        return current_registry.get<Component...>(entity);
    }

    template <typename... Component>
    [[nodiscard]] decltype(auto) get() const {
        return current_registry.get<const Component...>(entity);
    }

    template <typename Component, typename... Args>
    [[nodiscard]] decltype(auto) get_or_emplace(Args&&... args) {
        return current_registry.get_or_emplace<Component>(entity, std::forward<Args>(args)...);
    }

    template <typename... Component>
    [[nodiscard]] auto try_get() {
        return current_registry.try_get<Component...>(entity);
    }

    template <typename... Component>
    [[nodiscard]] auto try_get() const {
        return current_registry.try_get<const Component...>(entity);
    }

    template <typename... Component>
    [[nodiscard]] bool all_of() const {
        return current_registry.all_of<Component...>(entity);
    }

    template <typename... Component>
    [[nodiscard]] bool any_of() const {
        return current_registry.any_of<Component...>(entity);
    }

    static std::vector<Node> Convert(const std::vector<entt::entity>& entities, entt::registry& parent_registry) {
        std::vector<Node> nodes;
        nodes.reserve(entities.size());
        for (const auto& entity : entities) {
            nodes.emplace_back(entity, parent_registry);
        }
        return nodes;
    }
    std::vector<Node> Convert(const std::vector<entt::entity>& entities) const {
        std::vector<Node> nodes;
        nodes.reserve(entities.size());
        for (const auto& entity : entities) {
            nodes.emplace_back(entity, current_registry);
        }
        return nodes;
    }
};
}  // namespace cqsp::common
