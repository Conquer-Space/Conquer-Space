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
#pragma once

namespace cqsp::engine {
/// <summary>
/// The window handles the initialization of the callbacks, and all the input and output.
/// It also manages all the pointers for the windows.
/// </summary>
class Window {
 public:
    virtual bool ButtonIsHeld(int btn) const = 0;
    virtual bool ButtonIsReleased(int btn) const = 0;
    virtual bool ButtonIsPressed(int btn) const = 0;
    virtual double GetMouseX() const = 0;
    virtual double GetMouseY() const = 0;

    virtual bool MouseButtonIsHeld(int btn) const = 0;
    virtual bool MouseButtonIsReleased(int btn) const = 0;
    virtual bool MouseButtonIsPressed(int btn) const = 0;
    virtual bool MouseDragged() const = 0;
    virtual bool MouseMoved() const = 0;
    virtual double MouseButtonLastReleased(int btn) const = 0;
    virtual bool MouseButtonDoubleClicked(int btn) const = 0;

    virtual int GetScrollAmount() const = 0;

    virtual void SetWindowSize(int width, int height) = 0;
    virtual int GetWindowHeight() const = 0;
    virtual int GetWindowWidth() const = 0;

    virtual void SetCallbacks() = 0;
    virtual void Destroy() = 0;

    /// <summary>
    /// Any cleanups or clearing the window has to do each frame
    /// </summary>
    virtual void OnFrame() = 0;

    virtual bool InitWindow(int width, int height) = 0;

    virtual bool WindowSizeChanged() const = 0;

    virtual float GetTime() const = 0;

    virtual void SetFullScreen(bool fullscreen) const = 0;

    virtual bool ShouldExit() const = 0;
    virtual bool ExitApplication() = 0;

    virtual void SetIcon(std::string_view path) = 0;
};
}  // namespace cqsp::engine
