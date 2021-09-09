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
#include "engine/cqspgui.h"

namespace cqsp::engine {
void ParseType(GLenum type) {
    switch (type) {
        case GL_DEBUG_TYPE_ERROR:
            SPDLOG_ERROR("Type: Error");
            break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
            SPDLOG_ERROR("Type: Deprecated Behaviour");
            break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
            SPDLOG_ERROR("Type: Undefined Behaviour");
            break;
        case GL_DEBUG_TYPE_PORTABILITY:
            SPDLOG_ERROR("Type: Portability");
            break;
        case GL_DEBUG_TYPE_PERFORMANCE:
            SPDLOG_ERROR("Type: Performance");
            break;
        case GL_DEBUG_TYPE_MARKER:
            SPDLOG_ERROR("Type: Marker");
            break;
        case GL_DEBUG_TYPE_PUSH_GROUP:
            SPDLOG_ERROR("Type: Push Group");
            break;
        case GL_DEBUG_TYPE_POP_GROUP:
            SPDLOG_ERROR("Type: Pop Group");
            break;
        case GL_DEBUG_TYPE_OTHER:
            SPDLOG_ERROR("Type: Other");
            break;
    }
}

void ParseSeverity(GLenum severity) {
    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH:
            SPDLOG_ERROR("Severity: high");
            break;
        case GL_DEBUG_SEVERITY_MEDIUM:
            SPDLOG_ERROR("Severity: medium");
            break;
        case GL_DEBUG_SEVERITY_LOW:
            SPDLOG_ERROR("Severity: low");
            break;
        case GL_DEBUG_SEVERITY_NOTIFICATION:
            SPDLOG_ERROR("Severity: notification");
            break;
    }
}
void APIENTRY glDebugOutput(GLenum source, GLenum type, unsigned int id,
                            GLenum severity, GLsizei length,
                            const char* message, const void* userParam) {
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204)
        return;  // ignore these non-significant error codes

    SPDLOG_ERROR("Debug message ({}): {}", id, message);

    switch (source) {
        case GL_DEBUG_SOURCE_API:
            SPDLOG_ERROR("Source: API");
            break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
            SPDLOG_ERROR("Source: Window System");
            break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER:
            SPDLOG_ERROR("Source: Shader Compiler");
            break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:
            SPDLOG_ERROR("Source: Third Party");
            break;
        case GL_DEBUG_SOURCE_APPLICATION:
            SPDLOG_ERROR("Source: Application");
            break;
        case GL_DEBUG_SOURCE_OTHER:
            SPDLOG_ERROR("Source: Other");
            break;

        ParseType(type);

        ParseSeverity(severity);
    }
}

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
        window_size_changed = true;
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
        window_size_changed = false;
    }

    void SetWindowSize(int width, int height) {
        m_window_width = width;
        m_window_height = height;
        glfwSetWindowSize(window, width, height);
    }

    bool WindowSizeChanged() { return window_size_changed; }

    int GetScrollAmount() { return m_scroll_amount; }
    int GetWindowHeight() { return m_window_height; }
    int GetWindowWidth() { return m_window_width; }

    void InitWindow(int width, int height) {
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_SAMPLES, 4);
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);  

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
        int flags; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
        if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
            glEnable(GL_DEBUG_OUTPUT);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); // makes sure errors are displayed synchronously
            glDebugMessageCallback(glDebugOutput, nullptr);
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
        }
    }

    GLFWwindow* window;


   private:
    bool window_size_changed;
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
}  // namespace cqsp::engine

int cqsp::engine::Application::init() {
    LoggerInit();
    LogInfo();
    GlInit();

    manager.LoadDefaultTexture();
    // Init audio
    m_audio_interface = new cqsp::engine::audio::AudioInterface();
    m_audio_interface->Initialize();
    // Set option things
    m_audio_interface->SetMusicVolume(m_client_options.GetOptions()["audio"]["music"]);
    m_audio_interface->SetChannelVolume(1, m_client_options.GetOptions()["audio"]["ui"]);


    SetIcon();
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGui::StyleColorsDark();
    ImGui::GetIO().IniFilename = NULL;
    CQSPGui::SetApplication(this);
    InitFonts();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window(m_window), true);
    ImGui_ImplOpenGL3_Init("#version 130");

    if (full_screen) {
        SetFullScreen(true);
    }

    std::shared_ptr<Scene> initial_scene = std::make_shared<EmptyScene>(*this);
    m_scene_manager.SetInitialScene(initial_scene);

    m_script_interface = std::make_unique<cqsp::scripting::ScriptInterface>();
    m_universe = std::make_unique<cqsp::common::Universe>();
    m_script_interface->Init();
    return 0;
}

int cqsp::engine::Application::destroy() {
    // Delete scene
    m_scene_manager.GetScene().reset();
    // Clear assets
    m_universe.reset();
    m_script_interface.reset();
    manager.ClearAssets();
    m_audio_interface->Destruct();
    delete m_audio_interface;
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

cqsp::engine::Application::Application() {
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

void cqsp::engine::Application::run() {
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

bool cqsp::engine::Application::ShouldExit() {
    return !glfwWindowShouldClose(window(m_window));
}

void cqsp::engine::Application::ExitApplication() {
    glfwSetWindowShouldClose(window(m_window), true);
}

void cqsp::engine::Application::DrawText(const std::string& text,
                                                 float x, float y) {
    if (fontShader != nullptr && m_font != nullptr) {
        glm::mat4 projection =
            glm::ortho(0.0f, static_cast<float>(GetWindowWidth()), 0.0f,
                       static_cast<float>(GetWindowHeight()));
        fontShader->UseProgram();
        fontShader->setMat4("projection", projection);
        cqsp::asset::RenderText(*fontShader, *m_font, text, x, y, 16,
                                        glm::vec3(1.f, 1.f, 1.f));
    }
}

double cqsp::engine::Application::GetTime() { return glfwGetTime(); }

void cqsp::engine::Application::InitFonts() {
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

void cqsp::engine::Application::SetIcon() {
    GLFWimage images[1];
    images[0].pixels = stbi_load(("../data/" + icon_path).c_str(),
                                 &images[0].width, &images[0].height, 0, 4);
    glfwSetWindowIcon(window(m_window), 1, images);
    stbi_image_free(images[0].pixels);
}

void cqsp::engine::Application::GlInit() {
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

void cqsp::engine::Application::LoggerInit() {
    // Get path
    properties["data"] = GetcqspPath();
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

void cqsp::engine::Application::LogInfo() {
    SPDLOG_INFO("Conquer Space {} {}", CQSP_VERSION_STRING, GIT_INFO);
    SPDLOG_INFO("Platform: {}", PLATFORM_NAME);
    SPDLOG_INFO("Compiled {} {}", __DATE__, __TIME__);
#ifndef NDEBUG
    SPDLOG_INFO("In debug mode");
#endif
}

void cqsp::engine::Application::SetWindowDimensions(int width, int height) {
    m_window->SetWindowSize(width, height);
}

void cqsp::engine::Application::SetFullScreen(bool screen) {
    if (screen) {
        const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        glfwSetWindowMonitor(window(m_window), glfwGetPrimaryMonitor(), 0, 0,
                             mode->width, mode->height, GLFW_DONT_CARE);
    } else {
        const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        glfwSetWindowMonitor(
            window(m_window), NULL, 40, 40,
            GetClientOptions().GetOptions()["window"]["width"],
            GetClientOptions().GetOptions()["window"]["height"],
            mode->refreshRate);
    }
}

void cqsp::engine::SceneManager::SetInitialScene(std::shared_ptr<Scene>& scene) {
    m_scene = std::move(scene);
}

void cqsp::engine::SceneManager::SetScene(std::shared_ptr<Scene>& scene) {
    m_next_scene = std::move(scene);
    m_switch = true;
}

void cqsp::engine::SceneManager::SwitchScene() {
    m_scene = std::move(m_next_scene);
    SPDLOG_TRACE("Initializing scene");
    m_scene->Init();
    SPDLOG_TRACE("Done Initializing scene");
    m_switch = false;
}

std::shared_ptr<cqsp::engine::Scene>
cqsp::engine::SceneManager::GetScene() {
    return m_scene;
}
