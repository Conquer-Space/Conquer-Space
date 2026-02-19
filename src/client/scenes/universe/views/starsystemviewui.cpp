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
#include "core/components/bodies.h"
#include "core/components/surface.h"
#include "core/util/nameutil.h"
#include "engine/gui.h"

namespace cqsp::client::systems {
namespace components = core::components;
using core::util::GetName;

StarSystemViewUI::StarSystemViewUI(core::Universe& universe, SysStarSystemRenderer& renderer,
                                   StarSystemController& controller, StarSystemCamera& camera)
    : universe(universe), renderer(renderer), controller(controller), camera(camera) {
    province_color.fill(0);
}

namespace {
template <class... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};
// explicit deduction guide (not needed as of C++20)
template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;
}  // namespace

void StarSystemViewUI::RenderInformationWindow(double delta_time) {
    ImGui::Begin("Debug ui window", NULL, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::TextFmt("Mouse Pos: {} {}", controller.app.GetMouseX(), controller.app.GetMouseY());
    ImGui::TextFmt("Cam Pos: {} {} {}", camera.cam_pos.x, camera.cam_pos.y, camera.cam_pos.z);
    ImGui::TextFmt("Cam Coordinate: {} {}", controller.target_surface_coordinate.longitude(),
                   controller.target_surface_coordinate.latitude());
    ImGui::TextFmt("Cam Up: {:2.f} {:2.f} {:2.f} ({})", camera.cam_up.x, camera.cam_up.y, camera.cam_up.z,
                   camera.camera_time);
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

    ImGui::TextFmt("Hovering province {}", GetName(universe, controller.hovering_province));
    ImGui::TextFmt("Selected province {}", GetName(universe, controller.selected_province));
    ImGui::TextFmt("Focused planets: {}", GetName(universe, universe.view<FocusedPlanet>().front()));
    ImGui::TextFmt("Hovered planet: {}", GetName(universe, controller.hovering_planet));
    ImGui::TextFmt("Hovered position: {} {} {} ({})", controller.mouse_on_object_position.x,
                   controller.mouse_on_object_position.y, controller.mouse_on_object_position.z,
                   glm::length(controller.mouse_on_object_position));
    ImGui::TextFmt("Generated {} orbits last frame", renderer.orbit_geometry.GetOrbitsGenerated());
    auto intersection = controller.GetMouseSurfaceIntersection();
    ImGui::TextFmt("Intersection: {} {}", intersection.latitude(), intersection.longitude());
    auto& hovering_text = universe.ctx().at<client::ctx::HoveringItem>();
    std::visit(overloaded {[&](std::monostate) { ImGui::Text("No tooltip text"); },
                           [&](entt::entity entity) { ImGui::TextFmt("Tooltip on {}", GetName(universe, entity)); },
                           [&](const std::string& string) { ImGui::TextFmt("Tooltip on {}", string); }},
               hovering_text);
    ImGui::TextFmt("Just set {}", hovering_text.Set());

    ImGui::TextFmt("Tick Fraction: {}", universe.tick_fraction);
    if (ImGui::Button("Debug Spawn City")) {
        // Add a city founding entity
        entt::entity ent = universe.create();
        universe.emplace<CityFounding>(ent);
    }

    if (!universe.valid(controller.m_viewing_entity)) {
        ImGui::BeginDisabled();
    }

    bool open_popup = ImGui::ColorButton(
        "###province_color_button", ImVec4(province_color[0], province_color[1], province_color[2], province_color[3]));
    ImGui::SameLine();
    open_popup |= ImGui::Button("Province color");
    // Sets the color of the current province
    if (!universe.valid(controller.m_viewing_entity)) {
        // Then set the color of the province
        open_popup = false;
        ImGui::EndDisabled();
    }

    if (open_popup) {
        ImGui::OpenPopup("province_color_picker");
    }

    bool to_set_color = false;
    if (ImGui::BeginPopup("province_color_picker")) {
        to_set_color = ImGui::ColorPicker4("###Province Color Picker", province_color.data(),
                                           ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoSmallPreview);
        ImGui::EndPopup();
    }
    if (to_set_color) {
        renderer.UpdatePlanetProvinceColors(
            controller.m_viewing_entity, controller.selected_province,
            glm::vec4(province_color[0], province_color[1], province_color[2], province_color[3]));
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
