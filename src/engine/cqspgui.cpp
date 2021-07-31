#include "cqspgui.h"

//namespace CQSPGui
conquerspace::engine::Application* AppContext = nullptr;
void CQSPGui::SetApplication(conquerspace::engine::Application* context) {
    AppContext = context;
}

bool CQSPGui::DefaultButton(const char* name, const ImVec2& size) {
    if (ImGui::Button(name, size)) {
        AppContext->GetAudioInterface().PlayAudioClip("button.press");
        return true;
    }
    return false;
}

bool CQSPGui::SmallDefaultButton(const char* label) {
    if (ImGui::SmallButton(label)) {
        AppContext->GetAudioInterface().PlayAudioClip("button.press");
        return true;
    }
    return false;
}

bool CQSPGui::DefaultSelectable(const char* label,
                                bool selected, ImGuiSelectableFlags flags, const ImVec2& size) {
    if (ImGui::Selectable(label, selected, flags, size)) {
        AppContext->GetAudioInterface().PlayAudioClip("button.press");
        return true;
    }
    return false;
}

bool CQSPGui::DefaultSelectable(const char* label,
                            bool* p_selected, ImGuiSelectableFlags flags, const ImVec2& size) {
    if (ImGui::Selectable(label, p_selected, flags, size)) {
        AppContext->GetAudioInterface().PlayAudioClip("button.press");
        return true;
    }
    return false;
}

bool CQSPGui::DefaultCheckbox(const char* label, bool* v) {
    if (ImGui::Checkbox(label, v)) {
        AppContext->GetAudioInterface().PlayAudioClip("button.press");
        return true;
    }
    return false;
}

bool CQSPGui::ArrowButton(const char* label, ImGuiDir dir) {
    if (ImGui::ArrowButton(label, dir)) {
        AppContext->GetAudioInterface().PlayAudioClip("button.back");
        return true;
    }
    return false;
}

