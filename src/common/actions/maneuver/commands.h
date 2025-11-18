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

#include <initializer_list>

#include "common/components/maneuver.h"
#include "common/components/orbit.h"
#include "common/components/orders.h"
#include "common/universe.h"

namespace cqsp::common::systems::commands {
/// Verifies if the command is a valid ship command option.
/// This verifies if it's not null, and contains the
/// necessary components to make it a command
bool VerifyCommand(Node& command);
/// Returns true if a command is processed and popped, returns false if not
bool ProcessCommandQueue(Node& body, components::Trigger trigger);
void ExecuteCommand(Node& orbit_node, Node& command_node, components::Command command);
void TransferToMoon(Node& agent, Node& target);
void LeaveSOI(Node& agent, double altitude);
void LandOnMoon(Node& agent, Node& target, Node& city);
components::Maneuver_t MakeManeuver(const glm::dvec3& vector, double time);
void PushManeuver(Node& entity, components::Maneuver_t maneuver, double offset = 0);
void PushManeuvers(Node& entity, std::initializer_list<components::Maneuver_t> maneuver,
                   double offset = 0);
void PushManeuvers(Node& entity, components::HohmannPair_t hohmann_pair, double offset = 0);
std::vector<Node> GetSOIHierarchy(Node& source);
/**
 * Finds the common sphere of influence between the two bodies.
 * @param universe universe registry
 * @param source source entity
 * @param target target entity
 * @return least common sphere of influence between source and target. If null, then we are comparing two completely
 * separate SOI trees.
 */
Node GetCommonSOI(Node& source, Node& target);
}  // namespace cqsp::common::systems::commands
