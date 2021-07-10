/*
 * Copyright 2021 Conquer Space
 */
#include "engine/application.h"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <hjson.h>

#include <fmt/core.h>

#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <stb_image.h>

#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "common/util/profiler.h"
#include "engine/paths.h"

int conquerspace::engine::Application::init() {
    LoggerInit();
    GlInit();

    SetIcon();
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGui::StyleColorsDark();
    ImGui::GetIO().IniFilename = NULL;

    InitFonts();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    if (full_screen) {
        SetFullScreen(true);
    }

    std::shared_ptr<Scene> initial_scene = std::make_shared<EmptyScene>(*this);
    m_scene_manager.SetInitialScene(initial_scene);
    return 0;
}

int conquerspace::engine::Application::destroy() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    glfwDestroyWindow(m_window);
    glfwTerminate();

    // Save options
    std::ofstream config_path(m_program_options.GetDefaultLocation(), std::ios::trunc);
    m_program_options.WriteOptions(config_path);
    spdlog::shutdown();
    return 0;
}

conquerspace::engine::Application::Application() {
    std::ifstream config_path(m_program_options.GetDefaultLocation());
    if (config_path.good()) {
        m_program_options.LoadOptions(config_path);
    } else {
        m_program_options.LoadDefaultOptions();
    }
    Hjson::Value window_dimensions = m_program_options.GetOptions()["window"];
    m_window_width = window_dimensions["width"];
    m_window_height = window_dimensions["height"];

    // Set icon path
    icon_path = m_program_options.GetOptions()["icon"].to_string();

    full_screen = static_cast<bool>(m_program_options.GetOptions()["full_screen"]);
}

void conquerspace::engine::Application::run() {
    // Main loop
    int code = init();
    fps = 0;
    if (code != 0) {
        // Fail
        return;
    }

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

        DrawText(fmt::format("FPS: {:.0f}", fps), GetWindowWidth() - 80, GetWindowHeight() - 24);

        glfwSwapBuffers(m_window);

        // Before polling events, clear the buttons
        std::memset(m_keys_pressed, false, sizeof(m_keys_pressed));
        std::memset(m_keys_released, false, sizeof(m_keys_released));
        std::memset(m_mouse_keys_pressed, false, sizeof(m_mouse_keys_pressed));
        std::memset(m_mouse_keys_released, false, sizeof(m_mouse_keys_released));
        m_scroll_amount = 0;

        glfwPollEvents();
    }

    destroy();
}

void conquerspace::engine::Application::ExitApplication() {
    glfwSetWindowShouldClose(m_window, true);
}

void conquerspace::engine::Application::DrawText(const std::string& text, float x,
                                                 float y) {
    if (fontShader != nullptr && m_font != nullptr) {
    glm::mat4 projection =
        glm::ortho(0.0f, static_cast<float>(GetWindowWidth()), 0.0f,
                   static_cast<float>(GetWindowHeight()));
    fontShader->UseProgram();
    fontShader->setMat4("projection", projection);
    conquerspace::asset::RenderText(*fontShader, *m_font, text, x, y, 16, glm::vec3(1.f, 1.f, 1.f));
    }
}

double conquerspace::engine::Application::GetTime() { return glfwGetTime(); }

bool conquerspace::engine::Application::MouseDragged() {
    return !(m_mouse_x == m_mouse_x_on_pressed && m_mouse_y == m_mouse_y_on_pressed);
}

void conquerspace::engine::Application::AddCallbacks() {
    // Set user pointer
    glfwSetWindowUserPointer(m_window, this);

    auto key_callback = [](GLFWwindow* _w, int key, int scancode, int action,
                           int mods) {
        static_cast<Application*>(glfwGetWindowUserPointer(_w))
            ->KeyboardCallback(_w, key, scancode, action, mods);
    };

    auto cursor_position_callback = [](GLFWwindow* _w, double xpos,
                                       double ypos) {
        static_cast<Application*>(glfwGetWindowUserPointer(_w))
            ->MousePositionCallback(_w, xpos, ypos);
    };

    auto cursor_enter_callback = [](GLFWwindow* _w, int entered) {
        static_cast<Application*>(glfwGetWindowUserPointer(_w))->MouseEnterCallback(_w, entered);
    };

    auto mouse_button_callback = [](GLFWwindow* _w, int button, int action,
                                    int mods) {
        static_cast<Application*>(glfwGetWindowUserPointer(_w))
            ->MouseButtonCallback(_w, button, action, mods);
    };

    auto scroll_callback = [](GLFWwindow* _w, double xoffset, double yoffset) {
        static_cast<Application*>(glfwGetWindowUserPointer(_w))
            ->ScrollCallback(_w, xoffset, yoffset);
    };

    auto drop_callback = [](GLFWwindow* _w, int count, const char** paths) {
        static_cast<Application*>(glfwGetWindowUserPointer(_w))
            ->DropCallback(_w, count, paths);
    };

    auto frame_buffer_callback = [](GLFWwindow* _w, int width, int height) {
        static_cast<Application*>(glfwGetWindowUserPointer(_w))
            ->FrameBufferSizeCallback(_w, width, height);
    };

    glfwSetKeyCallback(m_window, key_callback);
    glfwSetCursorPosCallback(m_window, cursor_position_callback);
    glfwSetCursorEnterCallback(m_window, cursor_enter_callback);
    glfwSetMouseButtonCallback(m_window, mouse_button_callback);
    glfwSetScrollCallback(m_window, scroll_callback);
    glfwSetDropCallback(m_window, drop_callback);
    glfwSetFramebufferSizeCallback(m_window, frame_buffer_callback);
}

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

void conquerspace::engine::Application::KeyboardCallback(GLFWwindow* _w,
                                                         int key, int scancode,
                                                         int action, int mods) {
    if (action == GLFW_PRESS) {
        m_keys_held[key] = true;
        m_keys_pressed[key] = true;
    } else if (action == GLFW_RELEASE) {
        m_keys_held[key] = false;
        m_keys_released[key] = true;
    }
}

void conquerspace::engine::Application::MousePositionCallback(GLFWwindow* _w,
                                                              double xpos,
                                                              double ypos) {
    m_mouse_x = xpos;
    m_mouse_y = ypos;
}

void conquerspace::engine::Application::MouseEnterCallback(GLFWwindow* _w,
                                                           int entered) {}

void conquerspace::engine::Application::MouseButtonCallback(GLFWwindow* _w,
                                                            int button,
                                                            int action,
                                                            int mods) {
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

void conquerspace::engine::Application::ScrollCallback(GLFWwindow* _w,
                                                       double xoffset,
                                                       double yoffset) {
    m_scroll_amount = yoffset;
}

void conquerspace::engine::Application::DropCallback(GLFWwindow* _w, int count,
                                                     const char** paths) {}

void conquerspace::engine::Application::FrameBufferSizeCallback(
    GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);

    m_window_width = width;
    m_window_height = height;
}

void conquerspace::engine::Application::SetIcon() {
    GLFWimage images[1];
    images[0].pixels = stbi_load(("../data/" + icon_path).c_str(), &images[0].width,
                                                                    &images[0].height, 0, 4);
    glfwSetWindowIcon(m_window, 1, images);
    stbi_image_free(images[0].pixels);
}

void conquerspace::engine::Application::GlInit() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Create window
    m_window = glfwCreateWindow(m_window_width, m_window_height, "Conquer Space", NULL, NULL);
    if (m_window == NULL) {
        glfwTerminate();
        spdlog::error("Cannot load glfw");
    }

    glfwMakeContextCurrent(m_window);

    // Enable vsync
    glfwSwapInterval(1);

    // Add callbacks
    AddCallbacks();

    // Init glad
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        glfwTerminate();
        spdlog::error("Cannot load glad");
    }
}

void conquerspace::engine::Application::LoggerInit() {
    properties["data"] = GetConquerSpacePath();
    std::filesystem::path log_folder = std::filesystem::path(properties["data"]) / "logs";
    // Make logs folder
    // Initialize logger
    std::vector<spdlog::sink_ptr> sinks;
    auto error_log = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
                                                        (log_folder/"error.txt").string(), true);
    error_log->set_level(spdlog::level::err);
    sinks.push_back(error_log);

#ifdef NDEBUG
    spdlog::flush_every(std::chrono::seconds(3));
    auto basic_logger = spdlog::basic_logger_mt("application", log_folder/"cqsp.txt", true);
    sinks.push_back(basic_logger)
#else
    sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
#endif
    logger = std::make_shared<spdlog::logger>("application", sinks.begin(), sinks.end());
    spdlog::set_default_logger(logger);
}

void conquerspace::engine::Application::SetWindowDimensions(int width, int height) {
    glfwSetWindowSize(m_window, width, height);
}

void conquerspace::engine::Application::SetFullScreen(bool screen) {
    if (screen == true) {
        const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        glfwSetWindowMonitor(m_window, glfwGetPrimaryMonitor(), 0, 0,
                             mode->width, mode->height, mode->refreshRate);
    } else {
        const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        glfwSetWindowMonitor(m_window, NULL, 40, 40,
                                GetClientOptions().GetOptions()["window"]["width"],
                                GetClientOptions().GetOptions()["window"]["height"],
                                mode->refreshRate);
    }
}

void conquerspace::engine::SceneManager::SetInitialScene(
    std::shared_ptr<Scene>& scene) {
    m_scene = std::move(scene);
}

void conquerspace::engine::SceneManager::SetScene(
    std::shared_ptr<Scene>& scene) {
    m_next_scene = std::move(scene);
    m_switch = true;
}

void conquerspace::engine::SceneManager::SwitchScene() {
    m_scene = std::move(m_next_scene);
    spdlog::trace("Initializing scene");
    m_scene->Init();
    spdlog::trace("Done Initializing scene");
    m_switch = false;
}

std::shared_ptr<conquerspace::engine::Scene>
conquerspace::engine::SceneManager::GetScene() {
    return m_scene;
}
