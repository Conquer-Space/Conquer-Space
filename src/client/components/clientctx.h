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

#include <spdlog/spdlog.h>

#include <array>
#include <string>
#include <variant>

#include <entt/entity/entity.hpp>

namespace cqsp::client::ctx {

static const std::array tick_speeds {1000, 500, 333, 100, 50, 10, 1, -1, -2, -5, -10};

struct StarSystemViewDebug {
    bool to_show = false;
};

struct PauseOptions {
    bool to_tick = false;
    int tick_speed = 3;
    bool tick_once = false;
};

struct HoveringState {
    entt::entity hovering_planet;
    entt::entity hovering_country;
    entt::entity hovering_province;
};

namespace {
template <class... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};
// explicit deduction guide (not needed as of C++20)
template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;
}  // namespace

typedef std::variant<std::monostate, entt::entity, std::string> SelectedItem;
struct HoveringItem {
    SelectedItem world_space;
    SelectedItem ui_space;
};
struct HoveringItem : public std::variant<std::monostate, entt::entity, std::string> {
    using variant::variant;

    template <typename T>
    HoveringItem& operator=(T&& value) {
        std::variant<std::monostate, entt::entity, std::string>::operator=(std::forward<T>(value));
        last_set = true;
        return *this;
    }
    bool operator==(const HoveringItem& other) const {
        return static_cast<const variant&>(*this) == static_cast<const variant&>(other);
    }

    bool operator!=(const HoveringItem& other) const { return !(*this == other); }

    void Reset() { last_set = false; }

    bool Set() const { return last_set; }

 private:
    bool last_set = false;
};

struct SelectedCountry {};

struct SelectedProvince {};

struct GameLoad {
    std::string load_dir;
};

struct VisibleOrbit {};

enum class MapMode {
    NoMapMode,
    CountryMapMode,
    ProvinceMapMode,
    /*
    * This map mode is not intended to be actually used, this is just to force a reset of a map mode
    */
    InvalidMapMode
};
}  // namespace cqsp::client::ctx
