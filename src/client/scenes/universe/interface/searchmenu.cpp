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
#include <RmlUi/Core/Input.h>

#include <algorithm>
#include <cctype>
#include <string>

#include "client/components/provincecentering.h"
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
    if (results_el != nullptr) {
        results_el->RemoveEventListener(Rml::EventId::Click, &click_listener);
    }

    Rml::Element* main_window = document->GetFirstChild();
    if (main_window != nullptr) {
        main_window->RemoveEventListener(Rml::EventId::Keydown, &arrow_key_listener, true);
    }
}

void SearchMenu::Update(double delta_time) {
    bool ctrl = GetApp().ButtonIsHeld(engine::KeyInput::KEY_LEFT_CONTROL);
    bool p = GetApp().ButtonIsReleased(engine::KeyInput::KEY_P);
    if (ctrl && p) {
        if (document->IsVisible()) {
            document->Hide();
        } else {
            document->Show();
            Rml::Element* input = document->GetElementById("search_input");
            if (input != nullptr) input->Focus();
        }
    }
    if (GetApp().ButtonIsReleased(engine::KeyInput::KEY_ESCAPE)) {
        document->Hide();
    }

    if (document->IsVisible()) {
        document->PullToFront();
        Rml::Element* input = document->GetElementById("search_input");
        if (input != nullptr) input->Focus();
    }
    bool left_clicked = GetApp().MouseButtonIsPressed(engine::MouseInput::LEFT);
    bool right_clicked = GetApp().MouseButtonIsPressed(engine::MouseInput::RIGHT);
    bool center_clicked = GetApp().MouseButtonIsPressed(engine::MouseInput::MIDDLE);
    bool any_clicked = left_clicked || right_clicked || center_clicked;
    Rml::Element* element = Rml::GetContext(0)->GetFocusElement();
    if (any_clicked &&
        (!GetApp().RmlUiMouseProcessed() || (element != nullptr && element->GetOwnerDocument() != document))) {
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
    if (input != nullptr) {
        input->AddEventListener(Rml::EventId::Change, &search_listener);
    }
    Rml::Element* results_el = document->GetElementById("results");
    if (results_el != nullptr) {
        results_el->AddEventListener(Rml::EventId::Click, &click_listener);
    }

    Rml::Element* main_window = document->GetFirstChild();
    if (main_window != nullptr) {
        main_window->AddEventListener(Rml::EventId::Keydown, &arrow_key_listener, true);
    }
}

void SearchMenu::AddResult(const std::string& query, entt::entity entity, const std::string& name) {
    std::string lower_name = name;
    std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    if (lower_name.find(query) != std::string::npos) {
        results.push_back({name, std::to_string(static_cast<uint32_t>(entity))});
    }
}

void SearchMenu::SearchEventListener::ProcessEvent(Rml::Event& event) {
    auto* input = static_cast<Rml::ElementFormControlInput*>(event.GetTargetElement());
    std::string query = input->GetValue();

    menu.results.clear();
    menu.selected_index = -1;

    if (!query.empty()) {
        std::string lower_query = query;
        std::transform(lower_query.begin(), lower_query.end(), lower_query.begin(),
                       [](unsigned char c) { return std::tolower(c); });

        auto& universe = menu.GetUniverse();

        for (auto&& [entity, body, name] :
             universe.view<core::components::bodies::Body, core::components::Name>().each()) {
            menu.AddResult(lower_query, entity, name.name);
        }

        for (auto&& [entity, province, name] :
             universe.view<core::components::Province, core::components::Name>().each()) {
            menu.AddResult(lower_query, entity, name.name);
        }
    }

    menu.handle.DirtyAllVariables();
}

void SearchMenu::ClickEventListener::ProcessEvent(Rml::Event& event) {
    const Rml::Variant* attr = event.GetTargetElement()->GetAttribute("data-entity");
    if (attr == nullptr || attr->GetType() != Rml::Variant::STRING) return;

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
    // Clear input
    Rml::Element* input = menu.document->GetElementById("search_input");
    if (input != nullptr) {
        static_cast<Rml::ElementFormControlInput*>(input)->SetValue("");
    }
    menu.document->Hide();
}

void SearchMenu::KeyboardEventListener::ProcessEvent(Rml::Event& event) {
    auto key = static_cast<Rml::Input::KeyIdentifier>(event.GetParameter<int>("key_identifier", 0));
    int count = static_cast<int>(menu.results.size());
    if (count == 0) return;

    if (key == Rml::Input::KI_DOWN) {
        menu.selected_index = std::min(menu.selected_index + 1, count - 1);
        menu.UpdateSelection();
        event.StopPropagation();
    } else if (key == Rml::Input::KI_UP) {
        menu.selected_index = std::max(menu.selected_index - 1, 0);
        menu.UpdateSelection();
        event.StopPropagation();
    } else if (key == Rml::Input::KI_PRIOR) {
        // Page up
        menu.selected_index = std::max(menu.selected_index - 10, 0);
        menu.UpdateSelection();
        event.StopPropagation();
    } else if (key == Rml::Input::KI_NEXT) {
        // Page down
        menu.selected_index = std::min(menu.selected_index + 10, count - 1);
        menu.UpdateSelection();
        event.StopPropagation();
    } else if (key == Rml::Input::KI_RETURN && menu.selected_index >= 0) {
        uint32_t id = std::stoul(menu.results[menu.selected_index].entity_id);
        entt::entity entity = static_cast<entt::entity>(id);
        auto& universe = menu.GetUniverse();
        if (universe.all_of<core::components::bodies::Body>(entity)) {
            scene::SeePlanet(universe, entity);
        } else if (universe.all_of<core::components::Province>(entity)) {
            ctx::ZoomOntoProvince(universe, entity);
        }
        Rml::Element* input = menu.document->GetElementById("search_input");
        if (input != nullptr) {
            static_cast<Rml::ElementFormControlInput*>(input)->SetValue("");
        }
        menu.document->Hide();
    }
}

void SearchMenu::UpdateSelection() {
    Rml::Element* results_el = document->GetElementById("results");
    if (results_el == nullptr) return;
    for (int i = 0; i < results_el->GetNumChildren(); i++) {
        Rml::Element* row = results_el->GetChild(i);
        if (row == nullptr) continue;
        Rml::Element* link = row->GetFirstChild();
        if (link == nullptr) continue;
        link->SetClass("selected_result", i == selected_index);
        if (i == selected_index) {
            float item_top = row->GetOffsetTop();
            float item_bottom = item_top + row->GetOffsetHeight();
            float scroll_top = results_el->GetScrollTop();
            float visible_bottom = scroll_top + results_el->GetClientHeight();

            if (item_top < scroll_top) {
                results_el->SetScrollTop(item_top);
            } else if (item_bottom > visible_bottom) {
                results_el->SetScrollTop(item_bottom - results_el->GetClientHeight());
            }
        }
    }
}
}  // namespace cqsp::client::systems::rmlui
