/*
 * Copyright 2021 Conquer Space
 */
#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <spdlog/spdlog.h>

#include <imgui.h>
#include <imgui_markdown.h>

#include <memory>
#include <utility>
#include <string>

#include <spirit_po/spirit_po.hpp>

#include "client/clientoptions.h"

#include "engine/assetmanager.h"
#include "engine/engine.h"
#include "engine/scene.h"

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

    const client::ProgramOptions& GetClientOptions() const {
        return m_program_options;
    }

    const std::unique_ptr<spirit_po::default_catalog>& GetCatalogue() const {
        return catalog;
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

    SceneManager m_scene_manager;

    client::ProgramOptions m_program_options;

    double fps;

    double deltaTime, lastFrame;

    std::string locale;
    std::unique_ptr<spirit_po::default_catalog> catalog;

    std::shared_ptr<spdlog::logger> logger;

    conquerspace::asset::AssetManager manager;

    double m_mouse_x;
    double m_mouse_y;

    char m_keys_pressed[GLFW_KEY_LAST];
    int m_mods;
};
}  // namespace engine
}  // namespace conquerspace
