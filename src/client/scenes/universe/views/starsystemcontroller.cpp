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
#include "client/scenes/universe/views/starsystemcontroller.h"

#include <numbers>

#include <tracy/Tracy.hpp>

#include "client/components/clientctx.h"
#include "client/components/planetrendering.h"
#include "client/scenes/universe/views/starsystemview.h"
#include "core/actions/cityactions.h"
#include "core/components/bodies.h"
#include "core/components/coordinates.h"
#include "core/components/name.h"
#include "core/components/surface.h"

namespace cqsp::client::systems {
namespace components = core::components;
namespace bodies = components::bodies;
namespace types = components::types;

using bodies::Body;
using client::components::Offset;
using client::components::PlanetTexture;
using components::Habitation;
using components::Name;
using types::SurfaceCoordinate;

StarSystemController::StarSystemController(core::Universe& _u, engine::Application& _a, StarSystemCamera& _c)
    : universe(_u), app(_a), camera(_c) {}

void StarSystemController::Update(float delta_time) {
    ZoneScoped;
    universe.clear<systems::MouseOverEntity>();
    GetMouseOnObject(app.GetMouseX(), app.GetMouseY());

    double deltaX = previous_mouseX - app.GetMouseX();
    double deltaY = previous_mouseY - app.GetMouseY();

    is_founding_city = IsFoundingCity();

    CityDetection();

    // Discern between clicking on UI and game
    if (!ImGui::GetIO().WantCaptureMouse && !app.GetRmlUiContext()->IsMouseInteracting()) {
        CalculateScroll();

        CalculateViewChange(deltaX, deltaY);

        previous_mouseX = app.GetMouseX();
        previous_mouseY = app.GetMouseY();

        // If clicks on object, go to the planet
        entt::entity ent = universe.view<MouseOverEntity>().front();
        if (app.MouseButtonIsReleased(engine::MouseInput::LEFT) && ent != entt::null && !app.MouseDragged()) {
            FocusOnEntity(ent);

            if (is_founding_city) {
                FoundCity();
            }
        }
        // Some math if you're close enough you select the city instead of the planet
    }

    if (!ImGui::GetIO().WantCaptureKeyboard) {
        MoveCamera(delta_time);
    }

    mouse_on_object = GetMouseIntersectionOnObject(app.GetMouseX(), app.GetMouseY());

    // Calculate camera
    CenterCameraOnCity();
}

void StarSystemController::MoveCamera(double delta_time) {
    // Now navigation for changing the center
    glm::vec3 dir = (camera.view_center - camera.cam_pos);
    float velocity = delta_time * camera.scroll * CAMERA_MOVEMENT_SPEED;
    // Get distance from the pane
    // Remove y axis

    glm::vec3 forward = glm::normalize(glm::vec3(glm::sin(camera.view_x), glm::cos(camera.view_x), 0));
    glm::vec3 right = glm::normalize(glm::cross(forward, camera.cam_up));
    auto post_move = [&]() {
        universe.clear<FocusedPlanet>();
        m_viewing_entity = entt::null;
    };
    if (app.ButtonIsHeld(engine::KeyInput::KEY_W)) {
        // Get direction
        camera.view_center -= forward * velocity;
        post_move();
    }
    if (app.ButtonIsHeld(engine::KeyInput::KEY_S)) {
        camera.view_center += forward * velocity;
        post_move();
    }
    if (app.ButtonIsHeld(engine::KeyInput::KEY_A)) {
        camera.view_center += right * velocity;
        post_move();
    }
    if (app.ButtonIsHeld(engine::KeyInput::KEY_D)) {
        camera.view_center -= right * velocity;
        post_move();
    }
}

void StarSystemController::CalculateScroll() {
    double min_scroll = 0.1;
    if (m_viewing_entity != entt::null && universe.valid(m_viewing_entity) && universe.all_of<Body>(m_viewing_entity)) {
        min_scroll = std::max(universe.get<Body>(m_viewing_entity).radius * 1.1, 0.1);
    }
    double scroll_value = GetScrollValue();
    if (camera.scroll - scroll_value <= min_scroll) {
        return;
    }
    camera.scroll -= scroll_value;
}

float StarSystemController::GetScrollValue() { return app.GetScrollAmount() * camera.scroll * SCROLL_SENSITIVITY; }

void StarSystemController::CalculateViewChange(double deltaX, double deltaY) {
    if (!app.MouseButtonIsHeld(engine::MouseInput::LEFT)) {
        return;
    }
    camera.view_x -= deltaX / app.GetWindowWidth() * std::numbers::pi * PAN_SPEED;
    camera.view_y -= deltaY / app.GetWindowHeight() * std::numbers::pi * PAN_SPEED;

    if (glm::degrees(camera.view_y) > 89.f) {
        camera.view_y = glm::radians(89.f);
    }
    if (glm::degrees(camera.view_y) < -89.f) {
        camera.view_y = glm::radians(-89.f);
    }
    selected_city = entt::null;
}

bool StarSystemController::IsFoundingCity() { return !universe.view<CityFounding>().empty(); }

void StarSystemController::CityDetection() {
    ZoneScoped;
    if (on_planet == entt::null || !universe.valid(on_planet)) {
        return;
    }
    SurfaceCoordinate s = GetMouseSurfaceIntersection();

    if (!universe.any_of<PlanetTexture>(on_planet)) {
        return;
    }
    auto& planet_texture = universe.get<PlanetTexture>(on_planet);
    if (planet_texture.province_texture == nullptr) {
        return;
    }

    int _province_height = planet_texture.province_texture->height;
    int _province_width = planet_texture.province_texture->width;

    // Get the texture
    // Look for the vector
    int x = tex_x = ((-(s.latitude() * 2 - 180))) / 360 * _province_height;
    int y = tex_y = fmod(s.longitude() + 180, 360) / 360. * _province_width;
    int pos = (x * _province_width + y);
    if (pos < 0 || pos > _province_width * _province_height) {
        return;
    }

    ZoneNamed(LookforProvince, true);
    {
        hovering_province = planet_texture.province_map[pos];
        int color = universe.colors_province[on_planet][hovering_province];
        auto province_color = components::ProvinceColor::fromInt(color);
        hovering_province_color =
            (glm::vec3((float)province_color.r, (float)province_color.g, (float)province_color.b) / 255.f);
    }
}

SurfaceCoordinate StarSystemController::GetMouseSurfaceIntersection() {
    if (on_planet == entt::null || !universe.valid(on_planet)) {
        return SurfaceCoordinate(0, 0);
    }
    if (!universe.any_of<Body>(on_planet)) {
        return SurfaceCoordinate(0, 0);
    }

    glm::vec3 p = GetMouseOnObjectPosition() - CalculateCenteredObject(on_planet);
    p = glm::normalize(p);

    Body& planet_comp = universe.get<Body>(on_planet);
    glm::quat quat = GetBodyRotation(planet_comp.axial, planet_comp.rotation, planet_comp.rotation_offset);
    // Rotate the vector based on the axial tilt and rotation.
    p = glm::inverse(quat) * p;

    return types::ToSurfaceCoordinate(p);
}

glm::quat StarSystemController::GetBodyRotation(double axial, double rotation, double day_offset) {
    // Need to interpolate between the frames
    float rot = (float)bodies::GetPlanetRotationAngle(
        universe.date.ToSecond() + universe.tick_fraction * components::StarDate::TIME_INCREMENT, rotation, day_offset);
    if (rotation == 0) {
        rot = 0;
    }
    return glm::quat {{(float)-axial, 0, 0}} * glm::quat {{0, 0, (float)std::fmod(rot, types::TWOPI)}};
}

void StarSystemController::CenterCameraOnCity() {
    if (selected_city == entt::null) {
        return;
    }

    if (!universe.any_of<SurfaceCoordinate>(selected_city)) {
        return;
    }

    auto& surf = universe.get<SurfaceCoordinate>(selected_city);
    // TODO(EhWhoAmI): Change this so that it doesn't have to change the
    // coordinate system multiple times. Currently this changes from surface
    // coordinates to 3d coordinates to surface coordinates. I think it can be
    // solved with a basic formula.
    entt::entity planet = universe.view<FocusedPlanet>().front();

    if (!universe.valid(planet) || !universe.any_of<Body>(planet)) {
        return;
    }
    Body& body = universe.get<Body>(planet);

    glm::quat quat = GetBodyRotation(body.axial, body.rotation, body.rotation_offset);

    glm::vec3 vec = types::toVec3(surf.universe_view(), 1);
    auto s = quat * vec;
    // TODO(EhWhoAmI): Find a way to dynamically change our camera up when
    // we want to focus on a city.
    // camera.cam_up = quat * glm::vec3(0.0f, 0.0f, 1.0f);
    camera.view_y = std::asin(s.z);
    camera.view_x = std::atan2(s.x, s.y);
}

void StarSystemController::FocusOnEntity(entt::entity ent) {
    // Check the focused planet
    entt::entity focused_planet = universe.view<FocusedPlanet>().front();

    // if the focused planet is the current planet, then check if it's close
    // enough. If it is see the countries on the planet
    if (ent == focused_planet) {
        auto& body = universe.get<Body>(focused_planet);

        if (camera.scroll > body.radius * 10) {
            // Planet selection
            SeePlanet(ent);
        } else {
            // Check if the planet has stuff and then don't select if there's no countries on the planet
            SelectCountry();
        }
    } else {
        SeePlanet(ent);
    }
}

void StarSystemController::SelectCountry() {
    // Country selection
    // Then select planet and tell the state
    selected_province_color = hovering_province_color;
    selected_province = hovering_province;
    // Set the selected province
    if (!universe.valid(selected_province)) {
        return;
    }
    universe.clear<ctx::SelectedProvince>();
    // Get selected planet, then
    entt::entity focused_planet = universe.view<FocusedPlanet>().front();
    if (!universe.any_of<PlanetTexture>(focused_planet)) {
        return;
    }
    auto& tex = universe.get<PlanetTexture>(focused_planet);
    if (tex.province_texture != nullptr) {
        universe.emplace_or_replace<ctx::SelectedProvince>(selected_province);
    }
}

void StarSystemController::SeePlanet(entt::entity ent) {
    universe.clear<FocusedPlanet>();
    universe.emplace<FocusedPlanet>(ent);
}

void StarSystemController::FoundCity() {
    auto s = GetMouseSurfaceIntersection();
    SPDLOG_INFO("Founding city at {} {}", s.latitude(), s.longitude());
    core::Node planetnode(universe, on_planet);
    entt::entity settlement = core::actions::CreateCity(planetnode, s);
    // Set the name of the city
    Name& name = universe.emplace<Name>(settlement);
    name.name = universe.name_generators["Town Names"].Generate("1");

    // Set country
    // Add population and economy
    universe.emplace<components::IndustrialZone>(settlement);

    universe.clear<CityFounding>();

    CalculateCityPositions();
}

void StarSystemController::PreRender() {
    FocusPlanetView();
    FocusCityView();
}

void StarSystemController::FocusCityView() {
    ZoneScoped;
    auto focused_city_view = universe.view<FocusedCity>();
    // City to focus view on
    if (focused_city_view.empty()) {
        return;
    }
    selected_city = focused_city_view.front();
    universe.clear<FocusedCity>();

    CenterCameraOnCity();
    entt::entity planet = universe.view<FocusedPlanet>().front();
    Body& body = universe.get<Body>(planet);
    // 100 km above the city
    camera.scroll = body.radius + 100;
}

std::optional<glm::vec3> StarSystemController::CheckIntersection(const glm::vec3& object_pos, const glm::vec3& ray_wor,
                                                                 float radius) {
    // Check for intersection with sphere
    glm::vec3 sub = camera.cam_pos - object_pos;
    float b = glm::dot(ray_wor, sub);
    float c = glm::dot(sub, sub) - radius * radius;

    if ((b * b - c) >= 0) {
        // Return the point
        return std::optional<glm::vec3>(camera.cam_pos + (-b - sqrt(b * b - c)) * ray_wor);
    } else {
        return std::nullopt;
    }
}

glm::vec3 StarSystemController::GetMouseIntersectionOnObject(int mouse_x, int mouse_y) {
    ZoneScoped;
    glm::vec3 ray_wor = CalculateMouseRay(GetMouseInScreenSpace(mouse_x, mouse_y));

    for (entt::entity ent_id : universe.view<Body>()) {
        glm::vec3 object_pos = CalculateCenteredObject(ent_id);

        Body& body = universe.get<Body>(ent_id);
        auto intersection = CheckIntersection(object_pos, ray_wor, static_cast<float>(body.radius));

        // Get the closer value
        if (intersection) {
            glm::vec3 closest_hit = *intersection;
            is_rendering_founding_city = true;
            on_planet = ent_id;
            return closest_hit;
        }
    }
    is_rendering_founding_city = false;
    return glm::vec3(0, 0, 0);
}

glm::vec3 StarSystemController::CalculateMouseRay(const glm::vec3& ray_nds) {
    glm::vec4 ray_clip = glm::vec4(ray_nds.x, ray_nds.y, -1.0, 1.0);
    glm::vec4 ray_eye = glm::inverse(camera.projection) * ray_clip;
    ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0, 0.0);
    glm::vec4 inv = (glm::inverse(camera.camera_matrix) * ray_eye);

    // Normalize vector
    return glm::normalize(glm::vec3(inv.x, inv.y, inv.z));
}

void StarSystemController::CalculateCityPositions() {
    // Calculate offset for all cities on planet if they exist
    if (!universe.valid(m_viewing_entity)) {
        return;
    }
    if (!universe.all_of<Habitation>(m_viewing_entity)) {
        return;
    }
    std::vector<entt::entity> cities = universe.get<Habitation>(m_viewing_entity).settlements;
    if (cities.empty()) {
        return;
    }
    for (auto& city_entity : cities) {
        if (!universe.all_of<SurfaceCoordinate>(city_entity)) {
            continue;
        }
        auto& coord = universe.get<SurfaceCoordinate>(city_entity);
        Body parent = universe.get<Body>(m_viewing_entity);
        universe.emplace_or_replace<Offset>(city_entity, types::toVec3(coord.universe_view(), 1));
    }
    SPDLOG_INFO("Calculated offset");
}

glm::vec3 StarSystemController::GetMouseInScreenSpace(int mouse_x, int mouse_y) {
    float x = (2.0f * mouse_x) / app.GetWindowWidth() - 1.0f;
    float y = 1.0f - (2.0f * mouse_y) / app.GetWindowHeight();
    float z = 1.0f;
    return glm::vec3(x, y, z);
}

entt::entity StarSystemController::GetMouseOnObject(int mouse_x, int mouse_y) {
    // Loop through objects
    glm::vec3 ray_wor = CalculateMouseRay(GetMouseInScreenSpace(mouse_x, mouse_y));
    for (entt::entity body_id : universe.view<Body>()) {
        glm::vec3 object_pos = CalculateCenteredObject(body_id);
        // Check if the sphere is rendered or not
        // Normalize 3d device coordinates
        auto& body = universe.get<Body>(body_id);
        auto intersection = CheckIntersection(object_pos, ray_wor, static_cast<float>(body.radius));

        // Get the closer value
        if (intersection) {
            universe.emplace<MouseOverEntity>(body_id);
            return body_id;
        }
    }
    return entt::null;
}

bool StarSystemController::ShouldDrawCityPrototype() { return is_founding_city && is_rendering_founding_city; }

const glm::vec3& StarSystemController::SelectedProvinceColor() { return selected_province_color; }

void StarSystemController::SeeEntity() {
    // See the object
    camera.view_center = CalculateObjectPos(m_viewing_entity);

    // Set the variable
    if (universe.all_of<Body>(m_viewing_entity)) {
        camera.scroll = universe.get<Body>(m_viewing_entity).radius * 2.5;
        if (camera.scroll < 0.1) camera.scroll = 0.1;
    } else {
        camera.scroll = 5;
    }
    CalculateCityPositions();
}

glm::vec3 StarSystemController::CalculateCenteredObject(const entt::entity& ent) {
    return CalculateCenteredObject(CalculateObjectPos(ent));
}

glm::vec3 StarSystemController::CalculateCenteredObject(const glm::vec3& vec) { return vec - camera.view_center; }

glm::vec3 StarSystemController::CalculateObjectPos(const entt::entity& ent) {
    // Get the position
    if (!universe.all_of<types::Kinematics>(ent)) {
        return glm::vec3(0, 0, 0);
    }
    auto& kin = universe.get<types::Kinematics>(ent);
    const auto& pos = kin.position + kin.center;
    return pos;
}

void StarSystemController::FocusPlanetView() {
    // Seeing new planet
    entt::entity current_planet = universe.view<FocusedPlanet>().front();
    if (current_planet != m_viewing_entity && current_planet != entt::null) {
        SPDLOG_INFO("Switched displaying planet, seeing {}", current_planet);
        m_viewing_entity = current_planet;
        // Do terrain
        SeeEntity();
    }
}

}  // namespace cqsp::client::systems
