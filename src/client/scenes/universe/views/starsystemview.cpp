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
#include "client/scenes/universe/views/starsystemview.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

#include "client/components/clientctx.h"
#include "client/components/planetrendering.h"
#include "client/scenes/universe/interface/systooltips.h"
#include "common/components/area.h"
#include "common/components/bodies.h"
#include "common/components/coordinates.h"
#include "common/components/name.h"
#include "common/components/orbit.h"
#include "common/components/organizations.h"
#include "common/components/player.h"
#include "common/components/resource.h"
#include "common/components/ships.h"
#include "common/components/surface.h"
#include "common/components/units.h"
#include "common/systems/actions/cityactions.h"
#include "common/util/nameutil.h"
#include "common/util/profiler.h"
#include "engine/graphics/primitives/cube.h"
#include "engine/graphics/primitives/line.h"
#include "engine/graphics/primitives/pane.h"
#include "engine/graphics/primitives/polygon.h"
#include "engine/graphics/primitives/uvsphere.h"
#include "engine/renderer/renderer.h"
#include "entt/entt.hpp"
#include "glad/glad.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm/gtx/string_cast.hpp"
#include "stb_image.h"  // NOLINT: STB is rather annoying
#include "tracy/Tracy.hpp"

namespace cqsp::client::systems {

namespace components = cqsp::common::components;
namespace bodies = components::bodies;
namespace types = components::types;
namespace ships = components::ships;

using asset::ShaderDefinition;
using asset::ShaderProgram_t;
using asset::Texture;
using bodies::Body;
using bodies::LightEmitter;
using bodies::Planet;
using bodies::Star;
using bodies::TexturedTerrain;
using common::util::GetName;
using components::Habitation;
using components::Name;
using components::Province;
using components::Surface;
using cqsp::client::components::PlanetTerrainRender;
using entt::entity;
using glm::mat3;
using glm::mat4;
using glm::vec3;
using glm::vec4;
using ships::Ship;
using types::Kinematics;
using types::Orbit;
using types::SurfaceCoordinate;

SysStarSystemRenderer::SysStarSystemRenderer(common::Universe& _u, engine::Application& _a)
    : m_universe(_u),
      m_app(_a),
      scroll(5),
      view_x(0),
      view_y(0),
      view_center(vec3(1, 1, 1)),
      sun_color(vec3(10, 10, 10)) {}
struct Offset {
    vec3 offset;
};

struct TerrainTextureData {
    Texture* terrain_albedo = nullptr;
    Texture* heightmap = nullptr;

    void DeleteData() {
        delete terrain_albedo;
        delete heightmap;
    }
};

struct PlanetTexture {
    Texture* terrain = nullptr;
    Texture* normal = nullptr;
    Texture* roughness = nullptr;
    Texture* province_texture = nullptr;
    std::vector<entity> province_map;
};

struct PlanetOrbit {
    engine::Mesh_t orbit_mesh;

    ~PlanetOrbit() = default;
};

void SysStarSystemRenderer::Initialize() {
    InitializeMeshes();
    InitializeFramebuffers();

    LoadProvinceMap();

    // Zoom into the player's capital city
    entity player = m_universe.view<components::Player>().front();
    entity player_capital = m_universe.get<components::Country>(player).capital_city;
    if (player_capital != entt::null) {
        // Zoom into the thing
        m_universe.emplace_or_replace<FocusedCity>(player_capital);
    }
}

void SysStarSystemRenderer::OnTick() {
    entity current_planet = m_universe.view<FocusedPlanet>().front();
    if (current_planet != entt::null) {
        view_center = CalculateObjectPos(m_viewing_entity);
    }
}

void SysStarSystemRenderer::Render(float deltaTime) {
    ZoneScoped;
    // Seeing new planet
    entity current_planet = m_universe.view<FocusedPlanet>().front();
    if (current_planet != m_viewing_entity && current_planet != entt::null) {
        SPDLOG_INFO("Switched displaying planet, seeing {}", current_planet);
        m_viewing_entity = current_planet;
        // Do terrain
        SeeEntity();
    }

    FocusCityView();

    // Check for resized window
    window_ratio = static_cast<float>(m_app.GetWindowWidth()) / static_cast<float>(m_app.GetWindowHeight());

    GenerateOrbitLines();

    renderer.NewFrame(*m_app.GetWindow());

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    CalculateCamera();

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
    vis_shader->SetMVP(mat4(1.f), mat4(1.f), projection);
}

void SysStarSystemRenderer::SeeStarSystem() {
    GenerateOrbitLines();

    SPDLOG_INFO("Loading planet textures");
    LoadPlanetTextures();

    // Set scroll to radius of sun
    if (m_universe.any_of<Body>(m_universe.sun)) {
        scroll = m_universe.get<Body>(m_universe.sun).radius * 5;
    }
}

void SysStarSystemRenderer::SeeEntity() {
    // See the object
    view_center = CalculateObjectPos(m_viewing_entity);

    // Set the variable
    if (m_universe.all_of<Body>(m_viewing_entity)) {
        scroll = m_universe.get<Body>(m_viewing_entity).radius * 2.5;
        if (scroll < 0.1) scroll = 0.1;
    } else {
        scroll = 5;
    }
    CalculateCityPositions();
}

void SysStarSystemRenderer::Update(float deltaTime) {
    ZoneScoped;
    double deltaX = previous_mouseX - m_app.GetMouseX();
    double deltaY = previous_mouseY - m_app.GetMouseY();

    is_founding_city = IsFoundingCity(m_universe);

    CityDetection();

    // Discern between clicking on UI and game
    if (!ImGui::GetIO().WantCaptureMouse && !m_app.GetRmlUiContext()->IsMouseInteracting()) {
        CalculateScroll();

        CalculateViewChange(deltaX, deltaY);

        previous_mouseX = m_app.GetMouseX();
        previous_mouseY = m_app.GetMouseY();

        // If clicks on object, go to the planet
        entity ent = m_universe.view<MouseOverEntity>().front();
        if (m_app.MouseButtonIsReleased(engine::MouseInput::LEFT) && ent != entt::null && !m_app.MouseDragged()) {
            FocusOnEntity(ent);

            if (is_founding_city) {
                FoundCity();
            }
        }
        // Some math if you're close enough you select the city instead of the planet
    }

    if (!ImGui::GetIO().WantCaptureKeyboard) {
        MoveCamera(deltaTime);
    }

    mouse_on_object = GetMouseIntersectionOnObject(m_app.GetMouseX(), m_app.GetMouseY());

    /*
    // Check if it has terrain resource rendering, and make terrain thing
    if (m_viewing_entity != entt::null && m_universe.all_of<PlanetTerrainRender>(m_viewing_entity)) {
        CheckResourceDistRender();
    } else if (m_viewing_entity != entt::null) {
        // Reset to default
        planet.textures.reserve(2);
        planet.textures[0] = planet_texture;
        terrain_displaying = entt::null;
        // Also change up the shader
        planet.shaderProgram = pbr_shader;
    }*/
    // Calculate camera
    CenterCameraOnCity();
}

void SysStarSystemRenderer::SeePlanet(entt::entity ent) {
    m_universe.clear<FocusedPlanet>();
    m_universe.emplace<FocusedPlanet>(ent);
}

void SysStarSystemRenderer::DoUI(float deltaTime) {
#ifdef NDEBUG
    return;
#endif
    RenderInformationWindow(deltaTime);
    RenderSelectedObjectInformation();
}

void SysStarSystemRenderer::DrawStars() {
    ZoneScoped;
    // Draw stars
    auto stars = m_universe.view<Body, LightEmitter>();
    renderer.BeginDraw(physical_layer);
    for (auto ent_id : stars) {
        // Draw the star circle
        vec3 object_pos = CalculateCenteredObject(ent_id);
        sun_position = object_pos;
        DrawStar(ent_id, object_pos);
    }
    renderer.EndDraw(physical_layer);
}

void SysStarSystemRenderer::DrawBodies() {
    ZoneScoped;
    // Draw other bodies
    auto bodies = m_universe.view<Body>(entt::exclude<LightEmitter>);
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
    auto ships = m_universe.view<Ship, ctx::VisibleOrbit>();

    renderer.BeginDraw(ship_icon_layer);
    ship_overlay.shaderProgram->UseProgram();
    for (auto ent_id : ships) {
        // if it's not visible, then don't render
        vec3 object_pos = CalculateCenteredObject(ent_id);
        ship_overlay.shaderProgram->setVec4("color", 1, 0, 0, 1);
        // Interpolate so that it looks nice
        if (m_universe.any_of<types::FuturePosition>(ent_id)) {
            auto& future_comp = m_universe.get<types::FuturePosition>(ent_id);
            const auto& pos = future_comp.position + future_comp.center;
            vec3 future_pos = CalculateCenteredObject(ConvertPoint(pos));
            DrawShipIcon(glm::mix(object_pos, future_pos, m_universe.tick_fraction));
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
    sky.shaderProgram->setMat4("view", mat4(mat3(camera_matrix)));
    sky.shaderProgram->setMat4("projection", projection);
    glDepthFunc(GL_LEQUAL);
    // skybox cube
    engine::Draw(sky);
    glDepthFunc(GL_LESS);
    renderer.EndDraw(skybox_layer);
}

void SysStarSystemRenderer::DrawModels() {
    auto ships = m_universe.view<Ship, ctx::VisibleOrbit>();
    // Loop through the space bodies that are close
    for (entity body_entity : ships) {
        vec3 object_pos = CalculateCenteredObject(body_entity);
        if (glm::distance(cam_pos, object_pos) > 1000) {
            continue;
        }
        mat4 transform = mat4(1.f);
        transform = glm::translate(transform, object_pos);
        transform = glm::scale(transform, vec3(20));
        model_shader->UseProgram();
        model_shader->SetMVP(transform, camera_matrix, projection);
        iss_model->Draw(model_shader.get());
    }
}

void SysStarSystemRenderer::DrawEntityName(vec3& object_pos, entity ent_id) {
    std::string text = GetName(m_universe, ent_id);
    vec3 pos = GetBillboardPosition(object_pos);
    // Check if the position on screen is within bounds
    if (pos.z < 1 && pos.z > -1 &&
        (pos.x > 0 && pos.x < m_app.GetWindowWidth() && pos.y > 0 && pos.y < m_app.GetWindowHeight())) {
        m_app.DrawText(text, pos.x, pos.y, 20);
    }
}

void SysStarSystemRenderer::DrawPlanetIcon(vec3& object_pos) {
    mat4 planetDispMat = GetBillboardMatrix(GetBillboardPosition(object_pos));

    SetBillboardProjection(planet_circle.shaderProgram, planetDispMat);
    engine::Draw(planet_circle);
}

void SysStarSystemRenderer::DrawPlanetBillboards(const entity& ent_id, const vec3& object_pos) {
    vec3 pos = GetBillboardPosition(object_pos);
    vec4 gl_Position = CalculateGLPosition(object_pos);

    // Check if the position on screen is within bounds
    if (GLPositionNotInBounds(gl_Position, pos)) {
        return;
    }

    std::string text = GetName(m_universe, ent_id);

    mat4 planetDispMat = GetBillboardMatrix(pos);

    SetBillboardProjection(planet_circle.shaderProgram, planetDispMat);

    engine::Draw(planet_circle);

    m_app.DrawText(text, pos.x, pos.y, 20);
}

void SysStarSystemRenderer::DrawCityIcon(const vec3& object_pos) {
    vec3 pos = GetBillboardPosition(object_pos);
    if (pos.z >= 1 || pos.z <= -1) {
        return;
    }

    mat4 planetDispMat = GetBillboardMatrix(pos);
    // Scale it by the window ratio
    planetDispMat = glm::scale(planetDispMat, vec3(1, GetWindowRatio(), 1));

    SetBillboardProjection(city.shaderProgram, planetDispMat);
    city.shaderProgram->Set("color", 1, 0, 1, 1);

    engine::Draw(city);
}

void SysStarSystemRenderer::DrawAllCities(auto& bodies) {
    for (auto body_entity : bodies) {
        vec3 object_pos = CalculateCenteredObject(body_entity);
        // if (glm::distance(object_pos, cam_pos) <= dist) {
        RenderCities(object_pos, body_entity);
        //}
    }
}

void SysStarSystemRenderer::DrawShipIcon(const vec3& object_pos) {
    vec3 pos = GetBillboardPosition(object_pos);
    mat4 shipDispMat = GetBillboardMatrix(pos);
    float window_ratio = GetWindowRatio();
    vec4 gl_Position = CalculateGLPosition(object_pos);

    // Check if the position on screen is within bounds
    if (GLPositionNotInBounds(gl_Position, pos)) {
        return;
    }

    shipDispMat = glm::scale(shipDispMat, vec3(1, window_ratio, 1));
    SetBillboardProjection(ship_overlay.shaderProgram, shipDispMat);
    engine::Draw(ship_overlay);
}

void SysStarSystemRenderer::DrawTexturedPlanet(const vec3& object_pos, const entity entity) {
    bool have_normal = false;
    bool have_roughness = false;
    bool have_province;
    GetPlanetTexture(entity, have_normal, have_roughness, have_province);

    auto& body = m_universe.get<Body>(entity);

    mat4 position = mat4(1.f);
    position = glm::translate(position, object_pos);
    position *= mat4(GetBodyRotation(body.axial, body.rotation, body.rotation_offset));

    // Rotate
    float scale = body.radius;  // cqsp::common::components::types::toAU(body.radius)
                                // * view_scale;
    position = glm::scale(position, vec3(scale));

    auto shader = textured_planet.shaderProgram.get();
    if (scale < 10.f) {
        // then use different shader if it's close enough
        shader = near_shader.get();
        glDepthFunc(GL_ALWAYS);
    }

    shader->SetMVP(position, camera_matrix, projection);

    // Maybe a seperate shader for planets without normal maps would be better
    shader->setBool("haveNormal", have_normal);

    shader->setVec3("lightDir", glm::normalize(sun_position - object_pos));
    shader->setVec3("lightPosition", sun_position);
    shader->setVec3("lightColor", sun_color);
    shader->setVec3("viewPos", cam_pos);

    // If a country is clicked on...
    shader->setVec4("country_color", vec4(selected_country_color, 1));
    shader->setBool("country", have_province);
    shader->setBool("is_roughness", have_roughness);

    engine::Draw(textured_planet, shader);
    glDepthFunc(GL_LESS);
}

void SysStarSystemRenderer::GetPlanetTexture(const entity entity, bool& have_normal, bool& have_roughness,
                                             bool& have_province) {
    if (!m_universe.all_of<PlanetTexture>(entity)) {
        return;
    }
    auto& terrain_data = m_universe.get<PlanetTexture>(entity);
    textured_planet.textures.clear();
    textured_planet.textures.push_back(terrain_data.terrain);
    if (terrain_data.normal != nullptr) {
        have_normal = true;
        textured_planet.textures.push_back(terrain_data.normal);
    } else {
        textured_planet.textures.push_back(terrain_data.terrain);
    }
    if (terrain_data.roughness != nullptr) {
        have_roughness = true;
        textured_planet.textures.push_back(terrain_data.roughness);
    }
    // Add province data if they have it
    have_province = (terrain_data.province_texture != nullptr);
    if (terrain_data.province_texture != nullptr) {
        textured_planet.textures.push_back(terrain_data.province_texture);
    }
}

void SysStarSystemRenderer::DrawAllPlanets(auto& bodies) {
    ZoneScoped;
    for (entity body_entity : bodies) {
        vec3 object_pos = CalculateCenteredObject(body_entity);
        // This can probably switched to some log system based off the mass of
        // a planet.
        //if (true) {
        // Check if planet has terrain or not
        // Don't actually use proc-gen terrain for now
        // if (m_universe.all_of<cqspb::Terrain>(body_entity)) {
        // Do empty terrain
        // Check if the planet has the thing
        if (m_universe.all_of<TexturedTerrain>(body_entity)) {
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
        vec3 object_pos = CalculateCenteredObject(body_entity);

        //if (true) {
        // Check if it's obscured by a planet, but eh, we can deal with
        // it later Set planet circle color
        DrawPlanetBillboards(body_entity, object_pos);
        //continue;
        //}
    }
}

void SysStarSystemRenderer::DrawStar(const entity& entity, vec3& object_pos) {
    mat4 position = mat4(1.f);
    position = glm::translate(position, object_pos);

    mat4 transform = mat4(1.f);
    // Scale it by radius
    const double& radius = m_universe.get<Body>(entity).radius;
    double scale = radius;
    transform = glm::scale(transform, vec3(scale, scale, scale));
    position = position * transform;

    sun.SetMVP(position, camera_matrix, projection);
    sun.shaderProgram->setVec4("color", 1, 1, 1, 1);
    engine::Draw(sun);
}

void SysStarSystemRenderer::DrawTerrainlessPlanet(const entity& entity, vec3& object_pos) {
    mat4 position = mat4(1.f);
    position = glm::translate(position, object_pos);
    float scale = 300;
    if (m_universe.any_of<Body>(entity)) {
        scale = m_universe.get<Body>(entity).radius;
    }

    position = glm::scale(position, vec3(scale));
    mat4 transform = mat4(1.f);
    position = position * transform;

    sun.SetMVP(position, camera_matrix, projection);
    sun.shaderProgram->setVec4("color", 1, 0, 1, 1);
    engine::Draw(sun);
}

void SysStarSystemRenderer::RenderCities(vec3& object_pos, const entity& body_entity) {
    ZoneScoped;
    // Draw Cities
    if (!m_universe.all_of<Habitation>(body_entity)) {
        return;
    }
    std::vector<entity> cities = m_universe.get<Habitation>(body_entity).settlements;
    if (cities.empty()) {
        return;
    }

    auto& body = m_universe.get<Body>(body_entity);
    auto quat = GetBodyRotation(body.axial, body.rotation, body.rotation_offset);

    city.shaderProgram->UseProgram();
    city.shaderProgram->setVec4("color", 0.5, 0.5, 0.5, 1);
    for (auto city_entity : cities) {
        // Calculate position to render
        if (!m_universe.any_of<Offset>(city_entity)) {
            // Calculate offset
            continue;
        }
        Offset doffset = m_universe.get<Offset>(city_entity);
        vec3 city_pos = m_universe.get<Offset>(city_entity).offset * (float)body.radius;
        // Check if line of sight and city position intersects the sphere that is the planet
        city_pos = quat * city_pos;
        vec3 city_world_pos = city_pos + object_pos;
        if (CityIsVisible(city_world_pos, object_pos, cam_pos)) {
            // If it's reasonably close, then we can show city names
            //if (scroll < 3) {
            DrawEntityName(city_world_pos, city_entity);
            //}
            DrawCityIcon(city_world_pos);
        }
    }

    if (is_founding_city && is_rendering_founding_city) {
        DrawCityIcon(GetMouseOnObject());
    }
}

bool SysStarSystemRenderer::CityIsVisible(vec3 city_pos, vec3 planet_pos, vec3 cam_pos) {
    float dist = glm::dot((planet_pos - city_pos), (cam_pos - city_pos));
    return (dist < 0);
}

void SysStarSystemRenderer::CalculateCityPositions() {
    // Calculate offset for all cities on planet if they exist
    if (!m_universe.valid(m_viewing_entity)) {
        return;
    }
    if (!m_universe.all_of<Habitation>(m_viewing_entity)) {
        return;
    }
    std::vector<entity> cities = m_universe.get<Habitation>(m_viewing_entity).settlements;
    if (cities.empty()) {
        return;
    }
    for (auto& city_entity : cities) {
        if (!m_universe.all_of<SurfaceCoordinate>(city_entity)) {
            continue;
        }
        auto& coord = m_universe.get<SurfaceCoordinate>(city_entity);
        Body parent = m_universe.get<Body>(m_viewing_entity);
        m_universe.emplace_or_replace<Offset>(city_entity, types::toVec3(coord.universe_view(), 1));
    }
    SPDLOG_INFO("Calculated offset");
}

void SysStarSystemRenderer::CalculateScroll() {
    double min_scroll;
    if (m_viewing_entity == entt::null || !m_universe.all_of<Body>(m_viewing_entity)) {
        // Scroll i
        min_scroll = 0.1;
    } else {
        min_scroll = std::max(m_universe.get<Body>(m_viewing_entity).radius * 1.1, 0.1);
    }
    if (scroll - m_app.GetScrollAmount() * 3 * scroll / 33 <= min_scroll) {
        return;
    }
    scroll -= m_app.GetScrollAmount() * 3 * scroll / 33;
}

void SysStarSystemRenderer::LoadPlanetTextures() {
    for (auto body : m_universe.view<Orbit>()) {
        if (!m_universe.all_of<TexturedTerrain>(body)) {
            continue;
        }
        auto textures = m_universe.get<TexturedTerrain>(body);
        auto& data = m_universe.get_or_emplace<PlanetTexture>(body);
        data.terrain = m_app.GetAssetManager().GetAsset<Texture>(textures.terrain_name);
        if (!textures.normal_name.empty()) {
            data.normal = m_app.GetAssetManager().GetAsset<Texture>(textures.normal_name);
        }
        if (!textures.roughness_name.empty()) {
            data.roughness = m_app.GetAssetManager().GetAsset<Texture>(textures.roughness_name);
        }
        if (!m_universe.any_of<common::components::ProvincedPlanet>(body)) {
            continue;
        }
        auto& province_map = m_universe.get<common::components::ProvincedPlanet>(body);
        // Add province data if they have it
        data.province_texture = m_app.GetAssetManager().GetAsset<Texture>(province_map.province_texture);

        asset::BinaryAsset* bin_asset =
            m_app.GetAssetManager().GetAsset<cqsp::asset::BinaryAsset>(province_map.province_map);
        // Then create vector
        uint64_t file_size = bin_asset->data.size();
        int comp = 0;
        auto d = stbi_load_from_memory(bin_asset->data.data(), file_size, &province_width, &province_height, &comp, 0);

        // Set country map
        data.province_map.reserve(static_cast<long>(province_height * province_width));
        for (int x = 0; x < province_width; x++) {
            for (int y = 0; y < province_height; y++) {
                // Then get from the maps
                int pos = (x * province_height + y) * comp;
                std::tuple<int, int, int, int> t = std::make_tuple(d[pos], d[pos + 1], d[pos + 2], d[pos + 3]);
                int i = common::components::ProvinceColor::toInt(std::get<0>(t), std::get<1>(t), std::get<2>(t));
                if (m_universe.province_colors.find(i) != m_universe.province_colors.end()) {
                    data.province_map.push_back(m_universe.province_colors[i]);
                } else {
                    data.province_map.push_back(entt::null);
                }
            }
        }
        delete d;
    }
}

void SysStarSystemRenderer::InitializeFramebuffers() {
    auto buffer_shader = m_app.GetAssetManager().GetAsset<asset::ShaderDefinition>("core:framebuffer")->MakeShader();

    ship_icon_layer = renderer.AddLayer<engine::FramebufferRenderer>(buffer_shader, *m_app.GetWindow());
    physical_layer = renderer.AddLayer<engine::FramebufferRenderer>(buffer_shader, *m_app.GetWindow());
    planet_icon_layer = renderer.AddLayer<engine::FramebufferRenderer>(buffer_shader, *m_app.GetWindow());
    skybox_layer = renderer.AddLayer<engine::FramebufferRenderer>(buffer_shader, *m_app.GetWindow());
}

void SysStarSystemRenderer::LoadProvinceMap() {}

void SysStarSystemRenderer::InitializeMeshes() {
    // Initialize meshes, etc
    engine::Mesh_t sphere_mesh = cqsp::engine::primitive::ConstructSphereMesh(sphere_resolution, sphere_resolution);

    // Initialize sky box
    Texture* sky_texture = m_app.GetAssetManager().GetAsset<Texture>("core:skycubemap");

    sky.mesh = engine::primitive::MakeCube();
    sky.shaderProgram = m_app.GetAssetManager().GetAsset<ShaderDefinition>("core:skybox")->MakeShader();

    asset::ShaderProgram_t circle_shader =
        m_app.GetAssetManager().GetAsset<ShaderDefinition>("core:2dcolorshader")->MakeShader();

    planet_circle.mesh = engine::primitive::CreateFilledCircle();
    planet_circle.shaderProgram = circle_shader;

    ship_overlay.mesh = engine::primitive::CreateFilledTriangle();
    ship_overlay.shaderProgram = circle_shader;

    city.mesh = engine::primitive::MakeTexturedPaneMesh();
    city.shaderProgram = circle_shader;

    // Initialize shaders
    ShaderProgram_t planet_shader =
        m_app.GetAssetManager().GetAsset<ShaderDefinition>("core:planetshader")->MakeShader();

    // Planet spheres
    planet.mesh = sphere_mesh;
    planet.shaderProgram = planet_shader;

    // Initialize shaders
    ShaderProgram_t textured_planet_shader =
        m_app.GetAssetManager().GetAsset<ShaderDefinition>("core:planet_textureshader")->MakeShader();
    near_shader = m_app.GetAssetManager().GetAsset<ShaderDefinition>("core:neartexturedobject")->MakeShader();
    textured_planet.mesh = sphere_mesh;
    textured_planet.shaderProgram = textured_planet_shader;

    // Initialize sun
    sun.mesh = sphere_mesh;
    sun.shaderProgram = m_app.GetAssetManager().GetAsset<ShaderDefinition>("core:sunshader")->MakeShader();

    orbit_shader = m_app.GetAssetManager().GetAsset<ShaderDefinition>("core:orbitshader")->MakeShader();
    vis_shader = m_app.GetAssetManager().GetAsset<ShaderDefinition>("core:vertex_vis")->MakeShader();
    iss_model = m_app.GetAssetManager().GetAsset<asset::Model>("core:iss");
    model_shader = m_app.GetAssetManager().GetAsset<ShaderDefinition>("core:model_log_shader")->MakeShader();
}

glm::quat SysStarSystemRenderer::GetBodyRotation(double axial, double rotation, double day_offset) {
    // Need to interpolate between the frames
    float rot = (float)components::bodies::GetPlanetRotationAngle(
        m_universe.date.ToSecond() + m_universe.tick_fraction * components::StarDate::TIME_INCREMENT, rotation,
        day_offset);
    if (rotation == 0) {
        rot = 0;
    }
    return glm::quat {{0, 0, (float)-axial}} * glm::quat {{0, (float)std::fmod(rot, types::TWOPI), 0}};
}

void SysStarSystemRenderer::FocusCityView() {
    ZoneScoped;
    auto focused_city_view = m_universe.view<FocusedCity>();
    // City to focus view on
    if (focused_city_view.empty()) {
        return;
    }
    selected_city = focused_city_view.front();
    m_universe.clear<FocusedCity>();

    CenterCameraOnCity();
    entt::entity planet = m_universe.view<FocusedPlanet>().front();
    auto& body = m_universe.get<Body>(planet);
    // 100 km above the city
    scroll = body.radius + 100;
}

vec3 SysStarSystemRenderer::CalculateObjectPos(const entity& ent) {
    // Get the position
    if (!m_universe.all_of<Kinematics>(ent)) {
        return vec3(0, 0, 0);
    }
    auto& kin = m_universe.get<Kinematics>(ent);
    const auto& pos = kin.position + kin.center;
    return ConvertPoint(pos);
}

vec3 SysStarSystemRenderer::CalculateCenteredObject(const vec3& vec) { return vec - view_center; }

vec3 SysStarSystemRenderer::TranslateToNormalized(const vec3& pos) {
    return vec3((pos.x / m_app.GetWindowWidth() - 0.5) * 2, (pos.y / m_app.GetWindowHeight() - 0.5) * 2, 0);
}

vec3 SysStarSystemRenderer::ConvertPoint(const vec3& pos) { return vec3(pos.x, pos.z, -pos.y); }

vec4 SysStarSystemRenderer::CalculateGLPosition(const vec3& object_pos) {
    vec4 gl_Position = projection * camera_matrix * vec4(object_pos, 1.0);
    float C = 0.0001;
    float far = 9.461e12;
    gl_Position.z = 2.0 * log(gl_Position.w * C + 1) / log(far * C + 1) - 1;

    gl_Position.z *= gl_Position.w;
    return gl_Position;
}

bool SysStarSystemRenderer::GLPositionNotInBounds(const vec4& gl_Position, const vec3& pos) {
    return (isnan(gl_Position.z)) || pos.x <= 0 || pos.x >= m_app.GetWindowWidth() || pos.y <= 0 ||
           pos.y >= m_app.GetWindowHeight();
}

mat4 SysStarSystemRenderer::GetBillboardMatrix(const vec3& pos) {
    mat4 billboardDispMat = mat4(1.0f);

    billboardDispMat = glm::translate(billboardDispMat, TranslateToNormalized(pos));
    billboardDispMat = glm::scale(billboardDispMat, vec3(circle_size, circle_size, circle_size));
    return billboardDispMat;
}

vec3 SysStarSystemRenderer::GetBillboardPosition(const vec3& object_pos) {
    return glm::project(object_pos, camera_matrix, projection, viewport);
}

void SysStarSystemRenderer::SetBillboardProjection(cqsp::asset::ShaderProgram_t& shader, mat4 mat) {
    shader->UseProgram();
    shader->setMat4("model", mat);
    shader->setMat4("projection", mat4(1.0));
}

void SysStarSystemRenderer::CenterCameraOnCity() {
    if (selected_city == entt::null) {
        return;
    }

    if (!m_universe.any_of<SurfaceCoordinate>(selected_city)) {
        return;
    }

    auto& surf = m_universe.get<SurfaceCoordinate>(selected_city);
    // TODO(EhWhoAmI): Change this so that it doesn't have to change the
    // coordinate system multiple times. Currently this changes from surface
    // coordinates to 3d coordinates to surface coordinates. I think it can be
    // solved with a basic formula.
    entity planet = m_universe.view<FocusedPlanet>().front();

    if (!m_universe.any_of<Body>(planet)) {
        return;
    }
    auto& body = m_universe.get<Body>(planet);

    glm::quat quat = GetBodyRotation(body.axial, body.rotation, body.rotation_offset);

    vec3 vec = types::toVec3(surf.universe_view(), 1);
    auto s = quat * vec;
    vec3 pos = glm::normalize(s);
    view_x = atan2(s.x, s.z);
    view_y = -acos(s.y) + types::PI / 2;
}

vec3 SysStarSystemRenderer::CalculateCenteredObject(const entity& ent) {
    return CalculateCenteredObject(CalculateObjectPos(ent));
}

void SysStarSystemRenderer::CalculateCamera() {
    cam_pos = vec3(cos(view_y) * sin(view_x), sin(view_y), cos(view_y) * cos(view_x)) * (float)scroll;
    cam_up = vec3(0.0f, 1.0f, 0.0f);
    camera_matrix = glm::lookAt(cam_pos, vec3(0.f, 0.f, 0.f), cam_up);
    projection = glm::infinitePerspective(glm::radians(45.f), GetWindowRatio(), 0.1f);
    viewport = vec4(0.f, 0.f, m_app.GetWindowWidth(), m_app.GetWindowHeight());
}

void SysStarSystemRenderer::MoveCamera(double deltaTime) {
    // Now navigation for changing the center
    vec3 dir = (view_center - cam_pos);
    float velocity = deltaTime * 30.f * scroll / 40;
    // Get distance from the pane
    // Remove y axis
    vec3 forward = glm::normalize(vec3(glm::sin(view_x), 0, glm::cos(view_x)));
    vec3 right = glm::normalize(glm::cross(forward, cam_up));
    auto post_move = [&]() {
        m_universe.clear<FocusedPlanet>();
        m_viewing_entity = entt::null;
    };
    if (m_app.ButtonIsHeld(engine::KeyInput::KEY_W)) {
        // Get direction
        view_center -= forward * velocity;
        post_move();
    }
    if (m_app.ButtonIsHeld(engine::KeyInput::KEY_S)) {
        view_center += forward * velocity;
        post_move();
    }
    if (m_app.ButtonIsHeld(engine::KeyInput::KEY_A)) {
        view_center += right * velocity;
        post_move();
    }
    if (m_app.ButtonIsHeld(engine::KeyInput::KEY_D)) {
        view_center -= right * velocity;
        post_move();
    }
}

void SysStarSystemRenderer::CheckResourceDistRender() {
#if FALSE
    using components::PlanetTerrainRender;
    // Then check if it's the same rendered object
    auto& rend = m_universe.get<PlanetTerrainRender>(m_viewing_entity);
    if (rend.resource == terrain_displaying) {
        return;
    }

    // Check if it's the same
    using components::ResourceDistribution;
    if (!m_universe.any_of<ResourceDistribution>(m_viewing_entity)) {
        return;
    }

    auto& dist = m_universe.get<ResourceDistribution>(m_viewing_entity);
    TerrainImageGenerator gen;
    gen.terrain.seed = dist.dist[rend.resource];
    gen.GenerateHeightMap(3, 9);
    // Make the UI
    unsigned int a = GeneratePlanetTexture(gen.GetHeightMap());
    planet_resource = GenerateTexture(a, gen.GetHeightMap());
    terrain_displaying = rend.resource;
    // Switch view mode
    planet.textures[0] = planet_resource;
    planet.shaderProgram = no_light_shader;
#endif
}

vec3 SysStarSystemRenderer::CalculateMouseRay(const vec3& ray_nds) {
    vec4 ray_clip = vec4(ray_nds.x, ray_nds.y, -1.0, 1.0);
    vec4 ray_eye = glm::inverse(projection) * ray_clip;
    ray_eye = vec4(ray_eye.x, ray_eye.y, -1.0, 0.0);
    vec4 inv = (glm::inverse(camera_matrix) * ray_eye);

    // Normalize vector
    return glm::normalize(vec3(inv.x, inv.y, inv.z));
}

void SysStarSystemRenderer::CalculateViewChange(double deltaX, double deltaY) {
    if (!m_app.MouseButtonIsHeld(engine::MouseInput::LEFT)) {
        return;
    }
    view_x += deltaX / m_app.GetWindowWidth() * 3.1415 * 4;
    view_y -= deltaY / m_app.GetWindowHeight() * 3.1415 * 4;

    if (glm::degrees(view_y) > 89.f) {
        view_y = glm::radians(89.f);
    }
    if (glm::degrees(view_y) < -89.f) {
        view_y = glm::radians(-89.f);
    }
    selected_city = entt::null;
}

void SysStarSystemRenderer::FoundCity() {
    auto s = GetMouseSurfaceIntersection();
    SPDLOG_INFO("Founding city at {} {}", s.latitude(), s.longitude());

    entt::entity settlement = common::systems::actions::CreateCity(m_universe, on_planet, s.latitude(), s.longitude());
    // Set the name of the city
    Name& name = m_universe.emplace<Name>(settlement);
    name.name = m_universe.name_generators["Town Names"].Generate("1");

    // Set country
    // Add population and economy
    m_universe.emplace<components::IndustrialZone>(settlement);

    m_universe.clear<CityFounding>();

    CalculateCityPositions();
}

void SysStarSystemRenderer::SelectCountry() {
    // Country selection
    // Then select planet and tell the state
    selected_country_color = selected_province_color;
    selected_province = hovering_province;
    // Set the selected province
    if (!m_universe.valid(selected_province)) {
        return;
    }
    m_universe.clear<client::ctx::SelectedProvince>();
    // Get selected planet, then
    entity focused_planet = m_universe.view<FocusedPlanet>().front();
    if (!m_universe.any_of<PlanetTexture>(focused_planet)) {
        return;
    }
    auto& tex = m_universe.get<PlanetTexture>(focused_planet);
    if (tex.province_texture != nullptr) {
        m_universe.emplace_or_replace<client::ctx::SelectedProvince>(selected_province);
    }
}

void SysStarSystemRenderer::FocusOnEntity(entity ent) {
    // Check the focused planet
    entity focused_planet = m_universe.view<FocusedPlanet>().front();

    // if the focused planet is the current planet, then check if it's close
    // enough. If it is see the countries on the planet
    if (ent == focused_planet) {
        auto& body = m_universe.get<Body>(focused_planet);

        if (scroll > body.radius * 10) {
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

float SysStarSystemRenderer::GetWindowRatio() { return window_ratio; }

void SysStarSystemRenderer::GenerateOrbitLines() {
    ZoneScoped;
    SPDLOG_TRACE("Creating planet orbits");

    // Generates orbits for satellites
    auto orbit_list = m_universe.view<Orbit>(entt::exclude<PlanetOrbit>);
    orbits_generated = 0;
    for (auto body : orbit_list) {
        GenerateOrbit(body);
        orbits_generated++;
    }

    // Generate dirty orbits
    auto orbit4 = m_universe.view<Orbit, bodies::DirtyOrbit>();
    for (auto body : orbit4) {
        GenerateOrbit(body);
        m_universe.remove<bodies::DirtyOrbit>(body);
        orbits_generated++;
    }
}

void SysStarSystemRenderer::RenderInformationWindow(double deltaTime) {
    // FIXME(EhWhoamI)
    // auto &debug_info =
    // m_universe.ctx().emplace<ctx::StarSystemViewDebug>();
    ImGui::Begin("Debug ui window");
    ImGui::TextFmt("Cam Pos: {} {} {}", cam_pos.x, cam_pos.y, cam_pos.z);
    ImGui::TextFmt("View Center: {} {} {}", view_center.x, view_center.y, view_center.z);
    ImGui::TextFmt("Scroll: {}", scroll);
    ImGui::TextFmt("View {} {}", view_x, view_y);
    // Get the province name
    std::string country_name_t;
    if (m_universe.valid(selected_province) && m_universe.any_of<Province>(selected_province)) {
        country_name_t = GetName(m_universe, m_universe.get<Province>(selected_province).country);
    }
    ImGui::TextFmt("Hovering on Texture: {} {}", tex_x, tex_y);
    ImGui::TextFmt("Texture color: {} {} {}", tex_r, tex_g, tex_b);
    ImGui::TextFmt("Selected country color: {} {} {}", selected_country_color.x, selected_country_color.y,
                   selected_country_color.z);
    ImGui::TextFmt("Focused planets: {}", m_universe.view<FocusedPlanet>().size());
    ImGui::TextFmt("Generated {} orbits last frame", orbits_generated);
    entity earth = m_universe.planets["earth"];
    auto& b = m_universe.get<Body>(earth);
    // float offset = (float) b.rotation_offset;
    // ImGui::SliderAngle("asdf", &offset);
    // b.rotation_offset = (float) offset;
    ImGui::End();
}

void SysStarSystemRenderer::RenderSelectedObjectInformation() {
    // Get the focused object, and display their information
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - 200, ImGui::GetIO().DisplaySize.y - 300),
                            ImGuiCond_Appearing);
    ImGui::Begin("Focused Object");
    cqsp::client::systems::gui::EntityTooltipContent(m_universe, m_viewing_entity);
    // Edit the orbit if there is an issue
    // Get normalized vector
    if (m_universe.valid(m_viewing_entity) && m_universe.any_of<Kinematics>(m_viewing_entity)) {
        const auto& kin = m_universe.get<Kinematics>(m_viewing_entity);
        auto norm = glm::normalize(kin.velocity);
        ImGui::TextFmt("Prograde vector: {} {} {}", norm.x, norm.y, norm.z);

        static float delta_v = 0.01;
        norm *= delta_v;
        vec3 final_velocity = kin.velocity + norm;
        ImGui::TextFmt("Velocity vector: {} {} {}", final_velocity.x, final_velocity.y, final_velocity.z);

        if (ImGui::Button("Burn prograde")) {
            // Add 10m/s prograde or something
            auto& impulse = m_universe.get_or_emplace<common::components::types::Impulse>(m_viewing_entity);
            impulse.impulse += norm;
        }
        ImGui::SliderFloat("Text", &delta_v, -1, 1);
    }
    ImGui::End();
}

common::components::types::SurfaceCoordinate SysStarSystemRenderer::GetMouseSurfaceIntersection() {
    if (on_planet == entt::null || !m_universe.valid(on_planet)) {
        return SurfaceCoordinate(0, 0);
    }
    if (!m_universe.any_of<bodies::Body>(on_planet)) {
        return SurfaceCoordinate(0, 0);
    }

    vec3 p = GetMouseOnObject() - CalculateCenteredObject(on_planet);
    p = glm::normalize(p);

    auto& planet_comp = m_universe.get<bodies::Body>(on_planet);
    glm::quat quat = GetBodyRotation(planet_comp.axial, planet_comp.rotation, planet_comp.rotation_offset);
    // Rotate the vector based on the axial tilt and rotation.
    p = glm::inverse(quat) * p;

    SurfaceCoordinate s = types::ToSurfaceCoordinate(p);
    s = SurfaceCoordinate(s.latitude(), s.longitude() + 90);
    return s;
}

void SysStarSystemRenderer::CityDetection() {
    ZoneScoped;
    if (on_planet == entt::null || !m_universe.valid(on_planet)) {
        return;
    }
    auto s = GetMouseSurfaceIntersection();

    // Look for the vector
    // Rotate based on the axial tilt and roation
    int x = (-1 * (s.latitude() * 2 - 180)) / 360 * province_height;
    int y = fmod(s.longitude() + 180, 360) / 360. * province_width;
    int pos = (x * province_width + y);
    if (pos < 0 || pos > province_width * province_height) {
        return;
    }
    if (!m_universe.any_of<PlanetTexture>(on_planet)) {
        return;
    }
    auto& planet_texture = m_universe.get<PlanetTexture>(on_planet);
    if (planet_texture.province_texture == nullptr) {
        return;
    }
    ZoneNamed(LookforProvince, true);
    {
        hovering_province = planet_texture.province_map[pos];
        int color = m_universe.colors_province[hovering_province];
        auto province_color = cqsp::common::components::ProvinceColor::fromInt(color);
        selected_province_color =
            (vec3((float)province_color.r, (float)province_color.g, (float)province_color.b) / 255.f);
    }
}

vec3 SysStarSystemRenderer::GetMouseIntersectionOnObject(int mouse_x, int mouse_y) {
    ZoneScoped;
    // Normalize 3d device coordinates
    auto bodies = m_universe.view<Body>();
    // TODO(EhWhoAmI): Sort the bodies by distance before calculating intersection
    for (entity ent_id : bodies) {
        vec3 object_pos = CalculateCenteredObject(ent_id);
        float x = (2.0f * mouse_x) / m_app.GetWindowWidth() - 1.0f;
        float y = 1.0f - (2.0f * mouse_y) / m_app.GetWindowHeight();
        float z = 1.0f;

        vec3 ray_wor = CalculateMouseRay(vec3(x, y, z));
        auto& body = m_universe.get<Body>(ent_id);
        float radius = body.radius;

        // Check for intersection for sphere
        vec3 sub = cam_pos - object_pos;
        float b = glm::dot(ray_wor, sub);
        float c = glm::dot(sub, sub) - radius * radius;

        vec3 closest_hit = cam_pos + (-b - sqrt(b * b - c)) * ray_wor;
        // Get the closer value
        if ((b * b - c) >= 0) {
            is_rendering_founding_city = true;
            on_planet = ent_id;
            return closest_hit;
        }
    }
    is_rendering_founding_city = false;
    return vec3(0, 0, 0);
}

void SysStarSystemRenderer::GenerateOrbit(entity body) {
    // Then produce orbits
    ZoneScoped;
    // Generate the orbit
    auto& orb = m_universe.get<Orbit>(body);
    if (orb.semi_major_axis == 0) {
        return;
    }
    const int res = 500;
    std::vector<vec3> orbit_points;
    double SOI = std::numeric_limits<double>::infinity();
    if (m_universe.valid(orb.reference_body)) {
        SOI = m_universe.get<Body>(orb.reference_body).SOI;
    }

    orbit_points.reserve(res);
    // If hyperbolic
    if (orb.eccentricity > 1) {
        double v_inf = types::GetHyperbolicAsymptopeAnomaly(orb.eccentricity);
        // Remove one because it's slightly off.
        for (int i = 1; i < res; i++) {
            ZoneScoped;
            double theta = -v_inf * (1 - (double)i / (double)res) + v_inf * (((double)i / (double)res));

            vec3 vec = types::toVec3(orb, theta);
            // Check if the length is greater than the SOI, then we don't add it
            if (glm::length(vec) < SOI) {
                orbit_points.push_back(ConvertPoint(vec));
            }
        }
    } else {
        for (int i = 0; i <= res; i++) {
            ZoneScoped;
            double theta = 3.1415926535 * 2 / res * i;

            vec3 vec = types::toVec3(orb, theta);

            // If the length is greater than the sphere of influence, then
            // remove it
            if (length(vec) < SOI) {
                orbit_points.push_back(ConvertPoint(vec));
            }
        }
    }

    // m_universe.remove<cqspt::OrbitDirty>(body);
    auto& line = m_universe.get_or_emplace<PlanetOrbit>(body);
    // Get the orbit line
    // Do the points
    line.orbit_mesh = engine::primitive::CreateLineSequence(orbit_points);
}

entity SysStarSystemRenderer::GetMouseOnObject(int mouse_x, int mouse_y) {
    // Loop through objects
    auto bodies = m_universe.view<Body>();
    for (entity ent_id : bodies) {
        vec3 object_pos = CalculateCenteredObject(ent_id);
        // Check if the sphere is rendered or not
        // Normalize 3d device coordinates
        float x = (2.0f * mouse_x) / m_app.GetWindowWidth() - 1.0f;
        float y = 1.0f - (2.0f * mouse_y) / m_app.GetWindowHeight();
        float z = 1.0f;

        vec3 ray_wor = CalculateMouseRay(vec3(x, y, z));
        auto& body = m_universe.get<Body>(ent_id);
        float radius = body.radius;

        // Check for intersection for sphere
        vec3 sub = cam_pos - object_pos;
        float b = glm::dot(ray_wor, sub);
        // Object radius
        float c = glm::dot(sub, sub) - radius * radius;

        // Get the closer value
        if ((b * b - c) >= 0) {
            m_universe.emplace<MouseOverEntity>(ent_id);
            return ent_id;
        }
    }
    return entt::null;
}

bool SysStarSystemRenderer::IsFoundingCity(common::Universe& universe) {
    return !universe.view<CityFounding>().empty();
}

void SysStarSystemRenderer::DrawAllOrbits() {
    ZoneScoped;
    auto orbits = m_universe.view<Orbit>();
    // Always render planet orbits
    // Visible orbits will not be rendered
    for (entt::entity orbit_entity : orbits) {
        // Check the type of orbiting things, and then don't render if it doesn't fit the filter
        if (m_universe.any_of<Planet>(orbit_entity)) {
            // Render no matter what
            DrawOrbit(orbit_entity);
            continue;
        }
        if (!m_universe.any_of<ctx::VisibleOrbit>(orbit_entity)) {
            // Then don't render
            continue;
        }
        DrawOrbit(orbit_entity);
    }
}

void SysStarSystemRenderer::DrawOrbit(const entity& orbitentity) {
    if (!m_universe.any_of<PlanetOrbit>(orbitentity)) {
        return;
    }
    vec3 center = vec3(0, 0, 0);
    // If it has a parent, draw around the parent
    entity ref = m_universe.get<Orbit>(orbitentity).reference_body;
    if (ref != entt::null) {
        center = CalculateObjectPos(ref);
    } else {
        return;
    }
    mat4 transform = mat4(1.f);
    transform = glm::translate(transform, CalculateCenteredObject(center));
    // Actually you just need to rotate the orbit
    auto body = m_universe.get<Body>(ref);
    //transform *= glm::mat4(
    //    glm::quat{{0.f, 0, (float)body.axial}});
    // Draw orbit
    orbit_shader->SetMVP(transform, camera_matrix, m_app.Get3DProj());

    //Set the color of each orbit based on its distance from its center body

    auto& orb = m_universe.get<Orbit>(orbitentity);

    const double dis = orb.semi_major_axis;
    const double max_dis = body.SOI;
    const double inc = orb.inclination;

    const float min_launch_dis = body.radius;
    float col = dis - min_launch_dis;
    float r = log(col) / log(max_dis);
    float g = 1 - r;
    float b = inc / 3.15;
    vec4 color_v = {1, 1, 1, 1};
    orbit_shader->Set("color", color_v);

    //orbit_shader->Set("color", vec4(1, 1, 1, 1));
    // Set to the center of the universe
    auto& orbit = m_universe.get<PlanetOrbit>(orbitentity);

    orbit.orbit_mesh->Draw();

#if false
    // Get parent
    glm::vec3 object_pos = CalculateCenteredObject(entity);
    transform = glm::mat4(1.f);
    //transform = glm::translate(transform, object_pos);
    auto mesh = engine::primitive::CreateLineSequence(
        {object_pos, CalculateCenteredObject(glm::vec3(0, 0, 0))});
    orbit_shader->SetMVP(transform, camera_matrix, m_app.Get3DProj());

    mesh->Draw();
    delete mesh;
#endif
}

void SysStarSystemRenderer::OrbitEditor() {
#if 0
    static float semi_major_axis = 8000;
    static float inclination = 0;
    static float eccentricity = 0;
    static float arg_of_perapsis = 0;
    static float LAN = 0;
    ImGui::SliderFloat("Semi Major Axis", &semi_major_axis, 6000, 5000000);
    ImGui::SliderFloat("Eccentricity", &eccentricity, 0, 0.9999);
    ImGui::SliderAngle("Inclination", &inclination, 0, 180);
    ImGui::SliderAngle("Argument of perapsis", &arg_of_perapsis, 0, 360);
    ImGui::SliderAngle("Longitude of the ascending node", &LAN, 0, 360);
    if (ImGui::Button("Launch!")) {
        // Get reference body
        entt::entity reference_body = selected_planet;
        // Launch inclination will be the inclination of the thing
        double axial =
            GetUniverse().get<cqspc::bodies::Body>(selected_planet).axial;
        double inc =
            GetUniverse()
                .get<cqspc::types::SurfaceCoordinate>(selected_city_entity)
                .r_latitude();
        inc += axial;
        double sma = 0;
        // Currently selected city
        // entt::entity star_system =
        // GetUniverse().get<cqspc::bodies::Body>(selected_planet);
        // Launch
        cqspc::types::Orbit orb;
        orb.reference_body = selected_planet;
        orb.inclination = inclination;
        orb.semi_major_axis = semi_major_axis;
        orb.eccentricity = eccentricity;
        orb.w = arg_of_perapsis;
        orb.LAN = LAN;
    }
#endif
}

SysStarSystemRenderer::~SysStarSystemRenderer() = default;

}  // namespace cqsp::client::systems
