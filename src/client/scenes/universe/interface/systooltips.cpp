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
#include "client/scenes/universe/interface/systooltips.h"

#include <cmath>
#include <string>

#include "client/scenes/universe/interface/sysstockpileui.h"
#include "core/actions/population/cityinformation.h"
#include "core/components/area.h"
#include "core/components/bodies.h"
#include "core/components/coordinates.h"
#include "core/components/infrastructure.h"
#include "core/components/market.h"
#include "core/components/name.h"
#include "core/components/orbit.h"
#include "core/components/organizations.h"
#include "core/components/player.h"
#include "core/components/resource.h"
#include "core/components/science.h"
#include "core/components/ships.h"
#include "core/components/surface.h"
#include "core/util/nameutil.h"
#include "core/util/utilnumberdisplay.h"
#include "engine/gui.h"

namespace cqsp::client::systems::gui {

namespace components = core::components;
namespace types = components::types;
namespace bodies = components::bodies;
namespace ships = components::ships;
using bodies::Body;
using core::Universe;
using types::Orbit;
using util::NumberToHumanString;

void RenderEntityType(const Universe& universe, entt::entity entity) {
    std::string text = core::util::GetEntityType(universe, entity);
    if (text == "Player") {
        ImGui::TextColored(ImColor(252, 186, 3), "Player");
        return;
    } else {
        ImGui::TextFmt("{}", text);
    }
}

void ResourceTooltipSection(const Universe& universe, entt::entity entity) {
    if (universe.all_of<components::FailedResourceTransfer>(entity)) {
        ImGui::TextFmtColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "Failed resource transfer last tick");
    }
    if (universe.all_of<components::FailedResourceProduction>(entity)) {
        ImGui::TextFmtColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "Failed resource production last tick");
    }

    if (universe.all_of<components::ResourceStockpile>(entity)) {
        ImGui::Separator();
        ImGui::TextFmt("Resources");
        // Then do demand
        DrawLedgerTable("resourcesstockpiletooltip", universe, universe.get<components::ResourceStockpile>(entity));
    }
    if (universe.all_of<components::FactoryProducing>(entity)) {
        ImGui::Text("Producing next tick");
    }

    if (universe.all_of<components::IndustrySize>(entity)) {
        ImGui::TextFmt("Size: {}", universe.get<components::IndustrySize>(entity).size);
        ImGui::TextFmt("Utilization: {}", universe.get<components::IndustrySize>(entity).utilization);
        ImGui::TextFmt("Last Tick Difference: {}", universe.get<components::IndustrySize>(entity).diff);
    }

    if (universe.all_of<components::infrastructure::PowerConsumption>(entity)) {
        ImGui::Separator();
        auto& consumption = universe.get<components::infrastructure::PowerConsumption>(entity);
        ImGui::TextFmt("Power: {}", consumption.current);
        ImGui::TextFmt("Max Power: {}", consumption.max);
        ImGui::TextFmt("Min Power: {}", consumption.min);
    }
    if (universe.all_of<components::CostBreakdown>(entity)) {
        components::CostBreakdown costs = universe.get<components::CostBreakdown>(entity);
        ImGui::TextFmt("Material Cost: {}", NumberToHumanString(costs.material_costs));
        ImGui::TextFmt("Wage Cost: {}", NumberToHumanString(costs.wages));
        ImGui::TextFmt("Maintenance Cost: {}", NumberToHumanString(costs.maintenance));
        ImGui::TextFmt("Transport Costs: {}", NumberToHumanString(costs.transport));
        ImGui::Separator();
        ImGui::TextFmt("Profit: {}", NumberToHumanString(costs.profit));
        ImGui::TextFmt("Revenue: {}", NumberToHumanString(costs.revenue));
    }
    if (universe.all_of<components::Price>(entity)) {
        ImGui::TextFmt("Default Price: {}", universe.get<components::Price>(entity).price);
    }
}

void EntityTooltipContent(const Universe& universe, entt::entity entity) {
    if (entity == entt::null) {
        ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), "Null entity!");
        return;
    }
    if (!universe.valid(entity)) {
        ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), "Invalid entity!");
        return;
    }

    ImGui::TextFmt("{}", core::util::GetName(universe, entity));

    if (universe.any_of<components::Description>(entity)) {
        auto& desc = universe.get<components::Description>(entity);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.7f, 0.7f, 1.f));
        ImGui::TextWrapped("%s", desc.description.c_str());
        ImGui::PopStyleColor();
    }

    RenderEntityType(universe, entity);

    if (universe.all_of<components::Wallet>(entity)) {
        auto& balance = universe.get<components::Wallet>(entity);
        ImGui::TextFmt("Wallet: {}", util::NumberToHumanString(balance.GetBalance()));
    }

    if (universe.all_of<components::MarketAgent>(entity)) {
        ImGui::TextFmt("Is Market Participant");
    }
    if (universe.all_of<types::Kinematics>(entity)) {
        auto& a = universe.get<types::Kinematics>(entity);
        ImGui::TextFmt("Position: {} {} {} ({})", a.position.x, a.position.y, a.position.z, glm::length(a.position));
        ImGui::TextFmt("Velocity: {} {} {} ({})", a.velocity.x, a.velocity.y, a.velocity.z, glm::length(a.velocity));
    }

    if (universe.all_of<Body>(entity)) {
        auto& body = universe.get<Body>(entity);
        ImGui::TextFmt("Day length: {} days (offset {})", body.rotation / 86400, body.rotation_offset);
    }

    if (universe.all_of<components::Governed>(entity)) {
        auto& governed = universe.get<components::Governed>(entity);
        ImGui::TextFmt("Owned by: {}", core::util::GetName(universe, governed.governor));
    }

    // If it's a city do population
    if (universe.all_of<components::Settlement>(entity)) {
        core::Node node(universe, entity);
        ImGui::TextFmt("Population: {}", NumberToHumanString(core::actions::GetCityPopulation(node)));
    }
    if (universe.all_of<Body>(entity)) {
        auto& body = universe.get<Body>(entity);
        ImGui::Separator();
        ImGui::TextFmt("Radius: {:.3g} km", body.radius);
        ImGui::TextFmt("Mass: {:.3g} kg", body.mass);
        ImGui::TextFmt("SOI: {:.3g} km", body.SOI);
    }

    if (universe.all_of<types::Orbit>(entity)) {
        auto& orbit = universe.get<Orbit>(entity);
        ImGui::Separator();
        ImGui::TextFmt("Semi Major Axis: {} km", orbit.semi_major_axis);
        ImGui::TextFmt("Inclination: {} deg", types::toDegree(orbit.inclination));
        ImGui::TextFmt("Eccentricity: {}", orbit.eccentricity);
        ImGui::TextFmt("Longitude of Linear Node: {} deg", types::toDegree(orbit.LAN));
        ImGui::TextFmt("Argument of Periapsis: {} deg", types::toDegree(orbit.w));
        ImGui::TextFmt("True Anomaly: {} deg", types::toDegree(orbit.v));
        ImGui::TextFmt("Orbital Period: {} y {} d {} h {} m {} s", (int)(orbit.T() / (60 * 60 * 24 * 365)),
                       (int)std::fmod(orbit.T() / (60 * 60 * 24), 24), (int)std::fmod(orbit.T() / (60 * 60), 60),
                       (int)std::fmod(orbit.T() / 60, 60), std::fmod(orbit.T(), 60));
    }

    if (universe.all_of<components::ships::Crash>(entity)) {
        ImGui::TextFmt("Crashed");
    }

    if (universe.all_of<Orbit, types::Kinematics>(entity)) {
        auto ref = universe.get<Orbit>(entity).reference_body;
        if (universe.valid(ref) && universe.any_of<Body>(ref)) {
            const double radius = universe.get<Body>(ref).radius;
            double distance = glm::length(universe.get<types::Kinematics>(entity).position);
            ImGui::TextFmt("Altitude: {}", distance - radius);
        }
    }

    if (universe.all_of<types::SurfaceCoordinate>(entity)) {
        auto& pos = universe.get<types::SurfaceCoordinate>(entity);
        ImGui::TextFmt("Coordinates: {}, {}", pos.latitude(), pos.longitude());
    }

    // Resource stuff
    // TODO(EhWhoAmI): Set these text red, but too lazy to do it for now
    ResourceTooltipSection(universe, entity);
}

void EntityTooltipContent(const core::Universe& universe, core::components::GoodEntity entity) {
    EntityTooltipContent(universe, universe.GetGood(entity));
}

// TODO(EhWhoAmI): Organize this so that it makes logical sense and order.
// TODO(AGM): Support new production system
void EntityTooltip(const Universe& universe, entt::entity entity) {
    if (!ImGui::IsItemHovered()) {
        return;
    }

    ImGui::BeginTooltip();
    EntityTooltipContent(universe, entity);
    ImGui::EndTooltip();
}

void EntityTooltip(const core::Universe& universe, core::components::GoodEntity entity) {
    EntityTooltip(universe, universe.GetGood(entity));
}
}  // namespace cqsp::client::systems::gui
