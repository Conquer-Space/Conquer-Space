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
}
