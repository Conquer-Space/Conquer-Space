/*
 * Copyright 2021 Conquer Space
 */
#include "engine/application.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <fmt/core.h>
#include <hjson.h>

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <stb_image.h>

#include <boost/config.hpp>
#include <boost/version.hpp>
#include <filesystem>
#include <fstream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <string>
#include <vector>

#include "common/util/profiler.h"
#include "common/version.h"
#include "engine/audio/audiointerface.h"
#include "engine/paths.h"

namespace conquerspace::engine {
class GLWindow : public Window {
   public:
    bool ButtonIsHeld(int btn) { return m_keys_held[btn]; }
    bool ButtonIsReleased(int btn) { return m_keys_released[btn]; }
    bool ButtonIsPressed(int btn) { return m_keys_pressed[btn]; }
    double GetMouseX() { return m_mouse_x; }
    double GetMouseY() { return m_mouse_y; }

    bool MouseButtonIsHeld(int btn) { return m_mouse_keys_held[btn]; }
    bool MouseButtonIsReleased(int btn) { return m_mouse_keys_released[btn]; }
    bool MouseButtonIsPressed(int btn) { return m_mouse_keys_pressed[btn]; }

    bool MouseDragged() {
        return !(m_mouse_x == m_mouse_x_on_pressed &&
                 m_mouse_y == m_mouse_y_on_pressed);
    }

    void KeyboardCallback(GLFWwindow* _w, int key, int scancode, int action,
                          int mods) {
        if (action == GLFW_PRESS) {
            m_keys_held[key] = true;
            m_keys_pressed[key] = true;
        } else if (action == GLFW_RELEASE) {
            m_keys_held[key] = false;
            m_keys_released[key] = true;
        }
    }

    void MousePositionCallback(GLFWwindow* _w, double xpos, double ypos) {
        m_mouse_x = xpos;
        m_mouse_y = ypos;
    }

    void MouseEnterCallback(GLFWwindow* _w, int entered) {}

    void MouseButtonCallback(GLFWwindow* _w, int button, int action, int mods) {
        if (action == GLFW_PRESS) {
            m_mouse_keys_held[button] = true;
            m_mouse_keys_pressed[button] = true;
            m_mouse_x_on_pressed = m_mouse_x;
            m_mouse_y_on_pressed = m_mouse_y;
        } else if (action == GLFW_RELEASE) {
            m_mouse_keys_held[button] = false;
            m_mouse_keys_released[button] = true;
        }
    }

    void ScrollCallback(GLFWwindow* _w, double xoffset, double yoffset) {
        m_scroll_amount = yoffset;
    }

    void DropCallback(GLFWwindow* _w, int count, const char** paths) {}

    void FrameBufferSizeCallback(GLFWwindow* window, int width, int height) {
        glViewport(0, 0, width, height);

        m_window_width = width;
        m_window_height = height;
    }

    void SetCallbacks() {
        // Set user pointer
        glfwSetWindowUserPointer(window, this);

        auto key_callback = [](GLFWwindow* _w, int key, int scancode,
                               int action, int mods) {
            static_cast<GLWindow*>(glfwGetWindowUserPointer(_w))
                ->KeyboardCallback(_w, key, scancode, action, mods);
        };

        auto cursor_position_callback = [](GLFWwindow* _w, double xpos,
                                           double ypos) {
            static_cast<GLWindow*>(glfwGetWindowUserPointer(_w))
                ->MousePositionCallback(_w, xpos, ypos);
        };

        auto cursor_enter_callback = [](GLFWwindow* _w, int entered) {
            static_cast<GLWindow*>(glfwGetWindowUserPointer(_w))
                ->MouseEnterCallback(_w, entered);
        };

        auto mouse_button_callback = [](GLFWwindow* _w, int button, int action,
                                        int mods) {
            static_cast<GLWindow*>(glfwGetWindowUserPointer(_w))
                ->MouseButtonCallback(_w, button, action, mods);
        };

        auto scroll_callback = [](GLFWwindow* _w, double xoffset,
                                  double yoffset) {
            static_cast<GLWindow*>(glfwGetWindowUserPointer(_w))
                ->ScrollCallback(_w, xoffset, yoffset);
        };

        auto drop_callback = [](GLFWwindow* _w, int count, const char** paths) {
            static_cast<GLWindow*>(glfwGetWindowUserPointer(_w))
                ->DropCallback(_w, count, paths);
        };

        auto frame_buffer_callback = [](GLFWwindow* _w, int width, int height) {
            static_cast<GLWindow*>(glfwGetWindowUserPointer(_w))
                ->FrameBufferSizeCallback(_w, width, height);
        };

        glfwSetKeyCallback(window, key_callback);
        glfwSetCursorPosCallback(window, cursor_position_callback);
        glfwSetCursorEnterCallback(window, cursor_enter_callback);
        glfwSetMouseButtonCallback(window, mouse_button_callback);
        glfwSetScrollCallback(window, scroll_callback);
        glfwSetDropCallback(window, drop_callback);
        glfwSetFramebufferSizeCallback(window, frame_buffer_callback);
    }

    void OnFrame() {
        // Before polling events, clear the buttons
        std::memset(m_keys_pressed, false, sizeof(m_keys_pressed));
        std::memset(m_keys_released, false, sizeof(m_keys_released));
        std::memset(m_mouse_keys_pressed, false, sizeof(m_mouse_keys_pressed));
        std::memset(m_mouse_keys_released, false, sizeof(m_mouse_keys_released));
        m_scroll_amount = 0;
    }

    void SetWindowSize(int width, int height) {
        m_window_width = width;
        m_window_height = height;
        glfwSetWindowSize(window, width, height);
    }

    int GetScrollAmount() { return m_scroll_amount; }
    int GetWindowHeight() { return m_window_height; }
    int GetWindowWidth() { return m_window_width; }

    void InitWindow(int width, int height) {
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_SAMPLES, 4);

#ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
        m_window_width = width;
        m_window_height = height;

        // Create window
        window = glfwCreateWindow(width, height, "Conquer Space", NULL, NULL);
        if (window == NULL) {
            glfwTerminate();
            SPDLOG_CRITICAL("Cannot load glfw");
        }

        glfwMakeContextCurrent(window);

        // Enable vsync
        glfwSwapInterval(1);

        // Add callbacks
        SetCallbacks();

        // Init glad
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            glfwTerminate();
            SPDLOG_CRITICAL("Cannot load glad");
        }
    }

    GLFWwindow* window;

   private:

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

    int m_window_width, m_window_height;
};

GLFWwindow* window(Window* window) {
    return reinterpret_cast<GLWindow*>(window)->window;
}
}  // namespace conquerspace::engine

int conquerspace::engine::Application::init() {
    LoggerInit();
    LogInfo();
    GlInit();

    // Init audio
    m_audio_interface = new conquerspace::engine::audio::AudioInterface();
    m_audio_interface->Initialize();
    // Set option things
    m_audio_interface->SetMusicVolume(m_client_options.GetOptions()["audio"]["music"]);

    SetIcon();
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGui::StyleColorsDark();
    ImGui::GetIO().IniFilename = NULL;

    InitFonts();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window(m_window), true);
    ImGui_ImplOpenGL3_Init("#version 130");

    if (full_screen) {
        SetFullScreen(true);
    }

    std::shared_ptr<Scene> initial_scene = std::make_shared<EmptyScene>(*this);
    m_scene_manager.SetInitialScene(initial_scene);

    m_script_interface =
        std::make_unique<conquerspace::scripting::ScriptInterface>();
    m_universe = std::make_unique<conquerspace::common::components::Universe>();
    m_script_interface->Init();
    return 0;
}

int conquerspace::engine::Application::destroy() {
    m_audio_interface->Destruct();
    delete m_audio_interface;
    m_universe.reset();
    m_script_interface.reset();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    glfwDestroyWindow(window(m_window));
    glfwTerminate();

    SPDLOG_INFO("Killed GLFW");
    // Save options
    std::ofstream config_path(m_client_options.GetDefaultLocation(),
                              std::ios::trunc);
    m_client_options.WriteOptions(config_path);
    spdlog::shutdown();
    return 0;
}

conquerspace::engine::Application::Application() {
    std::ifstream config_path(m_client_options.GetDefaultLocation());
    if (config_path.good()) {
        m_client_options.LoadOptions(config_path);
    } else {
        m_client_options.LoadDefaultOptions();
    }

    // Set icon path
    icon_path = m_client_options.GetOptions()["icon"].to_string();

    full_screen = static_cast<bool>(m_client_options.GetOptions()["full_screen"]);
}

void conquerspace::engine::Application::run() {
    // Main loop
    int code = init();
    fps = 0;
    if (code != 0) {
        // Fail
        return;
    }
    m_audio_interface->StartWorker();
    while (ShouldExit()) {
        // Calculate FPS
        double currentFrame = GetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        fps = 1 / deltaTime;

        // Switch scene
        if (m_scene_manager.ToSwitchScene()) {
            m_scene_manager.SwitchScene();
        }

        // Update
        m_scene_manager.GetScene()->Update(deltaTime);

        // Init imgui
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Gui
        m_scene_manager.GetScene()->Ui(deltaTime);

        BEGIN_TIMED_BLOCK(ImGui_Render);
        ImGui::Render();
        END_TIMED_BLOCK(ImGui_Render);

        // Render
        glClearColor(0.f, 0.f, 0.f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        BEGIN_TIMED_BLOCK(Scene_Render);
        m_scene_manager.GetScene()->Render(deltaTime);
        END_TIMED_BLOCK(Scene_Render);

        BEGIN_TIMED_BLOCK(ImGui_Render_Draw);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        END_TIMED_BLOCK(ImGui_Render_Draw);

        DrawText(fmt::format("FPS: {:.0f}", fps), GetWindowWidth() - 80,
                 GetWindowHeight() - 24);

        glfwSwapBuffers(window(m_window));

        m_window->OnFrame();
        glfwPollEvents();
    }

    destroy();
}

bool conquerspace::engine::Application::ShouldExit() {
    return !glfwWindowShouldClose(window(m_window));
}

void conquerspace::engine::Application::ExitApplication() {
    glfwSetWindowShouldClose(window(m_window), true);
}

void conquerspace::engine::Application::DrawText(const std::string& text,
                                                 float x, float y) {
    if (fontShader != nullptr && m_font != nullptr) {
        glm::mat4 projection =
            glm::ortho(0.0f, static_cast<float>(GetWindowWidth()), 0.0f,
                       static_cast<float>(GetWindowHeight()));
        fontShader->UseProgram();
        fontShader->setMat4("projection", projection);
        conquerspace::asset::RenderText(*fontShader, *m_font, text, x, y, 16,
                                        glm::vec3(1.f, 1.f, 1.f));
    }
}

double conquerspace::engine::Application::GetTime() { return glfwGetTime(); }

void conquerspace::engine::Application::InitFonts() {
    Hjson::Value fontDatabase;
    Hjson::DecoderOptions decOpt;
    decOpt.comments = false;
    std::fstream stream("../data/core/gfx/fonts/fonts.hjson");

    stream >> Hjson::StreamDecoder(fontDatabase, decOpt);
    std::string fontPath = "../data/core/gfx/fonts/";
    ImGuiIO io = ImGui::GetIO();
    ImFont* defaultFont = io.Fonts->AddFontFromFileTTF(
        (fontPath + fontDatabase["default"]["path"]).c_str(),
        fontDatabase["default"]["size"]);
    io.FontDefault = defaultFont;

    ImFont* h1font = io.Fonts->AddFontFromFileTTF(
        (fontPath + fontDatabase["h1"]["path"]).c_str(),
        fontDatabase["h1"]["size"]);

    markdownConfig.headingFormats[0] = {h1font, true};

    ImFont* h2font = io.Fonts->AddFontFromFileTTF(
        (fontPath + fontDatabase["h2"]["path"]).c_str(),
        fontDatabase["h2"]["size"]);

    markdownConfig.headingFormats[1] = {h2font, true};

    ImFont* h3font = io.Fonts->AddFontFromFileTTF(
        (fontPath + fontDatabase["h3"]["path"]).c_str(),
        fontDatabase["h3"]["size"]);

    markdownConfig.headingFormats[2] = {h3font, true};
}

void conquerspace::engine::Application::SetIcon() {
    GLFWimage images[1];
    images[0].pixels = stbi_load(("../data/" + icon_path).c_str(),
                                 &images[0].width, &images[0].height, 0, 4);
    glfwSetWindowIcon(window(m_window), 1, images);
    stbi_image_free(images[0].pixels);
}

void conquerspace::engine::Application::GlInit() {
    m_window = new GLWindow();
    m_window->InitWindow(m_client_options.GetOptions()["window"]["width"], m_client_options.GetOptions()["window"]["height"]);

    // Print gl information
    SPDLOG_INFO(" --- GL information ---");
    SPDLOG_INFO("GL version: {}", glGetString(GL_VERSION));
    SPDLOG_INFO("GL vendor: {}", glGetString(GL_VENDOR));
    SPDLOG_INFO("GL Renderer: {}", glGetString(GL_RENDERER));
    SPDLOG_INFO("GL shading language: {}",
                glGetString(GL_SHADING_LANGUAGE_VERSION));
    SPDLOG_INFO(" --- GL information ---");
}

void conquerspace::engine::Application::LoggerInit() {
    // Get path
    properties["data"] = GetConquerSpacePath();
    std::filesystem::path log_folder =
        std::filesystem::path(properties["data"]) / "logs";
    // Make logs folder
    // Initialize logger
    std::vector<spdlog::sink_ptr> sinks;
    auto error_log = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
        (log_folder / "error.txt").string(), true);
    error_log->set_level(spdlog::level::err);
    sinks.push_back(error_log);

#ifdef NDEBUG
    spdlog::flush_every(std::chrono::seconds(3));
    auto basic_logger = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
        (log_folder / "info.txt").string(), true);
    sinks.push_back(basic_logger);
#else
    auto console_logger =
        std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    sinks.push_back(console_logger);
#endif
    logger = std::make_shared<spdlog::logger>("application", sinks.begin(),
                                              sinks.end());
    spdlog::set_default_logger(logger);
    spdlog::set_pattern("[%T.%e] [%^%l%$] [%n] [%s:%#] %v");
}

void conquerspace::engine::Application::LogInfo() {
    SPDLOG_INFO("Conquer Space {} {}", CQSP_VERSION_STRING, GIT_INFO);
    SPDLOG_INFO("Platform: {}", BOOST_PLATFORM);
    SPDLOG_INFO("Compiled {} {}", __DATE__, __TIME__);
    SPDLOG_INFO("Compiled on {} with {}", BOOST_COMPILER, BOOST_STDLIB);
#ifndef NDEBUG
    SPDLOG_INFO("In debug mode");
#endif
}

void conquerspace::engine::Application::SetWindowDimensions(int width, int height) {
    m_window->SetWindowSize(width, height);
}

void conquerspace::engine::Application::SetFullScreen(bool screen) {
    if (screen == true) {
        const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        glfwSetWindowMonitor(window(m_window), glfwGetPrimaryMonitor(), 0, 0,
                             mode->width, mode->height, mode->refreshRate);
    } else {
        const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        glfwSetWindowMonitor(
            window(m_window), NULL, 40, 40,
            GetClientOptions().GetOptions()["window"]["width"],
            GetClientOptions().GetOptions()["window"]["height"],
            mode->refreshRate);
    }
}

void conquerspace::engine::SceneManager::SetInitialScene(std::shared_ptr<Scene>& scene) {
    m_scene = std::move(scene);
}

void conquerspace::engine::SceneManager::SetScene(std::shared_ptr<Scene>& scene) {
    m_next_scene = std::move(scene);
    m_switch = true;
}

void conquerspace::engine::SceneManager::SwitchScene() {
    m_scene = std::move(m_next_scene);
    SPDLOG_TRACE("Initializing scene");
    m_scene->Init();
    SPDLOG_TRACE("Done Initializing scene");
    m_switch = false;
}

std::shared_ptr<conquerspace::engine::Scene>
conquerspace::engine::SceneManager::GetScene() {
    return m_scene;
}
