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
#include "turnsavewindow.h"

#include "client/components/clientctx.h"

namespace cqsp::client::systems::rmlui {
TurnSaveWindow::~TurnSaveWindow() {
    document->RemoveEventListener(Rml::EventId::Click, &listener);
    document->Close();
}

void TurnSaveWindow::Update(double delta_time) {
    auto& pause_opt = GetUniverse().ctx().at<client::ctx::PauseOptions>();

    const std::string date_text = fmt::format("{} {:02d}:{:02d}", GetUniverse().date.ToString(),
                                              GetUniverse().date.GetHour(), GetUniverse().date.GetMinute());
    time_element->SetInnerRML(date_text);
    static auto tick_speed = pause_opt.tick_speed;
    if (tick_speed != pause_opt.tick_speed) {
        speed_element->SetInnerRML(fmt::format("Speed: {}", pause_opt.tick_speed));
        tick_speed = pause_opt.tick_speed;
    }

    // Set the pause and play thingy if the game is halted or not
    if (pause_opt.to_tick != is_paused) {
        if (pause_opt.to_tick) {
            pause_element->SetAttribute("src", "pause-button.png");
        } else {
            pause_element->SetAttribute("src", "play-button.png");
        }
        is_paused = pause_opt.to_tick;
    }
}

void TurnSaveWindow::OpenDocument() {
    // Idk what
    document = GetApp().LoadDocument(file_name);
    document->Show();

    document->AddEventListener(Rml::EventId::Click, &listener);

    time_element = document->GetElementById("time");
    speed_element = document->GetElementById("speed");
    pause_element = document->GetElementById("pause_button");

    auto& pause_opt = GetUniverse().ctx().at<client::ctx::PauseOptions>();
    speed_element->SetInnerRML(fmt::format("Speed: {}", pause_opt.tick_speed));
}

void TurnSaveWindow::EventListener::ProcessEvent(Rml::Event& event) {
    // Clicked on button?
    auto& pause_opt = universe->ctx().at<client::ctx::PauseOptions>();
    std::string id_pressed = event.GetTargetElement()->GetId();

    if (id_pressed == "stop_time" || id_pressed == "pause_button") {
        pause_opt.to_tick = !pause_opt.to_tick;
    } else if (id_pressed == "slow_down") {
        if (pause_opt.tick_speed > 0) pause_opt.tick_speed--;
    } else if (id_pressed == "speed_up" || id_pressed == "fast_forward") {
        if (pause_opt.tick_speed < 6) {
            pause_opt.tick_speed++;
        }
    }
}
}  // namespace cqsp::client::systems::rmlui
