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
#include "client/scenes/universe/views/starsystemrenderer.h"

#include <cmath>
#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

#include "client/components/clientctx.h"
#include "client/components/planetrendering.h"
#include "core/components/bodies.h"
#include "core/components/coordinates.h"
#include "core/components/model.h"
#include "core/components/name.h"
#include "core/components/orbit.h"
#include "core/components/organizations.h"
#include "core/components/player.h"
#include "core/components/ships.h"
#include "core/components/surface.h"
#include "core/util/nameutil.h"
#include "engine/glfwdebug.h"
#include "engine/graphics/primitives/cube.h"
#include "engine/graphics/primitives/pane.h"
#include "engine/graphics/primitives/polygon.h"
#include "engine/graphics/primitives/uvsphere.h"
#include "engine/renderer/renderer.h"
#include "glad/glad.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/quaternion.hpp"
#include "stb_image.h"  // NOLINT: STB is rather annoying
#include "tracy/Tracy.hpp"

namespace cqsp::client::systems {
namespace components = core::components;
namespace bodies = components::bodies;
namespace types = components::types;
namespace ships = components::ships;

using asset::ShaderDefinition;
using asset::ShaderProgram_t;
using asset::Texture;
using client::components::Offset;
using client::components::OrbitMesh;
using client::components::PlanetTerrainRender;
using client::components::PlanetTexture;
using components::Name;
using components::Settlements;

using bodies::Body;
using bodies::LightEmitter;
using core::util::GetName;
using types::Orbit;
using types::SurfaceCoordinate;

SysStarSystemRenderer::SysStarSystemRenderer(core::Universe& _u, engine::Application& _a)
    : universe(_u),
      app(_a),
      controller(universe, app, camera),
      user_interface(universe, *this, controller, camera),
      orbit_geometry(universe),
      sun_color(glm::vec3(10, 10, 10)) {}

void SysStarSystemRenderer::Initialize() {
    InitializeMeshes();
    InitializeFramebuffers();

    LoadProvinceMap();
    SetupDummyTextures();
    // Zoom into the player's capital city
    entt::entity player = universe.view<components::Player>().front();
    entt::entity player_capital = universe.get<components::Country>(player).capital_city;
    if (player_capital != entt::null) {
        // Zoom into the thing
        universe.emplace_or_replace<FocusedCity>(player_capital);
    }
}

void SysStarSystemRenderer::OnTick() {
    entt::entity current_planet = universe.view<FocusedPlanet>().front();
    if (current_planet != entt::null) {
        camera.view_center = controller.CalculateObjectPos(controller.m_viewing_entity);
    }
}

void SysStarSystemRenderer::Render(float delta_time) {
    ZoneScoped;

    controller.PreRender();

    // Check for resized window
    window_ratio = static_cast<float>(app.GetWindowWidth()) / static_cast<float>(app.GetWindowHeight());

    orbit_geometry.GenerateOrbitLines();

    renderer.NewFrame(*app.GetWindow());

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    camera.CalculateCameraMatrix(app.GetWindowWidth(), app.GetWindowHeight(), delta_time);

    // FIXME(EhWhoAmI): Fix log renderer so that objects that are close are rendered with a
    // "normal" depth buffer, and objects far away will be rendered with a log buffer.
    // FIXME(EhWhoAmI): Unify all the rendering of planets and stars into one single loop
    // FIXME(EhWhoAmI): Orbit lines dissapear based on distance away from the planet.
    // make them dissapear if you're focused on the planet.
    DrawStars();
    DrawBodies();
    DrawShips();
    DrawSkybox();

    // Unable to render this
    renderer.DrawAllLayers();
    // Draw model
    vis_shader->UseProgram();
    vis_shader->SetMVP(glm::mat4(1.f), glm::mat4(1.f), camera.projection);
}

void SysStarSystemRenderer::SeeStarSystem() {
    orbit_geometry.GenerateOrbitLines();

    SPDLOG_INFO("Loading planet textures");
    LoadPlanetTextures();

    // Set scroll to radius of sun
    if (universe.sun != entt::null && universe.any_of<Body>(universe.sun)) {
        camera.scroll = universe.get<Body>(universe.sun).radius * 5;
    }
}

void SysStarSystemRenderer::Update(float delta_time) { controller.Update(delta_time); }

void SysStarSystemRenderer::DoUI(float delta_time) { user_interface.DoUI(delta_time); }

void SysStarSystemRenderer::DrawStars() {
    ZoneScoped;
    // Draw stars

    renderer.BeginDraw(physical_layer);
    for (entt::entity star : universe.view<Body, LightEmitter>()) {
        // Draw the star circle
        glm::vec3 object_pos = controller.CalculateCenteredObject(star);
        sun_position = object_pos;
        DrawStar(star, object_pos);
    }
    renderer.EndDraw(physical_layer);
}

void SysStarSystemRenderer::DrawBodies() {
    ZoneScoped;
    // Draw other bodies
    auto bodies = universe.view<Body>(entt::exclude<LightEmitter>);
    renderer.BeginDraw(planet_icon_layer);
    glDepthFunc(GL_ALWAYS);
    DrawAllPlanetBillboards(bodies);
    glDepthFunc(GL_LESS);
    renderer.EndDraw(planet_icon_layer);

    renderer.BeginDraw(physical_layer);
    DrawAllPlanets(bodies);
    DrawAllOrbits();
    DrawModels();
    renderer.EndDraw(physical_layer);

    // This is on the ship icon layer because the cities have to appear on top of planets
    // and planet_icon_layer is behind all the planets.
    renderer.BeginDraw(ship_icon_layer);
    DrawAllCities(bodies);
    renderer.EndDraw(ship_icon_layer);
}

void SysStarSystemRenderer::DrawShips() {
    ZoneScoped;
    // Draw Ships

    renderer.BeginDraw(ship_icon_layer);
    ship_overlay.shaderProgram->UseProgram();
    for (entt::entity ship : universe.view<ships::Ship, ctx::VisibleOrbit>()) {
        // if it's not visible, then don't render
        glm::vec3 object_pos = controller.CalculateCenteredObject(ship);
        ship_overlay.shaderProgram->setVec4("color", 1, 0, 0, 1);
        // Interpolate so that it looks nice
        if (universe.all_of<types::FuturePosition, types::Kinematics>(ship)) {
            auto& kinematics = universe.get<types::Kinematics>(ship);
            auto& future_comp = universe.get<types::FuturePosition>(ship);
            const auto& pos = future_comp.position + kinematics.center;
            glm::vec3 future_pos = controller.CalculateCenteredObject(pos);
            DrawShipIcon(glm::mix(object_pos, future_pos, universe.tick_fraction));
        } else {
            DrawShipIcon(object_pos);
        }
    }
    renderer.EndDraw(ship_icon_layer);
}

void SysStarSystemRenderer::DrawSkybox() {
    ZoneScoped;
    // Draw sky box
    renderer.BeginDraw(skybox_layer);
    sky.shaderProgram->UseProgram();
    sky.shaderProgram->setMat4("view", glm::mat4(glm::mat3(camera.camera_matrix)));
    sky.shaderProgram->setMat4("projection", camera.projection);
    glDepthFunc(GL_LEQUAL);
    // skybox cube
    engine::Draw(sky);
    glDepthFunc(GL_LESS);
    renderer.EndDraw(skybox_layer);
}

void SysStarSystemRenderer::DrawModels() {
    // Loop through the space bodies that are close
    for (entt::entity ship : universe.view<ships::Ship, ctx::VisibleOrbit>()) {
        // Get the model of the object
        entt::entity body_entity = ship;

        if (!universe.any_of<components::WorldModel>(body_entity)) {
            continue;
        }
        auto model_name = universe.get<components::WorldModel>(body_entity);
        glm::vec3 object_pos = controller.CalculateCenteredObject(body_entity);
        if (glm::distance(camera.cam_pos, object_pos) > 1000) {
            continue;
        }
        auto model = app.GetAssetManager().GetAsset<asset::Model>(model_name.name);
        glm::mat4 transform = glm::mat4(1.f);
        auto& kinematics = universe.get<types::Kinematics>(ship);
        auto& future_comp = universe.get<types::FuturePosition>(ship);
        const auto& pos = future_comp.position + kinematics.center;
        glm::vec3 future_pos = controller.CalculateCenteredObject(pos);
        transform = glm::rotate(transform, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        transform = glm::translate(transform, glm::mix(object_pos, future_pos, universe.tick_fraction));

        transform = glm::scale(transform, model->scale);
        model->shader->UseProgram();
        model->shader->SetMVP(transform, camera.camera_matrix, camera.projection);
        model->Draw();
    }
}

void SysStarSystemRenderer::DrawEntityName(glm::vec3& object_pos, entt::entity ent_id) {
    std::string text = GetName(universe, ent_id);
    glm::vec3 pos = GetBillboardPosition(object_pos);
    // Check if the position on screen is within bounds
    if (pos.z < 1 && pos.z > -1 &&
        (pos.x > 0 && pos.x < app.GetWindowWidth() && pos.y > 0 && pos.y < app.GetWindowHeight())) {
        app.DrawText(text, pos.x, pos.y, 20);
    }
}

void SysStarSystemRenderer::DrawPlanetBillboard(const entt::entity& ent_id, const glm::vec3& object_pos) {
    glm::vec3 pos = GetBillboardPosition(object_pos);

    // Check if the position on screen is within bounds
    if (GLPositionNotInBounds(CalculateGLPosition(object_pos), pos)) {
        return;
    }

    std::string text = GetName(universe, ent_id);

    glm::mat4 planetDispMat = GetBillboardMatrix(pos);

    SetBillboardProjection(planet_circle.shaderProgram, planetDispMat);

    engine::Draw(planet_circle);

    app.DrawText(text, pos.x, pos.y, 20);
}

void SysStarSystemRenderer::DrawCityIcon(const glm::vec3& object_pos, float alpha_value) {
    glm::vec3 pos = GetBillboardPosition(object_pos);
    if (pos.z >= 1 || pos.z <= -1) {
        return;
    }

    glm::mat4 planetDispMat = GetBillboardMatrix(pos);

    SetBillboardProjection(city.shaderProgram, planetDispMat);
    city.shaderProgram->Set("color", 1, 0, 1, alpha_value);

    engine::Draw(city);
}

void SysStarSystemRenderer::DrawAllCities(auto& bodies) {
    for (auto body_entity : bodies) {
        glm::vec3 object_pos = controller.CalculateCenteredObject(body_entity);
        RenderCities(object_pos, body_entity);
    }
}

void SysStarSystemRenderer::DrawShipIcon(const glm::vec3& object_pos) {
    glm::vec3 pos = GetBillboardPosition(object_pos);
    glm::mat4 shipDispMat = GetBillboardMatrix(pos);
    glm::vec4 gl_Position = CalculateGLPosition(object_pos);

    // Check if the position on screen is within bounds
    if (GLPositionNotInBounds(gl_Position, pos)) {
        return;
    }

    SetBillboardProjection(ship_overlay.shaderProgram, shipDispMat);
    engine::Draw(ship_overlay);
}

void SysStarSystemRenderer::DrawTexturedPlanet(const glm::vec3& object_pos, const entt::entity entity) {
    bool have_normal = false;
    bool have_roughness = false;
    bool have_province;
    GetPlanetTexture(entity, have_normal, have_roughness, have_province);

    this->have_province = have_province;

    auto& body = universe.get<bodies::Body>(entity);
    glm::mat4 position = glm::mat4(1.f);
    position = glm::translate(position, object_pos);
    position *= glm::mat4(controller.GetBodyRotation(body.axial, body.rotation, body.rotation_offset));

    // Rotate
    float scale = body.radius;
    position = glm::scale(position, glm::vec3(scale));

    auto shader = textured_planet.shaderProgram.get();
    if (scale < 10.f) {
        // then use different shader if it's close enough
        shader = near_shader.get();
        glDepthFunc(GL_ALWAYS);
    }
    LogGlError("Error when getting shader");
    shader->SetMVP(position, camera.camera_matrix, camera.projection);

    // Maybe a seperate shader for planets without normal maps would be better
    shader->setBool("haveNormal", have_normal);

    shader->setVec3("lightDir", glm::normalize(sun_position - object_pos));
    shader->setVec3("lightPosition", sun_position);
    shader->setVec3("lightColor", sun_color);
    shader->setVec3("viewPos", camera.cam_pos);

    // If a country is clicked on...
    shader->setBool("country", have_province);
    shader->setBool("is_roughness", have_roughness);

    engine::Draw(textured_planet, shader);
    glDepthFunc(GL_LESS);
}

void SysStarSystemRenderer::GetPlanetTexture(const entt::entity entity, bool& have_normal, bool& have_roughness,
                                             bool& have_province) {
    if (!universe.all_of<PlanetTexture>(entity)) {
        return;
    }
    auto& terrain_data = universe.get<PlanetTexture>(entity);
    textured_planet.textures.clear();
    textured_planet.textures.push_back(terrain_data.terrain);  // 0
    if (terrain_data.normal != nullptr) {                      // 1
        have_normal = true;
        textured_planet.textures.push_back(terrain_data.normal);
    } else {
        textured_planet.textures.push_back(terrain_data.terrain);
    }
    if (terrain_data.roughness != nullptr) {  // 2
        have_roughness = true;
        textured_planet.textures.push_back(terrain_data.roughness);
    } else {
        textured_planet.textures.push_back(terrain_data.terrain);
    }
    // Add province data if they have it
    have_province = (terrain_data.province_index_texture != nullptr);
    if (terrain_data.province_index_texture != nullptr) {  // 3
        textured_planet.textures.push_back(terrain_data.province_index_texture);
    } else {
        textured_planet.textures.push_back(dummy_index_texture);
    }

    if (terrain_data.province_color_map != nullptr) {  // 4
        textured_planet.textures.push_back(terrain_data.province_color_map);
    } else {
        textured_planet.textures.push_back(dummy_color_map);
    }
}

void SysStarSystemRenderer::DrawAllPlanets(auto& bodies) {
    ZoneScoped;
    for (entt::entity body_entity : bodies) {
        glm::vec3 object_pos = controller.CalculateCenteredObject(body_entity);

        // This can probably switched to some log system based off the mass of
        // a planet.
        //if (true) {
        // Check if planet has terrain or not
        // Don't actually use proc-gen terrain for now
        // if (universe.all_of<bodies::Terrain>(body_entity)) {
        // Do empty terrain
        // Check if the planet has the thing
        if (universe.all_of<bodies::TexturedTerrain>(body_entity)) {
            DrawTexturedPlanet(object_pos, body_entity);
        } else {
            DrawTerrainlessPlanet(body_entity, object_pos);
        }
        //}
    }
}

void SysStarSystemRenderer::DrawAllPlanetBillboards(auto& bodies) {
    ZoneScoped;
    planet_circle.shaderProgram->UseProgram();
    planet_circle.shaderProgram->setVec4("color", 0, 0, 1, 1);
    for (auto body_entity : bodies) {
        // Draw the planet circle
        glm::vec3 object_pos = controller.CalculateCenteredObject(body_entity);

        //if (true) {
        // Check if it's obscured by a planet, but eh, we can deal with
        // it later Set planet circle color
        // Get future position
        DrawPlanetBillboard(body_entity, object_pos);
        //continue;
        //}
    }
}

void SysStarSystemRenderer::DrawStar(const entt::entity& entity, glm::vec3& object_pos) {
    glm::mat4 position = glm::mat4(1.f);
    position = glm::translate(position, object_pos);

    glm::mat4 transform = glm::mat4(1.f);
    // Scale it by radius
    const double& radius = universe.get<Body>(entity).radius;
    double scale = radius;
    transform = glm::scale(transform, glm::vec3(scale, scale, scale));
    position = position * transform;

    sun.SetMVP(position, camera.camera_matrix, camera.projection);
    sun.shaderProgram->setVec4("color", 1, 1, 1, 1);
    engine::Draw(sun);
}

void SysStarSystemRenderer::DrawTerrainlessPlanet(const entt::entity& entity, glm::vec3& object_pos) {
    glm::mat4 position = glm::mat4(1.f);
    position = glm::translate(position, object_pos);
    float scale = 300;
    if (universe.any_of<Body>(entity)) {
        scale = universe.get<Body>(entity).radius;
    }

    position = glm::scale(position, glm::vec3(scale));
    glm::mat4 transform = glm::mat4(1.f);
    position = position * transform;

    sun.SetMVP(position, camera.camera_matrix, camera.projection);
    sun.shaderProgram->setVec4("color", 1, 0, 1, 1);
    engine::Draw(sun);
}

void SysStarSystemRenderer::RenderCities(glm::vec3& object_pos, const entt::entity& body_entity) {
    ZoneScoped;
    // Draw Cities
    if (!universe.all_of<Settlements>(body_entity)) {
        return;
    }
    std::vector<entt::entity> cities = universe.get<Settlements>(body_entity).settlements;
    if (cities.empty()) {
        return;
    }

    Body& body = universe.get<Body>(body_entity);
    auto quat = controller.GetBodyRotation(body.axial, body.rotation, body.rotation_offset);

    city.shaderProgram->UseProgram();
    city.shaderProgram->setVec4("color", 0.5, 0.5, 0.5, 1);
    for (auto city_entity : cities) {
        // Calculate position to render
        if (!universe.any_of<Offset>(city_entity)) {
            // Calculate offset
            continue;
        }
        Offset doffset = universe.get<Offset>(city_entity);
        glm::vec3 city_pos = universe.get<Offset>(city_entity).offset * (float)body.radius;
        // Check if line of sight and city position intersects the sphere that is the planet
        city_pos = quat * city_pos;
        glm::vec3 city_world_pos = city_pos + object_pos;
        if (CityIsVisible(city_world_pos, object_pos, camera.cam_pos)) {
            // If it's reasonably close, then we can show city names
            DrawEntityName(city_world_pos, city_entity);
            DrawCityIcon(city_world_pos,
                         Lerp(1, 0, (glm::length(camera.cam_pos - city_pos) - 6378.1 * 10) / (6378.1 * 10)));
        }
    }

    if (controller.ShouldDrawCityPrototype()) {
        DrawCityIcon(controller.GetMouseOnObjectPosition(), 1);
    }
}

bool SysStarSystemRenderer::CityIsVisible(glm::vec3 city_pos, glm::vec3 planet_pos, glm::vec3 cam_pos) {
    if (glm::length(cam_pos - city_pos) > 6378.1 * 100) {
        return false;
    }
    float dist = glm::dot((planet_pos - city_pos), (cam_pos - city_pos));
    return (dist < 0);
}

void SysStarSystemRenderer::LoadPlanetTextures() {
    for (auto body : universe.view<Orbit>()) {
        if (!universe.all_of<bodies::TexturedTerrain>(body)) {
            continue;
        }
        auto textures = universe.get<bodies::TexturedTerrain>(body);
        auto& data = universe.get_or_emplace<PlanetTexture>(body);
        data.terrain = app.GetAssetManager().GetAsset<Texture>(textures.terrain_name);
        if (!textures.normal_name.empty()) {
            data.normal = app.GetAssetManager().GetAsset<Texture>(textures.normal_name);
        }
        if (!textures.roughness_name.empty()) {
            data.roughness = app.GetAssetManager().GetAsset<Texture>(textures.roughness_name);
        }
        if (!universe.any_of<components::ProvincedPlanet>(body)) {
            continue;
        }
        auto& province_map = universe.get<components::ProvincedPlanet>(body);

        asset::BinaryAsset* bin_asset = app.GetAssetManager().GetAsset<asset::BinaryAsset>(province_map.province_map);
        if (bin_asset == nullptr) {
            SPDLOG_ERROR("Could not find the planet province map {}", province_map.province_map);
            continue;
        }
        // Then create vector
        uint64_t file_size = bin_asset->data.size();
        int comp = 0;
        stbi_set_flip_vertically_on_load(0);
        int province_width;
        int province_height;
        auto image_binary =
            stbi_load_from_memory(bin_asset->data.data(), file_size, &province_width, &province_height, &comp, 0);

        // Set country map
        data.province_map.reserve(static_cast<size_t>(province_height * province_width));
        data.province_indices.reserve(static_cast<size_t>(province_height * province_width));
        // Counter to assign to the array of colors
        uint16_t current_province_idx = 1;
        // We expect the province map will be the same dimensions as the province texture, so it should be fine?
        for (int idx = 0; idx < province_width * province_height; idx++) {
            // Position on the map
            int pos = idx * comp;
            std::tuple<int, int, int, int> t =
                std::make_tuple(image_binary[pos], image_binary[pos + 1], image_binary[pos + 2], image_binary[pos + 3]);
            int i = components::ProvinceColor::toInt(std::get<0>(t), std::get<1>(t), std::get<2>(t));
            if (universe.province_colors[body].find(i) != universe.province_colors[body].end()) {
                entt::entity province_id = universe.province_colors[body][i];
                data.province_map.push_back(province_id);
                if (!data.province_index_map.contains(province_id)) {
                    data.province_index_map[province_id] = current_province_idx;
                    current_province_idx++;
                }
                data.province_indices.push_back(data.province_index_map[province_id]);
            } else {
                // Most likely ocean
                // Maybe next time we should have ocean provinces
                data.province_map.push_back(entt::null);
                data.province_indices.push_back(0);
            }
        }
        stbi_image_free(image_binary);

        assert(data.province_indices.size() == province_width * province_height);
        GeneratePlanetProvinceMap(body, province_width, province_height, current_province_idx);

        data.has_provinces = true;
        data.width = province_width;
        data.height = province_height;
    }
}

void SysStarSystemRenderer::UpdatePlanetProvinceColors(entt::entity body, entt::entity province, glm::vec4 color) {
    // We should update the texture
    auto& data = universe.get<PlanetTexture>(body);
    if (!data.province_index_map.contains(province)) {
        return;
    }
    uint16_t province_idx = data.province_index_map[province];
    int stride = 4;
    data.province_colors[province_idx * stride] = color.r;
    data.province_colors[province_idx * stride + 1] = color.g;
    data.province_colors[province_idx * stride + 2] = color.b;
    data.province_colors[province_idx * stride + 3] = color.a;

    // Now we generate our province index map as an isampler2D
    // Now update the buffer
    unsigned int buf_id = dynamic_cast<asset::TBOTexture*>(data.province_color_map)->buffer_id;
    glBindBuffer(GL_TEXTURE_BUFFER, buf_id);
    void* ptr = glMapBuffer(GL_TEXTURE_BUFFER, GL_WRITE_ONLY);
    float* colors = static_cast<float*>(ptr);

    colors[province_idx * stride] = color.r;
    colors[province_idx * stride + 1] = color.g;
    colors[province_idx * stride + 2] = color.b;
    colors[province_idx * stride + 3] = color.a;
    glUnmapBuffer(GL_TEXTURE_BUFFER);
}

void SysStarSystemRenderer::MassUpdatePlanetProvinceColors(entt::entity entity) {
    // We reload the vector
    auto& data = universe.get<PlanetTexture>(entity);
    unsigned int buf_id = dynamic_cast<asset::TBOTexture*>(data.province_color_map)->buffer_id;
    glBindBuffer(GL_TEXTURE_BUFFER, buf_id);
    glBufferSubData(GL_TEXTURE_BUFFER, 0,
                    sizeof(decltype(data.province_colors)::value_type) * data.province_colors.size(),
                    static_cast<void*>(data.province_colors.data()));
}

/**
 * Generates the OpenGL textures for the province map and the colors we are to assign to the province map
 */
void SysStarSystemRenderer::GeneratePlanetProvinceMap(entt::entity entity, int province_width, int province_height,
                                                      uint16_t province_count) {
    assert(universe.all_of<PlanetTexture>(entity));
    auto& data = universe.get<PlanetTexture>(entity);

    // Generate int texture for assigning color indices
    unsigned int texid;
    glGenTextures(1, &texid);
    glBindTexture(GL_TEXTURE_2D, texid);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_R16UI, province_width, province_height, 0, GL_RED_INTEGER, GL_UNSIGNED_SHORT,
                 data.province_indices.data());
    data.province_index_texture = new Texture();
    data.province_index_texture->id = texid;
    data.province_index_texture->width = province_width;
    data.province_index_texture->height = province_height;
    data.province_index_texture->texture_type = GL_TEXTURE_2D;

    // Now let's generate our indices for the color for the province
    const size_t color_count = province_count * 4;
    data.province_colors.reserve(color_count);
    // Now let's just assign random colors...
    for (size_t i = 0; i < color_count; i++) {
        data.province_colors.push_back(0.f);
    }

    // Generate TBO
    GLuint tbo_buffer;
    glGenBuffers(1, &tbo_buffer);
    glBindBuffer(GL_TEXTURE_BUFFER, tbo_buffer);
    glBufferData(GL_TEXTURE_BUFFER, sizeof(decltype(data.province_colors)::value_type) * data.province_colors.size(),
                 static_cast<const void*>(data.province_colors.data()), GL_STATIC_DRAW);

    GLuint tbo_texture;
    glGenTextures(1, &tbo_texture);
    glBindTexture(GL_TEXTURE_BUFFER, tbo_texture);

    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, tbo_buffer);
    // Now let's save this texture
    data.province_color_map = new asset::TBOTexture();
    data.province_color_map->id = tbo_texture;
    dynamic_cast<asset::TBOTexture*>(data.province_color_map)->buffer_id = tbo_buffer;
    data.province_color_map->texture_type = GL_TEXTURE_BUFFER;
}

ShaderProgram_t SysStarSystemRenderer::ConstructShader(const std::string& key) {
    return app.GetAssetManager().GetAsset<ShaderDefinition>(key)->MakeShader();
}

void SysStarSystemRenderer::InitializeFramebuffers() {
    buffer_shader = ConstructShader("core:shader.framebuffer");

    ship_icon_layer = renderer.AddLayer<engine::FramebufferRenderer>(buffer_shader, *app.GetWindow());
    physical_layer = renderer.AddLayer<engine::FramebufferRenderer>(buffer_shader, *app.GetWindow());
    planet_icon_layer = renderer.AddLayer<engine::FramebufferRenderer>(buffer_shader, *app.GetWindow());
    skybox_layer = renderer.AddLayer<engine::FramebufferRenderer>(buffer_shader, *app.GetWindow());
}

void SysStarSystemRenderer::LoadProvinceMap() {}

void SysStarSystemRenderer::InitializeMeshes() {
    // Initialize meshes, etc
    engine::Mesh_t sphere_mesh = engine::primitive::ConstructSphereMesh(sphere_resolution, sphere_resolution);

    // Initialize shaders
    circle_shader = ConstructShader("core:shader.2dcolorshader");
    textured_planet_shader = ConstructShader("core:shader.planet_textureshader");
    near_shader = ConstructShader("core:shader.neartexturedobject");
    orbit_shader = ConstructShader("core:shader.orbitshader");
    vis_shader = ConstructShader("core:shader.vertex_vis");
    model_shader = ConstructShader("core:shader.model_pbr_log_shader");
    sun_shader = ConstructShader("core:shader.sunshader");
    skybox_shader = ConstructShader("core:shader.skybox");

    // Initialize sky box
    Texture* sky_texture = app.GetAssetManager().GetAsset<Texture>("core:skycubemap");
    sky.mesh = engine::primitive::MakeCube();
    sky.shaderProgram = skybox_shader;

    planet_circle.mesh = engine::primitive::CreateFilledCircle();
    planet_circle.shaderProgram = circle_shader;

    ship_overlay.mesh = engine::primitive::CreateFilledTriangle();
    ship_overlay.shaderProgram = circle_shader;

    city.mesh = engine::primitive::MakeTexturedPaneMesh();
    city.shaderProgram = circle_shader;

    // Planet spheres
    textured_planet.mesh = sphere_mesh;
    textured_planet.shaderProgram = textured_planet_shader;

    // Initialize sun
    sun.mesh = sphere_mesh;
    sun.shaderProgram = sun_shader;
}

glm::vec3 SysStarSystemRenderer::CalculateFutureObjectPos(const entt::entity& ent) {
    if (!universe.all_of<types::FuturePosition>(ent)) {
        return glm::vec3(0, 0, 0);
    }
    auto& kin = universe.get<types::FuturePosition>(ent);
    const auto& pos = kin.position + kin.center;
    return pos;
}

glm::vec3 SysStarSystemRenderer::TranslateToNormalized(const glm::vec3& pos) {
    return glm::vec3((pos.x / app.GetWindowWidth() - 0.5) * 2, (pos.y / app.GetWindowHeight() - 0.5) * 2, 0);
}

glm::vec4 SysStarSystemRenderer::CalculateGLPosition(const glm::vec3& object_pos) {
    glm::vec4 gl_Position = camera.projection * camera.camera_matrix * glm::vec4(object_pos, 1.0);
    float C = 0.0001;
    float far = 9.461e12;
    gl_Position.z = 2.0 * log(gl_Position.w * C + 1) / log(far * C + 1) - 1;

    gl_Position.z *= gl_Position.w;
    return gl_Position;
}

bool SysStarSystemRenderer::GLPositionNotInBounds(const glm::vec4& gl_Position, const glm::vec3& pos) {
    return (isnan(gl_Position.z)) || pos.x <= 0 || pos.x >= app.GetWindowWidth() || pos.y <= 0 ||
           pos.y >= app.GetWindowHeight();
}

glm::mat4 SysStarSystemRenderer::GetBillboardMatrix(const glm::vec3& pos) {
    glm::mat4 billboardDispMat = glm::mat4(1.0f);

    billboardDispMat = glm::translate(billboardDispMat, TranslateToNormalized(pos));
    billboardDispMat = glm::scale(billboardDispMat, glm::vec3(circle_size, circle_size, circle_size));
    billboardDispMat = glm::scale(billboardDispMat, glm::vec3(1, GetWindowRatio(), 1));
    return billboardDispMat;
}

glm::vec3 SysStarSystemRenderer::GetBillboardPosition(const glm::vec3& object_pos) {
    return glm::project(object_pos, camera.camera_matrix, camera.projection, camera.viewport);
}

void SysStarSystemRenderer::SetBillboardProjection(ShaderProgram_t& shader, glm::mat4 mat) {
    shader->UseProgram();
    shader->setMat4("model", mat);
    shader->setMat4("projection", glm::mat4(1.0));
}

float SysStarSystemRenderer::Lerp(float a, float b, float t) {
    if (t < 0) {
        return a;
    } else if (t > 1) {
        return b;
    }
    return std::lerp(a, b, t);
}

void SysStarSystemRenderer::SetupDummyTextures() {
    // Initialize dummy textures
    dummy_index_texture = new asset::Texture();
    dummy_index_texture->width = 1;
    dummy_index_texture->height = 1;
    dummy_index_texture->texture_type = GL_TEXTURE_2D;
    glGenTextures(1, &dummy_index_texture->id);
    glBindTexture(GL_TEXTURE_2D, dummy_index_texture->id);
    uint32_t zero = 0;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, 1, 1, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, &zero);

    dummy_color_map = new asset::TBOTexture();
    dummy_color_map->texture_type = GL_TEXTURE_BUFFER;
    glGenBuffers(1, &dummy_color_map->buffer_id);
    glBindBuffer(GL_TEXTURE_BUFFER, dummy_color_map->buffer_id);
    glm::vec3 dark(0, 0, 0);
    glBufferData(GL_TEXTURE_BUFFER, sizeof(glm::vec3), &dark, GL_STATIC_DRAW);
    glGenTextures(1, &dummy_color_map->id);
    glBindTexture(GL_TEXTURE_BUFFER, dummy_color_map->id);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32F, dummy_color_map->buffer_id);
}

void SysStarSystemRenderer::CheckResourceDistRender() {
#if FALSE
    // Then check if it's the same rendered object
    auto& rend = universe.get<PlanetTerrainRender>(m_viewing_entity);
    if (rend.resource == controller.terrain_displaying) {
        return;
    }

    // Check if it's the same
    if (!universe.any_of<components::ResourceDistribution>(m_viewing_entity)) {
        return;
    }

    auto& dist = universe.get<components::ResourceDistribution>(m_viewing_entity);
    TerrainImageGenerator gen;
    gen.terrain.seed = dist.dist[rend.resource];
    gen.GenerateHeightMap(3, 9);
    // Make the UI
    unsigned int a = GeneratePlanetTexture(gen.GetHeightMap());
    planet_resource = GenerateTexture(a, gen.GetHeightMap());
    controller.terrain_displaying = rend.resource;
    // Switch view mode
    planet.textures[0] = planet_resource;
    planet.shaderProgram = no_light_shader;
#endif
}

float SysStarSystemRenderer::GetWindowRatio() { return window_ratio; }

void SysStarSystemRenderer::DrawAllOrbits() {
    ZoneScoped;
    for (entt::entity orbit_entity : universe.view<Orbit>()) {
        // Check the type of orbiting things, and then don't render if it doesn't fit the filter
        if (universe.any_of<bodies::Planet>(orbit_entity)) {  // Always render planet orbits
            // Render no matter what
            DrawOrbit(orbit_entity);
            continue;
        }
        if (!universe.any_of<ctx::VisibleOrbit>(orbit_entity)) {  // Visible orbits will not be rendered
            continue;
        }
        DrawOrbit(orbit_entity);
    }
}

void SysStarSystemRenderer::DrawOrbit(const entt::entity& entity) {
    if (!universe.any_of<OrbitMesh>(entity)) {
        return;
    }
    glm::vec3 center = glm::vec3(0, 0, 0);
    // If it has a parent, draw around the parent
    entt::entity ref = universe.get<Orbit>(entity).reference_body;
    if (ref != entt::null) {
        center = controller.CalculateObjectPos(ref);
    } else {
        return;
    }
    glm::mat4 transform = glm::mat4(1.f);
    transform = glm::translate(transform, controller.CalculateCenteredObject(center));
    // Actually you just need to rotate the orbit
    auto body = universe.get<Body>(ref);
    //transform *= glm::mat4(
    //    glm::quat{{0.f, 0, (float)body.axial}});
    // Draw orbit
    orbit_shader->SetMVP(transform, camera.camera_matrix, app.Get3DProj());

    //Set the color of each orbit based on its distance from its center body

    auto& orb = universe.get<Orbit>(entity);

    const double dis = orb.semi_major_axis;
    const double max_dis = body.SOI;
    const double inc = orb.inclination;

    const float min_launch_dis = body.radius;
    float col = dis - min_launch_dis;
    float r = log(col) / log(max_dis);
    float g = 1 - r;
    float b = inc / 3.15;
    glm::vec4 color_v = {1.f, 1.f, 1.f, 0.7f};
    if (universe.any_of<ships::Ship>(entity)) {
        color_v = {0.10196078431372549f, 0.6313725490196078f, 0.24313725490196078f, 0.7f};
    }
    orbit_shader->Set("color", color_v);

    //orbit_shader->Set("color", glm::vec4(1, 1, 1, 1));
    // Set to the center of the universe
    auto& orbit = universe.get<OrbitMesh>(entity);

    orbit.orbit_mesh->Draw();
}

SysStarSystemRenderer::~SysStarSystemRenderer() {
    delete dummy_index_texture;
    delete dummy_color_map;
}
}  // namespace cqsp::client::systems
