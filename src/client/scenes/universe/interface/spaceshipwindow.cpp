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
    if (!GetUniverse().all_of<common::components::ships::Ship>(body)) {
        // Display the details of the spaceship
        return;
    }
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
        ImGui::TextFmt("Mean anomaly: {}\u00b0", toDegree(orbit.M0));
        ImGui::TextFmt("Epoch: {}s", orbit.epoch);
        ImGui::TextFmt("GM: {} km^3 * s^-2", orbit.GM);
        ImGui::TextFmt("Orbital period: {} s", orbit.T);
        ImGui::TextFmt("Orbiting: {}", common::util::GetName(GetUniverse(), orbit.reference_body).c_str());
        if (GetUniverse().any_of<common::components::bodies::Body>(orbit.reference_body)) {
            double r = orbit.GetOrbitingRadius();
            double p = GetUniverse().get<common::components::bodies::Body>(orbit.reference_body).radius;
            p = 0;
            ImGui::TextFmt("Altitude: {} km", (r - p));
            ImGui::TextFmt("Periapsis: {:<10} km ({:.1f} s)", orbit.GetPeriapsis() - p, orbit.TimeToMeanAnomaly(0));
            ImGui::TextFmt("Apoapsis: {:<10} km ({:.1f} s)", orbit.GetApoapsis() - p,
                           orbit.TimeToMeanAnomaly(common::components::types::PI));
        }
    }
    if (ImGui::CollapsingHeader("Orbital Vectors")) {
        auto& coords = GetUniverse().get<common::components::types::Kinematics>(body);
        ImGui::TextFmt("Position {} {} {}", coords.position.x, coords.position.y, coords.position.z);
        ImGui::TextFmt("Velocity {} {} {}", coords.velocity.x, coords.velocity.y, coords.velocity.z);
    }

    if (ImGui::CollapsingHeader("Maneuvers")) {
        if (GetUniverse().any_of<common::components::CommandQueue>(body)) {
            auto& queue = GetUniverse().get<common::components::CommandQueue>(body);
            for (auto& manuver : queue) {
                ImGui::TextFmt("Maneuver in {}", manuver.time - GetUniverse().date.ToSecond());
            }
        }
    }

    if (ImGui::Button("Circularize at apoapsis")) {
        double time = orbit.TimeToMeanAnomaly(common::components::types::PI);
        time = (double)GetUniverse().date.ToSecond() + time;
        // Add random delta v
        common::components::Maneuver maneuver;
        maneuver.time = time;
        glm::dvec3 velocity_vec = common::components::types::OrbitVelocityToVec3(orbit, common::components::types::PI);
        double circular_velocity =
            common::components::types::GetCircularOrbitingVelocity(orbit.GM, orbit.GetApoapsis());
        // So for apoapsis, we need this amount of delta v at prograde
        // Get the vector of the direction and then compute?
        // Then transform by the orbital math
        double orbit_velocity = common::components::types::OrbitVelocity(
            common::components::types::PI, orbit.eccentricity, orbit.semi_major_axis, orbit.GM);
        maneuver.delta_v = glm::dvec3(0, circular_velocity - orbit_velocity, 0);
        GetUniverse().get_or_emplace<common::components::CommandQueue>(body).commands.push_back(maneuver);
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
        double time = orbit.TimeToMeanAnomaly(0);
        time = (double)GetUniverse().date.ToSecond() + time;
        // Add random delta v
        common::components::Maneuver maneuver;
        maneuver.time = time;
        glm::dvec3 velocity_vec = common::components::types::OrbitVelocityToVec3(orbit, 0);
        double circular_velocity =
            common::components::types::GetCircularOrbitingVelocity(orbit.GM, orbit.GetPeriapsis());
        // So for apoapsis, we need this amount of delta v at prograde
        // Get the vector of the direction and then compute?
        // Then transform by the orbital math
        double orbit_velocity =
            common::components::types::OrbitVelocity(0, orbit.eccentricity, orbit.semi_major_axis, orbit.GM);
        maneuver.delta_v = glm::dvec3(0, circular_velocity - orbit_velocity, 0);
        GetUniverse().get_or_emplace<common::components::CommandQueue>(body).commands.push_back(maneuver);
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
    ImGui::InputDouble("Set Periapsis Altitude", &new_perigee, 0, 10000000);
    if (ImGui::Button("Set Periapsis")) {
        // Get apogee
        double new_sma = (orbit.GetApoapsis() + new_perigee) / 2;
        // Get velocity at the new apogee
        double new_v = common::components::types::OrbitVelocityAtR(orbit.GM, new_sma, orbit.GetApoapsis());
        // Get the velocity
        common::components::Maneuver maneuver;
        maneuver.time = orbit.TimeToMeanAnomaly(0) + (double)GetUniverse().date.ToSecond();

        double orbit_velocity =
            common::components::types::OrbitVelocity(0, orbit.eccentricity, orbit.semi_major_axis, orbit.GM);
        maneuver.delta_v = glm::vec3(0, new_v - orbit_velocity, 0);
        GetUniverse().get_or_emplace<common::components::CommandQueue>(body).commands.push_back(maneuver);
    }

    if (ImGui::IsItemHovered()) {
        double new_sma = (orbit.GetApoapsis() + new_perigee) / 2;
        // Get velocity at the new apogee
        double new_v = common::components::types::OrbitVelocityAtR(orbit.GM, new_sma, orbit.GetApoapsis());
        double orbit_velocity =
            common::components::types::OrbitVelocity(0, orbit.eccentricity, orbit.semi_major_axis, orbit.GM);
        ImGui::SetTooltip("Delta-v: %f km/s", new_v - orbit_velocity);
    }
    // Display spaceship delta v in the future
    // Display controls of the spaceship
    ImGui::End();
}

void cqsp::client::systems::SpaceshipWindow::DoUpdate(int delta_time) {}
