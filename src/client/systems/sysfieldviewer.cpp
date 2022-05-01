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
#include "client/systems/sysfieldviewer.h"

#include <map>
#include <tuple>

#include "common/components/science.h"
#include "common/components/name.h"
#include "client/systems/gui/systooltips.h"
#include "common/systems/science/fields.h"
#include "engine/cqspgui.h"

void cqsp::client::systems::SysFieldViewer::Init() {}

void cqsp::client::systems::SysFieldViewer::DoUI(int delta_time) {
    ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_Appearing);
    ImGui::Begin("Field Viewer");
    // Loop through fields
    const int height = 300;
    ImGui::BeginChild("Field List", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f -
                                 ImGui::GetStyle().ItemSpacing.y, height), true,
                      ImGuiWindowFlags_HorizontalScrollbar | window_flags);
    auto view = GetUniverse().view<common::components::science::Field>();
    for (entt::entity entity : view) {
        if (ImGui::Selectable(GetUniverse()
                                  .get<common::components::Name>(entity)
                                  .name.c_str(), selected_tech == entity)) {
            selected_tech = entity;
        }
    }
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("FieldInformation", ImVec2(-1, height), true,
                      ImGuiWindowFlags_HorizontalScrollbar | window_flags);
    FieldInformationWindow();
    ImGui::EndChild();
    ImGui::End();
}

void cqsp::client::systems::SysFieldViewer::DoUpdate(int delta_time) {}

void cqsp::client::systems::SysFieldViewer::FieldInformationWindow() {
    if (!GetUniverse().valid(selected_tech)) {
        return;
    }
    if (!GetUniverse().any_of<common::components::science::Field>(selected_tech)) {
        return;
    }
    auto& field = GetUniverse().get<common::components::science::Field>(selected_tech);
    auto& name = GetUniverse().get<common::components::Name>(selected_tech);
    ImGui::Text(name.name.c_str());
    if (GetUniverse().any_of<common::components::Description>(selected_tech)) {
        auto& desc = GetUniverse().get<common::components::Description>(selected_tech);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7, 0.7, 0.7, 1));
        ImGui::TextWrapped(desc.description.c_str());
        ImGui::PopStyleColor();
    }
    ImGui::Separator();
    ImGui::Text("Parents");
    for (auto& f : field.parents) {
        ImGui::Text(GetUniverse().get<common::components::Name>(f).name.c_str());
    }
    ImGui::Separator();
    ImGui::Text("Adjacents");

    for (auto& f : field.adjacent) {
        ImGui::Text(GetUniverse().get<common::components::Name>(f).name.c_str());
    }
}

void cqsp::client::systems::SysFieldNodeViewer::Init() {}

typedef std::map<entt::entity, std::tuple<int, int, int>> FieldNodeInformation;

entt::entity CalculateInputPair(const FieldNodeInformation& map, int pin_value) {
    entt::entity field = entt::null;
    int pv = (pin_value  - 1) / 4 * 4 + 2;
    for (auto it = map.begin(); it != map.end(); ++it) {
        if (std::get<0>(it->second) == pv) {
            field = it->first;
            break;
        }
    }
    return field;
}

bool VerifyFieldNode(int input_id, int output_id) {
    // 1 is child
    // 2 is adjacent
    // 3 is parent
    return (2 == input_id && input_id == output_id) ||
            (input_id == 1 && output_id == 3) ||
            (input_id == 3 && output_id == 1);
}

void AcceptNewItem(cqsp::common::Universe& universe, int input_type, entt::entity input_entity, entt::entity output_entity) {
    // Look for the link, then connect back
    // The initial pins should be in multiples of 4 because we make 4
    // Then connect the pins
    // Add to the input pin
    switch (input_type) {
    case 1: {
        // Then they want to be a child ofthe output entity
        auto& field = universe.get<cqsp::common::components::science::Field>(input_entity);
        if (std::find(field.parents.begin(), field.parents.end(),
                        output_entity) == field.parents.end()) {
            field.parents.push_back(output_entity);
        }
        break;
    }
    case 2: {
        auto& field = universe.get<cqsp::common::components::science::Field>(input_entity);
        if (std::find(field.adjacent.begin(), field.adjacent.end(),
                        output_entity) == field.adjacent.end()) {
            field.adjacent.push_back(output_entity);
        }
        break;
    }
    case 3: {
        // Then they want to be a child ofthe output entity
        auto& field = universe.get<cqsp::common::components::science::Field>(output_entity);
        if (std::find(field.parents.begin(), field.parents.end(),
                        input_entity) == field.parents.end()) {
            field.parents.push_back(input_entity);
        }
        break;
    }
    }
}

void CreateNewNode(cqsp::common::Universe& universe, FieldNodeInformation& map) {
    ed::PinId inputPinId, outputPinId;
    if (!ed::QueryNewLink(&inputPinId, &outputPinId)) {
        return;
    }
    int input_type = (inputPinId.Get() - 1) % 4;
    int output_type = (outputPinId.Get() - 1) % 4;
    if (inputPinId && outputPinId && VerifyFieldNode(input_type, output_type)) {
        entt::entity input_entity = CalculateInputPair(map, inputPinId.Get());
        entt::entity output_entity = CalculateInputPair(map, outputPinId.Get());
        if (input_entity != entt::null && output_entity != entt::null && ed::AcceptNewItem()) {
            AcceptNewItem(universe, input_type, input_entity, output_entity);
        }

        // You may choose to reject connection between these nodes
        // by calling ed::RejectNewItem(). This will allow editor to give
        // visual feedback by changing link thickness and color.
    } else {
        ed::RejectNewItem();
    }
}

void RemoveRelationship(cqsp::common::Universe& universe, int input_type, entt::entity input_entity, entt::entity output_entity) {
    switch (input_type) {
    case 1: {
        // Then they want to be a child ofthe output entity
        auto& field = universe.get<cqsp::common::components::science::Field>(input_entity);
        if (std::find(field.parents.begin(), field.parents.end(),
                        output_entity) != field.parents.end()) {
            field.parents.erase(std::remove(field.parents.begin(), field.parents.end(), output_entity), field.parents.end());
        }
        break;
    }
    case 2: {
        auto& field = universe.get<cqsp::common::components::science::Field>(input_entity);
        if (std::find(field.adjacent.begin(), field.adjacent.end(),
                        output_entity) != field.adjacent.end()) {
            field.adjacent.erase(std::remove(field.adjacent.begin(), field.adjacent.end(), output_entity), field.adjacent.end());
        }
        break;
    }
    case 3: {
        // Then they want to be a child ofthe output entity
        auto& field = universe.get<cqsp::common::components::science::Field>(output_entity);
        if (std::find(field.parents.begin(), field.parents.end(),
                        input_entity) != field.parents.end()) {
            field.parents.erase(std::remove(field.parents.begin(), field.parents.end(), input_entity), field.parents.end());
        }
        break;
    }
    }
}

void HandleDeleteRelationship(FieldNodeInformation& map, cqsp::common::Universe& universe) {
    ed::LinkId linkId = 0;
    while (ed::QueryDeletedLink(&linkId)) {
        if (!ed::AcceptDeletedItem()) { // We ignore deleted nodes for now, this is just to manage the relationships
            continue;
        }
        // Get the entity and delete
        ed::PinId input_pin;
        ed::PinId output_pin;
        if (!ed::GetLinkPins(linkId, &input_pin, &output_pin)) {
            continue;
        }
        // Then set the values
        entt::entity input_entity = CalculateInputPair(map, input_pin.Get());
        entt::entity output_entity = CalculateInputPair(map, output_pin.Get());
        // Remove the relationship
        int input_type = (input_pin.Get() - 1) % 4;
        RemoveRelationship(universe, input_type, input_entity, output_entity);
    }
}

void cqsp::client::systems::SysFieldNodeViewer::DoUI(int delta_time) {
    // View Fields
    ed::Begin("Field Viewer");
    ImGui::SetWindowSize(ImVec2(1000, 900), ImGuiCond_Appearing);
    int uniqueId = 1;
    FieldNodeInformation map;

    auto fields = GetUniverse().view<common::components::science::Field>();
    // Start drawing nodes
    for (const entt::entity& entity : fields) {
        ed::BeginNode(uniqueId++);
        ImGui::Text(gui::GetName(GetUniverse(), entity).c_str());
        ImGui::SetNextItemWidth(200);
        ImGui::InputText(fmt::format("##ne_identifier{}", entity).c_str(),
                         &(GetUniverse().get<common::components::Identifier>(entity).identifier));
        if (GetUniverse().all_of<common::components::Description>(entity)) {
            // Description text
            ImGui::SetNextItemWidth(200);
            std::string& description =GetUniverse().get<common::components::Description>(entity).description;
            ImGui::InputText(fmt::format("##ne_description{}", entity).c_str(),
                         &description);
            CQSPGui::SimpleTextTooltip(description);
        } else {
            if (ImGui::Button("+ Add Description")) {
                GetUniverse().emplace<common::components::Description>(entity);
            }
        }
        int a = uniqueId++;
        ed::BeginPin(a, ed::PinKind::Input);
        ax::Drawing::Icon(ImVec2(16, 16), ax::Drawing::IconType::Circle, true,
                    ImColor(0, 0, 255, 255), ImColor(32, 32, 32, 255));
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1, 1, 1, 0.8), "Child of");
        ed::EndPin();
        int b = uniqueId++;
        ed::BeginPin(b, ed::PinKind::Output);
        ax::Drawing::Icon(ImVec2(16, 16), ax::Drawing::IconType::Diamond, true,
                    ImColor(255, 0, 0, 255), ImColor(32, 32, 32, 255));
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1, 1, 1, 0.8), "Adjacent");

        ed::EndPin();

        int c = uniqueId++;
        ImGui::SameLine();
        ed::BeginPin(c, ed::PinKind::Output);
        ImGui::TextColored(ImVec4(1, 1, 1, 0.8), "Parent of");
        ImGui::SameLine();
                ax::Drawing::Icon(ImVec2(16, 16), ax::Drawing::IconType::Circle, true,
                          ImColor(0, 0, 255, 255), ImColor(32, 32, 32, 255));
        ed::EndPin();
        map[entity] = std::make_tuple(a, b, c);
        ed::EndNode();
    }

    // Draw more nodes
    for (const entt::entity& entity : fields) {
        auto& field = GetUniverse().get<common::components::science::Field>(entity);
        const auto& current_tup = map[entity];
        for (const auto& parent : field.parents) {
            const auto& other_tup = map[parent];
            ed::Link(uniqueId++, std::get<0>(current_tup), std::get<2>(other_tup));
        }

        for (const auto& parent : field.adjacent) {
            const auto& other_tup = map[parent];
            ed::Link(uniqueId++, std::get<1>(current_tup), std::get<1>(other_tup), ImVec4(1, 0, 0, 1));
        }
    }

    if (ed::BeginCreate()) {
        CreateNewNode(GetUniverse(), map);
    }
    ed::EndCreate();

    if (ed::BeginDelete()) {
        HandleDeleteRelationship(map, GetUniverse());
    }
    ed::EndDelete();

    ed::End();

    ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiCond_Appearing);
    ImGui::Begin("Field Hjson viewer");
    if (ImGui::Button("Make Fields to Hjson")) {
        // Make the hjson
        auto fields = common::systems::science::WriteFields(GetUniverse());
        Hjson::EncoderOptions eo;
        eo.indentBy = "    "; // 4 spaces
        hjson_content = Hjson::Marshal(fields, eo);
    }
    ImGui::SameLine();
    ImGui::Button("Save to file");
    ImGui::InputTextMultiline("field_hjson_viewer", &hjson_content, ImVec2(-1, -1));
    ImGui::End();
}

void cqsp::client::systems::SysFieldNodeViewer::DoUpdate(int delta_time) {}
