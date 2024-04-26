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
#include "sysevent.h"

#include <memory>
#include <string>

#include "common/components/player.h"
#include "engine/cqspgui.h"

using cqsp::client::systems::gui::SysEvent;
using cqsp::common::event::Event;
using cqsp::common::event::EventQueue;

void SysEvent::Init() {
#if false
    GetApp().markdownConfig.tooltipCallback = []( \
    ImGui::MarkdownTooltipCallbackData conf) {
        ImGui::BeginTooltip();
        ImGui::Text(std::string(conf.linkData.link, conf.linkData.linkLength).c_str());
        ImGui::EndTooltip();
    };
#endif
}

void SysEvent::DoUI(int delta_time) {
    auto events = GetUniverse().view<cqsp::common::components::Player, EventQueue>();
    for (auto [ent, queue] : events.each()) {
        if (queue.events.empty()) {
            continue;
        }
        // Halt the game
        GetUniverse().DisableTick();
        std::shared_ptr<Event> env = queue.events.front();
        ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f),
                                ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::Begin(env->title.c_str(), NULL, ImGuiWindowFlags_NoCollapse | window_flags |
            ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_AlwaysAutoResize);

        asset::Texture* texture = GetAssetManager().GetAsset<asset::Texture>(env->image);
        float multiplier = 450.f / texture->width;
        ImGui::Image(reinterpret_cast<void*>(texture->id),  // NOLINT
                     ImVec2(texture->width * multiplier, texture->height * multiplier));
        ImGui::Separator();
        ImGui::BeginChild("eventchild", ImVec2(-FLT_MIN, 150), false, window_flags);
        ImGui::Text("%s", env->content.c_str());
        ImGui::EndChild();
        if (env->actions.empty()) {
            if (CQSPGui::DefaultButton("Ok", ImVec2(-FLT_MIN, 0))) {
                queue.events.clear();
            }
        } else {
            int pressed = -1;
            int i = 0;
            for (auto& action_result : env->actions) {
                i++;
                if (CQSPGui::DefaultButton(action_result->name.c_str(), ImVec2(-FLT_MIN, 0))) {
                    pressed = i;
                    // Check if it has an event
                    if (action_result->has_event) {
                        sol::protected_function_result res = action_result->action(env->table);
                        GetScriptInterface().ParseResult(res);
                    }
                }
                if (ImGui::IsItemHovered() && !action_result->tooltip.empty()) {
                    ImGui::BeginTooltip();
                    ImGui::Text("%s", action_result->tooltip.c_str());
                    ImGui::EndTooltip();
                }
            }
            if (pressed >= 0) {
                queue.events.clear();
            }
        }
        ImGui::End();
    }
}

void SysEvent::DoUpdate(int delta_time) {}

void SysEvent::FireEvent() {}
