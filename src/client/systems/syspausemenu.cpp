/*
* Copyright 2021 Conquer Space
*/
#include "client/systems/syspausemenu.h"

#include "client/scenes/universescene.h"
#include "client/systems/sysoptionswindow.h"
#include "common/version.h"

void conquerspace::client::systems::SysPauseMenu::Init() {}

void conquerspace::client::systems::SysPauseMenu::DoUI(int delta_time) {
    if (!to_show) {
        return;
    }

    if (!to_show_options_window) {
        ImGui::SetNextWindowSize(ImVec2(200, -FLT_MIN), ImGuiCond_Always);
        ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f,
                                       ImGui::GetIO().DisplaySize.y * 0.5f),
                                ImGuiCond_Always, ImVec2(0.5f, 0.5f));

        ImGui::Begin("Pause menu", &to_show,
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDecoration);

        ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.5f, 0.5f));

        const float button_height = 0;
        if (ImGui::Button("Resume", ImVec2(-FLT_MIN, button_height))) {
            to_show = false;
            conquerspace::scene::SetGameHalted(false);
        }
        ImGui::Button("Save Game", ImVec2(-FLT_MIN, button_height));
        ImGui::Button("Load Game", ImVec2(-FLT_MIN, button_height));
        ImGui::Separator();

        if (ImGui::Button("Options", ImVec2(-FLT_MIN, button_height))) {
            to_show_options_window = true;
        }

        ImGui::Separator();
        ImGui::Button("Exit To Menu", ImVec2(-FLT_MIN, button_height));
        if (ImGui::Button("Exit Game", ImVec2(-FLT_MIN, button_height))) {
            // Kill game
            GetApp().ExitApplication();
        }
        ImGui::PopStyleVar();

        ImGui::Separator();
        ImGui::Text("Version: " CQSP_VERSION_STRING);

        ImGui::End();
    }

    if (to_show_options_window) {
        conquerspace::client::systems::ShowOptionsWindow(&to_show_options_window, GetApp());
    }
}

void conquerspace::client::systems::SysPauseMenu::DoUpdate(int delta_time) {
    if (GetApp().ButtonIsReleased(GLFW_KEY_ESCAPE)) {
        // Then pause
        to_show = !to_show;
        to_show_options_window = false;
        conquerspace::scene::SetGameHalted(to_show);
    }
}
