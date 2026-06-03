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
#include "client/scenes/universe/interface/searchmenu.h"

#include <RmlUi/Core/Elements/ElementFormControlInput.h>

#include <algorithm>
#include <cctype>
#include <string>

#include "client/scenes/universe/universescene.h"
#include "core/components/bodies.h"
#include "core/components/name.h"
#include "core/components/surface.h"
#include "engine/userinput.h"

namespace cqsp::client::systems::rmlui {

SearchMenu::~SearchMenu() {
    Rml::Element* element = document->GetElementById("search_input");
    element->RemoveEventListener(Rml::EventId::Change, &search_listener);
    Rml::Element* results_el = document->GetElementById("results");
    if (results_el) {
        results_el->RemoveEventListener(Rml::EventId::Click, &click_listener);
    }
}

void SearchMenu::Update(double delta_time) {
    bool ctrl = GetApp().ButtonIsHeld(engine::KeyInput::KEY_LEFT_CONTROL);
    bool p = GetApp().ButtonIsReleased(engine::KeyInput::KEY_P);
    if (ctrl && p) {
        if (document->IsVisible()) {
            document->Hide();
        } else {
            document->Show(Rml::ModalFlag::Modal);
            Rml::Element* input = document->GetElementById("search_input");
            if (input) input->Focus();
        }
    }
    if (GetApp().ButtonIsReleased(engine::KeyInput::KEY_ESCAPE)) {
        document->Hide();
    }

    if (document->IsVisible()) {
        document->PullToFront();
    }
    bool left_clicked = GetApp().MouseButtonIsPressed(engine::MouseInput::LEFT);
    bool right_clicked = GetApp().MouseButtonIsPressed(engine::MouseInput::RIGHT);
    bool center_clicked = GetApp().MouseButtonIsPressed(engine::MouseInput::MIDDLE);
    bool any_clicked = left_clicked || right_clicked || center_clicked;
    Rml::Element* element = Rml::GetContext(0)->GetFocusElement();
    if (any_clicked && (!GetApp().RmlUiMouseProcessed() || (element && element->GetOwnerDocument() != document))) {
        // then we should unfocus?
        document->Hide();
    }
}

void SearchMenu::OpenDocument() {
    Rml::DataModelConstructor ctor = GetApp().GetRmlUiContext()->CreateDataModel("search_results");
    if (auto s = ctor.RegisterStruct<SearchResult>()) {
        s.RegisterMember("name", &SearchResult::name);
        s.RegisterMember("entity_id", &SearchResult::entity_id);
    }
    ctor.RegisterArray<std::vector<SearchResult>>();
    ctor.Bind("result_list", &results);
    handle = ctor.GetModelHandle();

    document = GetApp().LoadDocument(file_name);
    SetupDocument();
    document->Hide();
}

void SearchMenu::ReloadWindow() {
    document = GetApp().ReloadDocument(file_name);
    SetupDocument();
    document->Hide();
}

void SearchMenu::SetupDocument() {
    Rml::Element* input = document->GetElementById("search_input");
    if (input) {
        input->AddEventListener(Rml::EventId::Change, &search_listener);
    }
    Rml::Element* results_el = document->GetElementById("results");
    if (results_el) {
        results_el->AddEventListener(Rml::EventId::Click, &click_listener);
    }
    Rml::Element* main_window = document->GetFirstChild();
    //if (main_window) {
    //    main_window->AddEventListener(Rml::EventId::Keydown, &click_listener);
    //    main_window->AddEventListener(Rml::EventId::Keyup, &click_listener);
    //}
}

void SearchMenu::SearchEventListener::ProcessEvent(Rml::Event& event) {
    auto* input = static_cast<Rml::ElementFormControlInput*>(event.GetTargetElement());
    std::string query = input->GetValue();

    menu.results.clear();

    if (!query.empty()) {
        std::string lower_query = query;
        std::transform(lower_query.begin(), lower_query.end(), lower_query.begin(),
                       [](unsigned char c) { return std::tolower(c); });

        auto& universe = menu.GetUniverse();
        for (auto&& [entity, province, name] :
             universe.view<core::components::Province, core::components::Name>().each()) {
            std::string lower_name = name.name;
            std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(),
                           [](unsigned char c) { return std::tolower(c); });
            if (lower_name.find(lower_query) != std::string::npos) {
                menu.results.push_back({name.name, std::to_string(static_cast<uint32_t>(entity))});
                if (menu.results.size() >= 10) break;
            }
        }
    }

    menu.handle.DirtyAllVariables();
}

void SearchMenu::ClickEventListener::ProcessEvent(Rml::Event& event) {
    const Rml::Variant* attr = event.GetTargetElement()->GetAttribute("data-entity");
    if (!attr || attr->GetType() != Rml::Variant::STRING) return;

    uint32_t id = std::stoul(attr->Get<std::string>());
    entt::entity entity = static_cast<entt::entity>(id);

    auto& universe = menu.GetUniverse();
    if (universe.all_of<core::components::bodies::Body>(entity)) {
        scene::SeePlanet(universe, entity);
    } else if (universe.all_of<core::components::Province>(entity)) {
        auto& province = universe.get<core::components::Province>(entity);
        if (province.planet != entt::null) {
            scene::SeePlanet(universe, province.planet);
        }
    }
    menu.document->Hide();
}
}  // namespace cqsp::client::systems::rmlui
