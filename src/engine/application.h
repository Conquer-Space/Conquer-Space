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
#pragma once

#include <spdlog/spdlog.h>

#include <RmlUi/Core.h>

#include <memory>
#include <utility>
#include <string>
#include <map>
#include <vector>

#include "engine/clientoptions.h"

#include "engine/engine.h"
#include "engine/scene.h"
#include "engine/asset/assetmanager.h"
#include "common/universe.h"
#include "engine/gui.h"
#include "engine/graphics/text.h"
#include "common/scripting/scripting.h"
#include "engine/audio/iaudiointerface.h"
#include "engine/window.h"

namespace cqsp {
namespace engine {
/// <summary>
/// Manages scenes for the application.
/// </summary>
class SceneManager {
 public:
    /// <summary>
    /// Sets the current scene.
    /// </summary>
    /// <param name="scene"></param>
    void SetInitialScene(std::unique_ptr<Scene> scene);

    /// <summary>
    /// Sets the next scene, and the scene will be switched when <code>SwitchScene</code> is executed.
    /// </summary>
    void SetScene(std::unique_ptr<Scene> scene);

    /// <summary>
    /// Sets the next scene to the current.
    /// </summary>
    void SwitchScene();

    /// <summary>
    /// Gets current running scene.
    /// </summary>
    /// <returns></returns>
    Scene* GetScene();

    /// <summary>
    /// Verifies if it is appropiate to switch scenes.
    /// </summary>
    /// <returns></returns>
    bool ToSwitchScene() { return (m_switch && m_next_scene != nullptr); }

    void Update(float deltaTime);

    void Ui(float deltaTime);

    void Render(float deltaTime);

    void DeleteCurrentScene();

 private:
    std::unique_ptr<Scene> m_scene;
    std::unique_ptr<Scene> m_next_scene;

    /// <summary>
    /// If the next scene has been set.
    /// </summary>
    bool m_switch;
};

class Application {
 public:
    Application(int _argc, char* _argv[]);
    /// We assume application is run from directory. Unless it's a test,
    /// Application is not really supposed to be used here.
    [[deprecated("This constructor is supposed to be used for testing purposes,"
                    "and is not recommended.")]]
    Application();

    /*
    * Runs the entire application.
    */
    void run();

    client::ClientOptions& GetClientOptions() {
        return m_client_options;
    }

    int GetWindowHeight() const { return m_window->GetWindowHeight(); }

    int GetWindowWidth() const { return m_window->GetWindowWidth(); }

    double GetDeltaTime() const { return deltaTime; }
    double GetFps() const { return fps; }

    cqsp::asset::AssetManager& GetAssetManager() { return manager; }

    bool ShouldExit();

    void ExitApplication();

    /*
    * Sets the next scene which will be run in the next frame.
    */
    template <class T>
    void SetScene() {
        std::unique_ptr<Scene> ptr = std::make_unique<T>(*this);
        m_scene_manager.SetScene(std::move(ptr));
    }

    ImGui::MarkdownConfig markdownConfig;
    Rml::Context* rml_context;

    cqsp::common::Universe& GetUniverse() { return *m_universe; }
    cqsp::scripting::ScriptInterface& GetScriptInterface() {
        return *m_script_interface;
    }
    cqsp::engine::audio::IAudioInterface& GetAudioInterface() {
        return *m_audio_interface;
    }

    bool ButtonIsHeld(int btn) { return m_window->ButtonIsHeld(btn); }
    bool ButtonIsReleased(int btn) { return m_window->ButtonIsReleased(btn); }
    bool ButtonIsPressed(int btn) { return m_window->ButtonIsPressed(btn); }
    double MouseButtonLastReleased(int btn) { return m_window->MouseButtonLastReleased(btn); }
    bool MouseButtonDoubleClicked(int btn) { return m_window->MouseButtonDoubleClicked(btn); }

    double GetScrollAmount() { return m_window->GetScrollAmount(); }

    double GetMouseX() { return m_window->GetMouseX(); }
    double GetMouseY() { return m_window->GetMouseY(); }

    bool MouseButtonIsHeld(int btn) { return  m_window->MouseButtonIsHeld(btn); }
    bool MouseButtonIsReleased(int btn) { return m_window->MouseButtonIsReleased(btn); }
    bool MouseButtonIsPressed(int btn) { return m_window->MouseButtonIsPressed(btn); }

    Window* GetWindow() { return m_window; }

    cqsp::asset::Font*& GetFont() { return m_font; }
    void DrawText(const std::string& text, float x, float y);
    void DrawText(const std::string& text, float x, float y, float size);
    // Draw text based on normalized device coordinates
    void DrawTextNormalized(const std::string& text, float x, float y);

    void SetFont(cqsp::asset::Font* font) { m_font = font; }
    void SetFontShader(cqsp::asset::ShaderProgram* shader) {
        fontShader = shader;
    }

    /// <summary>
    /// Time in seconds since start of application
    /// </summary>
    /// <returns></returns>
    double GetTime();

    bool MouseDragged() { return m_window->MouseDragged(); }

    void SetWindowDimensions(int width, int height);
    void SetFullScreen(bool screen);

    glm::mat4 Get2DProj() { return two_dim_projection; }
    glm::mat4 Get3DProj() { return three_dim_projection; }

    std::vector<std::string>& GetCmdLineArgs() { return cmd_line_args; }

    bool Screenshot(const char* path);

 private:
    void InitImguiFonts();

    void SetIcon();

    void GlInit();
    void LoggerInit();
    void LogInfo();
    void InitAutioInterface();
    void InitImgui();
    void InitRmlUi();

    /*
     * Intializes glfw and imgui.
     */
    int init();

    /*
     * Releases all data.
     */
    int destroy();

    void CalculateProjections();

    std::vector<std::string> cmd_line_args;
    bool full_screen;
    Window* m_window;
    std::string icon_path;

    cqsp::engine::SceneManager m_scene_manager;

    client::ClientOptions m_client_options;

    double fps;

    double deltaTime, lastFrame;

    std::string locale;

    std::shared_ptr<spdlog::logger> logger;

    cqsp::asset::AssetManager manager;

    std::unique_ptr<cqsp::common::Universe> m_universe;

    cqsp::asset::Font* m_font = nullptr;
    cqsp::asset::ShaderProgram* fontShader = nullptr;

    std::unique_ptr<cqsp::scripting::ScriptInterface> m_script_interface;
    std::unique_ptr<Rml::SystemInterface> m_system_interface;
    std::unique_ptr<Rml::RenderInterface> m_render_interface;

    cqsp::engine::audio::IAudioInterface *m_audio_interface;

    glm::mat4 two_dim_projection;
    glm::mat4 three_dim_projection;
    bool to_halt;
};
}  // namespace engine
}  // namespace cqsp
