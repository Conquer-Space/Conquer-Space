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
#include "client/systems/gui/systooltips.h"

#include <string>

#include "common/components/name.h"
#include "common/components/surface.h"
#include "common/components/bodies.h"
#include "common/components/area.h"
#include "common/components/resource.h"
#include "common/components/coordinates.h"
#include "common/components/economy.h"
#include "common/components/infrastructure.h"
#include "common/components/organizations.h"
#include "common/components/player.h"
#include "common/components/science.h"

#include "common/systems/population/cityinformation.h"
#include "common/util/utilnumberdisplay.h"

#include "client/systems/gui/sysstockpileui.h"
#include "engine/gui.h"

using cqsp::common::Universe;

namespace {
void RenderEntityType(const Universe& universe, entt::entity entity) {
    std::string text = cqsp::client::systems::gui::GetEntityType(universe, entity);
    if (text == "Player") {
        ImGui::TextColored(ImColor(252, 186, 3), "Player");
        return;
    } else {
        ImGui::TextFmt("{}", text);
    }
}

void ResourceTooltipSection(const Universe &universe, entt::entity entity) {
    namespace cqspc = cqsp::common::components;
    //TODO(EhWhoAmI): Set these text red, but too lazy to do it for now
    if (universe.all_of<cqspc::FailedResourceTransfer>(entity)) {
        ImGui::TextFmt("Failed resource transfer last tick");
    }
    if (universe.all_of<cqspc::FailedResourceProduction>(entity)) {
        ImGui::TextFmt("Failed resource production last tick");
    }

    if (universe.all_of<cqspc::ResourceStockpile>(entity)) {
        ImGui::Separator();
        ImGui::TextFmt("Resources");
        // Then do demand
        cqsp::client::systems::DrawLedgerTable(
            "resourcesstockpiletooltip", universe, universe.get<cqspc::ResourceStockpile>(entity));
    }
    if (universe.all_of<cqspc::FactoryProducing>(entity)) {
        ImGui::Text("Producing next tick");
    }

    if (universe.all_of<cqspc::IndustrySize>(entity)) {
        ImGui::TextFmt(
            "Size: {}",
                       universe.get<cqspc::IndustrySize>(entity).size);
        ImGui::TextFmt("Utilization: {}",
                       universe.get<cqspc::IndustrySize>(entity).utilization);
    }



    if (universe.all_of<cqspc::infrastructure::PowerConsumption>(entity)) {
        ImGui::Separator();
        auto& consumption = universe.get<cqspc::infrastructure::PowerConsumption>(entity);
        ImGui::TextFmt("Power: {}", consumption.current);
        ImGui::TextFmt("Max Power: {}", consumption.max);
           ImGui::TextFmt("Min Power: {}", consumption.min);
    }
    if (universe.all_of<cqspc::CostBreakdown>(entity)) {
        cqspc::CostBreakdown costs = universe.get<cqspc::CostBreakdown>(entity);
        ImGui::TextFmt("Material Cost: {}", costs.materialcosts * -1);
        ImGui::TextFmt("Wage Cost: {}", costs.wages * -1);
        ImGui::TextFmt("Maintenance Cost: {}", costs.maintenance * -1);
        ImGui::TextFmt("Gross Revenue Cost: {}", costs.profit);
        ImGui::TextFmt("Net Revenue Cost: {}", costs.net);
        ImGui::TextFmt("Transport Costs: {}", costs.transport);
    }
}
}  // namespace

namespace cqsp::client::systems::gui {
std::string GetName(const Universe& universe, entt::entity entity) {
    namespace cqspc = cqsp::common::components;
    if (universe.all_of<cqspc::Name>(entity)) {
        return universe.get<cqspc::Name>(entity);
    } else if (universe.all_of<cqspc::Identifier>(entity)) {
        return universe.get<cqspc::Identifier>(entity);
    } else {
        return fmt::format("{}", entity);
    }
}

namespace cqspc = cqsp::common::components;
void EntityTooltipContent(const Universe& universe, entt::entity entity) {
    if (entity == entt::null) {
        ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), "Null entity!");
        return;
    }
    if (!universe.valid(entity)) {
        ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), "Invalid entity!");
        return;
    }

    ImGui::TextFmt("{}", GetName(universe, entity));

    if (universe.any_of<common::components::Description>(entity)) {
        auto& desc = universe.get<common::components::Description>(entity);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7, 0.7, 0.7, 1));
        ImGui::TextWrapped(desc.description.c_str());
        ImGui::PopStyleColor();
    }

    RenderEntityType(universe, entity);

    if (universe.all_of<cqspc::Wallet>(entity)) {
        auto& balance = universe.get<cqsp::common::components::Wallet>(entity);
        ImGui::TextFmt("Wallet: {}", balance.GetBalance());
    }

    if (universe.all_of<cqspc::MarketAgent>(entity)) {
        ImGui::TextFmt("Is Market Participant");
    }
    if (universe.all_of<cqspc::types::Kinematics>(entity)) {
        auto& a =
            universe.get<cqsp::common::components::types::Kinematics>(entity);
        ImGui::TextFmt("Position: {} {} {} ({})", a.position.x, a.position.y,
                       a.position.z, glm::length(a.position));
        ImGui::TextFmt("Velocity: {} {} {} ({})", a.velocity.x, a.velocity.y,
                       a.velocity.z, glm::length(a.velocity));
    }

    if (universe.all_of<cqspc::bodies::Body>(entity)) {
        auto& body = universe.get<cqspc::bodies::Body>(entity);
        ImGui::TextFmt("Rotation: {} days", body.rotation / 86400);
    }

    if (universe.all_of<cqspc::Governed>(entity)) {
        auto& governed = universe.get<cqspc::Governed>(entity);
        ImGui::TextFmt("Owned by: {}", GetName(universe, governed.governor));
    }

    // If it's a city do population
    if (universe.all_of<cqspc::Settlement>(entity)) {
        ImGui::TextFmt("Population: {}", util::LongToHumanString(
                                             common::systems::GetCityPopulation(
                                                 universe, entity)));
    }
    if (universe.all_of<common::components::bodies::Body>(entity)) {
        auto& body = universe.get<common::components::bodies::Body>(entity);
        ImGui::Separator();
        ImGui::TextFmt("Radius: {:.3g} km", body.radius);
        ImGui::TextFmt("Mass: {:.3g} kg", body.mass);
        ImGui::TextFmt("SOI: {:.3g} km", body.SOI);
    }

    if (universe.all_of<common::components::types::Orbit>(entity)) {
        auto& orbit = universe.get<common::components::types::Orbit>(entity);
        ImGui::Separator();
        ImGui::TextFmt("Semi Major Axis: {}", orbit.semi_major_axis);
        ImGui::TextFmt("Inclination: {}", orbit.inclination);
        ImGui::TextFmt("Eccentricity: {}", orbit.eccentricity);
        ImGui::TextFmt("Longitude of Linear Node: {}", orbit.LAN);
        ImGui::TextFmt("Argument of Periapsis: {}", orbit.w);
        ImGui::TextFmt("True Anomaly: {}", orbit.v);
        ImGui::TextFmt("Orbital Period: {} y {} d {} h {} m {} s",
                       (int) (orbit.T / (60 * 60 * 24 * 365)),
                       (int) std::fmod(orbit.T / (60 * 60 * 24), 24),
                       (int) std::fmod(orbit.T / (60 * 60), 60),
                       (int) std::fmod(orbit.T / 60, 60),
                       std::fmod(orbit.T, 60));
    }

    if (universe.all_of<common::components::types::SurfaceCoordinate>(entity)) {
        auto& pos =
            universe.get<common::components::types::SurfaceCoordinate>(entity);
        ImGui::TextFmt("Coordinates: {}, {}", pos.latitude(), pos.longitude());
    }

    // Resource stuff
    // TODO(EhWhoAmI): Set these text red, but too lazy to do it for now
    ResourceTooltipSection(universe, entity);
}

std::string GetEntityType(const cqsp::common::Universe& universe,
                          entt::entity entity) {
    namespace cqspc = cqsp::common::components;
    // Then get type of entity
    if (entity == entt::null) {
        return "Null Entity";
    }
    if (universe.all_of<cqspc::bodies::Star>(entity)) {
        return "Star";
    } else if (universe.all_of<cqspc::bodies::Planet>(entity)) {
        return  "Planet";
    } else if (universe.any_of<cqspc::Settlement, cqspc::Habitation>(entity)) {
        return  "City";
    }  else if (universe.any_of<cqspc::Production>(entity)) {
        std::string production = "";
        auto& generator = universe.get<cqspc::Production>(entity);
        return fmt::format("{} Factory", cqsp::client::systems::gui::GetName(universe, generator.recipe));
    }  else if (universe.any_of<cqspc::Mine>(entity)) {
        /*
        std::string production = "";
        auto& generator = universe.get<cqspc::ResourceGenerator>(entity);
        for (auto it = generator.begin(); it != generator.end(); ++it) {
            production += universe.get<cqspc::Name>(it->first).name + ", ";
        }
        // Remove last comma
        if (!production.empty()) {
            production = production.substr(0, production.size() - 2);
        }
        return fmt::format("{} Mine", production);
        */
    } else if (universe.any_of<cqspc::Player>(entity)) {
        return "Player";
    } else if (universe.any_of<cqspc::Civilization>(entity)) {
        return "Civilization";
    } else if (universe.any_of<cqspc::Organization>(entity)) {
        return "Organization";
    } else if (universe.any_of<cqspc::science::Lab>(entity)) {
        return "Science Lab";
    } else if (universe.any_of<cqspc::Commercial>(entity)) {
        return "Commercial";
    } else {
        return "Unknown";
    }
}

// TODO(EhWhoAmI): Organize this so that it makes logical sense and order.
// TODO(AGM): Support new production system
void EntityTooltip(const Universe &universe, entt::entity entity) {
    if (!ImGui::IsItemHovered()) {
        return;
    }

    namespace cqspc = cqsp::common::components;
    ImGui::BeginTooltip();
    EntityTooltipContent(universe, entity);
    ImGui::EndTooltip();
}
}  // namespace cqsp::client::systems::gui
