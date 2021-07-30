#pragma once

#include "engine/application.h"
#include "engine/gui.h"

namespace CQSPGui {
bool DefaultButton(conquerspace::engine::Application& app, const char* name,
                   const ImVec2& size = ImVec2(0, 0));
bool SmallDefaultButton(conquerspace::engine::Application& app, const char* label);
bool DefaultSelectable(conquerspace::engine::Application& app, const char* label,
        bool selected = false, ImGuiSelectableFlags flags = 0, const ImVec2& size = ImVec2(0, 0));
bool DefaultSelectable(conquerspace::engine::Application& app, const char* label,
        bool* p_selected, ImGuiSelectableFlags flags = 0, const ImVec2& size = ImVec2(0, 0));
}
