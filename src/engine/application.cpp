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
#include <RmlUi/Core.h>
#include <RmlUi/Debugger.h>

#include <fmt/core.h>
#include <hjson.h>

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <stb_image.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <tracy/Tracy.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "common/util/profiler.h"
#include "common/version.h"
#include "engine/audio/audiointerface.h"
#include "common/util/paths.h"
#include "common/util/logging.h"
#include "engine/cqspgui.h"
#include "engine/ui/rmlrenderinterface.h"
#include "engine/ui/rmlsysteminterface.h"
#include "engine/userinput.h"
#include "engine/enginelogger.h"

namespace cqsp::engine {
namespace {
const char* ParseType(GLenum type) {
    switch (type) {
        case GL_DEBUG_TYPE_ERROR:
            return ("Error");
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
            return ("Deprecated Behaviour");
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
            return ("Undefined Behaviour");
        case GL_DEBUG_TYPE_PORTABILITY:
            return ("Portability");
        case GL_DEBUG_TYPE_PERFORMANCE:
            return ("Performance");
        case GL_DEBUG_TYPE_MARKER:
            return ("Marker");
        case GL_DEBUG_TYPE_PUSH_GROUP:
            return ("Push Group");
        case GL_DEBUG_TYPE_POP_GROUP:
            return ("Pop Group");
        case GL_DEBUG_TYPE_OTHER:
            return ("Other");
    }
}

const char* ParseSeverity(GLenum severity) {
    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH:
            return ("high");
        case GL_DEBUG_SEVERITY_MEDIUM:
            return ("medium");
        case GL_DEBUG_SEVERITY_LOW:
            return ("low");
        case GL_DEBUG_SEVERITY_NOTIFICATION:
            return ("notification");
    }
}

const char* ParseSource(GLenum source) {
    switch (source) {
    case GL_DEBUG_SOURCE_API:
        return ("API");
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        return ("Window System");
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
        return ("Shader Compiler");
    case GL_DEBUG_SOURCE_THIRD_PARTY:
        return ("Third Party");
    case GL_DEBUG_SOURCE_APPLICATION:
        return ("Application");
    case GL_DEBUG_SOURCE_OTHER:
        return ("Other");
    }
}

void APIENTRY glDebugOutput(GLenum source, GLenum type, unsigned int id,
                            GLenum severity, GLsizei length,
                            const char* message, const void* userParam) {
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204)
        return;  // ignore these non-significant error codes

    ENGINE_LOG_INFO("{} message from {} ({}:{}): {}", ParseType(type),
                    ParseSource(source), ParseSeverity(severity), id, message);
}

class GLWindow : public cqsp::engine::Window {
 public:
    bool ButtonIsHeld(int btn) const { return m_keys_held[btn]; }
    bool ButtonIsReleased(int btn) const { return m_keys_released[btn]; }
    bool ButtonIsPressed(int btn) const { return m_keys_pressed[btn]; }
    double GetMouseX() const { return m_mouse_x; }
    double GetMouseY() const { return m_mouse_y; }

    bool MouseButtonIsHeld(int btn) const { return m_mouse_keys_held[btn]; }
    bool MouseButtonIsReleased(int btn) const { return m_mouse_keys_released[btn]; }
    bool MouseButtonIsPressed(int btn) const { return m_mouse_keys_pressed[btn]; }

    bool MouseDragged() const {
        return !(m_mouse_x == m_mouse_x_on_pressed &&
                 m_mouse_y == m_mouse_y_on_pressed);
    }

    void KeyboardCallback(GLFWwindow* _w, int key, int scancode, int action,
                          int mods) {
        if (action == GLFW_PRESS) {
            m_keys_held[key] = true;
            m_keys_pressed[key] = true;
            keys_pressed_last.push_back(key);
            m_mods = mods;
        } else if (action == GLFW_RELEASE) {
            m_keys_held[key] = false;
            m_keys_released[key] = true;
            keys_released_last.push_back(key);
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
            m_mouse_pressed_time[button] = glfwGetTime() - m_mouse_keys_last_pressed[button];
            m_mouse_keys_last_pressed[button] = glfwGetTime();
        } else if (action == GLFW_RELEASE) {
            m_mouse_keys_held[button] = false;
            m_mouse_keys_released[button] = true;
        }
    }

    void ScrollCallback(GLFWwindow* _w, double xoffset, double yoffset) {
        m_scroll_amount = yoffset;
    }

    void CharacterCallback(GLFWwindow* window, unsigned int codepoint) {
        // Callback
        code_input.push_back(codepoint);
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

        auto character_callback = [](GLFWwindow* _w, unsigned int codepoint) {
            static_cast<GLWindow*>(glfwGetWindowUserPointer(_w))
                ->CharacterCallback(_w, codepoint);
        };

        glfwSetKeyCallback(window, key_callback);
        glfwSetCursorPosCallback(window, cursor_position_callback);
        glfwSetCursorEnterCallback(window, cursor_enter_callback);
        glfwSetMouseButtonCallback(window, mouse_button_callback);
        glfwSetScrollCallback(window, scroll_callback);
        glfwSetDropCallback(window, drop_callback);
        glfwSetFramebufferSizeCallback(window, frame_buffer_callback);
        glfwSetCharCallback(window, character_callback);
    }

    void OnFrame() {
        // Before polling events, clear the buttons
        std::memset(m_keys_pressed, false, sizeof(m_keys_pressed));
        std::memset(m_keys_released, false, sizeof(m_keys_released));
        std::memset(m_mouse_keys_pressed, false, sizeof(m_mouse_keys_pressed));
        std::memset(m_mouse_keys_released, false, sizeof(m_mouse_keys_released));
        m_scroll_amount = 0;
        keys_pressed_last.clear();
        keys_released_last.clear();
        code_input.clear();
        window_size_changed = false;
    }

    void SetWindowSize(int width, int height) {
        m_window_width = width;
        m_window_height = height;
        glfwSetWindowSize(window, width, height);
    }

    bool WindowSizeChanged() const { return window_size_changed; }

    int GetScrollAmount() const { return m_scroll_amount; }
    int GetWindowHeight() const { return m_window_height; }
    int GetWindowWidth() const { return m_window_width; }

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
            ENGINE_LOG_INFO("Cannot load glfw");
        }

        glfwMakeContextCurrent(window);

        // Enable vsync
        glfwSwapInterval(1);

        // Add callbacks
        SetCallbacks();

        // Init glad
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            glfwTerminate();
            ENGINE_LOG_CRITICAL("Cannot load glad");
        }
        int flags;
        glGetIntegerv(GL_CONTEXT_FLAGS, &flags);

        if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
            glEnable(GL_DEBUG_OUTPUT);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); // makes sure errors are displayed synchronously
            glDebugMessageCallback(glDebugOutput, nullptr);
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
        }
    }

    double MouseButtonLastReleased(int btn) const {
        return m_mouse_keys_last_pressed[btn];
    }

    bool MouseButtonDoubleClicked(int btn) const {
        bool is_pressed_long_enough = (m_mouse_pressed_time[btn]) <= 0.5f;
        return (MouseButtonIsPressed(btn) && is_pressed_long_enough);
    }

    GLFWwindow* window;

    Application* app;

 private:
    bool window_size_changed;
    double m_mouse_x;
    double m_mouse_y;

    double m_mouse_x_on_pressed;
    double m_mouse_y_on_pressed;

    bool m_mouse_keys_held[GLFW_MOUSE_BUTTON_LAST] = {false};
    bool m_mouse_keys_released[GLFW_MOUSE_BUTTON_LAST] = {false};
    bool m_mouse_keys_pressed[GLFW_MOUSE_BUTTON_LAST] = {false};

    double m_mouse_keys_last_pressed[GLFW_MOUSE_BUTTON_LAST] = { 0.0 };
    double m_mouse_pressed_time[GLFW_MOUSE_BUTTON_LAST] = { 0.0 };

    bool m_keys_held[GLFW_KEY_LAST] = {false};
    bool m_keys_released[GLFW_KEY_LAST] = {false};
    bool m_keys_pressed[GLFW_KEY_LAST] = {false};

    double m_scroll_amount;


    int m_window_width, m_window_height;

 public:
    std::vector<int> keys_pressed_last;
    std::vector<int> keys_released_last;
    std::vector<unsigned int> code_input;

    int m_mods;
};

GLFWwindow* window(cqsp::engine::Window* window) {
    return reinterpret_cast<GLWindow*>(window)->window;
}
}  // namespace

int Application::init() {
    LoggerInit();
    LogInfo();
    GlInit();

    manager.LoadDefaultTexture();
    // Init audio
    m_audio_interface = new audio::AudioInterface();
    m_audio_interface->Initialize();
    // Set option things
    m_audio_interface->SetMusicVolume(m_client_options.GetOptions()["audio"]["music"]);
    m_audio_interface->SetChannelVolume(1, m_client_options.GetOptions()["audio"]["ui"]);

    SetIcon();

    InitImgui();
    InitRmlUi();

    if (full_screen) {
        SetFullScreen(true);
    }

    std::unique_ptr<Scene> initial_scene = std::make_unique<EmptyScene>(*this);
    m_scene_manager.SetInitialScene(std::move(initial_scene));

    m_game = std::make_unique<cqsp::common::Game>();
    return 0;
}

void Application::InitImgui() {
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGui::StyleColorsDark();
    ImGui::GetIO().IniFilename = NULL;
    CQSPGui::SetApplication(this);
    m_ne_context = ax::NodeEditor::CreateEditor();
    ax::NodeEditor::SetCurrentEditor(m_ne_context);
    InitFonts();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window(m_window), true);
    ImGui_ImplOpenGL3_Init("#version 130");
}

void Application::ProcessRmlUiUserInput() {
    rml_context->SetDimensions(Rml::Vector2i(GetWindowWidth(), GetWindowHeight()));

    GLWindow* gl_window = reinterpret_cast<GLWindow*>(m_window);
    int mods = gl_window->m_mods;
    int key_modifier = 0;
    // Check for key mods
    if ((mods & GLFW_MOD_CONTROL) == GLFW_MOD_CONTROL) {
        key_modifier |= Rml::Input::KeyModifier::KM_CTRL;
    }
    if ((mods & GLFW_MOD_SHIFT) == GLFW_MOD_SHIFT) {
        key_modifier |= Rml::Input::KeyModifier::KM_SHIFT;
    }
    if ((mods & GLFW_MOD_ALT) == GLFW_MOD_ALT) {
        key_modifier |= Rml::Input::KeyModifier::KM_ALT;
    }
    if ((mods & GLFW_MOD_CAPS_LOCK) == GLFW_MOD_CAPS_LOCK) {
        key_modifier |= Rml::Input::KeyModifier::KM_CAPSLOCK;
    }
    if ((mods & GLFW_MOD_NUM_LOCK) == GLFW_MOD_NUM_LOCK) {
        Rml::Input::KeyModifier::KM_NUMLOCK;
    }
    rml_context->ProcessMouseMove(GetMouseX(), GetMouseY(), key_modifier);

    // Mouse down
    if (MouseButtonIsPressed(GLFW_MOUSE_BUTTON_LEFT)) {
        rml_context->ProcessMouseButtonDown(0, key_modifier);
    }
    if (MouseButtonIsPressed(GLFW_MOUSE_BUTTON_RIGHT)) {
        rml_context->ProcessMouseButtonDown(1, key_modifier);
    }
    if (MouseButtonIsReleased(GLFW_MOUSE_BUTTON_LEFT)) {
        rml_context->ProcessMouseButtonUp(0, key_modifier);
    }
    if (MouseButtonIsReleased(GLFW_MOUSE_BUTTON_RIGHT)) {
        rml_context->ProcessMouseButtonUp(1, key_modifier);
    }
    // Scroll wheel is flipped for some reason
    rml_context->ProcessMouseWheel(-GetScrollAmount(), key_modifier);

    // Process key inputs
    for (int key : gl_window->keys_pressed_last) {
        rml_context->ProcessKeyDown((Rml::Input::KeyIdentifier)GetRmlUiKey(key),
                                    key_modifier);
    }

    for (int key : gl_window->keys_released_last) {
        rml_context->ProcessKeyUp((Rml::Input::KeyIdentifier)GetRmlUiKey(key),
                                    key_modifier);
    }

    for (unsigned int key : gl_window->code_input) {
        rml_context->ProcessTextInput(key);
    }

    // Because the glfw's char callback does not register new line.
    if (ButtonIsPressed(GLFW_KEY_ENTER)) {
        rml_context->ProcessTextInput('\n');
    }
}

void Application::InitRmlUi() {
    // Begin by installing the custom interfaces.
    m_system_interface = std::make_unique<CQSPSystemInterface>(*this);
    m_render_interface = std::make_unique<CQSPRenderInterface>(*this);

    Rml::SetSystemInterface(m_system_interface.get());
    Rml::SetRenderInterface(m_render_interface.get());

    // Now we can initialize RmlUi.
    Rml::Initialise();

    rml_context = Rml::CreateContext("main", Rml::Vector2i(GetWindowWidth(), GetWindowHeight()));
    if (!rml_context) {
        ENGINE_LOG_CRITICAL("Unable to load rml context!");
    }

    // Disable debugger ui for now
#if 0
    Rml::Debugger::Initialise(rml_context);
    Rml::Debugger::SetVisible(true);
#endif

    // Load rmlui fonts
    // TODO(EhWhoAmI): Load this somewhere else
    Hjson::Value fontDatabase;
    Hjson::DecoderOptions decOpt;
    decOpt.comments = false;
    std::fstream stream(cqsp::common::util::GetCqspDataPath() +
                        "/core/gfx/fonts/fonts.hjson");
    stream >> Hjson::StreamDecoder(fontDatabase, decOpt);
    std::string fontPath =
        cqsp::common::util::GetCqspDataPath() + "/core/gfx/fonts/";

    // Load the fonts
    for (int index = 0; index < fontDatabase["rmlui"].size(); index++) {
        Rml::LoadFontFace((fontPath + fontDatabase["rmlui"][index]).c_str());
    }
    // Load fonts
    //m_event_instancer = std::make_unique<CqspEventInstancer>();
    //Rml::Factory::RegisterEventListenerInstancer(m_event_instancer.get());
}

int Application::destroy() {
    // Delete scene
    ENGINE_LOG_INFO("Destroying scene");
    m_scene_manager.DeleteCurrentScene();
    ENGINE_LOG_INFO("Done Destroying scene");

    // Clear assets
    ENGINE_LOG_INFO("Deleting game");
    m_game.reset();
    ENGINE_LOG_INFO("Deleted game");

    ENGINE_LOG_INFO("Deleting audio interface");
    m_audio_interface->Destruct();
    ENGINE_LOG_INFO("Deleted audio interface");

    ENGINE_LOG_INFO("Clearing assets");
    manager.ClearAssets();
    ENGINE_LOG_INFO("Cleared assets");

    ENGINE_LOG_INFO("Deleting audio interface");
    delete m_audio_interface;
    ENGINE_LOG_INFO("Deleted audio interface");

    ax::NodeEditor::DestroyEditor(m_ne_context);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
    ENGINE_LOG_INFO("Killed ImGui");

    Rml::Shutdown();
    ENGINE_LOG_INFO("Killed RmlUi");

    glfwDestroyWindow(window(m_window));
    glfwTerminate();
    ENGINE_LOG_INFO("Killed GLFW");

    ENGINE_LOG_INFO("Good bye");
    spdlog::shutdown();
    return 0;
}

void Application::CalculateProjections() {
    float window_ratio = static_cast<float>(GetWindowWidth()) /
                    static_cast<float>(GetWindowHeight());
    three_dim_projection = glm::infinitePerspective(glm::radians(45.f), window_ratio, 0.000001f);
    // For normal rendering
    two_dim_projection =
        glm::ortho(0.0f,
                   static_cast<float>(GetWindowWidth()), 0.0f,
                   static_cast<float>(GetWindowHeight()));
    // For rmlui
    rmlui_projection =
        glm::ortho(0.0f,
                   static_cast<float>(GetWindowWidth()),
                   static_cast<float>(GetWindowHeight()), 0.0f,
                   -1.f, 1.f);
}

Application::Application(int _argc, char* _argv[]) {
    cqsp::common::util::ExePath::exe_path = _argv[0];
    for (int i = 0; i < _argc; i++) {
        cmd_line_args.push_back(_argv[i]);
    }
    // Get exe path
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

void Application::run() {
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

        CalculateProjections();
        if (fontShader != nullptr && m_font != nullptr) {
            // Set font projection
            fontShader->UseProgram();
            fontShader->setMat4("projection", two_dim_projection);
        }

        // Switch scene
        if (m_scene_manager.ToSwitchScene()) {
            m_scene_manager.SwitchScene();
        }

        // Update
        m_scene_manager.Update(deltaTime);

        // Init imgui
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Gui
        BEGIN_TIMED_BLOCK(UiCreation);
        m_scene_manager.Ui(deltaTime);
        END_TIMED_BLOCK(UiCreation);

        BEGIN_TIMED_BLOCK(ImGui_Render);
        ImGui::Render();
        END_TIMED_BLOCK(ImGui_Render);

        ProcessRmlUiUserInput();
        rml_context->Update();

        // Clear screen
        glClearColor(0.f, 0.f, 0.f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        // Render scene
        BEGIN_TIMED_BLOCK(Scene_Render);
        m_scene_manager.Render(deltaTime);
        END_TIMED_BLOCK(Scene_Render);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        rml_context->Render();

        BEGIN_TIMED_BLOCK(ImGui_Render_Draw);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        END_TIMED_BLOCK(ImGui_Render_Draw);

        // FPS counter
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        DrawText(fmt::format("FPS: {:.0f}", fps), GetWindowWidth() - 80,
                 GetWindowHeight() - 24);

        glfwSwapBuffers(window(m_window));

        m_window->OnFrame();
        glfwPollEvents();
        FrameMark;
    }

    destroy();
}

bool Application::ShouldExit() {
    return !glfwWindowShouldClose(window(m_window));
}

void Application::ExitApplication() {
    glfwSetWindowShouldClose(window(m_window), true);
}

Rml::ElementDocument* Application::LoadDocument(const std::string& path) {
    auto document = rml_context->LoadDocument(path);
    if (!document) {
        ENGINE_LOG_WARN("Unable to load document {}", path);
    }
    loaded_documents[path] = document;
    return document;
}

void Application::CloseDocument(const std::string& path) {
    loaded_documents[path]->Close();
    loaded_documents.erase(path);
}

Rml::ElementDocument* Application::ReloadDocument(const std::string& path) {
    if (loaded_documents.find(path) == loaded_documents.end()) {
        return nullptr;
    }
    bool visible = loaded_documents[path]->IsVisible();
    loaded_documents[path]->Close();
    auto document = rml_context->LoadDocument(path);
    loaded_documents[path] = document;
    if (visible) {
        document->Show();
    }
    return document;
}

void Application::DrawText(const std::string& text, float x,
                                         float y) {
    if (fontShader != nullptr && m_font != nullptr) {
        // Render with size 16 white text
        cqsp::asset::RenderText(*fontShader, *m_font, text, x, y, 16, glm::vec3(1.f, 1.f, 1.f));
    }
}

void Application::DrawText(const std::string& text, const glm::vec3& color,
                           float x, float y) {
    if (fontShader != nullptr && m_font != nullptr) {
        // Render with size 16 white text
        cqsp::asset::RenderText(*fontShader, *m_font, text, x, y, 16, color);
    }
}

void Application::DrawText(const std::string& text, float x, float y, float size) {
    if (fontShader != nullptr && m_font != nullptr) {
        // Render with size 16 white text
        cqsp::asset::RenderText(*fontShader, *m_font, text, x, y, size, glm::vec3(1.f, 1.f, 1.f));
    }
}

void Application::DrawText(const std::string& text, const glm::vec3& color,
                           float x, float y, float size) {
    if (fontShader != nullptr && m_font != nullptr) {
        // Render with size 16 white text
        cqsp::asset::RenderText(*fontShader, *m_font, text, x, y, size, color);
    }
}

void Application::DrawTextNormalized(const std::string& text, float x, float y) {
    if (fontShader != nullptr && m_font != nullptr) {
        cqsp::asset::RenderText(*fontShader, *m_font, text, (x + 1) * GetWindowWidth()/2,
                                (y + 1) * GetWindowHeight() / 2, 16, glm::vec3(1.f, 1.f, 1.f));
    }
}

double Application::GetTime() { return glfwGetTime(); }

bool Application::Screenshot(const char* path) {
    // Take screenshot
    const int components = 3;
    const int byte_size = GetWindowWidth() * GetWindowHeight() * components;
    unsigned char* data = new unsigned char[byte_size];

    // Read data
    glReadPixels(0, 0, GetWindowWidth(), GetWindowHeight(), GL_RGB, GL_UNSIGNED_BYTE, data);

    std::string screenshot_name;
    if (path == NULL) {
        // Make screenshot folder
        std::filesystem::path screenshot_folder =
                            std::filesystem::path(cqsp::common::util::GetCqspSavePath()) / "screenshots";
        std::filesystem::create_directories(screenshot_folder);

        // Default file name is YYYY-MM-DD_HH.MM.SS.png in the data folder.
        auto time_now = std::chrono::system_clock::now();
        std::time_t time_pt = std::chrono::system_clock::to_time_t(time_now);

        // Why is put_time so annoyingly confusing??
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_pt), "%F_%H.%M.%S.png");
        screenshot_name = (screenshot_folder / screenshot_name).string();
    } else {
        screenshot_name = path;
    }

    bool success = cqsp::asset::SaveImage(screenshot_name.c_str(),
                                          GetWindowWidth(), GetWindowHeight(), components, data);
    delete[] data;
    return success;
}

void Application::InitFonts() {
    Hjson::Value fontDatabase;
    Hjson::DecoderOptions decOpt;
    decOpt.comments = false;
    std::fstream stream(cqsp::common::util::GetCqspDataPath() + "/core/gfx/fonts/fonts.hjson");

    stream >> Hjson::StreamDecoder(fontDatabase, decOpt);
    std::string fontPath = cqsp::common::util::GetCqspDataPath() + "/core/gfx/fonts/";
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

void Application::SetIcon() {
    GLFWimage images[1];
    images[0].pixels = stbi_load((cqsp::common::util::GetCqspDataPath() + "/" + icon_path).c_str(),
                                 &images[0].width, &images[0].height, 0, 4);
    glfwSetWindowIcon(window(m_window), 1, images);
    stbi_image_free(images[0].pixels);
}

void Application::GlInit() {
    m_window = new GLWindow();
    ((GLWindow*)m_window)->app = this;
    m_window->InitWindow(m_client_options.GetOptions()["window"]["width"],
                            m_client_options.GetOptions()["window"]["height"]);

    // Print gl information
    ENGINE_LOG_INFO(" --- Begin GL information ---");
    ENGINE_LOG_INFO("GL version: {}", glGetString(GL_VERSION));
    ENGINE_LOG_INFO("GL vendor: {}", glGetString(GL_VENDOR));
    ENGINE_LOG_INFO("GL Renderer: {}", glGetString(GL_RENDERER));
    ENGINE_LOG_INFO("GL shading language: {}",
                       glGetString(GL_SHADING_LANGUAGE_VERSION));
    ENGINE_LOG_INFO(" --- End of GL information ---");
}

void Application::LoggerInit() {
    // Get path
    properties["data"] = common::util::GetCqspSavePath();
    cqsp::engine::engine_logger = cqsp::common::util::make_logger("app", true);
    auto g_logger = cqsp::common::util::make_logger("game", true);
    spdlog::set_default_logger(g_logger);
}

void Application::LogInfo() {
#ifndef NDEBUG
    ENGINE_LOG_INFO("Conquer Space Debug {} {}", CQSP_VERSION_STRING,
                       GIT_INFO);
#else
    ENGINE_LOG_INFO("Conquer Space {} {}", CQSP_VERSION_STRING,
                       GIT_INFO);
#endif
    ENGINE_LOG_INFO("Platform: {}", PLATFORM_NAME);
    ENGINE_LOG_INFO("Compiled {} {}", __DATE__, __TIME__);
    ENGINE_LOG_INFO("Exe Path: {}", common::util::ExePath::exe_path);
    ENGINE_LOG_INFO("Data Path: {}", common::util::GetCqspDataPath());
    ENGINE_LOG_INFO("Save Path: {}", common::util::GetCqspSavePath());

#ifdef TRACY_ENABLE
    ENGINE_LOG_INFO("Tracy protocol version: {}", tracy::ProtocolVersion);
    ENGINE_LOG_INFO("Tracy broadcast version: {}", tracy::BroadcastVersion);
#endif  // TRACY_ENABLED
}

void Application::SetWindowDimensions(int width, int height) {
    m_window->SetWindowSize(width, height);
}

void Application::SetFullScreen(bool screen) {
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

void SceneManager::SetInitialScene(std::unique_ptr<Scene> scene) {
    m_scene = std::move(scene);
}

void SceneManager::SetScene(std::unique_ptr<Scene> scene) {
    m_next_scene = std::move(scene);
    m_switch = true;
}

void SceneManager::SwitchScene() {
    m_scene = std::move(m_next_scene);
    ENGINE_LOG_TRACE("Initializing scene");
    m_scene->Init();
    ENGINE_LOG_TRACE("Done Initializing scene");
    m_switch = false;
}

Scene* SceneManager::GetScene() {
    return m_scene.get();
}

void SceneManager::DeleteCurrentScene() { m_scene.reset(); }

void SceneManager::Update(float deltaTime) { m_scene->Update(deltaTime); }

void SceneManager::Ui(float deltaTime) { m_scene->Ui(deltaTime); }

void SceneManager::Render(float deltaTime) { m_scene->Render(deltaTime); }

Application::CqspEventInstancer::CqspEventInstancer() {}

Application::CqspEventInstancer::~CqspEventInstancer() {}

Rml::EventListener* Application::CqspEventInstancer::
                    InstanceEventListener(const Rml::String & value, Rml::Element * element) {
    // Add event activation, I guess
    return new CqspEventListener(value);
}

Application::CqspEventListener::~CqspEventListener() {
}

void Application::CqspEventListener::ProcessEvent(
    Rml::Event& event) {}
}  // namespace cqsp::engine
