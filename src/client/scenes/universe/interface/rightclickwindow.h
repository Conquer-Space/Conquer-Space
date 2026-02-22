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
#include <RmlUi/Core/Event.h>
#include <RmlUi/Core/Types.h>

#include <string>

#include "client/components/clientctx.h"
#include "client/systems/sysgui.h"

namespace cqsp::client::systems::rmlui {
class RightClickWindow : public SysRmlUiInterface {
 public:
    explicit RightClickWindow(engine::Application& _app) : SysRmlUiInterface(_app) {}
    ~RightClickWindow();
    void Update(double delta_time) override;
    void OpenDocument() override;
    void ReloadWindow() override;
    void SetupContent();

 private:
    void SetupDataModels();
    class EventListener : public Rml::EventListener {
     public:
        explicit EventListener(core::Universe& universe) : universe(universe) {}
        void ProcessEvent(Rml::Event& event);
        core::Universe& universe;
    } listener {GetUniverse()};

    std::string file_name = "../data/core/gui/mainscene/rightclickwindow.rml";
    double itemX;
    double itemY;
    Rml::Element* right_click_content = nullptr;
    Rml::Element* header_element;
    entt::entity right_click_item = entt::null;
    Rml::DataModelHandle handle;
    bool to_right_click = false;

    struct RightClickMenuItem {
        std::string name;
        std::string action;
    };
    void DetermineButtons(entt::entity entity);

    class ClickEventListener : public Rml::EventListener {
     public:
        explicit ClickEventListener(core::Universe& universe, RightClickWindow& window)
            : universe(universe), window(window) {}
        void ProcessEvent(Rml::Event& event);
        core::Universe& universe;
        RightClickWindow& window;
    } right_click_listener {GetUniverse(), *this};

    std::vector<RightClickMenuItem> buttons;

    void TestFunction(Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&);
};
}  // namespace cqsp::client::systems::rmlui
