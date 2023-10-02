/* Conquer Space
 * Copyright (C) 2021-2023 Conquer Space
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
#include "syspausemenu.h"

#include <fstream>
#include <string>

#include "GLFW/glfw3.h"
#include "client/scenes/universe/sysoptionswindow.h"
#include "client/scenes/universe/universescene.h"
#include "client/systems/savegame.h"
#include "common/util/paths.h"
#include "common/version.h"
#include "engine/cqspgui.h"

void cqsp::client::systems::SysPauseMenu::Init() {}

void cqsp::client::systems::SysPauseMenu::DoUI(int delta_time) {
    if (!to_show) {
        return;
    }

    if (!to_show_options_window) {
        ImGui::SetNextWindowSize(ImVec2(200, -FLT_MIN), ImGuiCond_Always);
        ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f),
                                ImGuiCond_Always, ImVec2(0.5f, 0.5f));

        ImGui::Begin("Pause menu", &to_show, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDecoration);

        ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.5f, 0.5f));

        const float button_height = 0;
        if (CQSPGui::DefaultButton("Resume", ImVec2(-FLT_MIN, button_height))) {
            to_show = false;
            cqsp::scene::SetGameHalted(false);
        }
        if (CQSPGui::DefaultButton("Save Game", ImVec2(-FLT_MIN, button_height))) {
            client::save::save_game(GetUniverse());
        }
        CQSPGui::DefaultButton("Load Game", ImVec2(-FLT_MIN, button_height));
        ImGui::Separator();

        if (CQSPGui::DefaultButton("Options", ImVec2(-FLT_MIN, button_height))) {
            to_show_options_window = true;
        }

        ImGui::Separator();
        CQSPGui::DefaultButton("Exit To Menu", ImVec2(-FLT_MIN, button_height));
        if (CQSPGui::DefaultButton("Exit Game", ImVec2(-FLT_MIN, button_height))) {
            // Kill game
            GetApp().ExitApplication();
        }
        ImGui::PopStyleVar();

        ImGui::Separator();
        ImGui::Text("Version: " CQSP_VERSION_STRING);

        ImGui::End();
    }

    if (to_show_options_window) {
        cqsp::client::systems::ShowOptionsWindow(&to_show_options_window, GetApp());
    }
}

void cqsp::client::systems::SysPauseMenu::DoUpdate(int delta_time) {
    if (GetApp().ButtonIsReleased(cqsp::engine::KEY_ESCAPE)) {
        // Then pause
        to_show = !to_show;
        to_show_options_window = false;
        cqsp::scene::SetGameHalted(to_show);
    }
}
