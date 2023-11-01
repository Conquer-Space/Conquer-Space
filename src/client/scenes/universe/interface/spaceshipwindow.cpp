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
        ImGui::Text("Semi-major axis: %f", orbit.semi_major_axis);
        ImGui::Text("Eccentricity: %f", orbit.eccentricity);
        ImGui::Text("Inclination: %f", orbit.inclination);
        ImGui::Text("Longitude of the ascending node: %f", orbit.LAN);
        ImGui::Text("Argument of periapsis: %f", orbit.w);
        ImGui::Text("Mean anomaly: %f", orbit.M0);
        ImGui::Text("Epoch: %f", orbit.epoch);
        ImGui::Text("GM: %f", orbit.GM);
        ImGui::Text("Orbital period: %f", orbit.T);
        ImGui::Text("Orbiting: %s", common::util::GetName(GetUniverse(), orbit.reference_body).c_str());
        if (GetUniverse().any_of<common::components::bodies::Body>(orbit.reference_body)) {
            double r = orbit.GetOrbitingRadius();
            double p = GetUniverse().get<common::components::bodies::Body>(orbit.reference_body).radius;
            ImGui::TextFmt("Altitude: {}", (r - p));
            ImGui::TextFmt("Periapsis: {} {}", orbit.GetPeriapsis() - p, orbit.TimeToMeanAnomaly(0));
            ImGui::TextFmt("Apoapsis: {} {}", orbit.GetApoapsis() - p,
                           orbit.TimeToMeanAnomaly(common::components::types::PI));
        }
    }
    if (ImGui::CollapsingHeader("Orbital Vectors")) {
        auto& coords = GetUniverse().get<common::components::types::Kinematics>(body);
        ImGui::TextFmt("Position {} {} {}", coords.position.x, coords.position.y, coords.position.z);
        ImGui::TextFmt("Velocity {} {} {}", coords.velocity.x, coords.velocity.y, coords.velocity.z);
    }

    if (ImGui::Button("Circularize at apoapsis")) {
        double time = orbit.TimeToMeanAnomaly(common::components::types::PI);
        time = (double)GetUniverse().date.ToSecond() + time;
        // Add random delta v
        common::components::Maneuver maneuver;
        maneuver.time = time;
        // I forgot it added 5km/s
        maneuver.delta_v = glm::dvec3(5, 0, 0);
        GetUniverse().get_or_emplace<common::components::CommandQueue>(body).commands.push(maneuver);
    }
    // Display spaceship delta v in the future
    // Display controls of the spaceship
    ImGui::End();
}

void cqsp::client::systems::SpaceshipWindow::DoUpdate(int delta_time) {}
