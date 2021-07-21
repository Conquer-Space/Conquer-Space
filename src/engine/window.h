/*
 * Copyright 2021 Conquer Space
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
};
}  // namespace engine
}  // namespace conquerspace
