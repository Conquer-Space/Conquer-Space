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
        break;
    case GLFW_KEY_B:
        return Rml::Input::KI_B;
        break;
    case GLFW_KEY_C:
        return Rml::Input::KI_C;
        break;
    case GLFW_KEY_D:
        return Rml::Input::KI_D;
        break;
    case GLFW_KEY_E:
        return Rml::Input::KI_E;
        break;
    case GLFW_KEY_F:
        return Rml::Input::KI_F;
        break;
    case GLFW_KEY_G:
        return Rml::Input::KI_G;
        break;
    case GLFW_KEY_H:
        return Rml::Input::KI_H;
        break;
    case GLFW_KEY_I:
        return Rml::Input::KI_I;
        break;
    case GLFW_KEY_J:
        return Rml::Input::KI_J;
        break;
    case GLFW_KEY_K:
        return Rml::Input::KI_K;
        break;
    case GLFW_KEY_L:
        return Rml::Input::KI_L;
        break;
    case GLFW_KEY_M:
        return Rml::Input::KI_M;
        break;
    case GLFW_KEY_N:
        return Rml::Input::KI_N;
        break;
    case GLFW_KEY_O:
        return Rml::Input::KI_O;
        break;
    case GLFW_KEY_P:
        return Rml::Input::KI_P;
        break;
    case GLFW_KEY_Q:
        return Rml::Input::KI_Q;
        break;
    case GLFW_KEY_R:
        return Rml::Input::KI_R;
        break;
    case GLFW_KEY_S:
        return Rml::Input::KI_S;
        break;
    case GLFW_KEY_T:
        return Rml::Input::KI_T;
        break;
    case GLFW_KEY_U:
        return Rml::Input::KI_U;
        break;
    case GLFW_KEY_V:
        return Rml::Input::KI_V;
        break;
    case GLFW_KEY_W:
        return Rml::Input::KI_W;
        break;
    case GLFW_KEY_X:
        return Rml::Input::KI_X;
        break;
    case GLFW_KEY_Y:
        return Rml::Input::KI_Y;
        break;
    case GLFW_KEY_Z:
        return Rml::Input::KI_Z;
        break;
    case GLFW_KEY_0:
        return Rml::Input::KI_0;
        break;
    case GLFW_KEY_1:
        return Rml::Input::KI_1;
        break;
    case GLFW_KEY_2:
        return Rml::Input::KI_2;
        break;
    case GLFW_KEY_3:
        return Rml::Input::KI_3;
        break;
    case GLFW_KEY_4:
        return Rml::Input::KI_4;
        break;
    case GLFW_KEY_5:
        return Rml::Input::KI_5;
        break;
    case GLFW_KEY_6:
        return Rml::Input::KI_6;
        break;
    case GLFW_KEY_7:
        return Rml::Input::KI_7;
        break;
    case GLFW_KEY_8:
        return Rml::Input::KI_8;
        break;
    case GLFW_KEY_9:
        return Rml::Input::KI_9;
        break;
    case GLFW_KEY_KP_0:
        return Rml::Input::KI_NUMPAD0;
        break;
    case GLFW_KEY_KP_1:
        return Rml::Input::KI_NUMPAD1;
        break;
    case GLFW_KEY_KP_2:
        return Rml::Input::KI_NUMPAD2;
        break;
    case GLFW_KEY_KP_3:
        return Rml::Input::KI_NUMPAD3;
        break;
    case GLFW_KEY_KP_4:
        return Rml::Input::KI_NUMPAD4;
        break;
    case GLFW_KEY_KP_5:
        return Rml::Input::KI_NUMPAD5;
        break;
    case GLFW_KEY_KP_6:
        return Rml::Input::KI_NUMPAD6;
        break;
    case GLFW_KEY_KP_7:
        return Rml::Input::KI_NUMPAD7;
        break;
    case GLFW_KEY_KP_8:
        return Rml::Input::KI_NUMPAD8;
        break;
    case GLFW_KEY_KP_9:
        return Rml::Input::KI_NUMPAD9;
        break;
    case GLFW_KEY_LEFT:
        return Rml::Input::KI_LEFT;
        break;
    case GLFW_KEY_RIGHT:
        return Rml::Input::KI_RIGHT;
        break;
    case GLFW_KEY_UP:
        return Rml::Input::KI_UP;
        break;
    case GLFW_KEY_DOWN:
        return Rml::Input::KI_DOWN;
        break;
    case GLFW_KEY_KP_ADD:
        return Rml::Input::KI_ADD;
        break;
    case GLFW_KEY_BACKSPACE:
        return Rml::Input::KI_BACK;
        break;
    case GLFW_KEY_DELETE:
        return Rml::Input::KI_DELETE;
        break;
    case GLFW_KEY_KP_DIVIDE:
        return Rml::Input::KI_DIVIDE;
        break;
    case GLFW_KEY_END:
        return Rml::Input::KI_END;
        break;
    case GLFW_KEY_ESCAPE:
        return Rml::Input::KI_ESCAPE;
        break;
    case GLFW_KEY_F1:
        return Rml::Input::KI_F1;
        break;
    case GLFW_KEY_F2:
        return Rml::Input::KI_F2;
        break;
    case GLFW_KEY_F3:
        return Rml::Input::KI_F3;
        break;
    case GLFW_KEY_F4:
        return Rml::Input::KI_F4;
        break;
    case GLFW_KEY_F5:
        return Rml::Input::KI_F5;
        break;
    case GLFW_KEY_F6:
        return Rml::Input::KI_F6;
        break;
    case GLFW_KEY_F7:
        return Rml::Input::KI_F7;
        break;
    case GLFW_KEY_F8:
        return Rml::Input::KI_F8;
        break;
    case GLFW_KEY_F9:
        return Rml::Input::KI_F9;
        break;
    case GLFW_KEY_F10:
        return Rml::Input::KI_F10;
        break;
    case GLFW_KEY_F11:
        return Rml::Input::KI_F11;
        break;
    case GLFW_KEY_F12:
        return Rml::Input::KI_F12;
        break;
    case GLFW_KEY_F13:
        return Rml::Input::KI_F13;
        break;
    case GLFW_KEY_F14:
        return Rml::Input::KI_F14;
        break;
    case GLFW_KEY_F15:
        return Rml::Input::KI_F15;
        break;
    case GLFW_KEY_HOME:
        return Rml::Input::KI_HOME;
        break;
    case GLFW_KEY_INSERT:
        return Rml::Input::KI_INSERT;
        break;
    case GLFW_KEY_LEFT_CONTROL:
        return Rml::Input::KI_LCONTROL;
        break;
    case GLFW_KEY_LEFT_SHIFT:
        return Rml::Input::KI_LSHIFT;
        break;
    case GLFW_KEY_KP_MULTIPLY:
        return Rml::Input::KI_MULTIPLY;
        break;
    case GLFW_KEY_PAUSE:
        return Rml::Input::KI_PAUSE;
        break;
    case GLFW_KEY_RIGHT_CONTROL:
        return Rml::Input::KI_RCONTROL;
        break;
    case GLFW_KEY_KP_ENTER:
        return Rml::Input::KI_RETURN;
        break;
    case GLFW_KEY_RIGHT_SHIFT:
        return Rml::Input::KI_RSHIFT;
        break;
    case GLFW_KEY_SPACE:
        return Rml::Input::KI_SPACE;
        break;
    case GLFW_KEY_KP_SUBTRACT:
        return Rml::Input::KI_SUBTRACT;
        break;
    case GLFW_KEY_TAB:
        return Rml::Input::KI_TAB;
        break;
    default:
        break;
    }
    return Rml::Input::KI_UNKNOWN;
}

int cqsp::engine::GetAsciiKey(int glfw_key) { return 0; }
