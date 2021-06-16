/*
* Copyright 2021 Conquer Space
*/
#pragma once

#include <entt/entt.hpp>


namespace conquerspace {
namespace systems {
namespace resource {
bool TransferResources(entt::registry&, entt::entity, entt::entity, entt::entity, double);
}
}  // namespace systems
}  // namespace conquerspace
