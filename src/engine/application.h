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

#include <memory>
#include <utility>
#include <string>
#include <map>

#include "engine/clientoptions.h"

#include "engine/assetmanager.h"
#include "engine/engine.h"
#include "engine/scene.h"
#include "common/universe.h"
#include "engine/gui.h"
#include "engine/renderer/text.h"
#include "common/scripting/scripting.h"
#include "engine/audio/iaudiointerface.h"
#include "engine/window.h"

namespace conquerspace {
namespace engine {

/**
* Manages scenes for the application.
*/
class SceneManager {
 public:
    /**
    * Sets the current scene. 
    */
    void SetInitialScene(std::shared_ptr<Scene>& scene);

    /**
    * Sets the next scene, and the scene will be switched when <code>SwitchScene</code> is executed.
    */
    void SetScene(std::shared_ptr<Scene>& scene);

    /**
    * Sets the next scene to the current.
    */
    void SwitchScene();

    /**
    * Gets current running scene.
    */
    std::shared_ptr<Scene> GetScene();

    /**
    * Verifies if it is appropiate to switch scenes.
    */
    bool ToSwitchScene() { return (m_switch
        && m_next_scene != nullptr); }

 private:
    std::shared_ptr<Scene> m_scene;
    std::shared_ptr<Scene> m_next_scene;

    /**
    * If the next scene has been set.
    */
    bool m_switch;
};

class Application {
 public:
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

    float GetDeltaTime() const { return deltaTime; }
    float GetFps() const { return fps; }

    conquerspace::asset::AssetManager& GetAssetManager() { return manager; }

    bool ShouldExit();

    void ExitApplication();

    /*
    * Sets the next scene which will be run in the next frame.
    */
    template <class T>
    void SetScene() {
        std::shared_ptr<Scene> ptr = std::make_shared<T>(*this);
        m_scene_manager.SetScene(ptr);
    }

    ImGui::MarkdownConfig markdownConfig;

    conquerspace::common::components::Universe& GetUniverse() { return *m_universe; }
    conquerspace::scripting::ScriptInterface& GetScriptInterface() {
        return *m_script_interface;
    }
    conquerspace::engine::audio::IAudioInterface& GetAudioInterface() {
        return *m_audio_interface;
    }

    bool ButtonIsHeld(int btn) { return m_window->ButtonIsHeld(btn); }
    bool ButtonIsReleased(int btn) { return m_window->ButtonIsReleased(btn); }
    bool ButtonIsPressed(int btn) { return m_window->ButtonIsPressed(btn); }

    double GetScrollAmount() { return m_window->GetScrollAmount(); }

    double GetMouseX() { return m_window->GetMouseX(); }
    double GetMouseY() { return m_window->GetMouseY(); }

    bool MouseButtonIsHeld(int btn) { return  m_window->MouseButtonIsHeld(btn); }
    bool MouseButtonIsReleased(int btn) { return m_window->MouseButtonIsReleased(btn); }
    bool MouseButtonIsPressed(int btn) { return m_window->MouseButtonIsPressed(btn); }

    Window* GetWindow() { return m_window; }

    conquerspace::asset::Font*& GetFont() { return m_font; }
    void DrawText(const std::string& text, float x, float y);

    void SetFont(conquerspace::asset::Font* font) { m_font = font; }
    void SetFontShader(conquerspace::asset::ShaderProgram* shader) {
        fontShader = shader;
    }

    double GetTime();

    bool MouseDragged() { return m_window->MouseDragged(); }

    void SetWindowDimensions(int width, int height);
    void SetFullScreen(bool screen);

 private:

    void InitFonts();

    void SetIcon();

    void GlInit();
    void LoggerInit();
    void LogInfo();
    /*
     * Intializes glfw and imgui.
     */
    int init();

    /*
     * Releases all data.
     */
    int destroy();

    bool full_screen;
    Window* m_window;
    std::string icon_path;

    SceneManager m_scene_manager;

    client::ClientOptions m_client_options;

    double fps;

    double deltaTime, lastFrame;

    std::string locale;

    std::shared_ptr<spdlog::logger> logger;

    conquerspace::asset::AssetManager manager;

    std::unique_ptr<conquerspace::common::components::Universe> m_universe;

    conquerspace::asset::Font* m_font = nullptr;
    conquerspace::asset::ShaderProgram* fontShader = nullptr;

    std::map<std::string, std::string> properties;

    std::unique_ptr<conquerspace::scripting::ScriptInterface> m_script_interface;

    conquerspace::engine::audio::IAudioInterface *m_audio_interface;

    bool to_halt;
};
}  // namespace engine
}  // namespace conquerspace
