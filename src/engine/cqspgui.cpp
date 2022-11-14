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
#include "engine/cqspgui.h"

cqsp::engine::Application* AppContext = nullptr;
namespace CQSPGui {
void SetApplication(cqsp::engine::Application* context) { AppContext = context; }

bool DefaultButton(const char* name, const ImVec2& size) {
    if (ImGui::Button(name, size)) {
        AppContext->GetAudioInterface().PlayAudioClip("button.press");
        return true;
    }
    return false;
}

bool SmallDefaultButton(const char* label) {
    if (ImGui::SmallButton(label)) {
        AppContext->GetAudioInterface().PlayAudioClip("button.press");
        return true;
    }
    return false;
}

bool DefaultSelectable(const char* label, bool selected, ImGuiSelectableFlags flags, const ImVec2& size) {
    if (ImGui::Selectable(label, selected, flags, size)) {
        AppContext->GetAudioInterface().PlayAudioClip("button.press");
        return true;
    }
    return false;
}

bool DefaultSelectable(const char* label, bool* p_selected, ImGuiSelectableFlags flags, const ImVec2& size) {
    if (ImGui::Selectable(label, p_selected, flags, size)) {
        AppContext->GetAudioInterface().PlayAudioClip("button.press");
        return true;
    }
    return false;
}

bool DefaultCheckbox(const char* label, bool* v) {
    if (ImGui::Checkbox(label, v)) {
        AppContext->GetAudioInterface().PlayAudioClip("button.press");
        return true;
    }
    return false;
}

bool ArrowButton(const char* label, ImGuiDir dir) {
    if (ImGui::ArrowButton(label, dir)) {
        AppContext->GetAudioInterface().PlayAudioClip("button.back");
        return true;
    }
    return false;
}

bool SliderFloat(const char* label, float* v, float v_min, float v_max, const char* format, ImGuiSliderFlags flags) {
    if (ImGui::SliderFloat(label, v, v_min, v_max, format, flags)) {
        AppContext->GetAudioInterface().PlayAudioClip("scroll.tick");
        return true;
    }
    return false;
}

bool SliderInt(const char* label, int* v, int v_min, int v_max, const char* format, ImGuiSliderFlags flags) {
    if (ImGui::SliderInt(label, v, v_min, v_max, format, flags)) {
        AppContext->GetAudioInterface().PlayAudioClip("scroll.tick");
        return true;
    }
    return false;
}

bool DragFloat(const char* label, float* v, float v_speed, float v_min, float v_max, const char* format,
               ImGuiSliderFlags flags) {
    if (ImGui::DragFloat(label, v, v_speed, v_min, v_max, format, flags)) {
        AppContext->GetAudioInterface().PlayAudioClip("scroll.tick");
        return true;
    }
    return false;
}

bool DragInt(const char* label, int* v, float v_speed, int v_min, int v_max, const char* format,
             ImGuiSliderFlags flags) {
    if (ImGui::DragInt(label, v, v_speed, v_min, v_max, format, flags)) {
        AppContext->GetAudioInterface().PlayAudioClip("scroll.tick");
        return true;
    }
    return false;
}
}  // namespace CQSPGui
