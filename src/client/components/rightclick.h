#pragma once

#include <string>
#include <variant>

#include <entt/entity/entity.hpp>
namespace cqsp::client::ctx {
template <class... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};

template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

typedef std::variant<std::monostate, entt::entity, std::string> SelectedItem;
struct HoveringItem {
    SelectedItem world_space;
    SelectedItem ui_space;

    void Reset() {
        world_space = std::monostate();
        ui_space = std::monostate();
    }
};
}  // namespace cqsp::client::ctx