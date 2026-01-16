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
#include <unordered_map>
#include <vector>

#include <entt/entt.hpp>

#include "core/actions/names/namegenerator.h"
#include "core/components/market.h"
#include "core/components/resourceledger.h"
#include "core/components/stardate.h"
#include "core/systems/economy/economyconfig.h"
#include "core/util/random/random.h"

namespace cqsp::core {
class Universe;
class Node : public entt::handle {
 public:
    explicit Node(const Universe& universe, const entt::entity entity);
    Node(const entt::handle handle, const entt::entity entity);
    explicit Node(Universe& universe);
    Universe& universe() const;
    auto Convert(const std::vector<entt::entity>& entities) const {
        return (entities |
                std::ranges::views::transform([this](entt::entity entity) { return Node(universe(), entity); }));
    }
    std::set<Node> Convert(const std::set<entt::entity>& entities) const;
    Node Convert(const entt::entity entity) const;

    // Overload equivalence against entt::null_t
    friend bool operator==(const Node& lhs, const entt::null_t&) { return lhs.entity() == entt::null; }
    friend bool operator==(const entt::null_t&, const Node& rhs) { return rhs.entity() == entt::null; }
    friend bool operator!=(const Node& lhs, const entt::null_t&) { return lhs.entity() != entt::null; }
    friend bool operator!=(const entt::null_t&, const Node& rhs) { return rhs.entity() != entt::null; }
};

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

    std::vector<entt::entity> good_vector;
    std::unordered_map<entt::entity, components::GoodEntity> good_map;

    entt::entity GetGood(const components::GoodEntity entity) const { return good_vector[static_cast<int>(entity)]; }

    auto GoodIterator() const {
        return (std::views::iota(0, static_cast<int>(good_vector.size())) |
                std::ranges::views::transform(
                    [](int in) -> components::GoodEntity { return static_cast<components::GoodEntity>(in); }));
    }

    size_t GoodCount() const { return good_vector.size(); }

    using entt::registry::all_of;
    using entt::registry::any_of;
    using entt::registry::get;

    template <typename... Component>
    [[nodiscard]] bool all_of(const components::GoodEntity entity) const {
        return entt::registry::all_of<Component...>(GetGood(entity));
    }

    template <typename... Component>
    [[nodiscard]] bool any_of(const components::GoodEntity entity) const {
        return entt::registry::any_of<Component...>(GetGood(entity));
    }

    template <typename... Component>
    [[nodiscard]] decltype(auto) get([[maybe_unused]] const components::GoodEntity entity) {
        return entt::registry::get<Component...>(GetGood(entity));
    }

    template <typename... Component>
    [[nodiscard]] decltype(auto) get([[maybe_unused]] const components::GoodEntity entity) const {
        return entt::registry::get<const Component...>(GetGood(entity));
    }

    void EnableTick() { to_tick = true; }
    void DisableTick() { to_tick = false; }
    bool ToTick() const { return to_tick; }
    void ToggleTick() { to_tick = !to_tick; }

    int GetDate() const { return static_cast<int>(date.GetDate()); }
    std::unique_ptr<cqsp::core::util::IRandom> random;
    std::string uuid;

    /// <summary>
    /// What is the current fraction of the wait of the tick we are processing
    /// </summary>
    double tick_fraction = 0;
    std::function<Node(entt::entity)> nodeFactory;
    auto NodeTransform() const { return std::views::transform(nodeFactory); }
    auto Convert(const std::vector<entt::entity>& entities) const {
        return (entities | std::ranges::views::transform([this](entt::entity entity) { return Node(*this, entity); }));
    }

    std::set<Node> Convert(const std::set<entt::entity>& entities) const;

    template <typename... Components>
    auto nodes() {
        return this->template view<Components...>() | NodeTransform();
    }
    template <typename... Components>
    auto nodes() const {
        return this->template view<Components...>() | NodeTransform();
    }
    template <typename... Components, typename... Exclude>
    auto nodes(entt::exclude_t<Exclude...> exclude) {
        return this->template view<Components...>(exclude) | NodeTransform();
    }
    template <typename... Components, typename... Exclude>
    auto nodes(entt::exclude_t<Exclude...> exclude) const {
        return this->template view<Components...>(exclude) | NodeTransform();
    }

    /**
     * Creates node with entity
     */
    Node operator()(const entt::entity entity) { return Node(*this, entity); }

    /**
     * Creates a new entity with node
     */
    Node operator()() { return Node(*this, create()); }

    /**
     * Returns null node
     */
    Node null() { return Node(*this, entt::null); }

    systems::EconomyConfig economy_config;

 private:
    bool to_tick = false;
};
}  // namespace cqsp::core

template <>
struct fmt::formatter<entt::entity> : formatter<std::string> {
    template <typename FormatContext>
    constexpr auto format(entt::entity entity, FormatContext& ctx) const {
        return formatter<std::string>::format(std::to_string((uint64_t)entity), ctx);
    }
};

template <>
struct fmt::formatter<cqsp::core::Node> : fmt::formatter<std::string> {
    template <typename FormatContext>
    auto format(const cqsp::core::Node ship, FormatContext& ctx) const {
        return fmt::formatter<entt::entity> {}.format(ship.entity(), ctx);
    }
};
