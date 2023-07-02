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
#pragma once

#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/ElementDocument.h>
#include <glad/glad.h>
#include <spdlog/spdlog.h>

#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "common/game.h"
#include "engine/asset/assetmanager.h"
#include "engine/audio/iaudiointerface.h"
#include "engine/clientoptions.h"
#include "engine/engine.h"
#include "engine/graphics/text.h"
#include "engine/gui.h"
#include "engine/scene.h"
#include "engine/scenemanager.h"
#include "engine/ui/RmlUi_Platform_GLFW.h"
#include "engine/ui/rmlrenderinterface.h"
#include "engine/userinput.h"
#include "engine/window.h"

namespace cqsp {
namespace engine {
class Application {
 public:
    Application(int _argc, char* _argv[]);

    /*
    * Runs the entire application.
    */
    void run();

    client::ClientOptions& GetClientOptions() { return m_client_options; }

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

    cqsp::common::Universe& GetUniverse() { return m_game->GetUniverse(); }

    cqsp::common::Game& GetGame() { return *m_game; }

    cqsp::scripting::ScriptInterface& GetScriptInterface() { return m_game->GetScriptInterface(); }

    cqsp::engine::audio::IAudioInterface& GetAudioInterface() { return *m_audio_interface; }

    bool ButtonIsHeld(int btn) { return m_window->ButtonIsHeld(btn); }
    bool ButtonIsHeld(KeyInput btn) { return ButtonIsHeld(GetGlfwKey(btn)); }

    bool ButtonIsReleased(int btn) { return m_window->ButtonIsReleased(btn); }
    bool ButtonIsReleased(KeyInput btn) { return ButtonIsReleased(GetGlfwKey(btn)); }

    bool ButtonIsPressed(int btn) { return m_window->ButtonIsPressed(btn); }
    bool ButtonIsPressed(KeyInput btn) { return ButtonIsPressed(GetGlfwKey(btn)); }

    double MouseButtonLastReleased(int btn) { return m_window->MouseButtonLastReleased(btn); }
    bool MouseButtonDoubleClicked(int btn) { return m_window->MouseButtonDoubleClicked(btn); }

    double GetScrollAmount() { return m_window->GetScrollAmount(); }

    double GetMouseX() { return m_window->GetMouseX(); }
    double GetMouseY() { return m_window->GetMouseY(); }

    bool MouseButtonIsHeld(int btn) { return m_window->MouseButtonIsHeld(btn); }
    bool MouseButtonIsReleased(int btn) { return m_window->MouseButtonIsReleased(btn); }
    bool MouseButtonIsPressed(int btn) { return m_window->MouseButtonIsPressed(btn); }

    Rml::ElementDocument* LoadDocument(const std::string& path);
    void CloseDocument(const std::string& path);

    /// <summary>
    /// Reloads the document from the file
    /// How to hotload the a document.
    ///
    /// Approximately how to use it
    /// After loading it with `app.LoadDocument()`, you can reload it, and
    /// remember to set the document variable that you may have, or it will be
    /// pointing to something empty.
    /// </summary>
    /// <param name="path"></param>
    /// <returns></returns>
    Rml::ElementDocument* ReloadDocument(const std::string& path);

    Window* GetWindow() { return m_window; }

    cqsp::asset::Font*& GetFont() { return m_font; }
    void DrawText(const std::string& text, float x, float y);
    void DrawText(const std::string& text, const glm::vec3& color, float x, float y);
    void DrawText(const std::string& text, float x, float y, float size);
    void DrawText(const std::string& text, const glm::vec3& color, float x, float y, float size);
    // Draw text based on normalized device coordinates
    void DrawTextNormalized(const std::string& text, float x, float y);

    void SetFont(cqsp::asset::Font* font) { m_font = font; }
    void SetFontShader(cqsp::asset::ShaderProgram* shader) { fontShader = shader; }

    /// <summary>
    /// Time in seconds
    /// </summary>
    /// <returns></returns>
    double GetTime();

    bool MouseDragged() { return m_window->MouseDragged(); }

    void SetWindowDimensions(int width, int height);
    void SetFullScreen(bool screen);

    glm::mat4 Get2DProj() { return two_dim_projection; }
    glm::mat4 Get3DProj() { return three_dim_projection; }
    glm::mat4 GetRmlUiProj() { return rmlui_projection; }

    std::vector<std::string>& GetCmdLineArgs() { return cmd_line_args; }

    bool HasCmdLineArgs(const std::string& arg) {
        return (std::find(GetCmdLineArgs().begin(), GetCmdLineArgs().end(), arg) != GetCmdLineArgs().end());
    }
    /// <summary>
    /// Screenshots the current framebuffer to the filename
    ///
    /// If it's null, it will put the screenshot into the default folder, with
    /// </summary>
    /// <param name="path"></param>
    bool Screenshot(const char* path = NULL);

    Rml::Context* GetRmlUiContext() { return rml_context; }

    class CqspEventInstancer : public Rml::EventListenerInstancer {
     public:
        CqspEventInstancer();
        virtual ~CqspEventInstancer();

        /// Instances a new event handle for Invaders.
        Rml::EventListener* InstanceEventListener(const Rml::String& value, Rml::Element* element) override;
    };

    typedef void (*EventListener)(Rml::Event&);

    class CqspEventListener : public Rml::EventListener {
     public:
        explicit CqspEventListener(const std::string& name) : name(name) {}
        ~CqspEventListener();
        void ProcessEvent(Rml::Event& event);
        std::string name;
    };

 private:
    void InitFonts();

    void SetIcon();

    void GlInit();
    void LoggerInit();
    void LogInfo();

    void InitRmlUi();
    void InitImgui();
    void ProcessRmlUiUserInput();
    void InitAudio();

    /*
     * Intializes glfw and imgui.
     */
    int init();

    void ResetGame() { m_game.reset(); }
    void InitGame() { m_game = std::make_unique<cqsp::common::Game>(); }
    /*
     * Releases all data.
     */
    int destroy();

    void CalculateProjections();

    std::vector<std::string> cmd_line_args;
    bool full_screen;
    Window* m_window;
    std::string icon_path;

    Rml::Context* rml_context;
    std::unique_ptr<SystemInterface_GLFW> m_system_interface;
    std::unique_ptr<Rml::RenderInterface> m_render_interface;
    ax::NodeEditor::EditorContext* m_ne_context = nullptr;

    std::unique_ptr<CqspEventInstancer> m_event_instancer;

    cqsp::engine::SceneManager m_scene_manager;

    client::ClientOptions m_client_options;

    double fps;

    double deltaTime, lastFrame;

    std::string locale;

    cqsp::asset::AssetManager manager;

    std::unique_ptr<cqsp::common::Game> m_game;

    cqsp::asset::Font* m_font = nullptr;
    cqsp::asset::ShaderProgram* fontShader = nullptr;

    std::map<std::string, std::string> properties;

    cqsp::engine::audio::IAudioInterface* m_audio_interface;

    std::map<std::string, Rml::ElementDocument*> loaded_documents;

    glm::mat4 two_dim_projection;
    glm::mat4 three_dim_projection;
    glm::mat4 rmlui_projection;
    bool to_halt;

    bool draw_fps = false;
};
}  // namespace engine
}  // namespace cqsp
