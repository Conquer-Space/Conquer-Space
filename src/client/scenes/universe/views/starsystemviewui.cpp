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
#include "client/scenes/universe/views/starsystemviewui.h"

#include "client/scenes/universe/interface/systooltips.h"
#include "client/scenes/universe/views/starsystemrenderer.h"
#include "client/scenes/universe/views/starsystemview.h"
#include "common/components/bodies.h"
#include "common/components/surface.h"
#include "common/util/nameutil.h"
#include "engine/gui.h"

namespace cqsp::client::systems {
namespace components = common::components;
using common::util::GetName;

StarSystemViewUI::StarSystemViewUI(common::Universe& universe, SysStarSystemRenderer& renderer,
                                   StarSystemController& controller, StarSystemCamera& camera)
    : universe(universe), renderer(renderer), controller(controller), camera(camera) {}

void StarSystemViewUI::RenderInformationWindow(double delta_time) {
    // FIXME(EhWhoamI)
    // auto &debug_info =
    // universe.ctx().emplace<ctx::StarSystemViewDebug>();
    ImGui::Begin("Debug ui window");
    ImGui::TextFmt("Cam Pos: {} {} {}", camera.cam_pos.x, camera.cam_pos.y, camera.cam_pos.z);
    ImGui::TextFmt("View Center: {} {} {}", camera.view_center.x, camera.view_center.y, camera.view_center.z);
    ImGui::TextFmt("Scroll: {}", camera.scroll);
    ImGui::TextFmt("View {} {}", camera.view_x, camera.view_y);
    // Get the province name
    std::string country_name_t;
    if (universe.valid(controller.selected_province) &&
        universe.any_of<components::Province>(controller.selected_province)) {
        country_name_t = GetName(universe, universe.get<components::Province>(controller.selected_province).country);
    }
    ImGui::TextFmt("Hovering on Texture: {} {}", controller.tex_x, controller.tex_y);
    ImGui::TextFmt("Texture color: {} {} {}", controller.tex_r, controller.tex_g, controller.tex_b);
    ImGui::TextFmt("Selected province color: {} {} {}", controller.selected_province_color.x,
                   controller.selected_province_color.y, controller.selected_province_color.z);

    ImGui::TextFmt("Hovered province color: {} {} {}", controller.hovering_province_color.x,
                   controller.hovering_province_color.y, controller.hovering_province_color.z);
    ImGui::TextFmt("Hovering province {}", GetName(universe, controller.hovering_province));
    ImGui::TextFmt("Focused planets: {}", universe.view<FocusedPlanet>().size());
    ImGui::TextFmt("Generated {} orbits last frame", renderer.orbit_geometry.GetOrbitsGenerated());
    auto intersection = controller.GetMouseSurfaceIntersection();
    ImGui::TextFmt("Intersection: {} {}", intersection.latitude(), intersection.longitude());

    if (ImGui::Button("Debug Spawn City")) {
        // Add a city founding entity
        entt::entity ent = universe.create();
        universe.emplace<CityFounding>(ent);
    }

    ImGui::End();
}

void StarSystemViewUI::RenderSelectedObjectInformation() {
    // Get the focused object, and display their information
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - 200, ImGui::GetIO().DisplaySize.y - 300),
                            ImGuiCond_Appearing);
    ImGui::Begin("Focused Object");
    gui::EntityTooltipContent(universe, controller.m_viewing_entity);
    // Edit the orbit if there is an issue
    // Get normalized vector
    if (universe.valid(controller.m_viewing_entity) &&
        universe.any_of<components::types::Kinematics>(controller.m_viewing_entity)) {
        const auto& kin = universe.get<components::types::Kinematics>(controller.m_viewing_entity);
        auto norm = glm::normalize(kin.velocity);
        ImGui::TextFmt("Prograde vector: {} {} {}", norm.x, norm.y, norm.z);

        static float delta_v = 0.01;
        norm *= delta_v;
        glm::vec3 final_velocity = kin.velocity + norm;
        ImGui::TextFmt("Velocity vector: {} {} {}", final_velocity.x, final_velocity.y, final_velocity.z);

        if (ImGui::Button("Burn prograde")) {
            // Add 10m/s prograde or something
            auto& impulse = universe.get_or_emplace<components::types::Impulse>(controller.m_viewing_entity);
            impulse.impulse += norm;
        }
        ImGui::SliderFloat("Text", &delta_v, -1, 1);
    }
    ImGui::End();
}

void StarSystemViewUI::DoUI(float delta_time) {
    RenderInformationWindow(delta_time);
    RenderSelectedObjectInformation();
}
}  // namespace cqsp::client::systems
