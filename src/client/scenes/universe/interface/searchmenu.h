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

#include <RmlUi/Core/DataModelHandle.h>

#include <string>
#include <vector>

#include "client/systems/sysgui.h"

namespace cqsp::client::systems::rmlui {
class SearchMenu : public SysRmlUiInterface {
 public:
    explicit SearchMenu(engine::Application& _app) : SysRmlUiInterface(_app) {}
    ~SearchMenu();
    void Update(double delta_time) override;
    void OpenDocument() override;
    void ReloadWindow() override;
    void SetupDocument();

 private:
    std::string file_name = "../data/core/gui/mainscene/searchmenu.rml";

    struct SearchResult {
        std::string name;
        std::string entity_id;
    };

    std::vector<SearchResult> results;
    Rml::DataModelHandle handle;

    class SearchEventListener : public Rml::EventListener {
     public:
        explicit SearchEventListener(SearchMenu& menu) : menu(menu) {}
        void ProcessEvent(Rml::Event& event) override;
        SearchMenu& menu;
    } search_listener {*this};

    class ClickEventListener : public Rml::EventListener {
     public:
        explicit ClickEventListener(SearchMenu& menu) : menu(menu) {}
        void ProcessEvent(Rml::Event& event) override;
        SearchMenu& menu;
    } click_listener {*this};
};
}  // namespace cqsp::client::systems::rmlui
