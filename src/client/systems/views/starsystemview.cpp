/* Conquer Space
* Copyright (C) 2021 Conquer Space
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
#include "client/systems/views/starsystemview.h"

#include <glad/glad.h>

#include <noise/noise.h>

#include <cmath>
#include <string>
#include <memory>
#include <vector>

#include <tracy/Tracy.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include "client/components/planetrendering.h"
#include "client/components/clientctx.h"

#include "engine/graphics/primitives/uvsphere.h"
#include "engine/renderer/renderer.h"
#include "engine/graphics/primitives/cube.h"
#include "engine/graphics/primitives/polygon.h"
#include "engine/graphics/primitives/pane.h"
#include "engine/graphics/primitives/line.h"

#include "common/components/bodies.h"
#include "common/components/surface.h"
#include "common/components/organizations.h"
#include "common/components/player.h"
#include "common/components/resource.h"
#include "common/components/coordinates.h"
#include "common/components/name.h"
#include "common/components/ships.h"
#include "common/components/units.h"
#include "common/components/area.h"
#include "common/util/profiler.h"
#include "common/systems/actions/cityactions.h"

using cqsp::client::systems::SysStarSystemRenderer;
SysStarSystemRenderer::SysStarSystemRenderer(cqsp::common::Universe &_u,
                                                cqsp::engine::Application &_a) :
                                                m_universe(_u), m_app(_a),
                                                scroll(5), view_x(0),
                                                view_y(0), view_center(glm::vec3(1, 1, 1)),
                                                sun_color(glm::vec3(10, 10, 10)) {
}

namespace {
struct Offset  {
    glm::vec3 offset;
};

struct TerrainTextureData {
    cqsp::asset::Texture* terrain_albedo = nullptr;
    cqsp::asset::Texture* heightmap = nullptr;

    void DeleteData() {
        delete terrain_albedo;
        delete heightmap;
    }
};

struct PlanetTexture {
    cqsp::asset::Texture* terrain = nullptr;
    cqsp::asset::Texture* normal = nullptr;
};

struct PlanetOrbit {
    cqsp::engine::Mesh *orbit_mesh;
};
}  // namespace

void SysStarSystemRenderer::Initialize() {
    // Initialize meshes, etc
    cqsp::engine::Mesh* sphere_mesh = cqsp::engine::primitive::ConstructSphereMesh(64, 64);

    // Initialize sky box
    asset::Texture* sky_texture = m_app.GetAssetManager().GetAsset<cqsp::asset::Texture>("core:skycubemap");

    sky.mesh = engine::primitive::MakeCube();
    sky.shaderProgram = m_app.GetAssetManager().GetAsset<asset::ShaderDefinition>("core:skybox")->MakeShader();

    asset::ShaderProgram_t circle_shader =
            m_app.GetAssetManager().GetAsset<asset::ShaderDefinition>("core:2dcolorshader")->MakeShader();

    planet_circle.mesh = engine::primitive::CreateFilledCircle();
    planet_circle.shaderProgram = circle_shader;

    ship_overlay.mesh = engine::primitive::CreateFilledTriangle();
    ship_overlay.shaderProgram = circle_shader;

    city.mesh = engine::primitive::MakeTexturedPaneMesh();
    city.shaderProgram = circle_shader;

    // Initialize shaders
    asset::ShaderProgram_t planet_shader =
        m_app.GetAssetManager().GetAsset<asset::ShaderDefinition>("core:planetshader")->MakeShader();

    // Planet spheres
    planet.mesh = sphere_mesh;
    planet.shaderProgram = planet_shader;

    // Initialize shaders
    asset::ShaderProgram_t textured_planet_shader =
        m_app.GetAssetManager().GetAsset<asset::ShaderDefinition>("core:planet_textureshader")->MakeShader();

    textured_planet.mesh = sphere_mesh;
    textured_planet.shaderProgram = textured_planet_shader;

    // Initialize sun
    sun.mesh = sphere_mesh;
    sun.shaderProgram = m_app.GetAssetManager().GetAsset<asset::ShaderDefinition>("core:sunshader")->MakeShader();
    auto buffer_shader = m_app.GetAssetManager().GetAsset<asset::ShaderDefinition>("core:framebuffer")->MakeShader();

    ship_icon_layer = renderer.AddLayer<engine::FramebufferRenderer>(buffer_shader, *m_app.GetWindow());
    physical_layer = renderer.AddLayer<engine::FramebufferRenderer>(buffer_shader, *m_app.GetWindow());
    planet_icon_layer = renderer.AddLayer<engine::FramebufferRenderer>(buffer_shader, *m_app.GetWindow());
    skybox_layer = renderer.AddLayer<engine::FramebufferRenderer>(buffer_shader, *m_app.GetWindow());

    orbit_line.mesh = engine::primitive::CreateLineCircle(256, 1);
    orbit_line.shaderProgram =
        m_app.GetAssetManager().GetAsset<asset::ShaderDefinition>("core:sunshader")->MakeShader();
    orb.semi_major_axis = 4;
    orb.eccentricity = 0;
    orb.inclination = 1;
    orb.LAN = 0;
    orb.w = 1;
}

void SysStarSystemRenderer::OnTick() {
    entt::entity current_planet = m_app.GetUniverse().view<FocusedPlanet>().front();
    if (current_planet != entt::null) {
        view_center = CalculateObjectPos(m_viewing_entity);
    }

    namespace cqspb = cqsp::common::components::bodies;

    auto system = m_app.GetUniverse().view<common::components::types::Orbit>();
    for (entt::entity ent : system) {
        m_app.GetUniverse().get_or_emplace<ToRender>(ent);
    }
}

void SysStarSystemRenderer::Render(float deltaTime) {
    ZoneScoped;
    namespace cqspb = cqsp::common::components::bodies;

    // Seeing new planet
    entt::entity current_planet = m_app.GetUniverse().view<FocusedPlanet>().front();
    if (current_planet != m_viewing_entity && current_planet != entt::null) {
        SPDLOG_INFO("Switched displaying planet, seeing {}", current_planet);
        m_viewing_entity = current_planet;
        // Do terrain
        SeeEntity();
    }

    FocusCityView();

    // Check for resized window
    window_ratio = static_cast<float>(m_app.GetWindowWidth()) /
                   static_cast<float>(m_app.GetWindowHeight());
    renderer.NewFrame(*m_app.GetWindow());

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    CalculateCamera();

    DrawStars();
    DrawBodies();
    DrawShips();
    DrawSkybox();

    renderer.DrawAllLayers();
}

void SysStarSystemRenderer::SeeStarSystem() {
    namespace cqspb = cqsp::common::components::bodies;
    m_universe.clear<ToRender>();

    GenerateOrbitLines();

    auto orbits = m_app.GetUniverse().view<common::components::types::Orbit>();

    for (auto body : orbits) {
        // Add a tag
        m_universe.get_or_emplace<ToRender>(body);
    }

    SPDLOG_INFO("Loading planet textures");
    for (auto body : orbits) {
        if (!m_app.GetUniverse().all_of<cqspb::TexturedTerrain>(body)) {
            continue;
        }
        auto textures = m_app.GetUniverse().get<cqspb::TexturedTerrain>(body);
        auto &data = m_universe.get_or_emplace<PlanetTexture>(body);
        data.terrain = m_app.GetAssetManager().GetAsset<cqsp::asset::Texture>("core:" + textures.terrain_name);
        if (textures.normal_name != "") {
            data.normal = m_app.GetAssetManager().GetAsset<cqsp::asset::Texture>("core:" + textures.normal_name);
        }
    }
}

void SysStarSystemRenderer::SeeEntity() {
    namespace cqspb = cqsp::common::components::bodies;

    // See the object
    view_center = CalculateObjectPos(m_viewing_entity);

    // Set the variable
    scroll = 5;
    CalculateCityPositions();
}

void SysStarSystemRenderer::Update(float deltaTime) {
    ZoneScoped;
    double deltaX = previous_mouseX - m_app.GetMouseX();
    double deltaY = previous_mouseY - m_app.GetMouseY();

    is_founding_city = IsFoundingCity(m_universe);

    if (!ImGui::GetIO().WantCaptureMouse) {
        scroll -= m_app.GetScrollAmount() * 3 * scroll/33;

        if (m_app.MouseButtonIsHeld(engine::MouseInput::LEFT)) {
            view_x += deltaX/m_app.GetWindowWidth()*3.1415*4;
            view_y -= deltaY/m_app.GetWindowHeight()*3.1415*4;

            if (glm::degrees(view_y) > 89.f) {
                view_y = glm::radians(89.f);
            }
            if (glm::degrees(view_y) < -89.f) {
                view_y = glm::radians(-89.f);
            }
        }

        previous_mouseX = m_app.GetMouseX();
        previous_mouseY = m_app.GetMouseY();

        // If clicks on object, go to the planet
        entt::entity ent = m_app.GetUniverse().view<MouseOverEntity>().front();
        if (m_app.MouseButtonIsReleased(engine::MouseInput::LEFT) && ent != entt::null && !m_app.MouseDragged()) {
            // Then go to the object
            SeePlanet(ent);
            // Discern between showing UI and other things

            if (is_founding_city) {
                // Found city
                glm::vec3 p = city_founding_position - CalculateCenteredObject(on_planet);
                namespace cqspt = cqsp::common::components::types;
                namespace cqspc = cqsp::common::components;

                double latitude = cqspt::toDegree(asin(p.y));
                double longitude = cqspt::toDegree(atan2(p.x, p.z));
                SPDLOG_INFO("Founding city at {} {} {}", latitude, longitude, glm::length(p));

                entt::entity settlement =
                    cqsp::common::actions::CreateCity(m_app.GetUniverse(),
                                                        on_planet, latitude, longitude);
                // Set the name of the city
                cqspc::Name& name = m_app.GetUniverse().emplace<cqspc::Name>(settlement);
                name.name = m_app.GetUniverse().name_generators["Town Names"].Generate("1");
                // Add population and economy
                m_app.GetUniverse().emplace<cqspc::Industry>(settlement);

                m_app.GetUniverse().clear<CityFounding>();

                CalculateCityPositions();
            }
        }
    }

    if (!ImGui::GetIO().WantCaptureKeyboard) {
        MoveCamera(deltaTime);
    }

    using cqsp::client::components::PlanetTerrainRender;
    city_founding_position = GetMouseIntersectionOnObject(m_app.GetMouseX(), m_app.GetMouseY());
    // If clicked, order a city at the place

    // Some way to communicate between the two about the information

    /*
    // Check if it has terrain resource rendering, and make terrain thing
    if (m_viewing_entity != entt::null && m_app.GetUniverse().all_of<PlanetTerrainRender>(m_viewing_entity)) {
        CheckResourceDistRender();
    } else if (m_viewing_entity != entt::null) {
        // Reset to default
        planet.textures.reserve(2);
        planet.textures[0] = planet_texture;
        terrain_displaying = entt::null;
        // Also change up the shader
        planet.shaderProgram = pbr_shader;
    }*/
}

void SysStarSystemRenderer::SeePlanet(entt::entity ent) {
    m_app.GetUniverse().clear<FocusedPlanet>();
    m_app.GetUniverse().emplace<FocusedPlanet>(ent);
}

void SysStarSystemRenderer::DoUI(float deltaTime) {
    // FIXME(EhWhoamI)
    //auto &debug_info = m_app.GetUniverse().ctx().emplace<ctx::StarSystemViewDebug>();
    /* if (debug_info.to_show) {
        ImGui::Begin("Debug ui window", &debug_info.to_show);
        ImGui::TextFmt("{} {} {}", cam_pos.x, cam_pos.y, cam_pos.z);
        ImGui::TextFmt("{} {} {}", view_center.x, view_center.y, view_center.z);
        ImGui::TextFmt("{}", scroll);
        ImGui::TextFmt("Focused planets: {}",
            m_universe.view<cqsp::client::systems::FocusedPlanet>().size());
        ImGui::End();
    }*/
}

void SysStarSystemRenderer::DrawStars() {
    ZoneScoped;
    // Draw stars
    namespace cqspb = cqsp::common::components::bodies;
    namespace cqsps = cqsp::common::components::ships;
    auto stars = m_app.GetUniverse().view<ToRender, cqspb::Body, cqspb::LightEmitter>();
    for (auto ent_id : stars) {
        // Draw the star circle
        glm::vec3 object_pos = CalculateCenteredObject(ent_id);
        sun_position = object_pos;
        if (glm::distance(object_pos, cam_pos) > 900) {
            // Check if it's obscured by a planet, but eh, we can deal with it later
            planet_circle.shaderProgram->UseProgram();
            planet_circle.shaderProgram->setVec4("color", 1, 1, 0, 1);
            DrawPlanetIcon(object_pos);
            continue;
        }
        renderer.BeginDraw(physical_layer);
        DrawStar(object_pos);
        renderer.EndDraw(physical_layer);
    }
}

void cqsp::client::systems::SysStarSystemRenderer::DrawBodies() {
    ZoneScoped;
    namespace cqspb = cqsp::common::components::bodies;
    namespace cqsps = cqsp::common::components::ships;
    // Draw other bodies
    auto bodies = m_app.GetUniverse().view<ToRender, cqspb::Body>(entt::exclude<cqspb::LightEmitter>);

    renderer.BeginDraw(planet_icon_layer);
    glDepthFunc(GL_ALWAYS);
    for (auto body_entity : bodies) {
        // Draw the planet circle
        glm::vec3 object_pos = CalculateCenteredObject(body_entity);

        // Draw Ships
        namespace cqspc = cqsp::common::components;
        namespace cqspt = cqsp::common::components::types;
        if (glm::distance(object_pos, cam_pos) > 200) {
            // Check if it's obscured by a planet, but eh, we can deal with it later
            // Set planet circle color
            planet_circle.shaderProgram->UseProgram();
            planet_circle.shaderProgram->setVec4("color", 0, 0, 1, 1);
            DrawPlanetIcon(object_pos);
            DrawEntityName(object_pos, body_entity);
            continue;
        }
    }
    renderer.EndDraw(planet_icon_layer);
    glDepthFunc(GL_LESS);
    renderer.BeginDraw(physical_layer);
    for (entt::entity body_entity : bodies) {
        glm::vec3 object_pos = CalculateCenteredObject(body_entity);

        // Draw Ships
        namespace cqspc = cqsp::common::components;
        namespace cqspt = cqsp::common::components::types;

        if (glm::distance(object_pos, cam_pos) <= 200) {
            // Check if planet has terrain or not
            if (m_app.GetUniverse().all_of<cqspb::Terrain>(body_entity)) {
                // Do empty terrain
                // Check if the planet has the thing
                DrawPlanet(object_pos, body_entity);
            } else if (m_app.GetUniverse().all_of<cqspb::TexturedTerrain>(body_entity)) {
                DrawTexturedPlanet(object_pos, body_entity);
            } else {
                DrawTerrainlessPlanet(object_pos);
            }
        }
        DrawOrbit(body_entity);
    }
    renderer.EndDraw(physical_layer);

    renderer.BeginDraw(ship_icon_layer);
    for (auto body_entity : bodies) {
        glm::vec3 object_pos = CalculateCenteredObject(body_entity);
        if (glm::distance(object_pos, cam_pos) <= 200) {
            RenderCities(object_pos, body_entity);
        }
    }
    renderer.EndDraw(ship_icon_layer);
}

void SysStarSystemRenderer::DrawShips() {
    ZoneScoped;
    namespace cqsps = cqsp::common::components::ships;
    // Draw Ships
    auto ships = m_app.GetUniverse().view<ToRender, cqsps::Ship>();

    renderer.BeginDraw(ship_icon_layer);
    ship_overlay.shaderProgram->UseProgram();
    for (auto ent_id : ships) {
        glm::vec3 object_pos = CalculateCenteredObject(ent_id);
        ship_overlay.shaderProgram->setVec4("color", 1, 0, 0, 1);
        DrawShipIcon(object_pos);
    }
    renderer.EndDraw(ship_icon_layer);
}

void SysStarSystemRenderer::DrawSkybox() {
    ZoneScoped;
    // Draw sky box
    renderer.BeginDraw(skybox_layer);
    sky.shaderProgram->UseProgram();
    sky.shaderProgram->setMat4("view", glm::mat4(glm::mat3(camera_matrix)));
    sky.shaderProgram->setMat4("projection", projection);
    glDepthFunc(GL_LEQUAL);
    // skybox cube
    engine::Draw(sky);
    glDepthFunc(GL_LESS);
    renderer.EndDraw(skybox_layer);
}

void SysStarSystemRenderer::DrawEntityName(glm::vec3 &object_pos,
                                           entt::entity ent_id) {
    using cqsp::common::components::Name;
    std::string text = "";
    if (m_app.GetUniverse().all_of<Name>(ent_id)) {
        text = m_app.GetUniverse().get<Name>(ent_id);
    } else {
        text = fmt::format("{}", ent_id);
    }
    glm::vec3 pos = glm::project(object_pos, camera_matrix, projection, viewport);
    // Check if the position on screen is within bounds
    if (!(pos.z >= 1 || pos.z <= -1) &&
        (pos.x > 0 && pos.x < m_app.GetWindowWidth() &&
            pos.y > 0 && pos.y < m_app.GetWindowHeight())) {
        m_app.DrawText(text, pos.x, pos.y, 20);
    }
}

void SysStarSystemRenderer::DrawPlanetIcon(glm::vec3 &object_pos) {
    glm::vec3 pos = glm::project(object_pos, camera_matrix, projection, viewport);
    glm::mat4 planetDispMat = glm::mat4(1.0f);
    if (pos.z >= 1 || pos.z <= -1) {
        return;
    }

    planetDispMat = glm::translate(planetDispMat, TranslateToNormalized(pos));
    planetDispMat = glm::scale(planetDispMat, glm::vec3(circle_size, circle_size, circle_size));

    float window_ratio = GetWindowRatio();
    planetDispMat = glm::scale(planetDispMat, glm::vec3(1, window_ratio, 1));
    glm::mat4 twodimproj =  glm::mat4(1.0f);

    planet_circle.shaderProgram->UseProgram();
    planet_circle.shaderProgram->setMat4("model", planetDispMat);
    planet_circle.shaderProgram->setMat4("projection", twodimproj);

    engine::Draw(planet_circle);
}

void SysStarSystemRenderer::DrawCityIcon(glm::vec3 &object_pos) {
    glm::vec3 pos = glm::project(object_pos, camera_matrix, projection, viewport);
    glm::mat4 planetDispMat = glm::mat4(1.0f);
    if (pos.z >= 1 || pos.z <= -1) {
        return;
    }

    planetDispMat = glm::translate(planetDispMat, TranslateToNormalized(pos));

    planetDispMat = glm::scale(planetDispMat, glm::vec3(circle_size, circle_size, circle_size));

    float window_ratio = GetWindowRatio();
    planetDispMat = glm::scale(planetDispMat, glm::vec3(1, window_ratio, 1));
    glm::mat4 twodimproj = glm::mat4(1.0f);
    city.shaderProgram->UseProgram();
    city.shaderProgram->Set("color", 1, 0, 1, 1);
    city.shaderProgram->setMat4("model", planetDispMat);
    city.shaderProgram->setMat4("projection", twodimproj);

    engine::Draw(city);
}

void SysStarSystemRenderer::DrawShipIcon(glm::vec3 &object_pos) {
    glm::vec3 pos = glm::project(object_pos, camera_matrix, projection, viewport);
    glm::mat4 shipDispMat = glm::mat4(1.0f);
    if (pos.z >= 1 || pos.z <= -1) {
        return;
    }

    shipDispMat = glm::translate(shipDispMat, TranslateToNormalized(pos));

    shipDispMat = glm::scale(shipDispMat, glm::vec3(circle_size, circle_size, circle_size));

    float window_ratio = GetWindowRatio();
    shipDispMat = glm::scale(shipDispMat, glm::vec3(1, window_ratio, 1));
    glm::mat4 twodimproj = glm::mat4(1.0f);
    ship_overlay.shaderProgram->UseProgram();
    ship_overlay.shaderProgram->setMat4("model", shipDispMat);
    ship_overlay.shaderProgram->setMat4("projection", twodimproj);

    engine::Draw(ship_overlay);
}

void SysStarSystemRenderer::DrawTexturedPlanet(glm::vec3 &object_pos, entt::entity entity) {
    bool have_normal = false;
    if (m_universe.all_of<PlanetTexture>(entity)) {
        auto& terrain_data = m_universe.get<PlanetTexture>(entity);
        textured_planet.textures.clear();
        textured_planet.textures.push_back(terrain_data.terrain);
        if (terrain_data.normal) {
            have_normal = true;
            textured_planet.textures.push_back(terrain_data.normal);
        }
    }
    glm::mat4 position = glm::mat4(1.f);
    position = glm::translate(position, object_pos);

    namespace cqspb = cqsp::common::components::bodies;
    auto& body = m_universe.get<cqspb::Body>(entity);
    float scale = body.radius / 50000;
    position = glm::scale(position, glm::vec3(scale));

    textured_planet.SetMVP(position, camera_matrix, projection);
    textured_planet.shaderProgram->UseProgram();

    // Maybe a seperate shader for planets without normal maps would be better
    textured_planet.shaderProgram->setBool("haveNormal", have_normal);

    textured_planet.shaderProgram->setVec3("lightDir", glm::normalize(sun_position - object_pos));

    textured_planet.shaderProgram->setVec3("lightDir", glm::normalize(sun_position - object_pos));
    textured_planet.shaderProgram->setVec3("lightPosition", sun_position);

    textured_planet.shaderProgram->setVec3("lightColor", sun_color);
    textured_planet.shaderProgram->setVec3("viewPos", cam_pos);

    engine::Draw(textured_planet);
}

void SysStarSystemRenderer::DrawPlanet(glm::vec3 &object_pos, entt::entity entity) {
    if (m_universe.all_of<TerrainTextureData>(entity)) {
        auto& terrain_data = m_universe.get<TerrainTextureData>(entity);
        planet.textures.clear();
        planet.textures.push_back(terrain_data.terrain_albedo);
        planet.textures.push_back(terrain_data.heightmap);
    }
    glm::mat4 position = glm::mat4(1.f);
    position = glm::translate(position, object_pos);

    glm::mat4 transform = glm::mat4(1.f);

    position = position * transform;

    planet.SetMVP(position, camera_matrix, projection);
    planet.shaderProgram->UseProgram();

    planet.shaderProgram->setVec3("lightDir", glm::normalize(sun_position - object_pos));
    planet.shaderProgram->setVec3("lightPosition", sun_position);

    planet.shaderProgram->setVec3("lightColor", sun_color);
    planet.shaderProgram->setVec3("viewPos", cam_pos);

    using cqsp::common::components::bodies::TerrainData;
    entt::entity terrain = m_universe.get<cqsp::common::components::bodies::Terrain>(entity).terrain_type;
    planet.shaderProgram->Set("seaLevel", m_universe.get<TerrainData>(terrain).sea_level);
    engine::Draw(planet);
}

void SysStarSystemRenderer::DrawStar(glm::vec3 &object_pos) {
    glm::mat4 position = glm::mat4(1.f);
    position = glm::translate(position, object_pos);

    glm::mat4 transform = glm::mat4(1.f);
    transform = glm::scale(transform, glm::vec3(0.1, 0.1, 0.1));
    position = position * transform;

    sun.SetMVP(position, camera_matrix, projection);
    sun.shaderProgram->setVec4("color", 1, 1, 1, 1);
    engine::Draw(sun);
}

void SysStarSystemRenderer::DrawTerrainlessPlanet(glm::vec3 &object_pos) {
    namespace cqspb = cqsp::common::components::bodies;

    glm::mat4 position = glm::mat4(1.f);
    position = glm::translate(position, object_pos);
    float scale = 1000 / 50000;
    position = glm::scale(position, glm::vec3(scale));
    glm::mat4 transform = glm::mat4(1.f);
    position = position * transform;

    sun.SetMVP(position, camera_matrix, projection);
    sun.shaderProgram->setVec4("color", 1, 0, 1, 1);
    engine::Draw(sun);
}

void SysStarSystemRenderer::RenderCities(glm::vec3 &object_pos, const entt::entity &body_entity) {
    ZoneScoped;
    // Draw Cities
    namespace cqspc = cqsp::common::components;
    namespace cqspt = cqsp::common::components::types;
    if (!m_app.GetUniverse().all_of<cqspc::Habitation>(body_entity)) {
        return;
    }
    std::vector<entt::entity> cities = m_app.GetUniverse().get<cqspc::Habitation>(body_entity).settlements;
    if (cities.empty()) {
        return;
    }

    // Put in same layer as ships
    city.shaderProgram->UseProgram();
    city.shaderProgram->setVec4("color", 0.5, 0.5, 0.5, 1);
    for (auto city_entity : cities) {
        // Calculate position to render
        if (!m_app.GetUniverse().any_of<Offset>(city_entity)) {
            // Calculate offset
        }
        glm::vec3 city_pos = m_app.GetUniverse().get<Offset>(city_entity).offset;
        // Check if line of sight and city position intersects the sphere that is the planet

        glm::vec3 city_world_pos = city_pos + object_pos;
        if (CityIsVisible(city_world_pos, object_pos, cam_pos)) {
            // If it's reasonably close, then we can show city names
            if (scroll < 3) {
                DrawEntityName(city_world_pos, city_entity);
            }
            DrawCityIcon(city_world_pos);
        }
    }

    if (is_founding_city && is_rendering_founding_city) {
        DrawCityIcon(city_founding_position);
    }
}

bool SysStarSystemRenderer::CityIsVisible(glm::vec3 city_pos, glm::vec3 planet_pos, glm::vec3 cam_pos) {
    float d = glm::distance(cam_pos, city_pos);
    float D = glm::distance(cam_pos, planet_pos);
    const float radius = 1;

    float discriminant = sqrt(D * D + radius * radius);
    // If the discriminant is greater than d, then it's hidden by the sphere
    return (d+1e-5 < discriminant);
}

void SysStarSystemRenderer::CalculateCityPositions() {
    namespace cqspc = cqsp::common::components;
    namespace cqspt = cqsp::common::components::types;
    // Calculate offset for all cities on planet if they exist
    if (!m_app.GetUniverse().valid(m_viewing_entity)) {
        return;
    }
    if (!m_app.GetUniverse().all_of<cqspc::Habitation>(m_viewing_entity)) {
        return;
    }
    std::vector<entt::entity> cities = m_app.GetUniverse().get<cqspc::Habitation>(m_viewing_entity).settlements;
    if (cities.empty()) {
        return;
    }
    for (auto city_entity : cities) {
        if (!m_app.GetUniverse().all_of<cqspt::SurfaceCoordinate>(city_entity)) {
            continue;
        }
        auto& coord = m_app.GetUniverse().get<cqspt::SurfaceCoordinate>(city_entity);
        m_app.GetUniverse().emplace_or_replace<Offset>(city_entity, cqspt::toVec3(coord,  1));
    }
    SPDLOG_INFO("Calculated offset");
}

void cqsp::client::systems::SysStarSystemRenderer::FocusCityView() {
    namespace cqspt = cqsp::common::components::types;
    auto focused_city_view = m_app.GetUniverse().view<FocusedCity>();
    entt::entity city_entity = focused_city_view.front();

    // City to focus view on
    if (focused_city_view.empty()) {
        return;
    }
    m_universe.clear<FocusedCity>();

    // Then select the city
    // Focus view on city
    // Calculate the vector we can get to see the city, then see it
    if (!m_universe.any_of<cqspt::SurfaceCoordinate>(city_entity)) {
        return;
    }
    auto& surf = m_universe.get<cqspt::SurfaceCoordinate>(city_entity);
    view_x = surf.r_longitude();
    view_y = surf.r_latitude();
    scroll = 1.5;
}

glm::vec3 SysStarSystemRenderer::CalculateObjectPos(const entt::entity &ent) {
    namespace cqspt = cqsp::common::components::types;
    // Get the position
    if (m_universe.all_of<cqspt::Kinematics>(ent)) {
        return (m_universe.get<cqspt::Kinematics>(ent).position * 10.f);
    }
    return glm::vec3(0, 0, 0);
}

glm::vec3 SysStarSystemRenderer::CalculateCenteredObject(const glm::vec3 &vec) {
    return vec - view_center;
}

glm::vec3 SysStarSystemRenderer::TranslateToNormalized(const glm::vec3 &pos) {
    return glm::vec3((pos.x / m_app.GetWindowWidth() - 0.5) * 2,
            (pos.y / m_app.GetWindowHeight() - 0.5) * 2, 0);
}

glm::vec3 SysStarSystemRenderer::CalculateCenteredObject(const entt::entity &ent) {
    return CalculateCenteredObject(CalculateObjectPos(ent));
}

void SysStarSystemRenderer::CalculateCamera() {
    cam_pos = glm::vec3(
                cos(view_y) * sin(view_x),
                sin(view_y),
                cos(view_y) * cos(view_x)) * scroll;
    cam_up = glm::vec3(0.0f, 1.0f, 0.0f);
    camera_matrix = glm::lookAt(cam_pos, glm::vec3(0.f, 0.f, 0.f), cam_up);
    projection = glm::infinitePerspective(glm::radians(45.f), GetWindowRatio(), 0.1f);
    viewport = glm::vec4(0.f, 0.f, m_app.GetWindowWidth(), m_app.GetWindowHeight());
}

void SysStarSystemRenderer::MoveCamera(double deltaTime) {
    // Now navigation for changing the center
    glm::vec3 dir = (view_center - cam_pos);
    float velocity = deltaTime * 30 * scroll/40;
    // Get distance from the pane
    // Remove y axis
    glm::vec3 forward = glm::normalize(glm::vec3(glm::sin(view_x), 0, glm::cos(view_x)));
    glm::vec3 right = glm::normalize(glm::cross(forward, cam_up));
    auto post_move = [&]() {
        m_universe.clear<cqsp::client::systems::FocusedPlanet>();
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
    using cqsp::client::components::PlanetTerrainRender;
    // Then check if it's the same rendered object
    auto &rend = m_app.GetUniverse().get<PlanetTerrainRender>(m_viewing_entity);
    if (rend.resource == terrain_displaying) {
        return;
    }

    // Check if it's the same
    using cqsp::common::components::ResourceDistribution;
    if (!m_app.GetUniverse().any_of<ResourceDistribution>(m_viewing_entity)) {
        return;
    }

    auto &dist = m_app.GetUniverse().get<ResourceDistribution>(m_viewing_entity);
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

glm::vec3 SysStarSystemRenderer::CalculateMouseRay(const glm::vec3 &ray_nds) {
    glm::vec4 ray_clip = glm::vec4(ray_nds.x, ray_nds.y, -1.0, 1.0);
    glm::vec4 ray_eye = glm::inverse(projection) * ray_clip;
    ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0, 0.0);
    glm::vec4 inv = (glm::inverse(camera_matrix) * ray_eye);

    // Normalize vector
    return glm::normalize(glm::vec3(inv.x, inv.y, inv.z));
}

float SysStarSystemRenderer::GetWindowRatio() {
    return window_ratio;
}

void SysStarSystemRenderer::GenerateOrbitLines() {
    SPDLOG_INFO("Creating planet orbits");
    auto system = m_app.GetUniverse().view<common::components::types::Orbit>();
    // Initialize all the orbits and stuff
    for (auto body : system) {
        // Generate the orbit
        auto& orb = m_universe.get<common::components::types::Orbit>(body);
        if (orb.semi_major_axis == 0) {
            continue;
        }
        std::vector<glm::vec3> orbit_points;
        int res = 500;
        for (int i = 0; i <= res; i++) {
            double theta = 3.1415926535 * 2 / res * i;
            glm::vec3 vec = common::components::types::toVec3AU(orb, theta);
            orbit_points.push_back(vec);
        }
        auto& line = m_universe.emplace<PlanetOrbit>(body);
        // Get the orbit line
        // Do the points
        line.orbit_mesh = engine::primitive::CreateLineSequence(orbit_points);
    }
}

glm::vec3 SysStarSystemRenderer::GetMouseIntersectionOnObject(int mouse_x, int mouse_y) {
    // Normalize 3d device coordinates
    namespace cqspb = cqsp::common::components::bodies;
    auto bodies = m_app.GetUniverse().view<ToRender, cqspb::Body>();
    for (entt::entity ent_id : bodies) {
        glm::vec3 object_pos = CalculateCenteredObject(ent_id);
        float x = (2.0f * mouse_x) / m_app.GetWindowWidth() - 1.0f;
        float y = 1.0f - (2.0f * mouse_y) / m_app.GetWindowHeight();
        float z = 1.0f;

        glm::vec3 ray_wor = CalculateMouseRay(glm::vec3(x, y, z));

        float radius = 1;
        if (m_app.GetUniverse().all_of<cqspb::LightEmitter>(ent_id)) {
            radius = 10;
        }

        // Check for intersection for sphere
        glm::vec3 sub = cam_pos - object_pos;
        float b = glm::dot(ray_wor, sub);
        float c = glm::dot(sub, sub) - radius * radius;

        glm::vec3 closest_hit = cam_pos + (-b - sqrt(b * b - c)) * ray_wor;
        // Get the closer value
        if ((b * b - c) >= 0) {
            is_rendering_founding_city = true;
            on_planet = ent_id;
            return closest_hit;
        }
    }
    is_rendering_founding_city = false;
    return glm::vec3(0, 0, 0);
}

entt::entity SysStarSystemRenderer::GetMouseOnObject(int mouse_x, int mouse_y) {
    namespace cqspb = cqsp::common::components::bodies;

    // Loop through objects
    auto bodies = m_app.GetUniverse().view<ToRender, cqspb::Body>();
    for (entt::entity ent_id : bodies) {
        glm::vec3 object_pos = CalculateCenteredObject(ent_id);
        // Check if the sphere is rendered or not
        if (glm::distance(object_pos, cam_pos) > 100) {
            // Calculate circle
            glm::vec3 pos = glm::project(object_pos, camera_matrix, projection, viewport);
            if (pos.z >= 1) {
                continue;
            }

            // Check if it's intersecting
            float dim = circle_size * m_app.GetWindowHeight();
            if (glm::distance(glm::vec2(pos.x, m_app.GetWindowHeight() - pos.y),
                    glm::vec2(mouse_x, mouse_y)) <= dim) {
                m_app.GetUniverse().emplace<MouseOverEntity>(ent_id);
                return ent_id;
            }
        } else {
            // Normalize 3d device coordinates
            float x = (2.0f * mouse_x) / m_app.GetWindowWidth() - 1.0f;
            float y = 1.0f - (2.0f * mouse_y) / m_app.GetWindowHeight();
            float z = 1.0f;

            glm::vec3 ray_wor = CalculateMouseRay(glm::vec3(x, y, z));

            float radius = 1;
            if (m_app.GetUniverse().all_of<cqspb::LightEmitter>(ent_id)) {
                radius = 10;
            }

            // Check for intersection for sphere
            glm::vec3 sub = cam_pos - object_pos;
            float b = glm::dot(ray_wor, sub);
            float c = glm::dot(sub, sub) - radius * radius;

            // Get the closer value
            if ((b * b - c) >= 0) {
                m_app.GetUniverse().emplace<MouseOverEntity>(ent_id);
                return ent_id;
            }
        }
    }
    return entt::null;
}

bool cqsp::client::systems::SysStarSystemRenderer::IsFoundingCity(common::Universe& universe) {
    return universe.view<CityFounding>().size() >= 1;
}

void SysStarSystemRenderer::DrawOrbit(const entt::entity &entity) {
    if (!m_universe.any_of<PlanetOrbit>(entity)) {
        return;
    }
    glm::vec3 center = glm::vec3(0, 0, 0);
    // If it has a parent, draw around the parent
    entt::entity ref;
    if ((ref = m_universe.get<common::components::types::Orbit>(entity).reference_body) != entt::null) {
        center = m_universe.get<common::components::types::Kinematics>(ref).position * scale_size;
    }
    glm::mat4 transform = glm::mat4(1.f);
    transform = glm::translate(transform, CalculateCenteredObject(center));
    transform = glm::scale(transform, glm::vec3(scale_size, scale_size, scale_size));
    // Draw orbit
    orbit_line.SetMVP(transform, camera_matrix, m_app.Get3DProj());
    orbit_line.shaderProgram->Set("color", glm::vec4(1, 1, 1, 1));
    // Set to the center of the universe
    auto& orbit = m_universe.get<PlanetOrbit>(entity);
    orbit.orbit_mesh->Draw();
}

SysStarSystemRenderer::~SysStarSystemRenderer() {
}
