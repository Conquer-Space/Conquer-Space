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
#include <stb_image.h>

#include <cmath>
#include <string>
#include <memory>
#include <limits>
#include <vector>
#include <algorithm>
#include <tuple>

#include <tracy/Tracy.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/quaternion.hpp>

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
#include "client/systems/gui/systooltips.h"

namespace cqspb = cqsp::common::components::bodies;

namespace cqsp::client::systems {
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
    cqsp::asset::Texture* roughness = nullptr;
};

struct PlanetOrbit {
    cqsp::engine::Mesh *orbit_mesh;

    ~PlanetOrbit() {
        delete orbit_mesh;
    }
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
    near_shader =
        m_app.GetAssetManager().GetAsset<asset::ShaderDefinition>("core:neartexturedobject")->MakeShader();
    textured_planet.mesh = sphere_mesh;
    textured_planet.shaderProgram = textured_planet_shader;

    // Initialize sun
    sun.mesh = sphere_mesh;
    sun.shaderProgram = m_app.GetAssetManager().GetAsset<asset::ShaderDefinition>("core:sunshader")->MakeShader();

    orbit_shader = sun.shaderProgram;

    auto buffer_shader = m_app.GetAssetManager().GetAsset<asset::ShaderDefinition>("core:framebuffer")->MakeShader();

    ship_icon_layer = renderer.AddLayer<engine::FramebufferRenderer>(buffer_shader, *m_app.GetWindow());
    physical_layer = renderer.AddLayer<engine::FramebufferRenderer>(buffer_shader, *m_app.GetWindow());
    planet_icon_layer = renderer.AddLayer<engine::FramebufferRenderer>(buffer_shader, *m_app.GetWindow());
    skybox_layer = renderer.AddLayer<engine::FramebufferRenderer>(buffer_shader, *m_app.GetWindow());

    earth_map_texture =
        m_app.GetAssetManager().GetAsset<asset::Texture>("province_map_texture");
        // Get the country

    auto bin_asset =
        m_app.GetAssetManager().GetAsset<asset::BinaryAsset>("province_map");
    uint64_t file_size = bin_asset->data.size();
    int comp = 0;
    auto d = stbi_load_from_memory(bin_asset->data.data(), file_size, &province_width, &province_height,
                                   &comp, 0);

    std::copy(&d[0], &d[province_width * province_height * comp],
              std::back_inserter(country_map));

    namespace cqspt = cqsp::common::components::types;
    auto orbits = m_app.GetUniverse().view<cqspt::Orbit>();
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
        if (textures.roughness_name != "") {
            data.roughness = m_app.GetAssetManager().GetAsset<cqsp::asset::Texture>("core:" + textures.roughness_name);
        }
    }

    // Set scroll to radius of sun
    if (m_universe.sun != entt::null && m_universe.any_of<cqspb::Body>(m_universe.sun)) {
        scroll = m_universe.get<cqspb::Body>(m_universe.sun).radius * 5;
    }
}

void SysStarSystemRenderer::SeeEntity() {
    namespace cqspb = cqsp::common::components::bodies;

    // See the object
    view_center = CalculateObjectPos(m_viewing_entity);

    // Set the variable
    if (m_app.GetUniverse().all_of<cqspb::Body>(m_viewing_entity)) {
        scroll =
            m_app.GetUniverse().get<cqspb::Body>(m_viewing_entity).radius * 2.5;
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

    // Discern between showing UI and other things
    CityDetection();

    if (!ImGui::GetIO().WantCaptureMouse && !m_app.GetRmlUiContext()->IsMouseInteracting()) {
        CalculateScroll();

        if (m_app.MouseButtonIsHeld(engine::MouseInput::LEFT)) {
            view_x += deltaX/m_app.GetWindowWidth()*3.1415*4;
            view_y -= deltaY/m_app.GetWindowHeight()*3.1415*4;

            if (glm::degrees(view_y) > 89.f) {
                view_y = glm::radians(89.f);
            }
            if (glm::degrees(view_y) < -89.f) {
                view_y = glm::radians(-89.f);
            }
            selected_city = entt::null;
        }

        previous_mouseX = m_app.GetMouseX();
        previous_mouseY = m_app.GetMouseY();

        // If clicks on object, go to the planet
        entt::entity ent = m_app.GetUniverse().view<MouseOverEntity>().front();
        if (m_app.MouseButtonIsReleased(engine::MouseInput::LEFT) && ent != entt::null && !m_app.MouseDragged()) {
            // Then go to the object
            // See if you're seeing planet
            // Check the focused planet
            entt::entity focused_planet =
                m_app.GetUniverse().view<FocusedPlanet>().front();

            // if the focused planet is the current planet, then check if it's close enough, and then do the things
            if (ent == focused_planet) {
                auto& body = m_universe.get<cqsp::common::components::bodies::Body>(focused_planet);
                // 100 km above the city
                // Then select city
                // Get distance
                auto& kin =
                    m_universe.get<cqsp::common::components::types::Kinematics>(
                        focused_planet);

                if (scroll > body.radius * 10) {
                    // Planet selection
                    SeePlanet(ent);
                } else {
                        // Country selection
                        // Then select planet and tell the state
                        selected_country_color = selected_province_color;
                        selected_province = hovering_province;
                        // Set the selected province
                        if (m_universe.valid(selected_province)) {
                        m_universe.clear<cqsp::client::ctx::SelectedProvince>();
                        m_universe.emplace_or_replace<
                            cqsp::client::ctx::SelectedProvince>(
                            selected_province);
                        countries = true;
                        }
                }
            } else {
                SeePlanet(ent);
            }

            if (is_founding_city) {
                namespace cqspt = cqsp::common::components::types;
                namespace cqspc = cqsp::common::components;

                auto s = GetCitySurfaceCoordinate();
                SPDLOG_INFO("Founding city at {} {}", s.latitude(), s.longitude());

                entt::entity settlement =
                    cqsp::common::actions::CreateCity(m_app.GetUniverse(), on_planet, s.latitude(), s.longitude());
                // Set the name of the city
                cqspc::Name& name = m_app.GetUniverse().emplace<cqspc::Name>(settlement);
                name.name = m_app.GetUniverse().name_generators["Town Names"].Generate("1");

                // Set country
                // Add population and economy
                m_app.GetUniverse().emplace<cqspc::IndustrialZone>(settlement);

                m_app.GetUniverse().clear<CityFounding>();

                CalculateCityPositions();
            }
        }
        // Some math if you're close enough you select the city instead of the planet
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
    // Calculate camera
    CenterCameraOnCity();
}

void SysStarSystemRenderer::SeePlanet(entt::entity ent) {
    m_app.GetUniverse().clear<FocusedPlanet>();
    m_app.GetUniverse().emplace<FocusedPlanet>(ent);
}

void SysStarSystemRenderer::DoUI(float deltaTime) {
#ifdef NDEBUG
    return;
#endif
    // FIXME(EhWhoamI)
    //auto &debug_info = m_app.GetUniverse().ctx().emplace<ctx::StarSystemViewDebug>();
    ImGui::Begin("Debug ui window");
    ImGui::TextFmt("{} {} {}", cam_pos.x, cam_pos.y, cam_pos.z);
    ImGui::TextFmt("{} {} {}", view_center.x, view_center.y, view_center.z);
    ImGui::TextFmt("{}", scroll);
    ImGui::TextFmt("{} {}", view_x, view_y);
    // Get the province name
    std::string country_name_t = "";
    if (m_universe.valid(selected_province) && m_universe.any_of<common::components::Province>(selected_province)) {
        country_name_t =
            systems::gui::GetName(m_universe, m_universe.get<common::components::Province>(selected_province).country);
    }
    ImGui::TextFmt("{} {}", tex_x, tex_y);
    ImGui::TextFmt("{} {} {}", tex_r, tex_g, tex_b);
    ImGui::TextFmt("{} {} {}", selected_country_color.x, selected_country_color.y, selected_country_color.z);
    ImGui::TextFmt("Focused planets: {}",
        m_universe.view<FocusedPlanet>().size());
    entt::entity earth = m_universe.planets["earth"];
    auto& b = m_universe.get<cqspb::Body>(earth);
    float offset = (float) b.rotation_offset;
    ImGui::SliderAngle("asdf", &offset);
    b.rotation_offset = (float) offset;
    ImGui::End();

    // Get the focused object, and display their information
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - 200,
                                   ImGui::GetIO().DisplaySize.y - 300), ImGuiCond_Appearing);
    ImGui::Begin("Focused Object");
    cqsp::client::systems::gui::EntityTooltipContent(m_universe,
                                                     m_viewing_entity);
    // Edit the orbit if there is an issue
    // Get normalized vector
    if (m_universe.valid(m_viewing_entity) &&
            m_universe
            .any_of<common::components::types::Kinematics>(
            m_viewing_entity)) {
        const auto& kin = m_universe.get<common::components::types::Kinematics>(
            m_viewing_entity);
        auto norm = glm::normalize(kin.velocity);
        ImGui::TextFmt("Prograde vector: {} {} {}", norm.x, norm.y, norm.z);

        static float delta_v = 0.01;
        norm *= delta_v;
        glm::vec3 final_velocity = kin.velocity + norm;
        ImGui::TextFmt("Velocity vector: {} {} {}", final_velocity.x,
                       final_velocity.y, final_velocity.z);

        if (ImGui::Button("Burn prograde")) {
            // Add 10m/s prograde or something
            auto& impulse =
                m_universe.get_or_emplace<common::components::types::Impulse>(
                    m_viewing_entity);
            impulse.impulse += norm;
        }
        ImGui::SliderFloat("Text", &delta_v, -1, 1);
    }
    ImGui::End();
}

void SysStarSystemRenderer::DrawStars() {
    ZoneScoped;
    // Draw stars
    namespace cqspb = cqsp::common::components::bodies;
    namespace cqsps = cqsp::common::components::ships;
    auto stars = m_app.GetUniverse().view<ToRender, cqspb::Body, cqspb::LightEmitter>();
    renderer.BeginDraw(physical_layer);
    for (auto ent_id : stars) {
        // Draw the star circle
        glm::vec3 object_pos = CalculateCenteredObject(ent_id);
        sun_position = object_pos;
        DrawStar(ent_id, object_pos);
    }
    renderer.EndDraw(physical_layer);
}

void SysStarSystemRenderer::DrawBodies() {
    ZoneScoped;
    namespace cqspb = cqsp::common::components::bodies;
    namespace cqsps = cqsp::common::components::ships;
    namespace cqspt = cqsp::common::components::types;

    // Draw other bodies
    auto bodies = m_app.GetUniverse().view<ToRender, cqspb::Body>(entt::exclude<cqspb::LightEmitter>);
    const double dist = 0.4;
    renderer.BeginDraw(planet_icon_layer);
    glDepthFunc(GL_ALWAYS);
    {
        ZoneScopedN("PlanetBillboards");
        for (auto body_entity : bodies) {
            // Draw the planet circle
            glm::vec3 object_pos = CalculateCenteredObject(body_entity);

            namespace cqspc = cqsp::common::components;
            if (glm::distance(object_pos, cam_pos) > dist || true) {
                // Check if it's obscured by a planet, but eh, we can deal with
                // it later Set planet circle color
                planet_circle.shaderProgram->UseProgram();
                planet_circle.shaderProgram->setVec4("color", 0, 0, 1, 1);
                // DrawPlanetIcon(object_pos);
                // DrawEntityName(object_pos, body_entity);
                DrawPlanetBillboards(body_entity, object_pos);
                continue;
            }
        }
    }
    renderer.EndDraw(planet_icon_layer);
    glDepthFunc(GL_LESS);
    renderer.BeginDraw(physical_layer);
    {
        ZoneScopedN("Planets");
        for (entt::entity body_entity : bodies) {
            glm::vec3 object_pos = CalculateCenteredObject(body_entity);

            namespace cqspc = cqsp::common::components;

            // This can probably switched to some log system based off the mass of
            // a planet.
            if (glm::distance(object_pos, cam_pos) <= dist || true) {
                // Check if planet has terrain or not
                // Don't actually use proc-gen terrain for now
                //if (m_app.GetUniverse().all_of<cqspb::Terrain>(body_entity)) {
                    // Do empty terrain
                    // Check if the planet has the thing
                    //DrawPlanet(object_pos, body_entity);
                if (m_app.GetUniverse().all_of<cqspb::TexturedTerrain>(body_entity)) {
                    DrawTexturedPlanet(object_pos, body_entity);
                } else {
                    DrawTerrainlessPlanet(body_entity, object_pos);
                }
            }
        }
    }

    auto orbits = m_universe.view<cqspt::Orbit>();

    {
        ZoneScopedN("Orbits")
        for (entt::entity orbit_entity : orbits) {
            DrawOrbit(orbit_entity);
        }
    }
    renderer.EndDraw(physical_layer);

    renderer.BeginDraw(ship_icon_layer);
    for (auto body_entity : bodies) {
        glm::vec3 object_pos = CalculateCenteredObject(body_entity);
        // if (glm::distance(object_pos, cam_pos) <= dist) {
        RenderCities(object_pos, body_entity);
        //}
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
    // Shrink everything, I guess
    glm::mat4 planetDispMat = glm::mat4(1.0f);

    // Calculate camera direction and position
    // The two directions
    // Get angle between 2 things

    // Check if it's in front of camera or behind

    planetDispMat = glm::translate(planetDispMat, TranslateToNormalized(pos));
    planetDispMat = glm::scale(planetDispMat, glm::vec3(circle_size, circle_size, circle_size));

    float window_ratio = GetWindowRatio();
    planetDispMat = glm::scale(planetDispMat, glm::vec3(1, window_ratio, 1));
    glm::mat4 twodimproj = glm::mat4(1.0f);

    planet_circle.shaderProgram->setMat4("model", planetDispMat);
    planet_circle.shaderProgram->setMat4("projection", twodimproj);

    engine::Draw(planet_circle);
}

void SysStarSystemRenderer::DrawPlanetBillboards(const entt::entity& ent_id,
                                                                        const glm::vec3& object_pos) {
    using cqsp::common::components::Name;
    std::string text = "";
    if (m_app.GetUniverse().all_of<Name>(ent_id)) {
        text = m_app.GetUniverse().get<Name>(ent_id);
    } else {
        text = fmt::format("{}", ent_id);
    }
    // Camera looking direction is
    // textured_planet.SetMVP(position, camera_matrix, projection);
    glm::vec3 pos = glm::project(object_pos, camera_matrix, projection, viewport);
    glm::vec4  gl_Position = projection * camera_matrix * glm::vec4(object_pos, 1.0);
    float C = 0.0001;
    float far = 9.461e12;
    gl_Position.z = 2.0* log(gl_Position.w*C + 1)/log(far*C + 1) - 1;

    gl_Position.z *= gl_Position.w;
    text = fmt::format("{}", text);

    // Check if the position on screen is within bounds
    if (!(!(isnan(gl_Position.z)) &&
        (pos.x > 0 && pos.x < m_app.GetWindowWidth() &&
            pos.y > 0 && pos.y < m_app.GetWindowHeight()))) {
        return;
    }

    auto planetDispMat = glm::translate(glm::mat4(1.0f), TranslateToNormalized(pos));
    planetDispMat = glm::scale(planetDispMat, glm::vec3(circle_size, circle_size, circle_size));

    planetDispMat = glm::scale(planetDispMat, glm::vec3(1, GetWindowRatio(), 1));
    glm::mat4 twodimproj = glm::mat4(1.0f);

    planet_circle.shaderProgram->UseProgram();
    planet_circle.shaderProgram->setMat4("model", planetDispMat);
    planet_circle.shaderProgram->setMat4("projection", twodimproj);

    engine::Draw(planet_circle);

    m_app.DrawText(text, pos.x, pos.y, 20);
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

    shipDispMat = glm::translate(shipDispMat, TranslateToNormalized(pos));

    shipDispMat = glm::scale(shipDispMat, glm::vec3(circle_size, circle_size, circle_size));

    float window_ratio = GetWindowRatio();
    glm::vec4 gl_Position =
        projection * camera_matrix * glm::vec4(object_pos, 1.0);
    float C = 0.0001;
    float far = 9.461e12;
    gl_Position.z = 2.0 * log(gl_Position.w * C + 1) / log(far * C + 1) - 1;

    gl_Position.z *= gl_Position.w;

    // Check if the position on screen is within bounds
    if (!(!(isnan(gl_Position.z)) &&
          (pos.x > 0 && pos.x < m_app.GetWindowWidth() && pos.y > 0 &&
           pos.y < m_app.GetWindowHeight()))) {
        return;
    }

    shipDispMat = glm::scale(shipDispMat, glm::vec3(1, window_ratio, 1));
    glm::mat4 twodimproj = glm::mat4(1.0f);
    ship_overlay.shaderProgram->UseProgram();
    ship_overlay.shaderProgram->setMat4("model", shipDispMat);
    ship_overlay.shaderProgram->setMat4("projection", twodimproj);

    engine::Draw(ship_overlay);
}

void SysStarSystemRenderer::DrawTexturedPlanet(glm::vec3 &object_pos, entt::entity entity) {
    bool have_normal = false;
    bool have_roughness = false;
    if (m_universe.all_of<PlanetTexture>(entity)) {
        auto& terrain_data = m_universe.get<PlanetTexture>(entity);
        textured_planet.textures.clear();
        textured_planet.textures.push_back(terrain_data.terrain);
        if (terrain_data.normal) {
            have_normal = true;
            textured_planet.textures.push_back(terrain_data.normal);
        } else {
            textured_planet.textures.push_back(terrain_data.terrain);
        }
        textured_planet.textures.push_back(earth_map_texture);
        if (terrain_data.roughness) {
            have_roughness = true;
            textured_planet.textures.push_back(terrain_data.roughness);
        }
    }

    namespace cqspb = cqsp::common::components::bodies;
    namespace cqspt = cqsp::common::components::types;
    auto& body = m_universe.get<cqspb::Body>(entity);

    glm::mat4 position = glm::mat4(1.f);
    position = glm::translate(position, object_pos);
    position *= glm::mat4(GetBodyRotation(body.axial, body.rotation, body.rotation_offset));

    // Rotate
    float scale = body.radius;  // cqsp::common::components::types::toAU(body.radius)
                                // * view_scale;
    position = glm::scale(position, glm::vec3(scale));

    auto shader = textured_planet.shaderProgram.get();
    if (scale < 10.f) {
        // then use different shader if it's close enough
        // This is relatively hacky, so try not to do it
        shader = near_shader.get();
        glDepthFunc(GL_ALWAYS);
    }

    shader->SetMVP(position, camera_matrix, projection);
    shader->UseProgram();

    // Maybe a seperate shader for planets without normal maps would be better
    shader->setBool("haveNormal", have_normal);

    shader->setVec3("lightDir", glm::normalize(sun_position - object_pos));

    shader->setVec3("lightDir", glm::normalize(sun_position - object_pos));
    shader->setVec3("lightPosition", sun_position);

    shader->setVec3("lightColor", sun_color);
    shader->setVec3("viewPos", cam_pos);
    // If a country is clicked on...
    shader->setVec4("country_color", glm::vec4(selected_country_color, 1));
    shader->setBool("country", countries);
    shader->setBool("is_roughness", have_roughness);

    engine::Draw(textured_planet, shader);
    glDepthFunc(GL_LESS);
}

void SysStarSystemRenderer::DrawPlanet(glm::vec3 &object_pos, entt::entity entity) {
    // TODO(EhWhoAmI): Maybe don't reload the textures all the time
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
    planet.shaderProgram->setVec4("country_color", glm::vec4(selected_province_color, 1));
    planet.shaderProgram->setBool("country", true);
    using cqsp::common::components::bodies::TerrainData;
    entt::entity terrain = m_universe.get<cqsp::common::components::bodies::Terrain>(entity).terrain_type;
    planet.shaderProgram->Set("seaLevel", m_universe.get<TerrainData>(terrain).sea_level);
    engine::Draw(planet);
}

void SysStarSystemRenderer::DrawStar(const entt::entity& entity, glm::vec3 &object_pos) {
    glm::mat4 position = glm::mat4(1.f);
    position = glm::translate(position, object_pos);

    glm::mat4 transform = glm::mat4(1.f);
    // Scale it by radius
    const double& radius = m_universe.get<common::components::bodies::Body>(entity).radius;
    double scale = radius;
    transform = glm::scale(transform, glm::vec3(scale, scale, scale));
    position = position * transform;

    sun.SetMVP(position, camera_matrix, projection);
    sun.shaderProgram->setVec4("color", 1, 1, 1, 1);
    engine::Draw(sun);
}

void SysStarSystemRenderer::DrawTerrainlessPlanet(const entt::entity& entity, glm::vec3 &object_pos) {
    namespace cqspb = cqsp::common::components::bodies;

    glm::mat4 position = glm::mat4(1.f);
    position = glm::translate(position, object_pos);
    float scale = 300;
    if (m_universe.any_of<cqspb::Body>(entity)) {
        scale = m_universe.get<cqspb::Body>(entity).radius;
    }

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

    auto& body = m_app.GetUniverse().get<cqspc::bodies::Body>(body_entity);
    auto quat = GetBodyRotation(body.axial, body.rotation, body.rotation_offset);

    // Rotate the body
    // Put in same layer as ships
    city.shaderProgram->UseProgram();
    city.shaderProgram->setVec4("color", 0.5, 0.5, 0.5, 1);
    for (auto city_entity : cities) {
        // Calculate position to render
        if (!m_app.GetUniverse().any_of<Offset>(city_entity)) {
            // Calculate offset
            return;
        }
        Offset doffset = m_app.GetUniverse().get<Offset>(city_entity);
        glm::vec3 city_pos = m_app.GetUniverse().get<Offset>(city_entity).offset * (float)body.radius;
        // Check if line of sight and city position intersects the sphere that is the planet
        city_pos = quat * city_pos;
        glm::vec3 city_world_pos = city_pos + object_pos;
        if (CityIsVisible(city_world_pos, object_pos, cam_pos, body.radius)) {
            // If it's reasonably close, then we can show city names
            //if (scroll < 3) {
                DrawEntityName(city_world_pos, city_entity);
            //}
            DrawCityIcon(city_world_pos);
        }
    }

    if (is_founding_city && is_rendering_founding_city) {
        DrawCityIcon(city_founding_position);
    }
}

bool SysStarSystemRenderer::CityIsVisible(glm::vec3 city_pos, glm::vec3 planet_pos, glm::vec3 cam_pos, double radius) {
    float d = glm::distance(cam_pos, city_pos);
    float D = glm::distance(cam_pos, planet_pos);

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
    for (auto& city_entity : cities) {
        if (!m_app.GetUniverse().all_of<cqspt::SurfaceCoordinate>(city_entity)) {
            continue;
        }
        auto& coord = m_app.GetUniverse().get<cqspt::SurfaceCoordinate>(city_entity);
        cqspb::Body parent = m_app.GetUniverse().get<cqspb::Body>(m_viewing_entity);
        m_app.GetUniverse().emplace_or_replace<Offset>(
            city_entity, cqspt::toVec3(coord.universe_view(), 1));
    }
    SPDLOG_INFO("Calculated offset");
}

void SysStarSystemRenderer::CalculateScroll() {
    namespace cqspb = cqsp::common::components::bodies;
    double min_scroll;
    if (m_viewing_entity == entt::null || !m_app.GetUniverse().all_of<cqspb::Body>(m_viewing_entity)) {
        // Scroll i
        min_scroll = 0.1;
    } else {
        min_scroll = std::max(m_app.GetUniverse().get<cqspb::Body>(m_viewing_entity).radius * 1.1, 0.1);
    }
    if (scroll - m_app.GetScrollAmount() * 3 * scroll / 33 <= min_scroll) {
        return;
    }
    scroll -= m_app.GetScrollAmount() * 3 * scroll / 33;
}

glm::quat SysStarSystemRenderer::GetBodyRotation(double axial, double rotation, double day_offset) {
    namespace cqspt = cqsp::common::components::types;
    float rot = (float)common::components::bodies::GetPlanetRotationAngle(
        m_universe.date.ToSecond(), rotation, day_offset);
    if (rotation == 0) {
        rot = 0;
    }
    return glm::quat {{(float)-axial, 0, 0}} *
           glm::quat {{0, (float)std::fmod(rot, cqspt::TWOPI), 0}};
}

void SysStarSystemRenderer::FocusCityView() {
    ZoneScoped;
    auto focused_city_view = m_app.GetUniverse().view<FocusedCity>();
    // City to focus view on
    if (focused_city_view.empty()) {
        return;
    }
    selected_city = focused_city_view.front();
    m_universe.clear<FocusedCity>();

    CenterCameraOnCity();
    entt::entity planet = m_app.GetUniverse().view<FocusedPlanet>().front();
    auto& body = m_universe.get<cqsp::common::components::bodies::Body>(planet);
    // 100 km above the city
    scroll = body.radius + 100;
}

glm::vec3 SysStarSystemRenderer::CalculateObjectPos(const entt::entity &ent) {
    namespace cqspt = cqsp::common::components::types;
    // Get the position
    if (m_universe.all_of<cqspt::Kinematics>(ent)) {
        auto& kin = m_universe.get<cqspt::Kinematics>(ent);
        const auto& pos = kin.position + kin.center;
        return ConvertPoint(pos);
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

glm::vec3 SysStarSystemRenderer::ConvertPoint(const glm::vec3& pos) {
    return glm::vec3(pos.x, pos.z, -pos.y);
}

void SysStarSystemRenderer::CenterCameraOnCity() {
    namespace cqspt = common::components::types;
    if (selected_city == entt::null) {
        return;
    }

    if (!m_universe.any_of<cqspt::SurfaceCoordinate>(selected_city)) {
        return;
    }

    auto& surf = m_universe.get<cqspt::SurfaceCoordinate>(selected_city);
    // TODO(EhWhoAmI): Change this so that it doesn't have to change the
    // coordinate system multiple times. Currently this changes from surface
    // coordinates to 3d coordinates to surface coordinates. I think it can be
    // solved with a basic formula.
    entt::entity planet = m_app.GetUniverse().view<FocusedPlanet>().front();
    auto& body = m_universe.get<cqsp::common::components::bodies::Body>(planet);

    glm::quat quat =
        GetBodyRotation(body.axial, body.rotation, body.rotation_offset);

    glm::vec3 vec = cqspt::toVec3(surf, 1);
    auto s = quat * vec;
    glm::vec3 pos = glm::normalize(s);
    view_x = atan2(s.x, s.z);
    view_y = -acos(s.y) + cqspt::PI / 2;
}

glm::vec3 SysStarSystemRenderer::CalculateCenteredObject(const entt::entity &ent) {
    return CalculateCenteredObject(CalculateObjectPos(ent));
}

void SysStarSystemRenderer::CalculateCamera() {
    cam_pos = glm::vec3(
                cos(view_y) * sin(view_x),
                sin(view_y),
                cos(view_y) * cos(view_x)) * (float) scroll;
    cam_up = glm::vec3(0.0f, 1.0f, 0.0f);
    camera_matrix = glm::lookAt(cam_pos, glm::vec3(0.f, 0.f, 0.f), cam_up);
    projection = glm::infinitePerspective(glm::radians(45.f), GetWindowRatio(), 0.1f);
    viewport = glm::vec4(0.f, 0.f, m_app.GetWindowWidth(), m_app.GetWindowHeight());
}

void SysStarSystemRenderer::MoveCamera(double deltaTime) {
    // Now navigation for changing the center
    glm::vec3 dir = (view_center - cam_pos);
    float velocity = deltaTime * 30.f * scroll / 40;
    // Get distance from the pane
    // Remove y axis
    glm::vec3 forward = glm::normalize(glm::vec3(glm::sin(view_x), 0, glm::cos(view_x)));
    glm::vec3 right = glm::normalize(glm::cross(forward, cam_up));
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
    ZoneScoped;
    SPDLOG_TRACE("Creating planet orbits");
    namespace cqspt = common::components::types;
    auto orbits = m_app.GetUniverse().view<cqspt::Orbit, cqspt::OrbitDirty>();

    // Initialize all the orbits and stuff
    // Get sun orbits
    /*
    for (auto body : orbits) {
        ZoneScoped;
        // Generate the orbit
        auto& orb = m_universe.get<common::components::types::Orbit>(body);
        if (orb.semi_major_axis == 0) {
            continue;
        }
        const int res = 500;
        std::vector<glm::vec3> orbit_points;
        orbit_points.reserve(res);
        for (int i = 0; i <= res; i++) {
            ZoneScoped;
            double theta = 3.1415926535 * 2 / res * i;
            glm::vec3 vec = common::components::types::toVec3(orb, theta);
            // Convert to opengl
            orbit_points.push_back(glm::vec3(vec.x, vec.z, vec.y));
        }
        m_universe.remove<cqspt::OrbitDirty>(body);
        auto& line = m_universe.get_or_emplace<PlanetOrbit>(body);
        // Get the orbit line
        // Do the points
        delete line.orbit_mesh;
        line.orbit_mesh = engine::primitive::CreateLineSequence(orbit_points);
    }*/

    auto orbit2 = m_universe.view<cqspt::Orbit>(entt::exclude<PlanetOrbit>);
    int i = 0;
    for (auto body : orbit2) {
        GenerateOrbit(body);
        i++;
    }

    auto orbit4 = m_universe.view<cqspt::Orbit, cqspb::DirtyOrbit>();
    for (auto body : orbit4) {
        GenerateOrbit(body);
        m_universe.remove<cqspb::DirtyOrbit>(body);
    }
}

common::components::types::SurfaceCoordinate
SysStarSystemRenderer::GetCitySurfaceCoordinate() {
    namespace cqspt = cqsp::common::components::types;
    namespace cqspc = cqsp::common::components;

    if (on_planet == entt::null || !m_universe.valid(on_planet)) {
        return cqspt::SurfaceCoordinate(0, 0);
    }

    glm::vec3 p = city_founding_position - CalculateCenteredObject(on_planet);
    p = glm::normalize(p);

    auto& planet_comp = m_app.GetUniverse().get<cqspc::bodies::Body>(on_planet);
    auto quat = GetBodyRotation(planet_comp.axial, planet_comp.rotation,
                                planet_comp.rotation_offset);
    // Rotate the vector based on the axial tilt and rotation.
    p = glm::inverse(quat) * p;
    cqspt::SurfaceCoordinate s = cqspt::ToSurfaceCoordinate(p);
    s = cqspt::SurfaceCoordinate(s.latitude(), s.longitude() + 90);
    return s;
}

void SysStarSystemRenderer::CityDetection() {
    ZoneScoped;
    if (on_planet == entt::null || !m_universe.valid(on_planet)) {
        return;
    }
    auto s = GetCitySurfaceCoordinate();

    // Look for the vector
    // Rotate based on the axial tilt and roation
    int x = (-1 * (s.latitude() * 2 - 180)) / 360 * province_height;
    int y = fmod(s.longitude() + 180, 360) / 360. * province_width;
    int pos = (x * province_width + y) * 4;
    tex_x = x;
    tex_y = y;
    std::tuple<int, int, int, int> t =
        std::make_tuple(country_map[pos], country_map[pos + 1],
                        country_map[pos + 2], country_map[pos + 3]);
    tex_r = std::get<0>(t);
    tex_g = std::get<1>(t);
    tex_b = std::get<2>(t);

    if (std::get<3>(t) == 0) {
        // Then ignore because it is ocean
        return;
    }
    ZoneNamed(LookforProvince, true);
    {
        selected_province_color =
            (glm::vec3(std::get<0>(t), std::get<1>(t), std::get<2>(t)) / 255.f);
        int i = common::components::ProvinceColor::toInt(tex_r, tex_g, tex_b);
        if (m_universe.province_colors.find(i) !=
            m_universe.province_colors.end()) {
            hovering_province = m_universe.province_colors[i];
        } else {
        }
    }
}

glm::vec3 SysStarSystemRenderer::GetMouseIntersectionOnObject(int mouse_x, int mouse_y) {
    ZoneScoped;
    // Normalize 3d device coordinates
    namespace cqspb = cqsp::common::components::bodies;
    auto bodies = m_app.GetUniverse().view<ToRender, cqspb::Body>();
    for (entt::entity ent_id : bodies) {
        glm::vec3 object_pos = CalculateCenteredObject(ent_id);
        float x = (2.0f * mouse_x) / m_app.GetWindowWidth() - 1.0f;
        float y = 1.0f - (2.0f * mouse_y) / m_app.GetWindowHeight();
        float z = 1.0f;

        glm::vec3 ray_wor = CalculateMouseRay(glm::vec3(x, y, z));
        auto& body = m_app.GetUniverse().get<cqspb::Body>(ent_id);
        float radius = body.radius;

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

void SysStarSystemRenderer::GenerateOrbit(entt::entity body) {
    // Then produce orbits
    ZoneScoped;
    // Generate the orbit
    auto& orb = m_universe.get<common::components::types::Orbit>(body);
    if (orb.semi_major_axis == 0) {
        return;
    }
    const int res = 500;
    std::vector<glm::vec3> orbit_points;
    double SOI = std::numeric_limits<double>::infinity();
    if (m_universe.valid(orb.reference_body)) {
        SOI =
            m_universe.get<common::components::bodies::Body>(orb.reference_body)
                .SOI;
    }

    orbit_points.reserve(res);
    for (int i = 0; i <= res; i++) {
        ZoneScoped;
        double theta = 3.1415926535 * 2 / res * i;
        glm::vec3 vec = common::components::types::toVec3(orb, theta);
        // If the length is greater than the sphere of influence, then remove it
        if (glm::length(vec) < SOI) {
            // Convert to opengl
            orbit_points.push_back(ConvertPoint(vec));
        }
    }
    // m_universe.remove<cqspt::OrbitDirty>(body);
    auto& line = m_universe.get_or_emplace<PlanetOrbit>(body);
    // Get the orbit line
    // Do the points
    delete line.orbit_mesh;
    line.orbit_mesh = engine::primitive::CreateLineSequence(orbit_points);
}

entt::entity SysStarSystemRenderer::GetMouseOnObject(int mouse_x, int mouse_y) {
    namespace cqspb = cqsp::common::components::bodies;

    // Loop through objects
    auto bodies = m_app.GetUniverse().view<ToRender, cqspb::Body>();
    for (entt::entity ent_id : bodies) {
        glm::vec3 object_pos = CalculateCenteredObject(ent_id);
        // Check if the sphere is rendered or not
        // Normalize 3d device coordinates
        float x = (2.0f * mouse_x) / m_app.GetWindowWidth() - 1.0f;
        float y = 1.0f - (2.0f * mouse_y) / m_app.GetWindowHeight();
        float z = 1.0f;

        glm::vec3 ray_wor = CalculateMouseRay(glm::vec3(x, y, z));
        auto& body = m_app.GetUniverse().get<cqspb::Body>(ent_id);
        float radius = body.radius;

        // Check for intersection for sphere
        glm::vec3 sub = cam_pos - object_pos;
        float b = glm::dot(ray_wor, sub);
        // Object radius
        float c = glm::dot(sub, sub) - radius * radius;

        // Get the closer value
        if ((b * b - c) >= 0) {
            m_app.GetUniverse().emplace<MouseOverEntity>(ent_id);
            return ent_id;
        }
    }
    return entt::null;
}

bool SysStarSystemRenderer::IsFoundingCity(common::Universe& universe) {
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
        center = CalculateObjectPos(ref);
    } else {
        return;
    }
    glm::mat4 transform = glm::mat4(1.f);
    transform = glm::translate(transform, CalculateCenteredObject(center));
    // Actually you just need to rotate the orbit
    auto body = m_universe.get<common::components::bodies::Body>(ref);
    //transform *= glm::mat4(
    //    glm::quat{{0.f, 0, (float)body.axial}});
    // Draw orbit
    orbit_shader->SetMVP(transform, camera_matrix, m_app.Get3DProj());
    orbit_shader->Set("color", glm::vec4(1, 1, 1, 1));
    // Set to the center of the universe
    auto& orbit = m_universe.get<PlanetOrbit>(entity);
    orbit.orbit_mesh->Draw();
}

void SysStarSystemRenderer::OrbitEditor() {
    /*
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
    }*/
}

SysStarSystemRenderer::~SysStarSystemRenderer() {}
}  // namespace cqsp::client::systems
