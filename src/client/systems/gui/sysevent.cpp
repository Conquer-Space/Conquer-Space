/*
 * Copyright 2021 Conquer Space
 */
#include "client/systems/gui/sysevent.h"

#include <memory>
#include <string>

#include "common/components/player.h"

void conquerspace::client::systems::gui::SysEvent::Init() {
    GetApp().markdownConfig.tooltipCallback = [](ImGui::MarkdownTooltipCallbackData conf) {
        ImGui::BeginTooltip();
        ImGui::Text(std::string(conf.linkData.link, conf.linkData.linkLength).c_str());
        ImGui::EndTooltip();
    };
}

void conquerspace::client::systems::gui::SysEvent::DoUI(int delta_time) {
    auto events =
        GetApp().GetUniverse().view<conquerspace::common::components::Player,
                                            conquerspace::common::event::EventQueue>();
    for (auto [ent, queue] : events.each()) {
        if (queue.events.empty()) {
            continue;
        }
        // Halt the game
        GetApp().GetUniverse().DisableTick();
        std::shared_ptr<conquerspace::common::event::Event> env = queue.events.front();
        ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f,
                                       ImGui::GetIO().DisplaySize.y * 0.5f),
                                ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::Begin(env->title.c_str(), NULL,
                     ImGuiWindowFlags_NoCollapse | window_flags |
                         ImGuiWindowFlags_NoScrollbar |
                         ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Image(reinterpret_cast<void*>(GetApp().GetAssetManager().
                                                        GetAsset<asset::Texture>(env->image)->id),
                                                        ImVec2(450, 150));
        ImGui::Separator();
        ImGui::BeginChild("eventchild", ImVec2(-FLT_MIN, 150), false,
                          window_flags);
        ImGui::Markdown(env->content.c_str(), env->content.length(),
                        GetApp().markdownConfig);
        ImGui::EndChild();
        if (env->actions.empty()) {
            if (ImGui::Button("Ok", ImVec2(-FLT_MIN, 0))) {
                queue.events.clear();
            }
        } else {
            int pressed = -1;
            int i = 0;
            for (auto& action_result : env->actions) {
                i++;
                if (ImGui::Button(action_result->name.c_str(), ImVec2(-FLT_MIN, 0))) {
                    pressed = i;
                    // Check if it has an event
                    if (action_result->has_event) {
                        sol::protected_function_result res = action_result->action(env->table);
                        GetApp().GetScriptInterface().ParseResult(res);
                    }
                }
                if (ImGui::IsItemHovered() && !action_result->tooltip.empty()) {
                    ImGui::BeginTooltip();
                    ImGui::Text(action_result->tooltip.c_str());
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

void conquerspace::client::systems::gui::SysEvent::DoUpdate(int delta_time) {}

void conquerspace::client::systems::gui::SysEvent::FireEvent() {}
