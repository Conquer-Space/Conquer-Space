/* Conquer Space
 * Copyright (C) 2021-2025 Conquer Space
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

#include <RmlUi/Core.h>
#include <RmlUi/Debugger.h>
#include <fmt/core.h>
#include <glad/glad.h>
#include <hjson.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <stb_image.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <tracy/Tracy.hpp>

#include "common/util/logging.h"
#include "common/util/paths.h"
#include "common/util/profiler.h"
#include "common/version.h"
#include "engine/audio/audiointerface.h"
#include "engine/cqspgui.h"
#include "engine/enginelogger.h"
#include "engine/glfwwindow.h"
#include "engine/ui/RmlUi_Platform_GLFW.h"
#include "engine/ui/RmlUi_Renderer_GL3.h"
#include "engine/userinput.h"

namespace cqsp::engine {
namespace {
GLFWwindow* window(cqsp::engine::Window* window) { return reinterpret_cast<GLWindow*>(window)->window; }
}  // namespace

int Application::init() {
    LoggerInit();
    LogInfo();
    GlInit();

    manager.LoadDefaultTexture();
    SetIcon();

    InitAudio();

    InitImgui();
    InitRmlUi();

    if (full_screen) {
        SetFullScreen(true);
    }

    std::unique_ptr<Scene> initial_scene = std::make_unique<EmptyScene>(*this);
    m_scene_manager.SetInitialScene(std::move(initial_scene));

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

    SPDLOG_INFO("ImGui Version: {}", ImGui::GetVersion());
}

void Application::ProcessRmlUiUserInput() {
    ZoneScoped;
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
        key_modifier |= Rml::Input::KeyModifier::KM_NUMLOCK;
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
        rml_context->ProcessKeyDown((Rml::Input::KeyIdentifier)GetRmlUiKey(key), key_modifier);
    }

    for (int key : gl_window->keys_released_last) {
        rml_context->ProcessKeyUp((Rml::Input::KeyIdentifier)GetRmlUiKey(key), key_modifier);
    }

    for (unsigned int key : gl_window->code_input) {
        rml_context->ProcessTextInput(key);
    }

    // Because the glfw's char callback does not register new line.
    if (ButtonIsPressed(GLFW_KEY_ENTER)) {
        rml_context->ProcessTextInput('\n');
    }
}

void Application::InitAudio() {
    // Init audio
    m_audio_interface = new audio::AudioInterface();
    m_audio_interface->Initialize();
    // Set option things
    m_audio_interface->SetMusicVolume(m_client_options.GetOptions()["audio"]["music"]);
    m_audio_interface->SetChannelVolume(1, m_client_options.GetOptions()["audio"]["ui"]);
}

void Application::InitRmlUi() {
    // Begin by installing the custom interfaces.
    m_system_interface = std::make_unique<SystemInterface_GLFW>();
    m_system_interface->SetWindow((static_cast<GLWindow*>(GetWindow())->window));
    m_render_interface = std::make_unique<RenderInterface_GL3>();

    Rml::SetSystemInterface(m_system_interface.get());
    Rml::SetRenderInterface(m_render_interface.get());
    reinterpret_cast<RenderInterface_GL3*>(m_render_interface.get())->SetViewport(GetWindowWidth(), GetWindowHeight());
    // Now we can initialize RmlUi.
    Rml::Initialise();

    rml_context = Rml::CreateContext("main", Rml::Vector2i(GetWindowWidth(), GetWindowHeight()));
    if (rml_context == nullptr) {
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
    std::fstream stream(cqsp::common::util::GetCqspDataPath() + "/core/gfx/fonts/fonts.hjson");
    stream >> Hjson::StreamDecoder(fontDatabase, decOpt);
    std::string fontPath = cqsp::common::util::GetCqspDataPath() + "/core/gfx/fonts/";

    // Load the fonts
    for (int index = 0; index < fontDatabase["rmlui"].size(); index++) {
        Rml::LoadFontFace(fontPath + fontDatabase["rmlui"][index]);
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
    ResetGame();
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

    m_window->Destroy();
    ENGINE_LOG_INFO("Killed GLFW");

    // Save options
    m_client_options.WriteOptions();
    ENGINE_LOG_INFO("Saved Options");

    ENGINE_LOG_INFO("Good bye");
    spdlog::shutdown();
    return 0;
}

void Application::CalculateProjections() {
    float window_ratio = static_cast<float>(GetWindowWidth()) / static_cast<float>(GetWindowHeight());
    three_dim_projection = glm::infinitePerspective(glm::radians(45.f), window_ratio, 0.000001f);
    // For normal rendering
    two_dim_projection =
        glm::ortho(0.0f, static_cast<float>(GetWindowWidth()), 0.0f, static_cast<float>(GetWindowHeight()));
    // For rmlui
    rmlui_projection =
        glm::ortho(0.0f, static_cast<float>(GetWindowWidth()), static_cast<float>(GetWindowHeight()), 0.0f, -1.f, 1.f);
}

Application::Application(int _argc, char* _argv[]) {
    cqsp::common::util::ExePath::exe_path = _argv[0];
    for (int i = 0; i < _argc; i++) {
        cmd_line_args.emplace_back(_argv[i]);
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
        //BEGIN_TIMED_BLOCK(UiCreation);
        m_scene_manager.Ui(deltaTime);
        //END_TIMED_BLOCK(UiCreation);

        //BEGIN_TIMED_BLOCK(ImGui_Render);
        {
            ZoneScopedN("ImGui::Render");
            ImGui::Render();
        }
        //END_TIMED_BLOCK(ImGui_Render);

        //ProcessRmlUiUserInput();
        rml_context->Update();
        m_audio_interface->OnFrame();

        // Clear screen
        glClearColor(0.f, 0.f, 0.f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        // Render scene
        //BEGIN_TIMED_BLOCK(Scene_Render);
        m_scene_manager.Render(deltaTime);
        //END_TIMED_BLOCK(Scene_Render);

        // Shut the opengl debugger up
        int drawFboId = 0;
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFboId);
        if (drawFboId != 0) {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
        reinterpret_cast<RenderInterface_GL3*>(m_render_interface.get())->BeginFrame();
        rml_context->Render();
        reinterpret_cast<RenderInterface_GL3*>(m_render_interface.get())->EndFrame();

        // BEGIN_TIMED_BLOCK(ImGui_Render_Draw);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        //END_TIMED_BLOCK(ImGui_Render_Draw);

        // FPS counter
        if (draw_fps) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            DrawText(fmt::format("FPS: {:.0f}", fps), GetWindowWidth() - 80, GetWindowHeight() - 24);
        }

        m_window->OnFrame();
        FrameMark;
    }

    destroy();
}

bool Application::ShouldExit() { return m_window->ShouldExit(); }

void Application::ExitApplication() { m_window->ExitApplication(); }

Rml::ElementDocument* Application::LoadDocument(const std::string& path) {
    std::filesystem::path doc_path =
        std::filesystem::relative(std::filesystem::path(common::util::GetCqspDataPath()) / path);
    std::string path_name = doc_path.string();
    std::replace(path_name.begin(), path_name.end(), '\\', '/');
    auto document = rml_context->LoadDocument(path_name);
    SPDLOG_INFO("Loading document {}", path_name);
    if (document == nullptr) {
        ENGINE_LOG_WARN("Unable to load document {}", path);
    }
    loaded_documents[path_name] = document;
    return document;
}

void Application::CloseDocument(const std::string& path) {
    std::filesystem::path doc_path =
        std::filesystem::relative(std::filesystem::path(common::util::GetCqspDataPath()) / path);
    std::string path_name = doc_path.string();
    std::replace(path_name.begin(), path_name.end(), '\\', '/');
    loaded_documents[path_name]->Close();
    loaded_documents.erase(path_name);
}

Rml::ElementDocument* Application::ReloadDocument(const std::string& path) {
    std::filesystem::path doc_path =
        std::filesystem::canonical(std::filesystem::path(common::util::GetCqspDataPath()) / path);
    std::string path_name = doc_path.string();
    std::replace(path_name.begin(), path_name.end(), '\\', '/');
    if (loaded_documents.find(path_name) == loaded_documents.end()) {
        return nullptr;
    }
    bool visible = loaded_documents[path_name]->IsVisible();
    loaded_documents[path_name]->Close();
    auto document = rml_context->LoadDocument(path_name);
    loaded_documents[path_name] = document;
    if (visible) {
        document->Show();
    }
    return document;
}

void Application::DrawText(const std::string& text, float x, float y) {
    if (fontShader != nullptr && m_font != nullptr) {
        // Render with size 16 white text
        cqsp::asset::RenderText(*fontShader, *m_font, text, x, y, 16, glm::vec3(1.f, 1.f, 1.f));
    }
}

void Application::DrawText(const std::string& text, const glm::vec3& color, float x, float y) {
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

void Application::DrawText(const std::string& text, const glm::vec3& color, float x, float y, float size) {
    if (fontShader != nullptr && m_font != nullptr) {
        // Render with size 16 white text
        cqsp::asset::RenderText(*fontShader, *m_font, text, x, y, size, color);
    }
}

void Application::DrawTextNormalized(const std::string& text, float x, float y) {
    if (fontShader != nullptr && m_font != nullptr) {
        cqsp::asset::RenderText(*fontShader, *m_font, text, (x + 1) * GetWindowWidth() / 2,
                                (y + 1) * GetWindowHeight() / 2, 16, glm::vec3(1.f, 1.f, 1.f));
    }
}

double Application::GetTime() { return m_window->GetTime(); }

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
            std::filesystem::path(cqsp::common::util::GetCqspAppDataPath()) / "screenshots";
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

    bool success =
        cqsp::asset::SaveImage(screenshot_name.c_str(), GetWindowWidth(), GetWindowHeight(), components, data);
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
    ImFont* defaultFont = io.Fonts->AddFontFromFileTTF((fontPath + fontDatabase["default"]["path"]).c_str(),
                                                       fontDatabase["default"]["size"]);
    io.FontDefault = defaultFont;
}

void Application::SetIcon() { m_window->SetIcon(cqsp::common::util::GetCqspDataPath() + "/" + icon_path); }

void Application::GlInit() {
    m_window = new GLWindow(this);
    m_window->InitWindow(m_client_options.GetOptions()["window"]["width"],
                         m_client_options.GetOptions()["window"]["height"]);

    // Print gl information
    ENGINE_LOG_INFO(" --- Begin GL information ---");
    ENGINE_LOG_INFO("GL version: {}", (char*)glGetString(GL_VERSION));
    ENGINE_LOG_INFO("GL vendor: {}", (char*)glGetString(GL_VENDOR));
    ENGINE_LOG_INFO("GL Renderer: {}", (char*)glGetString(GL_RENDERER));
    ENGINE_LOG_INFO("GL shading language: {}", (char*)glGetString(GL_SHADING_LANGUAGE_VERSION));
    ENGINE_LOG_INFO(" --- End of GL information ---");
}

void Application::LoggerInit() {
    // Get path
    properties["data"] = common::util::GetCqspAppDataPath();
    cqsp::engine::engine_logger = cqsp::common::util::make_logger("app", true);
    auto g_logger = cqsp::common::util::make_logger("game", true);
    spdlog::set_default_logger(g_logger);
}

void Application::LogInfo() {
#ifndef NDEBUG
    ENGINE_LOG_INFO("Conquer Space Debug {} {}", CQSP_VERSION_STRING, GIT_INFO);
#else
    ENGINE_LOG_INFO("Conquer Space {} {}", CQSP_VERSION_STRING, GIT_INFO);
#endif
    ENGINE_LOG_INFO("Platform: {}", PLATFORM_NAME);
    ENGINE_LOG_INFO("Compiled {} {}", __DATE__, __TIME__);
    ENGINE_LOG_INFO("Exe Path: {}", common::util::ExePath::exe_path);
    ENGINE_LOG_INFO("Data Path: {}", common::util::GetCqspDataPath());
    ENGINE_LOG_INFO("Save Path: {}", common::util::GetCqspAppDataPath());

#ifdef TRACY_ENABLE
    ENGINE_LOG_INFO("Tracy protocol version: {}", (uint16_t)tracy::ProtocolVersion);
    ENGINE_LOG_INFO("Tracy broadcast version: {}", (uint16_t)tracy::BroadcastVersion);
#endif  // TRACY_ENABLED
}

void Application::SetWindowDimensions(int width, int height) { m_window->SetWindowSize(width, height); }

void Application::SetFullScreen(bool screen) { m_window->SetFullScreen(screen); }

Application::CqspEventInstancer::CqspEventInstancer() = default;

Application::CqspEventInstancer::~CqspEventInstancer() = default;

Rml::EventListener* Application::CqspEventInstancer::InstanceEventListener(const Rml::String& value,
                                                                           Rml::Element* element) {
    // Add event activation, I guess
    return new CqspEventListener(value);
}

Application::CqspEventListener::~CqspEventListener() = default;

void Application::CqspEventListener::ProcessEvent(Rml::Event& event) {}
}  // namespace cqsp::engine
