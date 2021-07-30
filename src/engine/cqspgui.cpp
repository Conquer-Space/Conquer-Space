#include "cqspgui.h"

bool CQSPGui::DefaultButton(conquerspace::engine::Application& app,
                            const char* name, const ImVec2& size) {
    if (ImGui::Button(name, size)) {
        app.GetAudioInterface().PlayAudioClip("button.press");
        return true;
    }
    return false;
}

bool CQSPGui::SmallDefaultButton(conquerspace::engine::Application& app, const char* label) {
    if (ImGui::SmallButton(label)) {
        app.GetAudioInterface().PlayAudioClip("button.press");
        return true;
    }
    return false;
}

bool CQSPGui::DefaultSelectable(conquerspace::engine::Application& app, const char* label,
                                bool selected, ImGuiSelectableFlags flags, const ImVec2& size) {
    if (ImGui::Selectable(label, selected, flags, size)) {
        app.GetAudioInterface().PlayAudioClip("button.press");
        return true;
    }
    return false;
}

bool CQSPGui::DefaultSelectable(conquerspace::engine::Application& app, const char* label,
                            bool* p_selected, ImGuiSelectableFlags flags, const ImVec2& size) {
    if (ImGui::Selectable(label, p_selected, flags, size)) {
        app.GetAudioInterface().PlayAudioClip("button.press");
        return true;
    }
    return false;
}
