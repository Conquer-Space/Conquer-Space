#pragma once

#include "engine/application.h"
#include "engine/gui.h"

/// <summary>
/// Gui with sound
/// </summary>
namespace CQSPGui {
void SetApplication(conquerspace::engine::Application*);
bool DefaultButton(const char* name,
                   const ImVec2& size = ImVec2(0, 0));
bool SmallDefaultButton(const char* label);
bool DefaultSelectable(const char* label,
        bool selected = false, ImGuiSelectableFlags flags = 0, const ImVec2& size = ImVec2(0, 0));
bool DefaultSelectable(const char* label,
        bool* p_selected, ImGuiSelectableFlags flags = 0, const ImVec2& size = ImVec2(0, 0));
bool DefaultCheckbox(const char* label, bool* v);
bool ArrowButton(const char* label, ImGuiDir dir);
bool SliderFloat(const char* label, float* v, float v_min, float v_max, const char* format = "%.3f", ImGuiSliderFlags flags = 0);
bool SliderInt(const char* label, int* v, int v_min, int v_max, const char* format = "%d", ImGuiSliderFlags flags = 0);
}
