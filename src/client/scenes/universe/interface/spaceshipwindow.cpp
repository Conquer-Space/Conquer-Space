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
#include "client/scenes/universe/interface/spaceshipwindow.h"

#include "client/scenes/universe/views/starsystemview.h"
#include "common/components/coordinates.h"
#include "common/components/movement.h"
#include "common/components/orbit.h"
#include "common/components/ships.h"
#include "common/components/surface.h"
#include "common/systems/maneuver/commands.h"
#include "common/systems/maneuver/hohmann.h"
#include "common/systems/maneuver/maneuver.h"
#include "common/systems/maneuver/rendezvous.h"
#include "common/util/nameutil.h"

void cqsp::client::systems::SpaceshipWindow::Init() {}

void cqsp::client::systems::SpaceshipWindow::DoUI(int delta_time) {
    using common::components::types::toDegree;
    // Get selected spaceship, and then do the window
    // Check if the selected body is a spaceship
    // Then display information on it
    entt::entity body = GetUniverse().view<FocusedPlanet>().front();
    if (!GetUniverse().valid(body)) {
        return;
    }
    if (!GetUniverse().all_of<common::components::ships::Ship, common::components::types::Orbit>(body)) {
        return;
    }
    // Display the details of the spaceship
    ImGui::Begin(fmt::format("{}", common::util::GetName(GetUniverse(), body)).c_str());
    // Display orbital elements
    auto& orbit = GetUniverse().get<common::components::types::Orbit>(body);
    if (ImGui::CollapsingHeader("Orbital Elements", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::TextFmt("Semi-major axis: {} km", orbit.semi_major_axis);
        ImGui::TextFmt("Eccentricity: {}", orbit.eccentricity);
        // Degree symbol is broken :(
        ImGui::TextFmt("Inclination: {}\u00b0", toDegree(orbit.inclination));
        ImGui::TextFmt("Longitude of the ascending node: {}\u00b0", toDegree(orbit.LAN));
        ImGui::TextFmt("Argument of periapsis: {}\u00b0", toDegree(orbit.w));
        ImGui::TextFmt("Mean anomaly at Epoch: {}\u00b0", toDegree(orbit.M0));
        ImGui::TextFmt("True anomaly: {}", orbit.v);
        ImGui::TextFmt("Epoch: {}s", orbit.epoch);
        ImGui::TextFmt("GM: {} km^3 * s^-2", orbit.GM);
        ImGui::TextFmt("Orbital period: {} s", orbit.T());
        ImGui::TextFmt("Orbiting: {}", common::util::GetName(GetUniverse(), orbit.reference_body).c_str());
        if (GetUniverse().valid(orbit.reference_body) &&
            GetUniverse().any_of<common::components::bodies::Body>(orbit.reference_body)) {
            double r = orbit.GetOrbitingRadius();
            double p = GetUniverse().get<common::components::bodies::Body>(orbit.reference_body).radius;
            p = 0;
            ImGui::TextFmt("Altitude: {} km", (r - p));
            ImGui::TextFmt("Periapsis: {:<10} km ({:.1f} s)", orbit.GetPeriapsis() - p, orbit.TimeToTrueAnomaly(0));
            ImGui::TextFmt("Apoapsis: {:<10} km ({:.1f} s)", orbit.GetApoapsis() - p,
                           orbit.TimeToTrueAnomaly(common::components::types::PI));
        }
    }
    if (GetUniverse().all_of<common::components::types::Kinematics>(body) &&
        ImGui::CollapsingHeader("Orbital Vectors")) {
        auto& coords = GetUniverse().get<common::components::types::Kinematics>(body);
        ImGui::TextFmt("Position {} {} {}", coords.position.x, coords.position.y, coords.position.z);
        ImGui::TextFmt("Velocity {} {} {}", coords.velocity.x, coords.velocity.y, coords.velocity.z);
    }

    if (ImGui::CollapsingHeader("Maneuver Queue")) {
        if (GetUniverse().any_of<common::components::CommandQueue>(body)) {
            auto& queue = GetUniverse().get<common::components::CommandQueue>(body);
            for (auto& maneuver : queue) {
                ImGui::TextFmt("Maneuver in {}", maneuver.time - GetUniverse().date.ToSecond());
            }
        }
    }

    if (ImGui::CollapsingHeader("Basic Orbital Maneuvers")) {
        static float true_anomaly = 0;
        ImGui::SliderAngle("trueanomaly", &true_anomaly, (0));
        if (ImGui::Button("Fix True anomaly")) {
            // Emplace
            GetUniverse().emplace_or_replace<common::components::types::SetTrueAnomaly>(body, true_anomaly);
        }
        if (GetUniverse().any_of<common::components::types::SetTrueAnomaly>(body)) {
            // Now set the true anomaly consistently or something
        }
        if (ImGui::Button("Circularize at apoapsis")) {
            // Add random delta v
            common::systems::commands::PushManeuvers(GetUniverse(), body,
                                                     {common::systems::CircularizeAtApoapsis(orbit)});
        }
        if (ImGui::IsItemHovered()) {
            double circular_velocity =
                common::components::types::GetCircularOrbitingVelocity(orbit.GM, orbit.GetApoapsis());
            // So for apoapsis, we need this amount of delta v at prograde
            // Get the vector of the direction and then compute?
            // Then transform by the orbital math
            double orbit_velocity = common::components::types::OrbitVelocity(
                common::components::types::PI, orbit.eccentricity, orbit.semi_major_axis, orbit.GM);
            ImGui::SetTooltip("Delta-v: %f km/s", circular_velocity - orbit_velocity);
        }

        if (ImGui::Button("Circularize at perapsis")) {
            common::systems::commands::PushManeuvers(GetUniverse(), body,
                                                     {common::systems::CircularizeAtPeriapsis(orbit)});
        }

        if (ImGui::IsItemHovered()) {
            double circular_velocity =
                common::components::types::GetCircularOrbitingVelocity(orbit.GM, orbit.GetPeriapsis());
            // So for apoapsis, we need this amount of delta v at prograde
            // Get the vector of the direction and then compute?
            // Then transform by the orbital math
            double orbit_velocity =
                common::components::types::OrbitVelocity(0, orbit.eccentricity, orbit.semi_major_axis, orbit.GM);
            ImGui::SetTooltip("Delta-v: %f km/s", circular_velocity - orbit_velocity);
        }

        // Change apogee
        ImGui::InputDouble("Set Apoapsis Altitude", &new_perigee, 0, 10000000);
        if (ImGui::Button("Set Apoapsis")) {
            // Get velocity at the new apogee
            // Get the velocity
            auto maneuver = common::systems::SetApoapsis(orbit, new_perigee);
            common::systems::commands::PushManeuvers(GetUniverse(), body, {maneuver});
        }

        if (ImGui::IsItemHovered()) {
            double new_sma = (orbit.GetApoapsis() + new_perigee) / 2;
            // Get velocity at the new apogee
            double new_v = common::components::types::OrbitVelocityAtR(orbit.GM, new_sma, orbit.GetApoapsis());
            double orbit_velocity =
                common::components::types::OrbitVelocity(0, orbit.eccentricity, orbit.semi_major_axis, orbit.GM);
            ImGui::SetTooltip("Delta-v: %f km/s", new_v - orbit_velocity);
        }

        // Change apogee
        ImGui::InputDouble("Set Periapsis Altitude", &new_apogee, 0, 10000000);
        if (ImGui::Button("Set Periapsis")) {
            // Get velocity at the new apogee
            // Get the velocity
            auto maneuver = common::systems::SetPeriapsis(orbit, new_apogee);
            common::systems::commands::PushManeuvers(GetUniverse(), body, {maneuver});
        }

        if (ImGui::IsItemHovered()) {
            double new_sma = (orbit.GetPeriapsis() + new_apogee) / 2;
            // Get velocity at the new apogee
            double new_v = common::components::types::OrbitVelocityAtR(orbit.GM, new_sma, orbit.GetPeriapsis());
            double orbit_velocity =
                common::components::types::OrbitVelocity(0, orbit.eccentricity, orbit.semi_major_axis, orbit.GM);
            ImGui::SetTooltip("Delta-v: %f km/s", new_v - orbit_velocity);
        }

        ImGui::InputDouble("Set Orbit Altitude", &new_hohmann, 0, 10000000);
        if (ImGui::Button("Set New Orbit")) {
            // Get velocity at the new apogee
            // Get the velocity
            auto hohmann = common::systems::HohmannTransfer(orbit, new_hohmann);
            if (hohmann.has_value()) {
                common::systems::commands::PushManeuvers(GetUniverse(), body, *hohmann);
            } else {
                SPDLOG_INFO("Orbit is not circular!");
            }
        }
    }

    // Display spaceship delta v in the future
    // Display controls of the spaceship
    if (ImGui::CollapsingHeader("Rendezvous")) {
        auto& o_system = GetUniverse().get<cqsp::common::components::bodies::OrbitalSystem>(orbit.reference_body);
        static entt::entity selected = o_system.children.empty() ? entt::null : o_system.children.front();
        auto& target = GetUniverse().get<common::components::types::Orbit>(selected);

        // Get distance from target
        glm::vec3 target_distance = GetUniverse().get<common::components::types::Kinematics>(selected).position -
                                    GetUniverse().get<common::components::types::Kinematics>(body).position;
        ImGui::TextFmt("Distance to target: {}", glm::length(target_distance));
        if (ImGui::Button("Rendez-vous!")) {
            // Rdv with target
            auto pair = cqsp::common::systems::CoplanarIntercept(orbit, target, GetUniverse().date.ToSecond());
            common::systems::commands::PushManeuvers(GetUniverse(), body, pair);
        }
        if (ImGui::Button("Maneuver to point")) {
            auto pair = cqsp::common::systems::CoplanarIntercept(orbit, target, GetUniverse().date.ToSecond());
            common::systems::commands::PushManeuvers(GetUniverse(), body, {pair.first});
        }
        if (ImGui::Button("Match Planes")) {
            auto maneuver = cqsp::common::systems::MatchPlanes(orbit, target);
            common::systems::commands::PushManeuvers(GetUniverse(), body, {maneuver});
        }
        ImGui::TextFmt("Phase angle: {}", cqsp::common::components::types::CalculatePhaseAngle(
                                              orbit, target, GetUniverse().date.ToSecond()));
        ImGui::TextFmt("Transfer angle: {}", cqsp::common::components::types::CalculateTransferAngle(orbit, target));
        double ttma = orbit.TimeToTrueAnomaly(common::components::types::AscendingTrueAnomaly(orbit, target));
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
    if (ImGui::CollapsingHeader("Moon Transfers")) {
        auto& o_system = GetUniverse().get<cqsp::common::components::bodies::OrbitalSystem>(orbit.reference_body);
        static entt::entity selected = entt::null;
        if (selected == entt::null) {
            ImGui::BeginDisabled(true);
        }
        if (ImGui::Button("Transfer to Moon")) {
            common::systems::commands::TransferToMoon(GetUniverse(), body, selected);
        }

        // Land on body?
        if (ImGui::Button("Land On City")) {
            // Check if the targeted body has a settlement to land on
            // Just grab the first one
            auto& cities = GetUniverse().get<common::components::Habitation>(selected);
            if (!cities.settlements.empty()) {
                common::systems::commands::LandOnMoon(GetUniverse(), body, selected, cities.settlements.front());
            }
        }
        if (selected == entt::null) {
            ImGui::EndDisabled();
        }

        if (ImGui::BeginChild("Landing Target")) {
            for (auto& entity : o_system.children) {
                if (!GetUniverse().all_of<common::components::bodies::Planet, common::components::types::Orbit>(
                        entity)) {
                    continue;
                }
                if (ImGui::Selectable(common::util::GetName(GetUniverse(), entity).c_str(), selected == entity)) {
                    selected = entity;
                }
            }
        }
        ImGui::EndChild();
    }
    ImGui::End();
}

void cqsp::client::systems::SpaceshipWindow::DoUpdate(int delta_time) {}
