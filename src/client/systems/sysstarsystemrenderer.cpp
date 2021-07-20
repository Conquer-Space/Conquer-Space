/*
 * Copyright 2021 Conquer Space
 */
#include "client/systems/sysstarsystemrenderer.h"

#include <noise/noise.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/polar_coordinates.hpp>
#include <glm/gtx/string_cast.hpp>

#include "engine/renderer/primitives/uvsphere.h"
#include "engine/renderer/renderer.h"
#include "engine/renderer/primitives/cube.h"
#include "engine/renderer/primitives/polygon.h"
#include "engine/renderer/primitives/pane.h"

#include "common/components/bodies.h"
#include "common/components/organizations.h"
#include "common/components/player.h"
#include "common/components/orbit.h"
#include "common/components/name.h"
#include "common/components/ships.h"

conquerspace::client::systems::SysStarSystemRenderer::SysStarSystemRenderer
                                                (conquerspace::common::components::Universe &_u,
                                                conquerspace::engine::Application &_a) :
                                                m_universe(_u), m_app(_a),
                                                scroll(5), view_x(0),
                                                view_y(0), view_center(glm::vec3(1, 1, 1)),
                                                sun_color(glm::vec3(10, 10, 10)) {
}

void conquerspace::client::systems::SysStarSystemRenderer::Initialize() {
    // Initialize meshes, etc
    conquerspace::engine::Mesh* sphere_mesh = new conquerspace::engine::Mesh();
    conquerspace::primitive::ConstructSphereMesh(64, 64, *sphere_mesh);

    // Initialize sky box
    asset::Texture* sky_texture = m_app.GetAssetManager()
        .GetAsset<conquerspace::asset::Texture>("skycubemap");

    asset::ShaderProgram* skybox_shader =
                        m_app.GetAssetManager().CreateShaderProgram("skycubevert", "skycubefrag");

    sky.mesh = new conquerspace::engine::Mesh();
    primitive::MakeCube(*sky.mesh);
    sky.shaderProgram = skybox_shader;
    sky.SetTexture("texture0", 0, sky_texture);

    asset::ShaderProgram* circle_shader =
                    m_app.GetAssetManager().
                    CreateShaderProgram("shader.pane.vert", "coloredcirclefrag");

    planet_circle.mesh = new conquerspace::engine::Mesh();
    primitive::CreateFilledCircle(*planet_circle.mesh);
    planet_circle.shaderProgram = circle_shader;

    ship_overlay.mesh = new conquerspace::engine::Mesh();
    primitive::CreateFilledTriangle(*ship_overlay.mesh);
    ship_overlay.shaderProgram = circle_shader;

    // Initialize shaders
    asset::ShaderProgram* planet_shader =
                            m_app.GetAssetManager().CreateShaderProgram("objectvert",
                                                                        "planetfrag");
    // Planet spheres
    planet.mesh = sphere_mesh;
    planet_shader->UseProgram();
    planet_shader->setInt("albedomap", 0);
    planet_shader->setInt("heightmap", 1);

    planet.shaderProgram = planet_shader;

    // Initialize sun
    asset::ShaderProgram* star_shader =
                            m_app.GetAssetManager().CreateShaderProgram("objectvert", "sunshader");
    sun.mesh = sphere_mesh;
    sun.shaderProgram = star_shader;


    buffer_renderer.InitTexture();
    primitive::MakeTexturedPaneMesh(buffer_renderer.mesh_output, true);
    buffer_renderer.buffer_shader = *m_app.GetAssetManager().CreateShaderProgram("framebuffervert", "framebufferfrag");
    buffer_renderer.buffer_shader.setInt("screenTexture", 0);


    planet_renderer.InitTexture();
    primitive::MakeTexturedPaneMesh(planet_renderer.mesh_output, true);
    planet_renderer.buffer_shader = *m_app.GetAssetManager().CreateShaderProgram("framebuffervert", "framebufferfrag");
    planet_renderer.buffer_shader.setInt("screenTexture", 0);

    skybox_renderer.InitTexture();
    primitive::MakeTexturedPaneMesh(skybox_renderer.mesh_output, true);
    skybox_renderer.buffer_shader = *m_app.GetAssetManager().CreateShaderProgram("framebuffervert", "framebufferfrag");
    skybox_renderer.buffer_shader.setInt("screenTexture", 0);
}

void conquerspace::client::systems::SysStarSystemRenderer::Render() {
    namespace cqspb = conquerspace::common::components::bodies;
    namespace cqsps = conquerspace::common::components::ships;
    buffer_renderer.BeginDraw();
    buffer_renderer.Clear();
    buffer_renderer.EndDraw();
    planet_renderer.BeginDraw();
    planet_renderer.Clear();
    planet_renderer.EndDraw();

    glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    entt::entity current_planet = m_app.GetUniverse().view<RenderingPlanet>().front();
    if (current_planet != m_viewing_entity) {
        SPDLOG_INFO("Switched displaying planet, seeing {}", current_planet);
        m_viewing_entity = current_planet;
        // Do terrain
        SeeEntity();
    } else if (current_planet != entt::null) {
        //view_center = CalculateObjectPos(m_viewing_entity);
    }

    m_star_system  = m_app.GetUniverse().view<RenderingStarSystem>().front();

    CalculateCamera();

    if (second_terrain_complete && !terrain_complete) {
        SPDLOG_INFO("Done less detailed planet generation");
        less_detailed_terrain_generator_thread.join();
        SetPlanetTexture(intermediate_image_generator);
        second_terrain_complete = false;
    }

    if (terrain_complete) {
        SPDLOG_INFO("Done terrain generation");
        terrain_generator_thread.join();
        SetPlanetTexture(final_image_generator);
        terrain_complete = false;
    }

    // Draw stars
    auto stars = m_app.GetUniverse().view<ToRender, cqspb::Body, cqspb::LightEmitter>();
    for (auto [ent_id, body] : stars.each()) {
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

        DrawStar(object_pos);
    }

    // Draw other bodies
    auto bodies = m_app.GetUniverse().view<ToRender,
                cqspb::Body>(entt::exclude<cqspb::LightEmitter>);
    for (auto [ent_id, body] : bodies.each()) {
        // Draw the planet circle
        glm::vec3 object_pos = CalculateCenteredObject(ent_id);

        if (glm::distance(object_pos, cam_pos) > 200) {
            // Check if it's obscured by a planet, but eh, we can deal with it later
            // Set planet circle color
            planet_circle.shaderProgram->UseProgram();
            planet_circle.shaderProgram->setVec4("color", 0, 0, 1, 1);
            DrawEntityName(object_pos, ent_id);
            DrawPlanetIcon(object_pos);
            continue;
        }

        // Check if planet has terrain or not
        if (m_app.GetUniverse().all_of<cqspb::Terrain>(m_viewing_entity)) {
            // Do empty terrain
            DrawPlanet(object_pos);
        } else {
            DrawTerrainlessPlanet(object_pos);
        }
    }
    // Draw Ships
    auto ships = m_app.GetUniverse().view<ToRender, cqsps::Ship>();
    for (auto [ent_id, ship] : ships.each()) {
        glm::vec3 object_pos = CalculateCenteredObject(ent_id);
        ship_overlay.shaderProgram->setVec4("color", 1, 0, 0, 1);
        DrawShipIcon(object_pos);
    }

    // Draw sky box
    skybox_renderer.BeginDraw();
    skybox_renderer.Clear();
    sky.shaderProgram->UseProgram();
    sky.shaderProgram->setMat4("view", glm::mat4(glm::mat3(camera_matrix)));
    sky.shaderProgram->setMat4("projection", projection);
    glDepthFunc(GL_LEQUAL);
    // skybox cube
    engine::Draw(sky);
    glDepthFunc(GL_LESS);
    skybox_renderer.EndDraw();

    buffer_renderer.RenderBuffer();
    planet_renderer.RenderBuffer();
    skybox_renderer.RenderBuffer();
}

void conquerspace::client::systems::SysStarSystemRenderer::SeeStarSystem(
    entt::entity system) {
    namespace cqspb = conquerspace::common::components::bodies;
    if (m_star_system != entt::null &&
        m_universe.all_of<cqspb::StarSystem>(m_star_system)) {
        // Remove tags
        auto star_system_component =
            m_universe.get<cqspb::StarSystem>(m_star_system);
        for (auto body : star_system_component.bodies) {
            // Add a tag
            m_universe.remove_if_exists<ToRender>(body);
        }
        for (auto ship : star_system_component.ships) {
            // Add a tag
            m_universe.remove_if_exists<ToRender>(ship);
        }
    }

    m_star_system = system;
    auto star_system_component =
        m_universe.get<cqspb::StarSystem>(m_star_system);
    for (auto body : star_system_component.bodies) {
        // Add a tag
        m_universe.emplace_or_replace<ToRender>(body);
    }
    for (auto ship : star_system_component.ships) {
        // Add a tag
        m_universe.emplace_or_replace<ToRender>(ship);
    }
}

void conquerspace::client::systems::SysStarSystemRenderer::SeeEntity() {
    namespace cqspb = conquerspace::common::components::bodies;
    // See the object
    view_center = CalculateObjectPos(m_viewing_entity);

    // If it has a terrain, then do things, if it doesn't have a terrain, render a blank sphere
    int seed = 0;
    if (!m_app.GetUniverse().all_of<cqspb::Terrain>(m_viewing_entity)) {
        return;
    }

    // Set seed
    seed = m_app.GetUniverse().get<cqspb::Terrain>(m_viewing_entity).seed;

    conquerspace::client::systems::TerrainImageGenerator generator;
    generator.seed = seed;
    generator.GenerateTerrain(1, 2);
    SetPlanetTexture(generator);

    // TODO(EhWhoAmI):  If it's in the process of generating, find some way to kill the gen,
    // and move on to the new terrain.

    // Generate terrain
    intermediate_image_generator.seed = seed;
    final_image_generator.seed = seed;

    if (less_detailed_terrain_generator_thread.joinable()) {
        less_detailed_terrain_generator_thread.join();
    }
    if (terrain_generator_thread.joinable()) {
        terrain_generator_thread.join();
    }

    less_detailed_terrain_generator_thread = std::thread([&]() {
        // Generate slightly less detailed terrain so that it looks better at first
        intermediate_image_generator.GenerateTerrain(6, 5);
        second_terrain_complete = true;
    });

    terrain_generator_thread = std::thread([&]() {
        // Generate slightly less detailed terrain so that it looks better at first
        final_image_generator.GenerateTerrain(8, 9);
        terrain_complete = true;
    });
}

void conquerspace::client::systems::SysStarSystemRenderer::DrawEntityName(
    glm::vec3 &object_pos, entt::entity ent_id) {

    if (m_app.GetUniverse().all_of<conquerspace::common::components::Name>(
        ent_id)) {
    std::string &name =
        m_app.GetUniverse()
            .get<conquerspace::common::components::Name>(ent_id)
            .name;
    glm::vec3 pos =
        glm::project(object_pos, camera_matrix, projection, viewport);

    buffer_renderer.BeginDraw();
    if (pos.z >= 1 || pos.z <= -1) {
    } else {
        m_app.DrawText(name, pos.x, pos.y);
    }
    buffer_renderer.EndDraw();
    } else {
        glm::vec3 pos =
            glm::project(object_pos, camera_matrix, projection, viewport);
        buffer_renderer.BeginDraw();
        if (pos.z >= 1 || pos.z <= -1) {
        } else {
            m_app.DrawText(fmt::format("{}", ent_id), pos.x, pos.y);
        }
        buffer_renderer.EndDraw();
    }
}


void conquerspace::client::systems::SysStarSystemRenderer::DrawPlanetIcon(glm::vec3 &object_pos) {


    glm::vec3 pos = glm::project(object_pos, camera_matrix, projection, viewport);
    glm::mat4 planetDispMat = glm::mat4(1.0f);
    if (pos.z >= 1 || pos.z <= -1) {
        return;
    }

    planetDispMat = glm::translate(planetDispMat,
        glm::vec3((pos.x / m_app.GetWindowWidth() - 0.5) * 2,
            (pos.y / m_app.GetWindowHeight() - 0.5) * 2, 0));

    planetDispMat =
        glm::scale(planetDispMat, glm::vec3(circle_size, circle_size, circle_size));

    float window_ratio = GetWindowRatio();
    planetDispMat = glm::scale(planetDispMat, glm::vec3(1, window_ratio, 1));
    glm::mat4 twodimproj = glm::scale(glm::mat4(1.0f), glm::vec3(1, window_ratio, 1));

    buffer_renderer.BeginDraw();
    twodimproj = glm::mat4(1.0f);
    planet_circle.shaderProgram->UseProgram();
    planet_circle.shaderProgram->setMat4("model", planetDispMat);
    planet_circle.shaderProgram->setMat4("projection", twodimproj);

    engine::Draw(planet_circle);
    buffer_renderer.EndDraw();
}

void conquerspace::client::systems::SysStarSystemRenderer::DrawShipIcon(
    glm::vec3 &object_pos) {
    glm::vec3 pos =
        glm::project(object_pos, camera_matrix, projection, viewport);
    glm::mat4 shipDispMat = glm::mat4(1.0f);
    if (pos.z >= 1 || pos.z <= -1) {
        return;
    }

    shipDispMat = glm::translate(
        shipDispMat,
        glm::vec3((pos.x / m_app.GetWindowWidth() - 0.5) * 2,
                  (pos.y / m_app.GetWindowHeight() - 0.5) * 2, 0));

    shipDispMat = glm::scale(
        shipDispMat,
        glm::vec3(circle_size * 2, circle_size * 2, circle_size * 2));

    float window_ratio = GetWindowRatio();
    shipDispMat = glm::scale(shipDispMat, glm::vec3(1, window_ratio, 1));
    glm::mat4 twodimproj =
        glm::scale(glm::mat4(1.0f), glm::vec3(1, window_ratio, 1));

    buffer_renderer.BeginDraw();
    twodimproj = glm::mat4(1.0f);
    ship_overlay.shaderProgram->UseProgram();
    ship_overlay.shaderProgram->setMat4("model", shipDispMat);
    ship_overlay.shaderProgram->setMat4("projection", twodimproj);

    engine::Draw(ship_overlay);
    buffer_renderer.EndDraw();
}

void conquerspace::client::systems::SysStarSystemRenderer::DrawPlanet(glm::vec3 &object_pos) {
    glm::mat4 position = glm::mat4(1.f);
    position = glm::translate(position, object_pos);

    glm::mat4 transform = glm::mat4(1.f);
    // For some reason, the sphere we make needs to be inverted
    transform = glm::scale(transform, glm::vec3(1, 1, 1));

    position = position * transform;

    planet.SetMVP(position, camera_matrix, projection);
    planet.shaderProgram->UseProgram();

    planet.shaderProgram->setVec3("lightDir", glm::normalize(sun_position - object_pos));
    planet.shaderProgram->setVec3("lightPosition", sun_position);

    planet.shaderProgram->setVec3("lightColor", sun_color);
    planet.shaderProgram->setVec3("viewPos", cam_pos);
    planet_renderer.BeginDraw();
    engine::Draw(planet);
    planet_renderer.EndDraw();
}

void conquerspace::client::systems::SysStarSystemRenderer::DrawStar(glm::vec3 &object_pos) {
    glm::mat4 position = glm::mat4(1.f);
    position = glm::translate(position, object_pos);

    glm::mat4 transform = glm::mat4(1.f);
    transform = glm::scale(transform, glm::vec3(5, 5, 5));
    position = position * transform;

    sun.SetMVP(position, camera_matrix, projection);
    sun.shaderProgram->setVec4("color", 1, 1, 1, 1);
    planet_renderer.BeginDraw();
    engine::Draw(sun);
    planet_renderer.EndDraw();
}

void conquerspace::client::systems::SysStarSystemRenderer::DrawTerrainlessPlanet(
                                            glm::vec3 &object_pos) {
    glm::mat4 position = glm::mat4(1.f);
    position = glm::translate(position, object_pos);

    glm::mat4 transform = glm::mat4(1.f);
    position = position * transform;

    sun.SetMVP(position, camera_matrix, projection);
    sun.shaderProgram->setVec4("color", 1, 0, 1, 1);
    planet_renderer.BeginDraw();
    engine::Draw(sun);
    planet_renderer.EndDraw();
}

glm::vec3 conquerspace::client::systems::SysStarSystemRenderer::CalculateObjectPos(
    entt::entity &ent) {
    namespace cqspb = conquerspace::common::components::bodies;
    namespace cqspt = conquerspace::common::components::types;
    cqspt::Orbit &orbit = m_app.GetUniverse().get<cqspt::Orbit>(ent);
    cqspt::Vec2 &vec = cqspt::toVec2(orbit);
    return glm::vec3(vec.x / divider, 0, vec.y / divider);
}

glm::vec3
conquerspace::client::systems::SysStarSystemRenderer::CalculateCenteredObject(
    entt::entity &ent) {
    return CalculateObjectPos(ent) - view_center;
}

void conquerspace::client::systems::SysStarSystemRenderer::CalculateCamera() {
    cam_pos = glm::vec3(
                cos(view_y) * sin(view_x) * scroll,
                sin(view_y) * scroll,
                cos(view_y) * cos(view_x) * scroll);
    cam_up = glm::vec3(0.0f, 1.0f, 0.0f);
    camera_matrix = glm::lookAt(cam_pos, glm::vec3(0.f, 0.f, 0.f), cam_up);
    projection = glm::infinitePerspective(glm::radians(45.f), GetWindowRatio(), 0.1f);
    viewport = glm::vec4(0.f, 0.f, m_app.GetWindowWidth(), m_app.GetWindowHeight());
}

void conquerspace::client::systems::SysStarSystemRenderer::SetPlanetTexture(
    TerrainImageGenerator &generator) {
    unsigned int albedo_map = GeneratePlanetTexture(generator.GetAlbedoMap());
    unsigned int height_map  = GeneratePlanetTexture(generator.GetHeightMap());
    generator.ClearData();

    // Free textures in texture
    for (auto t : planet.textures) {
        delete t;
    }
    planet.textures.clear();

    // Assign textures
    planet.textures.push_back(GenerateTexture(albedo_map, generator.GetAlbedoMap()));
    planet.textures.push_back(GenerateTexture(height_map, generator.GetHeightMap()));
}

unsigned int
conquerspace::client::systems::SysStarSystemRenderer::GeneratePlanetTexture(
    noise::utils::Image &image) {
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_BGRA, image.GetWidth(), image.GetHeight(), 0, GL_RGBA,
                          GL_UNSIGNED_INT_8_8_8_8, image.GetConstSlabPtr());
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    return texture;
}

glm::vec3 conquerspace::client::systems::SysStarSystemRenderer::CalculateMouseRay(
    glm::vec3 &ray_nds) {
    glm::vec4 ray_clip = glm::vec4(ray_nds.x, ray_nds.y, -1.0, 1.0);
    glm::vec4 ray_eye = glm::inverse(projection) * ray_clip;
    ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0, 0.0);
    glm::vec4 inv = (glm::inverse(camera_matrix) * ray_eye);

    // Normalize vector
    return glm::normalize(glm::vec3(inv.x, inv.y, inv.z));
}

float conquerspace::client::systems::SysStarSystemRenderer::GetWindowRatio() {
    return static_cast<float>(m_app.GetWindowWidth()) /
                                static_cast<float>(m_app.GetWindowHeight());
}

conquerspace::asset::Texture *
conquerspace::client::systems::SysStarSystemRenderer::GenerateTexture(
    unsigned int tex, noise::utils::Image &image) {
    conquerspace::asset::Texture *texture = new conquerspace::asset::Texture();
    texture->id = tex;
    texture->width = image.GetWidth();
    texture->height = image.GetHeight();
    return texture;
}

entt::entity conquerspace::client::systems::SysStarSystemRenderer::GetMouseOnObject(
    int mouse_x, int mouse_y) {
    namespace cqspb = conquerspace::common::components::bodies;

    // Loop through objects
    auto bodies = m_app.GetUniverse().view<ToRender,
                            conquerspace::common::components::bodies::Body>();
    for (auto [ent_id, body] : bodies.each()) {
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
            if ((b * b - c) >= 0) {
                m_app.GetUniverse().emplace<MouseOverEntity>(ent_id);
                return ent_id;
            }
        }
    }
    return entt::null;
}
