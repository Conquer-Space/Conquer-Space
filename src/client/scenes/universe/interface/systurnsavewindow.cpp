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
#include "systurnsavewindow.h"

#include "engine/cqspgui.h"
#include "engine/gui.h"
#include "glad/glad.h"

using cqsp::client::systems::SysTurnSaveWindow;

void SysTurnSaveWindow::Init() {}

void SysTurnSaveWindow::DoUI(int delta_time) {
    // Turn window
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x, 30), ImGuiCond_Always, ImVec2(1.f, 0.f));
    ImGui::SetNextWindowSize(ImVec2(170, 80), ImGuiCond_Always);
    bool to_show = true;
    ImGui::Begin(
        "TS window", &to_show,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | window_flags);
    // Show date
    ImGui::TextFmt("Date: {} {}:00", GetUniverse().date.ToString(), GetUniverse().date.GetDate() % 24);
    ImGui::TextFmt("Speed: {}", tick_speed);
    // Get time
    if (CQSPGui::DefaultButton("<<")) {
        // Slower
        if (tick_speed > 0) {
            tick_speed--;
        }
    }
    ImGui::SameLine();
    if (CQSPGui::DefaultButton(to_tick ? "Running" : "Paused")) {
        TogglePlayState();
    }
    ImGui::SameLine();
    if (CQSPGui::DefaultButton(">>")) {
        // Faster
        if (tick_speed < tick_speeds.size() - 1) {
            tick_speed++;
        }
    }
    ImGui::End();
}

void SysTurnSaveWindow::DoUpdate(int delta_time) {
    if (!ImGui::GetIO().WantCaptureKeyboard) {
        if (GetApp().ButtonIsReleased(engine::KeyInput::KEY_SPACE)) {
            TogglePlayState();
        }
    }
    // Update tick
    if (to_tick && GetApp().GetTime() - last_tick > static_cast<float>(tick_speeds[tick_speed]) / 1000.f) {
        GetUniverse().EnableTick();
        last_tick = GetApp().GetTime();
    }
}

void SysTurnSaveWindow::TogglePlayState() { to_tick = !to_tick; }
