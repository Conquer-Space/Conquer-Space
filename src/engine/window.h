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

namespace conquerspace {
namespace engine {
class Window {
   public:
    virtual bool ButtonIsHeld(int btn) = 0;
    virtual bool ButtonIsReleased(int btn) = 0;
    virtual bool ButtonIsPressed(int btn) = 0;
    virtual double GetMouseX() = 0;
    virtual double GetMouseY() = 0;

    virtual bool MouseButtonIsHeld(int btn) = 0;
    virtual bool MouseButtonIsReleased(int btn) = 0;
    virtual bool MouseButtonIsPressed(int btn) = 0;
    virtual bool MouseDragged() = 0;

    virtual int GetScrollAmount() = 0;

    virtual void SetWindowSize(int width, int height) = 0;
    virtual int GetWindowHeight() = 0;
    virtual int GetWindowWidth() = 0;

    virtual void SetCallbacks() = 0;

    virtual void OnFrame() = 0;

    virtual void InitWindow(int width, int height) = 0;

    virtual bool WindowSizeChanged() = 0;
};
}  // namespace engine
}  // namespace conquerspace
