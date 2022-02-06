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
#include "engine/userinput.h"

#include <RmlUi/Core/Input.h>
#include <GLFW/glfw3.h>

int cqsp::engine::GetRmlUiKey(int glfw_key) {
    switch (glfw_key) {
    case GLFW_KEY_A:
        return Rml::Input::KI_A;
    case GLFW_KEY_B:
        return Rml::Input::KI_B;
    case GLFW_KEY_C:
        return Rml::Input::KI_C;
    case GLFW_KEY_D:
        return Rml::Input::KI_D;
    case GLFW_KEY_E:
        return Rml::Input::KI_E;
    case GLFW_KEY_F:
        return Rml::Input::KI_F;
    case GLFW_KEY_G:
        return Rml::Input::KI_G;
    case GLFW_KEY_H:
        return Rml::Input::KI_H;
    case GLFW_KEY_I:
        return Rml::Input::KI_I;
    case GLFW_KEY_J:
        return Rml::Input::KI_J;
    case GLFW_KEY_K:
        return Rml::Input::KI_K;
    case GLFW_KEY_L:
        return Rml::Input::KI_L;
    case GLFW_KEY_M:
        return Rml::Input::KI_M;
    case GLFW_KEY_N:
        return Rml::Input::KI_N;
    case GLFW_KEY_O:
        return Rml::Input::KI_O;
    case GLFW_KEY_P:
        return Rml::Input::KI_P;
    case GLFW_KEY_Q:
        return Rml::Input::KI_Q;
    case GLFW_KEY_R:
        return Rml::Input::KI_R;
    case GLFW_KEY_S:
        return Rml::Input::KI_S;
    case GLFW_KEY_T:
        return Rml::Input::KI_T;
    case GLFW_KEY_U:
        return Rml::Input::KI_U;
    case GLFW_KEY_V:
        return Rml::Input::KI_V;
    case GLFW_KEY_W:
        return Rml::Input::KI_W;
    case GLFW_KEY_X:
        return Rml::Input::KI_X;
    case GLFW_KEY_Y:
        return Rml::Input::KI_Y;
    case GLFW_KEY_Z:
        return Rml::Input::KI_Z;
    case GLFW_KEY_0:
        return Rml::Input::KI_0;
    case GLFW_KEY_1:
        return Rml::Input::KI_1;
    case GLFW_KEY_2:
        return Rml::Input::KI_2;
    case GLFW_KEY_3:
        return Rml::Input::KI_3;
    case GLFW_KEY_4:
        return Rml::Input::KI_4;
    case GLFW_KEY_5:
        return Rml::Input::KI_5;
    case GLFW_KEY_6:
        return Rml::Input::KI_6;
    case GLFW_KEY_7:
        return Rml::Input::KI_7;
    case GLFW_KEY_8:
        return Rml::Input::KI_8;
    case GLFW_KEY_9:
        return Rml::Input::KI_9;
    case GLFW_KEY_KP_0:
        return Rml::Input::KI_NUMPAD0;
    case GLFW_KEY_KP_1:
        return Rml::Input::KI_NUMPAD1;
    case GLFW_KEY_KP_2:
        return Rml::Input::KI_NUMPAD2;
    case GLFW_KEY_KP_3:
        return Rml::Input::KI_NUMPAD3;
    case GLFW_KEY_KP_4:
        return Rml::Input::KI_NUMPAD4;
    case GLFW_KEY_KP_5:
        return Rml::Input::KI_NUMPAD5;
    case GLFW_KEY_KP_6:
        return Rml::Input::KI_NUMPAD6;
    case GLFW_KEY_KP_7:
        return Rml::Input::KI_NUMPAD7;
    case GLFW_KEY_KP_8:
        return Rml::Input::KI_NUMPAD8;
    case GLFW_KEY_KP_9:
        return Rml::Input::KI_NUMPAD9;
    case GLFW_KEY_LEFT:
        return Rml::Input::KI_LEFT;
    case GLFW_KEY_RIGHT:
        return Rml::Input::KI_RIGHT;
    case GLFW_KEY_UP:
        return Rml::Input::KI_UP;
    case GLFW_KEY_DOWN:
        return Rml::Input::KI_DOWN;
    case GLFW_KEY_KP_ADD:
        return Rml::Input::KI_ADD;
    case GLFW_KEY_BACKSPACE:
        return Rml::Input::KI_BACK;
    case GLFW_KEY_DELETE:
        return Rml::Input::KI_DELETE;
    case GLFW_KEY_KP_DIVIDE:
        return Rml::Input::KI_DIVIDE;
    case GLFW_KEY_END:
        return Rml::Input::KI_END;
    case GLFW_KEY_ESCAPE:
        return Rml::Input::KI_ESCAPE;
    case GLFW_KEY_F1:
        return Rml::Input::KI_F1;
    case GLFW_KEY_F2:
        return Rml::Input::KI_F2;
    case GLFW_KEY_F3:
        return Rml::Input::KI_F3;
    case GLFW_KEY_F4:
        return Rml::Input::KI_F4;
    case GLFW_KEY_F5:
        return Rml::Input::KI_F5;
    case GLFW_KEY_F6:
        return Rml::Input::KI_F6;
    case GLFW_KEY_F7:
        return Rml::Input::KI_F7;
    case GLFW_KEY_F8:
        return Rml::Input::KI_F8;
    case GLFW_KEY_F9:
        return Rml::Input::KI_F9;
    case GLFW_KEY_F10:
        return Rml::Input::KI_F10;
    case GLFW_KEY_F11:
        return Rml::Input::KI_F11;
    case GLFW_KEY_F12:
        return Rml::Input::KI_F12;
    case GLFW_KEY_F13:
        return Rml::Input::KI_F13;
    case GLFW_KEY_F14:
        return Rml::Input::KI_F14;
    case GLFW_KEY_F15:
        return Rml::Input::KI_F15;
    case GLFW_KEY_HOME:
        return Rml::Input::KI_HOME;
    case GLFW_KEY_INSERT:
        return Rml::Input::KI_INSERT;
    case GLFW_KEY_LEFT_CONTROL:
        return Rml::Input::KI_LCONTROL;
    case GLFW_KEY_LEFT_SHIFT:
        return Rml::Input::KI_LSHIFT;
    case GLFW_KEY_KP_MULTIPLY:
        return Rml::Input::KI_MULTIPLY;
    case GLFW_KEY_PAUSE:
        return Rml::Input::KI_PAUSE;
    case GLFW_KEY_RIGHT_CONTROL:
        return Rml::Input::KI_RCONTROL;
    case GLFW_KEY_KP_ENTER:
        return Rml::Input::KI_RETURN;
    case GLFW_KEY_RIGHT_SHIFT:
        return Rml::Input::KI_RSHIFT;
    case GLFW_KEY_SPACE:
        return Rml::Input::KI_SPACE;
    case GLFW_KEY_KP_SUBTRACT:
        return Rml::Input::KI_SUBTRACT;
    case GLFW_KEY_TAB:
        return Rml::Input::KI_TAB;
    default:
        return Rml::Input::KI_UNKNOWN;
    }
}

int cqsp::engine::GetAsciiKey(int glfw_key) { return 0; }
