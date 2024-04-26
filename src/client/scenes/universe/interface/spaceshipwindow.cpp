/* Conquer Space
 * Copyright (C) 2021-2023 Conquer Space
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
#include "client/scenes/universe/interface/spaceshipwindow.h"

#include "client/scenes/universe/views/starsystemview.h"
#include "common/components/coordinates.h"
#include "common/components/movement.h"
#include "common/components/orbit.h"
#include "common/components/ships.h"
#include "common/systems/maneuver/hohmann.h"
#include "common/systems/maneuver/maneuver.h"
#include "common/systems/maneuver/rendezvous.h"
#include "common/util/nameutil.h"

namespace components = cqsp::common::components;
namespace types = cqsp::common::components::types;
namespace ships = cqsp::common::components::ships;
namespace systems = cqsp::common::systems;
using components::CommandQueue;
using components::Maneuver;
using types::Kinematics;
using types::Orbit;

using types::OrbitVelocity;

using components::bodies::Body;
using cqsp::client::systems::SpaceshipWindow;
using entt::entity;
using cqsp::common::util::GetName;

void SpaceshipWindow::Init() {}

void SpaceshipWindow::DoUI(int delta_time) {
    using types::toDegree;
    // Get selected spaceship, and then do the window
    // Check if the selected body is a spaceship
    // Then display information on it
    entity body = GetUniverse().view<FocusedPlanet>().front();
    if (!GetUniverse().valid(body)) {
        return;
    }
    if (!GetUniverse().all_of<ships::Ship>(body)) {
        // Display the details of the spaceship
        return;
    }
    ImGui::Begin(fmt::format("{}", GetName(GetUniverse(), body)).c_str());
    // Display orbital elements
    auto& orbit = GetUniverse().get<Orbit>(body);
    if (ImGui::CollapsingHeader("Orbital Elements", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::TextFmt("Semi-major axis: {} km", orbit.semi_major_axis);
        ImGui::TextFmt("Eccentricity: {}", orbit.eccentricity);
        // Degree symbol is broken :(
        ImGui::TextFmt("Inclination: {}\u00b0", toDegree(orbit.inclination));
        ImGui::TextFmt("Longitude of the ascending node: {}\u00b0", toDegree(orbit.LAN));
        ImGui::TextFmt("Argument of periapsis: {}\u00b0", toDegree(orbit.w));
        ImGui::TextFmt("Mean anomaly at Epoch: {}\u00b0", toDegree(orbit.M0));
        ImGui::TextFmt("Epoch: {}s", orbit.epoch);
        ImGui::TextFmt("GM: {} km^3 * s^-2", orbit.GM);
        ImGui::TextFmt("Orbital period: {} s", orbit.T());
        ImGui::TextFmt("Orbiting: {}", GetName(GetUniverse(), orbit.reference_body).c_str());
        if (GetUniverse().any_of<Body>(orbit.reference_body)) {
            double r = orbit.GetOrbitingRadius();
            double p = GetUniverse().get<Body>(orbit.reference_body).radius;
            p = 0;
            ImGui::TextFmt("Altitude: {} km", (r - p));
            ImGui::TextFmt("Periapsis: {:<10} km ({:.1f} s)", orbit.GetPeriapsis() - p, orbit.TimeToMeanAnomaly(0));
            ImGui::TextFmt("Apoapsis: {:<10} km ({:.1f} s)", orbit.GetApoapsis() - p,
                           orbit.TimeToMeanAnomaly(components::types::PI));
        }
    }
    if (ImGui::CollapsingHeader("Orbital Vectors")) {
        auto& coords = GetUniverse().get<Kinematics>(body);
        ImGui::TextFmt("Position {} {} {}", coords.position.x, coords.position.y, coords.position.z);
        ImGui::TextFmt("Velocity {} {} {}", coords.velocity.x, coords.velocity.y, coords.velocity.z);
    }

    if (ImGui::CollapsingHeader("Maneuver Queue")) {
        if (GetUniverse().any_of<CommandQueue>(body)) {
            auto& queue = GetUniverse().get<CommandQueue>(body);
            for (auto& manuver : queue) {
                ImGui::TextFmt("Maneuver in {}", manuver.time - GetUniverse().date.ToSecond());
            }
        }
    }

    if (ImGui::CollapsingHeader("Basic Orbital Maneuvers")) {
        if (ImGui::Button("Circularize at apoapsis")) {
            // Add random delta v
            Maneuver maneuver(common::systems::CircularizeAtApoapsis(orbit));
            maneuver.time += GetUniverse().date.ToSecond();
            GetUniverse().get_or_emplace<CommandQueue>(body).commands.push_back(maneuver);
        }
        if (ImGui::IsItemHovered()) {
            double circular_velocity = types::GetCircularOrbitingVelocity(orbit.GM, orbit.GetApoapsis());
            // So for apoapsis, we need this amount of delta v at prograde
            // Get the vector of the direction and then compute?
            // Then transform by the orbital math
            double orbit_velocity = OrbitVelocity(types::PI, orbit.eccentricity, orbit.semi_major_axis, orbit.GM);
            ImGui::SetTooltip("Delta-v: %f km/s", circular_velocity - orbit_velocity);
        }

        if (ImGui::Button("Circularize at perapsis")) {
            Maneuver maneuver(common::systems::CircularizeAtPeriapsis(orbit));
            maneuver.time += GetUniverse().date.ToSecond();
            GetUniverse().get_or_emplace<CommandQueue>(body).commands.push_back(maneuver);
        }

        if (ImGui::IsItemHovered()) {
            double circular_velocity = types::GetCircularOrbitingVelocity(orbit.GM, orbit.GetPeriapsis());
            // So for apoapsis, we need this amount of delta v at prograde
            // Get the vector of the direction and then compute?
            // Then transform by the orbital math
            double orbit_velocity = OrbitVelocity(0, orbit.eccentricity, orbit.semi_major_axis, orbit.GM);
            ImGui::SetTooltip("Delta-v: %f km/s", circular_velocity - orbit_velocity);
        }

        // Change apogee
        ImGui::InputDouble("Set Apoapsis Altitude", &new_perigee, 0, 10000000);
        if (ImGui::Button("Set Apoapsis")) {
            // Get velocity at the new apogee
            // Get the velocity
            Maneuver maneuver;
            auto m = common::systems::SetApoapsis(orbit, new_perigee);
            maneuver.delta_v = m.first;
            maneuver.time = GetUniverse().date.ToSecond() + m.second;
            GetUniverse().get_or_emplace<CommandQueue>(body).commands.push_back(maneuver);
        }

        if (ImGui::IsItemHovered()) {
            double new_sma = (orbit.GetApoapsis() + new_perigee) / 2;
            // Get velocity at the new apogee
            double new_v = types::OrbitVelocityAtR(orbit.GM, new_sma, orbit.GetApoapsis());
            double orbit_velocity =
                types::OrbitVelocity(0, orbit.eccentricity, orbit.semi_major_axis, orbit.GM);
            ImGui::SetTooltip("Delta-v: %f km/s", new_v - orbit_velocity);
        }

        // Change apogee
        ImGui::InputDouble("Set Periapsis Altitude", &new_apogee, 0, 10000000);
        if (ImGui::Button("Set Periapsis")) {
            // Get velocity at the new apogee
            // Get the velocity
            Maneuver maneuver;
            auto m = common::systems::SetPeriapsis(orbit, new_apogee);
            maneuver.delta_v = m.first;
            maneuver.time = GetUniverse().date.ToSecond() + m.second;
            GetUniverse().get_or_emplace<CommandQueue>(body).commands.push_back(maneuver);
        }

        if (ImGui::IsItemHovered()) {
            double new_sma = (orbit.GetPeriapsis() + new_apogee) / 2;
            // Get velocity at the new apogee
            double new_v = types::OrbitVelocityAtR(orbit.GM, new_sma, orbit.GetPeriapsis());
            double orbit_velocity = types::OrbitVelocity(0, orbit.eccentricity, orbit.semi_major_axis, orbit.GM);
            ImGui::SetTooltip("Delta-v: %f km/s", new_v - orbit_velocity);
        }

        ImGui::InputDouble("Set Orbit Altitude", &new_hohmann, 0, 10000000);
        if (ImGui::Button("Set New Orbit")) {
            // Get velocity at the new apogee
            // Get the velocity
            auto hohmann = common::systems::HohmannTransfer(orbit, new_hohmann);
            if (hohmann.has_value()) {
                auto& queue = GetUniverse().get_or_emplace<CommandQueue>(body);
                Maneuver man_1(hohmann->first);
                Maneuver man_2(hohmann->second);
                man_1.time += (GetUniverse().date.ToSecond() + 1000);
                man_2.time += (GetUniverse().date.ToSecond() + 1000);
                queue.commands.push_back(man_1);
                queue.commands.push_back(man_2);
            } else {
                SPDLOG_INFO("Orbit is not circular!");
            }
        }
    }

    // Display spaceship delta v in the future
    // Display controls of the spaceship
    if (ImGui::CollapsingHeader("Rendezvous")) {
        auto& o_system = GetUniverse().get<components::bodies::OrbitalSystem>(orbit.reference_body);
        static entity selected = o_system.children.empty() ? entt::null : o_system.children.front();
        auto& target = GetUniverse().get<Orbit>(selected);

        // Get distance from target
        glm::vec3 target_distance = GetUniverse().get<Kinematics>(selected).position -
                                    GetUniverse().get<Kinematics>(body).position;
        ImGui::TextFmt("Distance to target: {}", glm::length(target_distance));
        if (ImGui::Button("Rendez-vous!")) {
            // Rdv with target
            auto pair = cqsp::common::systems::CoplanarIntercept(orbit, target, GetUniverse().date.ToSecond());
            auto& queue = GetUniverse().get_or_emplace<CommandQueue>(body);
            Maneuver man_1(pair.first);
            Maneuver man_2(pair.second);
            man_1.time += GetUniverse().date.ToSecond();
            man_2.time += GetUniverse().date.ToSecond();
            queue.commands.push_back(man_1);
            queue.commands.push_back(man_2);
        }
        if (ImGui::Button("Maneuver to point")) {
            auto pair = cqsp::common::systems::CoplanarIntercept(orbit, target, GetUniverse().date.ToSecond());
            auto& queue = GetUniverse().get_or_emplace<CommandQueue>(body);
            common::components::Maneuver man_1(pair.first);
            man_1.time += GetUniverse().date.ToSecond();
            queue.commands.push_back(man_1);
        }
        if (ImGui::Button("Match Planes")) {
            auto& queue = GetUniverse().get_or_emplace<CommandQueue>(body);
            auto maneuver = cqsp::common::systems::MatchPlanes(orbit, target);
            maneuver.second += GetUniverse().date.ToSecond();
            queue.commands.emplace_back(maneuver);
        }
        ImGui::TextFmt("Phase angle: {}", types::CalculatePhaseAngle(orbit, target, GetUniverse().date.ToSecond()));
        ImGui::TextFmt("Transfer angle: {}", types::CalculateTransferAngle(orbit, target));
        double ttma = orbit.TimeToMeanAnomaly(types::AscendingTrueAnomaly(orbit, target));
        ImGui::TextFmt("Time to ascending node: {}", ttma);
        if (ImGui::BeginChild("Rendezvous Target")) {
            for (auto& entity : o_system.children) {
                if (ImGui::Selectable(common::util::GetName(GetUniverse(), entity).c_str(), selected == entity)) {
                    selected = entity;
                }
            }
        }
        ImGui::EndChild();
    }
    ImGui::End();
}

void SpaceshipWindow::DoUpdate(int delta_time) {}
