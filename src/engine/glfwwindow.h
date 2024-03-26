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

#include <vector>

#include "engine/application.h"
#include "engine/window.h"

namespace cqsp::engine {
class GLWindow : public cqsp::engine::Window {
 public:
    explicit GLWindow(Application* app) : app(app) {}

    bool ButtonIsHeld(int btn) const { return m_keys_held[btn]; }
    bool ButtonIsReleased(int btn) const { return m_keys_released[btn]; }
    bool ButtonIsPressed(int btn) const { return m_keys_pressed[btn]; }
    double GetMouseX() const { return m_mouse_x; }
    double GetMouseY() const { return m_mouse_y; }

    bool MouseButtonIsHeld(int btn) const { return m_mouse_keys_held[btn]; }
    bool MouseButtonIsReleased(int btn) const { return m_mouse_keys_released[btn]; }
    bool MouseButtonIsPressed(int btn) const { return m_mouse_keys_pressed[btn]; }

    bool MouseDragged() const { return !(m_mouse_x == m_mouse_x_on_pressed && m_mouse_y == m_mouse_y_on_pressed); }

    void KeyboardCallback(GLFWwindow* _w, int key, int scancode, int action, int mods);

    void MousePositionCallback(GLFWwindow* _w, double xpos, double ypos) {
        m_mouse_x = xpos;
        m_mouse_y = ypos;
        RmlGLFW::ProcessCursorPosCallback(app->GetRmlUiContext(), xpos, ypos, 0);
    }

    void MouseEnterCallback(GLFWwindow* _w, int entered) {
        RmlGLFW::ProcessCursorEnterCallback(app->GetRmlUiContext(), entered);
    }

    void MouseButtonCallback(GLFWwindow* _w, int button, int action, int mods);

    void ScrollCallback(GLFWwindow* _w, double xoffset, double yoffset) {
        m_scroll_amount = yoffset;
        RmlGLFW::ProcessScrollCallback(app->GetRmlUiContext(), yoffset, 0);
    }

    void CharacterCallback(GLFWwindow* window, unsigned int codepoint) {
        // Callback
        code_input.push_back(codepoint);
        RmlGLFW::ProcessCharCallback(app->GetRmlUiContext(), codepoint);
    }

    void DropCallback(GLFWwindow* _w, int count, const char** paths) {}

    void FrameBufferSizeCallback(GLFWwindow* window, int width, int height);

    void SetCallbacks();

    void OnFrame();

    void SetWindowSize(int width, int height) {
        m_window_width = width;
        m_window_height = height;
        glfwSetWindowSize(window, width, height);
    }

    bool WindowSizeChanged() const { return window_size_changed; }

    int GetScrollAmount() const { return m_scroll_amount; }
    int GetWindowHeight() const { return m_window_height; }
    int GetWindowWidth() const { return m_window_width; }

    void InitWindow(int width, int height);

    double MouseButtonLastReleased(int btn) const { return m_mouse_keys_last_pressed[btn]; }

    bool MouseButtonDoubleClicked(int btn) const {
        bool is_pressed_long_enough = (m_mouse_pressed_time[btn]) <= 0.5f;
        return (MouseButtonIsPressed(btn) && is_pressed_long_enough);
    }

    void Destroy();

    float GetTime() const;

    void SetFullScreen(bool fullscreen) const;

    bool ShouldExit() const;
    bool ExitApplication();

    void SetIcon(std::string_view path);

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

    double m_mouse_keys_last_pressed[GLFW_MOUSE_BUTTON_LAST] = {0.0};
    double m_mouse_pressed_time[GLFW_MOUSE_BUTTON_LAST] = {0.0};

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
}  // namespace cqsp::engine
