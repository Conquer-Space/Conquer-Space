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

#include <utility>

#include "engine/application.h"
#include "engine/gui.h"

/// <summary>
/// Gui with sound
/// </summary>
namespace CQSPGui {
void SetApplication(cqsp::engine::Application*);
bool DefaultButton(const char* name, const ImVec2& size = ImVec2(0, 0));
bool SmallDefaultButton(const char* label);
bool DefaultSelectable(const char* label, bool selected = false, ImGuiSelectableFlags flags = 0,
                       const ImVec2& size = ImVec2(0, 0));
bool DefaultSelectable(const char* label, bool* p_selected, ImGuiSelectableFlags flags = 0,
                       const ImVec2& size = ImVec2(0, 0));
bool DefaultCheckbox(const char* label, bool* v);
bool ArrowButton(const char* label, ImGuiDir dir);
bool SliderFloat(const char* label, float* v, float v_min, float v_max, const char* format = "%.3f",
                 ImGuiSliderFlags flags = 0);
bool SliderInt(const char* label, int* v, int v_min, int v_max, const char* format = "%d", ImGuiSliderFlags flags = 0);
bool DragFloat(const char* label, float* v, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f,
               const char* format = "%.3f", ImGuiSliderFlags flags = 0);
bool DragInt(const char* label, int* v, float v_speed = 1.0f, int v_min = 0, int v_max = 0, const char* format = "%d",
             ImGuiSliderFlags flags = 0);

template <typename... Args>
void SimpleTextTooltip(fmt::format_string<Args...> fmt, Args&&... args) {
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::Text("%s", fmt::format(fmt, std::forward<Args>(args)...).c_str());
        ImGui::EndTooltip();
    }
}
}  // namespace CQSPGui
