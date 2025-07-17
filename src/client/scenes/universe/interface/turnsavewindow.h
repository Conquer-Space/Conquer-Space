/* Conquer Space
 * Copyright (C) 2021-2025 Conquer Space
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
#pragma once

#include <string>

#include "client/systems/sysgui.h"

namespace cqsp::client::systems::rmlui {
class TurnSaveWindow : public SysRmlUiInterface {
 public:
    explicit TurnSaveWindow(engine::Application& _app) : SysRmlUiInterface(_app) {}
    ~TurnSaveWindow();
    void Update(double delta_time) override;
    void OpenDocument() override;

 private:
    std::string file_name = "../data/core/gui/mainscene/turnsavewindow.rml";
    Rml::ElementDocument* document;

    class EventListener : public Rml::EventListener {
     public:
        explicit EventListener(common::Universe* universe) : universe(universe) {}
        void ProcessEvent(Rml::Event& event);
        common::Universe* universe;
    } listener {&GetUniverse()};

    bool is_paused;

    Rml::Element* time_element;
    Rml::Element* pause_element;
    Rml::Element* speed_element;
};
}  // namespace cqsp::client::systems::rmlui
