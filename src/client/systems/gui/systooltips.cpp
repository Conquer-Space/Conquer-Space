#include "systooltips.h"

#include "common/components/name.h"
#include "common/components/surface.h"
#include "common/components/bodies.h"
#include "engine/gui.h"

void conquerspace::client::systems::gui::EntityTooltip(
    entt::entity entity, conquerspace::common::components::Universe &universe) {
    if (!ImGui::IsItemHovered()) {
        return;
    }
    namespace cqspc = conquerspace::common::components;
    ImGui::BeginTooltip();
    if (universe.all_of<cqspc::Name>(entity)) {
        ImGui::TextFmt("{}", universe.get<cqspc::Name>(entity).name);
    } else {
        ImGui::TextFmt("{}", entity);
    }
    // Then get type of entity
    // And then tooltip, I guess
    if (universe.all_of<cqspc::bodies::Star>(entity)) {
        ImGui::TextFmt("Star");
    } else if (universe.all_of<cqspc::bodies::Planet>(entity)) {
        ImGui::TextFmt("Planet");
    } else if (universe.any_of<cqspc::Settlement, cqspc::Habitation>(entity)) {
        ImGui::TextFmt("City");
    } else {
        ImGui::TextFmt("Nothing");
    }
    ImGui::EndTooltip();
}
