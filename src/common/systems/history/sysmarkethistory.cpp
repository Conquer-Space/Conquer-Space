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
#include "common/systems/history/sysmarkethistory.h"

#include "client/components/clientctx.h"
#include "common/actions/maneuver/commands.h"
#include "common/actions/shiplaunchaction.h"
#include "common/components/history.h"
#include "common/components/infrastructure.h"
#include "common/components/market.h"
#include "common/components/name.h"
#include "common/components/orbit.h"
#include "common/components/organizations.h"
#include "common/components/population.h"
#include "common/components/resource.h"
#include "common/components/ships.h"
#include "common/components/spaceport.h"
#include "common/components/surface.h"
#include "common/util/nameutil.h"
#include "common/util/utilnumberdisplay.h"

namespace cqsp::common::systems::history {
using components::Market;
using components::MarketHistory;

namespace infrastructure = components::infrastructure;
namespace types = components::types;
namespace ships = components::ships;
namespace bodies = components::bodies;
using common::util::GetName;
using components::PopulationSegment;
using components::Settlement;
using components::Wallet;
void SysMarketHistory::DoSystem() {
    /*
    auto view = GetUniverse().view<Market, MarketHistory>();

    for (entt::entity marketentity : GetUniverse().view<Market>()) {
        Market& market_data = GetUniverse().get<Market>(marketentity);
        market_data.history.push_back(market_data);
    }
    auto view = GetUniverse().view<Market, MarketHistory>();
    for (entt::entity entity : view) {
        auto& history = GetUniverse().get<MarketHistory>(entity);
        Market& market_data = GetUniverse().get<Market>(entity);
        // Loop through the prices
        for (auto resource : market_data.market_information) {
            history.price_history[resource.first].push_back(resource.second.price);
            history.volume[resource.first].push_back(market_data.last_market_information[resource.first].demand);
        }
        double val = 0;
        for (entt::entity ent : market_data.participants) {
            if (GetUniverse().any_of<components::Wallet>(ent)) {
                auto& wallet = GetUniverse().get<components::Wallet>(ent);
                val += wallet.GetGDPChange();
            }
        }
        history.gdp.push_back(val);
    */

    // TODO: REMOVE
    // Get reference body
    entt::entity reference_body = GetUniverse().planets["moon"];
    // Launch inclination will be the inclination of the thing
    double axial = GetUniverse().get<components::bodies::Body>(reference_body).axial;

    types::Orbit orb(8000, 0.0001, 0.001, 0, 0, 0, reference_body);
    entt::entity ship = common::actions::LaunchShip(GetUniverse(), orb);
    // Also compute the value
    GetUniverse().emplace<client::ctx::VisibleOrbit>(ship);

    common::systems::commands::LeaveSOI(GetUniverse(), ship, 1000);
    // Add maneuver like 1000 seconds in the future
    common::systems::commands::PushManeuver(GetUniverse(), ship,
                                            common::systems::commands::MakeManeuver(glm::dvec3(0, 0, 0), 1000));

    // Also self destruct after leaving soi
    entt::entity escape_action = GetUniverse().create();
    GetUniverse().emplace<common::components::Trigger>(escape_action, common::components::Trigger::OnExitSOI);
    GetUniverse().emplace<common::components::Command>(escape_action, common::components::Command::SelfDestruct);

    auto& command_queue = GetUniverse().get_or_emplace<components::CommandQueue>(ship);
    command_queue.commands.push_back(escape_action);
}
}  // namespace cqsp::common::systems::history
