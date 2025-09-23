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

#include <fmt/format.h>

#include <map>
#include <memory>
#include <ranges>
#include <string>
#include <vector>

#include <entt/entt.hpp>

#include "common/actions/names/namegenerator.h"
#include "common/systems/economy/economyconfig.h"
#include "common/components/stardate.h"
#include "common/util/random/random.h"

namespace cqsp::common {

class Node;

class Universe : public entt::registry {
 public:
    explicit Universe(std::string uuid);
    Universe();

    components::StarDate date;

    std::map<std::string, entt::entity> goods;
    std::vector<entt::entity> consumergoods;
    std::map<std::string, entt::entity> recipes;
    std::map<std::string, entt::entity> terrain_data;
    std::map<std::string, actions::NameGenerator> name_generators;
    std::map<std::string, entt::entity> fields;
    std::map<std::string, entt::entity> technologies;
    std::map<std::string, entt::entity> planets;
    std::map<std::string, entt::entity> time_zones;
    std::map<std::string, entt::entity> countries;
    std::map<std::string, entt::entity> provinces;
    std::map<std::string, entt::entity> cities;
    // color -> province map
    std::map<entt::entity, std::map<int, entt::entity>> province_colors;
    // province -> color
    std::map<entt::entity, std::map<entt::entity, int>> colors_province;
    entt::entity sun;

    void EnableTick() { to_tick = true; }
    void DisableTick() { to_tick = false; }
    bool ToTick() { return to_tick; }
    void ToggleTick() { to_tick = !to_tick; }

    int GetDate() { return date.GetDate(); }
    std::unique_ptr<cqsp::common::util::IRandom> random;
    std::string uuid;

    /// <summary>
    /// What is the current fraction of the wait of the tick we are processing
    /// </summary>
    double tick_fraction = 0;
    std::function<Node(entt::entity)> nodeFactory;
    auto nodeTransform() { return std::views::transform(nodeFactory); }
    std::vector<Node> Convert(const std::vector<entt::entity>& entities);
    template <typename... Components>
    auto nodes() {
        return this->template view<Components...>() | nodeTransform();
    }

    systems::EconomyConfig economy_config;

 private:
    bool to_tick = false;
};

class Node : public entt::handle {
 public:
    explicit Node(Universe& universe, entt::entity entity);
    Node(entt::handle handle, entt::entity entity);
    explicit Node(Universe& universe);
    Universe& universe() const;
    std::vector<Node> Convert(const std::vector<entt::entity>& entities);
    Node Convert(const entt::entity entity);
};

}  // namespace cqsp::common

template <>
struct fmt::formatter<entt::entity> : formatter<std::string> {
    template <typename FormatContext>
    constexpr auto format(entt::entity entity, FormatContext& ctx) const {
        return formatter<std::string>::format(std::to_string((uint64_t)entity), ctx);
    }
};
