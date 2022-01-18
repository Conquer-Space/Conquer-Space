/* Conquer Space
* Copyright (C) 2021 Conquer Space
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
#include "client/systems/gui/sysevent.h"

#include <memory>
#include <string>

#include "common/components/player.h"
#include "engine/cqspgui.h"

using cqsp::client::systems::gui::SysEvent;
void SysEvent::Init() {
    GetApp().markdownConfig.tooltipCallback = [](ImGui::MarkdownTooltipCallbackData conf) {
        ImGui::BeginTooltip();
        ImGui::TextFmt(std::string(conf.linkData.link, conf.linkData.linkLength));
        ImGui::EndTooltip();
    };
}

void SysEvent::DoUI(float delta_time) {
    using cqsp::common::event::EventQueue;
    using cqsp::common::event::Event;
    auto events = GetApp().GetUniverse().view<cqsp::common::components::Player, EventQueue>();
    for (auto [ent, queue] : events.each()) {
        if (queue.events.empty()) {
            continue;
        }
        // Halt the game
        GetUniverse().DisableTick();
        std::shared_ptr<Event> env = queue.events.front();
        ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f,
                                       ImGui::GetIO().DisplaySize.y * 0.5f),
                                ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::Begin(env->title.c_str(), nullptr,
                     ImGuiWindowFlags_NoCollapse | window_flags |
                         ImGuiWindowFlags_NoScrollbar |
                         ImGuiWindowFlags_AlwaysAutoResize);

        asset::Texture* texture = GetAssetManager().GetAsset<asset::Texture>(env->image);
        float multiplier = 450.f / static_cast<float>(texture->width);
        // NOLINTNEXTLINE(performance-no-int-to-ptr)
        ImGui::Image(reinterpret_cast<void*>(texture->id), ImVec2(static_cast<float>(texture->width) * multiplier,
                     static_cast<float>(texture->height) * multiplier));
        ImGui::Separator();
        ImGui::BeginChild("eventchild", ImVec2(-FLT_MIN, 150), false,
                          window_flags);
        ImGui::Markdown(env->content.c_str(), env->content.length(), GetApp().markdownConfig);
        ImGui::EndChild();
        if (env->actions.empty()) {
            if (CQSPGui::DefaultButton("Ok", ImVec2(-FLT_MIN, 0))) {
                queue.events.clear();
            }
        } else {
            ProcessActionResult(env, queue);
        }
        ImGui::End();
    }
}


void SysEvent::DoUpdate(float delta_time) {}

void SysEvent::ProcessActionResult(const std::shared_ptr<cqsp::common::event::Event>& env,
                                   cqsp::common::event::EventQueue& queue) {
    int pressed = -1;
    int i = 0;
    for (auto& action_result : env->actions) {
        i++;
        if (CQSPGui::DefaultButton(action_result->name.c_str(), ImVec2(-FLT_MIN, 0))) {
            pressed = i;
            // Check if it has an event
            if (action_result->has_event) {
                sol::protected_function_result res =
                    action_result->action(env->table);
                GetApp().GetScriptInterface().ParseResult(res);
            }
        }
        if (ImGui::IsItemHovered() && !action_result->tooltip.empty()) {
            ImGui::BeginTooltip();
            ImGui::TextFmt(action_result->tooltip);
            ImGui::EndTooltip();
        }
    }
    if (pressed >= 0) {
        queue.events.clear();
    }
}
