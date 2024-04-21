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
#include "sysfieldviewer.h"

#include <filesystem>
#include <map>
#include <string>
#include <tuple>
#include <vector>

#include "client/scenes/universe/interface/systooltips.h"
#include "common/components/name.h"
#include "common/components/science.h"
#include "common/systems/science/fields.h"
#include "common/util/paths.h"
#include "engine/cqspgui.h"

using cqsp::common::Universe;
using cqsp::common::components::Description;
using cqsp::common::components::Identifier;
using cqsp::common::components::Name;
using cqsp::common::components::science::Field;
namespace components = cqsp::common::components;
namespace science = cqsp::common::components::science;
using entt::entity;

namespace cqsp::client::systems {
void SysFieldViewer::Init() {}

void SysFieldViewer::DoUI(int delta_time) {
    ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_Appearing);
    ImGui::Begin("Field Viewer");
    // Loop through fields
    const int height = 300;
    ImGui::BeginChild("Field List",
                      ImVec2(ImGui::GetContentRegionAvail().x * 0.5f - ImGui::GetStyle().ItemSpacing.y, height), true,
                      ImGuiWindowFlags_HorizontalScrollbar | window_flags);
    auto view = GetUniverse().view<science::Field>();
    for (entity entity : view) {
        if (ImGui::Selectable(GetUniverse().get<components::Name>(entity).name.c_str(),
                              selected_tech == entity)) {
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

void SysFieldViewer::DoUpdate(int delta_time) {}

void SysFieldViewer::FieldInformationWindow() {
    if (!GetUniverse().valid(selected_tech)) {
        return;
    }
    if (!GetUniverse().any_of<science::Field>(selected_tech)) {
        return;
    }
    auto& field = GetUniverse().get<science::Field>(selected_tech);
    auto& name = GetUniverse().get<components::Name>(selected_tech);
    ImGui::Text("%s", name.name.c_str());
    if (GetUniverse().any_of<components::Description>(selected_tech)) {
        auto& desc = GetUniverse().get<components::Description>(selected_tech);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7, 0.7, 0.7, 1));
        ImGui::TextWrapped("%s", desc.description.c_str());
        ImGui::PopStyleColor();
    }
    ImGui::Separator();
    ImGui::Text("Parents");
    for (auto& f : field.parents) {
        ImGui::Text("%s", GetUniverse().get<components::Name>(f).name.c_str());
    }
    ImGui::Separator();
    ImGui::Text("Adjacents");

    for (auto& f : field.adjacent) {
        ImGui::Text("%s", GetUniverse().get<components::Name>(f).name.c_str());
    }
}

void SysFieldNodeViewer::Init() {}

namespace {
using FieldNodeInformation = std::map<entity, std::tuple<int, int, int>>;

    entity CalculateInputPair(const FieldNodeInformation& map, int pin_value) {
    entity field = entt::null;
    int pv = (pin_value - 1) / 4 * 4 + 2;
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
    return (2 == input_id && input_id == output_id) || (input_id == 1 && output_id == 3) ||
           (input_id == 3 && output_id == 1);
}

void RemoveFieldConnection(std::vector<entity>& vec, entity ent) {
    if (std::find(vec.begin(), vec.end(), ent) != vec.end()) {
        vec.erase(std::remove(vec.begin(), vec.end(), ent), vec.end());
    }
}

void AcceptNewItem(Universe& universe, int input_type, entity input_entity,
                   entity output_entity) {
    // Look for the link, then connect back
    // The initial pins should be in multiples of 4 because we make 4
    // Then connect the pins
    // Add to the input pin
    switch (input_type) {
        case 1: {
            // Then they want to be a child ofthe output entity
            auto& field = universe.get<science::Field>(input_entity);
            if (std::find(field.parents.begin(), field.parents.end(), output_entity) == field.parents.end()) {
                field.parents.push_back(output_entity);
            }
            break;
        }
        case 2: {
            auto& field = universe.get<science::Field>(input_entity);
            if (std::find(field.adjacent.begin(), field.adjacent.end(), output_entity) == field.adjacent.end()) {
                field.adjacent.push_back(output_entity);
            }
            break;
        }
        case 3: {
            // Then they want to be a child ofthe output entity
            auto& field = universe.get<science::Field>(output_entity);
            if (std::find(field.parents.begin(), field.parents.end(), input_entity) == field.parents.end()) {
                field.parents.push_back(input_entity);
            }
            break;
        }
    }
}

void CreateNewNode(Universe& universe, FieldNodeInformation& map) {
    ed::PinId inputPinId;
    ed::PinId outputPinId;
    if (!ed::QueryNewLink(&inputPinId, &outputPinId)) {
        return;
    }
    int input_type = (inputPinId.Get() - 1) % 4;
    int output_type = (outputPinId.Get() - 1) % 4;
    if (inputPinId && outputPinId && VerifyFieldNode(input_type, output_type)) {
        entity input_entity = CalculateInputPair(map, inputPinId.Get());
        entity output_entity = CalculateInputPair(map, outputPinId.Get());
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

void RemoveRelationship(Universe& universe, int input_type, entity input_entity,
                        entity output_entity) {
    switch (input_type) {
        case 1: {
            // Then they want to be a child ofthe output entity
            auto& field = universe.get<science::Field>(input_entity);
            RemoveFieldConnection(field.parents, output_entity);
            break;
        }
        case 2: {
            auto& field = universe.get<science::Field>(input_entity);
            RemoveFieldConnection(field.adjacent, output_entity);
            break;
        }
        case 3: {
            // Then they want to be a child ofthe output entity
            auto& field = universe.get<science::Field>(output_entity);
            RemoveFieldConnection(field.adjacent, input_entity);
            break;
        }
    }
}

void HandleDeletedRelationship(const ed::LinkId& linkId, FieldNodeInformation& map, Universe& universe) {
    if (!ed::AcceptDeletedItem()) {
        return;
    }
    // Get the entity and delete
    ed::PinId input_pin;
    ed::PinId output_pin;
    if (!ed::GetLinkPins(linkId, &input_pin, &output_pin)) {
        return;
    }
    // Then set the values
    entity input_entity = CalculateInputPair(map, input_pin.Get());
    entity output_entity = CalculateInputPair(map, output_pin.Get());
    // Remove the relationship
    int input_type = (input_pin.Get() - 1) % 4;
    RemoveRelationship(universe, input_type, input_entity, output_entity);
}

void HandleDeletedNode(const ed::NodeId& nodeId, FieldNodeInformation& map, Universe& universe) {
    if (!ed::AcceptDeletedItem()) {
        return;
    }

    // Delete entities
    entity ent = CalculateInputPair(map, nodeId.Get());
    //universe.get<
    auto fields = universe.view<science::Field>();
    for (entity pot : fields) {
        auto& field_comp = universe.get<science::Field>(pot);
        RemoveFieldConnection(field_comp.adjacent, ent);
        RemoveFieldConnection(field_comp.parents, ent);
    }
    universe.destroy(ent);
}

void HandleNodeDelete(FieldNodeInformation& map, Universe& universe) {
    ed::LinkId linkId = 0;
    while (ed::QueryDeletedLink(&linkId)) {
        HandleDeletedRelationship(linkId, map, universe);
    }
    ed::NodeId node_id = 0;
    while (ed::QueryDeletedNode(&node_id)) {
        HandleDeletedNode(node_id, map, universe);
    }
}
}  // namespace

void SysFieldNodeViewer::DoUI(int delta_time) {
    FieldNodeViewerWindow();
    FieldHjsonViewerWindow();
}

void SysFieldNodeViewer::DoUpdate(int delta_time) {}

void SysFieldNodeViewer::FieldNodeViewerWindow() {
    // View Fields
    ImGui::SetNextWindowSize(ImVec2(1400, 900), ImGuiCond_Appearing);
    ImGui::Begin("Field Node Viewer Window");
    if (ImGui::Button("Add New Node")) {
        entity field = GetUniverse().create();
        GetUniverse().emplace<Field>(field);
        GetUniverse().emplace<Name>(field);
        GetUniverse().emplace<Identifier>(field);
    }
    ed::Begin("Field Viewer");
    int uniqueId = 1;
    FieldNodeInformation map;

    auto fields = GetUniverse().view<Field>();
    // Start drawing nodes
    for (const entt::entity& entity : fields) {
        int node_id = uniqueId++;
        if (ed::GetHoveredNode().Get() != node_id) {
            // Then make it light
            ed::PushStyleColor(ax::NodeEditor::StyleColor_Bg, ImVec4(ImColor(255, 0, 0, 255)));
        }
        ed::BeginNode(node_id);
        ImGui::SetNextItemWidth(200);
        ImGui::InputText(fmt::format("##ne_name_{}", entity).c_str(), &(GetUniverse().get<Name>(entity).name));
        ImGui::SetNextItemWidth(200);
        ImGui::InputText(fmt::format("##ne_identifier{}", entity).c_str(),
                         &(GetUniverse().get<Identifier>(entity).identifier));
        if (GetUniverse().all_of<Description>(entity)) {
            // Description text
            ImGui::SetNextItemWidth(200);
            std::string& description = GetUniverse().get<Description>(entity).description;
            ImGui::InputText(fmt::format("##ne_description{}", entity).c_str(), &description);
        } else {
            if (ImGui::Button("+ Add Description")) {
                GetUniverse().emplace<Description>(entity);
            }
        }
        ImGui::TextFmt("Entity Id: {}", entity);
        int a = uniqueId++;
        ed::BeginPin(a, ed::PinKind::Input);
        ed::PinPivotAlignment(ImVec2(0.1, 0.5f));
        ed::PinPivotSize(ImVec2(0, 0));
        ax::Drawing::Icon(ImVec2(16, 16), ax::Drawing::IconType::Circle, true, ImColor(0, 0, 255, 255),
                          ImColor(32, 32, 32, 255));
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1, 1, 1, 0.8), "Child of");
        ed::EndPin();
        int b = uniqueId++;
        ed::BeginPin(b, ed::PinKind::Output);
        ed::PinPivotAlignment(ImVec2(0.1, 0.5f));
        ed::PinPivotSize(ImVec2(0, 0));
        ax::Drawing::Icon(ImVec2(16, 16), ax::Drawing::IconType::Diamond, true, ImColor(255, 0, 0, 255),
                          ImColor(32, 32, 32, 255));
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1, 1, 1, 0.8), "Adjacent");
        ed::EndPin();

        int c = uniqueId++;
        ImGui::SameLine();
        ImGui::Dummy(ImVec2(15, 0));
        ImGui::SameLine();
        ed::BeginPin(c, ed::PinKind::Output);
        ed::PinPivotAlignment(ImVec2(0.9, 0.5f));
        ed::PinPivotSize(ImVec2(0, 0));
        ImGui::TextColored(ImVec4(1, 1, 1, 0.8), "Parent of");
        ImGui::SameLine();
        ax::Drawing::Icon(ImVec2(16, 16), ax::Drawing::IconType::Circle, true, ImColor(0, 0, 255, 255),
                          ImColor(32, 32, 32, 255));
        ed::EndPin();
        map[entity] = std::make_tuple(a, b, c);
        ed::EndNode();
        if (ed::GetHoveredNode().Get() != node_id) {
            // Then make it light
            ed::PopStyleColor();
        }
    }

    // Draw more nodes
    for (const entity& entity : fields) {
        auto& field = GetUniverse().get<Field>(entity);
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
        HandleNodeDelete(map, GetUniverse());
    }
    ed::EndDelete();

    ed::End();
    ImGui::End();
}

void SysFieldNodeViewer::FieldHjsonViewerWindow() {
    ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiCond_Appearing);
    ImGui::Begin("Field Hjson viewer");
    if (ImGui::Button("Make Fields to Hjson")) {
        // Make the hjson
        Hjson::Value fields = common::systems::science::WriteFields(GetUniverse());
        Hjson::EncoderOptions eo;
        eo.indentBy = "    ";  // 4 spaces
        hjson_content = Hjson::Marshal(fields, eo);
    }
    ImGui::SameLine();
    if (ImGui::Button("Save to file")) {
        // Write to the hjson file, which should remain the same
        std::filesystem::path p = cqsp::common::util::GetCqspDataPath();
        std::filesystem::path default_path = p / "core" / "data" / "science" / "fields" / "default.hjson";
        // Update content
        std::ofstream output(default_path, std::ios::trunc);
        Hjson::EncoderOptions eo;
        eo.indentBy = "    ";  // 4 spaces
        Hjson::Value fields = common::systems::science::WriteFields(GetUniverse());
        Hjson::MarshalToFile(fields, default_path.string(), eo);
    }
    ImGui::InputTextMultiline("field_hjson_viewer", &hjson_content, ImVec2(-1, -1));
    ImGui::End();
}
}  // namespace cqsp::client::systems
