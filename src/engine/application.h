/*
 * Copyright 2021 Conquer Space
 */
#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <spdlog/spdlog.h>


#include <memory>
#include <utility>
#include <string>

#include "client/clientoptions.h"

#include "engine/assetmanager.h"
#include "engine/engine.h"
#include "engine/scene.h"
#include "common/universe.h"
#include "engine/gui.h"
#include "engine/renderer/text.h"

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

    client::ProgramOptions& GetClientOptions() {
        return m_program_options;
    }

    int GetWindowHeight() const { return m_window_height; }

    int GetWindowWidth() const { return m_window_width; }

    float GetDeltaTime() const { return deltaTime; }
    float GetFps() const { return fps; }

    conquerspace::asset::AssetManager& GetAssetManager() { return manager; }

    bool ShouldExit() {return !glfwWindowShouldClose(m_window); }

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

    conquerspace::common::components::Universe& GetUniverse() { return m_universe; }

    bool ButtonIsHeld(int btn) { return m_keys_held[btn]; }
    bool ButtonIsReleased(int btn) { return m_keys_released[btn]; }
    bool ButtonIsPressed(int btn) { return m_keys_pressed[btn]; }

    double GetScrollAmount() { return m_scroll_amount; }

    double GetMouseX() { return m_mouse_x; }
    double GetMouseY() { return m_mouse_y; }

    bool MouseButtonIsHeld(int btn) { return m_mouse_keys_held[btn]; }
    bool MouseButtonIsReleased(int btn) { return m_mouse_keys_released[btn]; }
    bool MouseButtonIsPressed(int btn) { return m_mouse_keys_pressed[btn]; }

    conquerspace::asset::Font*& GetFont() { return m_font; }
    void DrawText(const std::string& text, float x, float y);

    void SetFont(conquerspace::asset::Font* font) { m_font = font; }
    void SetFontShader(conquerspace::asset::ShaderProgram* shader) {
        fontShader = shader;
    }

    double GetTime();

    bool MouseDragged();

    void SetWindowDimensions(int width, int height);
    void SetFullScreen(bool screen);

 private:
    void AddCallbacks();

    void InitFonts();

    void KeyboardCallback(GLFWwindow* _w, int key, int scancode, int action,
                          int mods);
    void MousePositionCallback(GLFWwindow* _w, double xpos, double ypos);
    void MouseEnterCallback(GLFWwindow* _w, int entered);
    void MouseButtonCallback(GLFWwindow* _w, int button, int action, int mods);
    void ScrollCallback(GLFWwindow* _w, double xoffset, double yoffset);
    void DropCallback(GLFWwindow* _w, int count, const char** paths);
    void FrameBufferSizeCallback(GLFWwindow* window, int width, int height);

    void SetIcon();

    void GlInit();
    void LoggerInit();
    /*
     * Intializes glfw and imgui.
     */
    int init();

    /*
     * Releases all data.
     */
    int destroy();

    GLFWwindow *m_window;
    int m_window_width, m_window_height;
    bool full_screen;
    std::string icon_path;

    SceneManager m_scene_manager;

    client::ProgramOptions m_program_options;

    double fps;

    double deltaTime, lastFrame;

    std::string locale;

    std::shared_ptr<spdlog::logger> logger;

    conquerspace::asset::AssetManager manager;

    conquerspace::common::components::Universe m_universe;

    double m_mouse_x;
    double m_mouse_y;

    double m_mouse_x_on_pressed;
    double m_mouse_y_on_pressed;

    bool m_mouse_keys_held[GLFW_MOUSE_BUTTON_LAST] = {false};
    bool m_mouse_keys_released[GLFW_MOUSE_BUTTON_LAST] = {false};
    bool m_mouse_keys_pressed[GLFW_MOUSE_BUTTON_LAST] = {false};

    bool m_keys_held[GLFW_KEY_LAST] = {false};
    bool m_keys_released[GLFW_KEY_LAST] = {false};
    bool m_keys_pressed[GLFW_KEY_LAST] = {false};

    double m_scroll_amount;

    int m_mods;

    conquerspace::asset::Font* m_font = nullptr;
    conquerspace::asset::ShaderProgram* fontShader = nullptr;

    std::map<std::string, std::string> properties;
};
}  // namespace engine
}  // namespace conquerspace
