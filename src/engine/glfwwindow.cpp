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
#include "engine/glfwwindow.h"

#include <stb_image.h>  // NOLINT

#include <tracy/Tracy.hpp>

#include "engine/glfwdebug.h"

namespace cqsp::engine {
void GLWindow::KeyboardCallback(GLFWwindow* _w, int key, int scancode, int action, int mods) {
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
    RmlGLFW::ProcessKeyCallback(app->GetRmlUiContext(), key, action, mods);
}

void GLWindow::MouseButtonCallback(GLFWwindow* _w, int button, int action, int mods) {
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
    RmlGLFW::ProcessMouseButtonCallback(app->GetRmlUiContext(), button, action, mods);
}

void GLWindow::FrameBufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);

    m_window_width = width;
    m_window_height = height;
    window_size_changed = true;
    RmlGLFW::ProcessFramebufferSizeCallback(app->GetRmlUiContext(), width, height);
}

void GLWindow::SetCallbacks() {
    // Set user pointer
    glfwSetWindowUserPointer(window, this);

    auto key_callback = [](GLFWwindow* _w, int key, int scancode, int action, int mods) {
        static_cast<GLWindow*>(glfwGetWindowUserPointer(_w))->KeyboardCallback(_w, key, scancode, action, mods);
    };

    auto cursor_position_callback = [](GLFWwindow* _w, double xpos, double ypos) {
        static_cast<GLWindow*>(glfwGetWindowUserPointer(_w))->MousePositionCallback(_w, xpos, ypos);
    };

    auto cursor_enter_callback = [](GLFWwindow* _w, int entered) {
        static_cast<GLWindow*>(glfwGetWindowUserPointer(_w))->MouseEnterCallback(_w, entered);
    };

    auto mouse_button_callback = [](GLFWwindow* _w, int button, int action, int mods) {
        static_cast<GLWindow*>(glfwGetWindowUserPointer(_w))->MouseButtonCallback(_w, button, action, mods);
    };

    auto scroll_callback = [](GLFWwindow* _w, double xoffset, double yoffset) {
        static_cast<GLWindow*>(glfwGetWindowUserPointer(_w))->ScrollCallback(_w, xoffset, yoffset);
    };

    auto drop_callback = [](GLFWwindow* _w, int count, const char** paths) {
        static_cast<GLWindow*>(glfwGetWindowUserPointer(_w))->DropCallback(_w, count, paths);
    };

    auto frame_buffer_callback = [](GLFWwindow* _w, int width, int height) {
        static_cast<GLWindow*>(glfwGetWindowUserPointer(_w))->FrameBufferSizeCallback(_w, width, height);
    };

    auto character_callback = [](GLFWwindow* _w, unsigned int codepoint) {
        static_cast<GLWindow*>(glfwGetWindowUserPointer(_w))->CharacterCallback(_w, codepoint);
    };

    auto error_callback = [](int error_code, const char* description) {
        ENGINE_LOG_ERROR("Error {}: {}", error_code, description);
    };

    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetCursorEnterCallback(window, cursor_enter_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetDropCallback(window, drop_callback);
    glfwSetFramebufferSizeCallback(window, frame_buffer_callback);
    glfwSetCharCallback(window, character_callback);
    glfwSetErrorCallback(error_callback);
}

void GLWindow::Destroy() {
    glfwDestroyWindow(window);
    glfwTerminate();
}

float GLWindow::GetTime() const { return static_cast<float>(glfwGetTime()); }

void GLWindow::SetFullScreen(bool fullscreen) const {
    if (fullscreen) {
        const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 0, 0, mode->width, mode->height, GLFW_DONT_CARE);
    } else {
        const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        glfwSetWindowMonitor(window, NULL, 40, 40, app->GetClientOptions().GetOptions()["window"]["width"],
                             app->GetClientOptions().GetOptions()["window"]["height"], mode->refreshRate);
    }
}

bool GLWindow::ShouldExit() const { return glfwWindowShouldClose(window) == 0; }

bool GLWindow::ExitApplication() {
    glfwSetWindowShouldClose(window, 1);
    return true;
}

void GLWindow::SetIcon(std::string_view path) {
    GLFWimage images[1];
    images[0].pixels = stbi_load((path).data(), &images[0].width, &images[0].height, 0, 4);
    glfwSetWindowIcon(window, 1, images);
    stbi_image_free(images[0].pixels);
}

void GLWindow::OnFrame() {
    ZoneScoped;
    glfwSwapBuffers(window);
    // Before polling events, clear the buttons
    std::memset(m_keys_pressed, (int)false, sizeof(m_keys_pressed));
    std::memset(m_keys_released, (int)false, sizeof(m_keys_released));
    std::memset(m_mouse_keys_pressed, (int)false, sizeof(m_mouse_keys_pressed));
    std::memset(m_mouse_keys_released, (int)false, sizeof(m_mouse_keys_released));
    m_scroll_amount = 0;
    keys_pressed_last.clear();
    keys_released_last.clear();
    code_input.clear();
    window_size_changed = false;
    glfwPollEvents();
}

void GLWindow::InitWindow(int width, int height) {
    if (glfwInit() != GLFW_TRUE) {
        // Then rip
        ENGINE_LOG_CRITICAL("Cannot initialize GLFW");
    }
    if (GLAD_GL_VERSION_4_3 != 0) {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    } else {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    }

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, app->GetClientOptions().GetOptions()["samples"].to_int64());
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, (int)true);
    glfwWindowHint(GLFW_DOUBLEBUFFER, (int)true);
    glfwWindowHint(GLFW_DECORATED,
                   ((bool)app->GetClientOptions().GetOptions()["window"]["decorated"]) ? GLFW_TRUE : GLFW_FALSE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    m_window_width = width;
    m_window_height = height;

    // Create window
    window = glfwCreateWindow(width, height, "Conquer Space", NULL, NULL);
    if (window == NULL) {
        glfwTerminate();
        ENGINE_LOG_CRITICAL("Cannot load glfw");
    }

    glfwMakeContextCurrent(window);

    // Enable vsync
    glfwSwapInterval(1);

    // Add callbacks
    SetCallbacks();

    // Init glad
    if (gladLoadGLLoader((GLADloadproc)glfwGetProcAddress) == 0) {
        glfwTerminate();
        ENGINE_LOG_CRITICAL("Cannot load glad");
    }
    int flags;
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);

    if ((flags & GL_CONTEXT_FLAG_DEBUG_BIT) != 0) {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);  // makes sure errors are displayed synchronously
        if (GLAD_GL_VERSION_4_3 != 0) {
            glDebugMessageCallback(glDebugOutput, nullptr);
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
        }
    }

    // Fix the weird black bar we have on top of windows
    glViewport(0, 0, width, height);
}
}  // namespace cqsp::engine
