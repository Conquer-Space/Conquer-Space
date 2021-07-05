/*
* Copyright 2021 Conquer Space
*/
#pragma once

#include <entt/entt.hpp>


namespace conquerspace {
namespace common {
namespace systems {
namespace resource {
/// <summary>
/// Transfers resources from the first entity to the second
/// </summary>
/// <param name="registry">Registry to transfer</param>
/// <param name="from">Needs to be a resource stockpile</param>
/// <param name="to">Needs to be a resource stockpile</param>
/// <param name="good">good to transfer</param>
/// <param name="amount">amount of resources to transfer</param>
/// <returns>If transfering resources failed.</returns>
bool TransferResources(entt::registry &registry, entt::entity from, entt::entity to,
                                                        entt::entity good, double amount);
}
}  // namespace systems
}  // namespace common
}  // namespace conquerspace
