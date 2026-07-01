/* Conquer Space
 * Copyright (C) 2021-2026 Conquer Space
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
#include "core/systems/modifierhelper.h"

#include <algorithm>

#include "core/components/area.h"
#include "core/components/modifier.h"

namespace cqsp::core::systems {

namespace {
// Visits the ModifiableValue for the given target type, calling fn(val) regardless of Op.
template <typename F>
void VisitModifiableValue(Universe& universe, entt::entity target, components::ModifierTarget type, F&& fn) {
    switch (type) {
        case components::ModifierTarget::ExpertiseGain:
            fn(universe.get<components::ProductionUnit>(target).expertise_gain);
            break;
        case components::ModifierTarget::MaxExpertise:
            fn(universe.get<components::ProductionUnit>(target).max_expertise);
            break;
    }
}
}  // namespace

void ApplyModifier(Universe& universe, entt::entity modifier_entity, entt::entity target_entity) {
    const auto& modifier = universe.get<components::Modifier>(modifier_entity);
    VisitModifiableValue(universe, target_entity, modifier.target, [&](auto& val) {
        val.modifiers.emplace_back(modifier_entity, modifier.amount);
        val.recalculate();
    });
}

void RemoveModifier(Universe& universe, entt::entity modifier_entity, entt::entity target_entity) {
    const auto& modifier = universe.get<components::Modifier>(modifier_entity);
    VisitModifiableValue(universe, target_entity, modifier.target, [&](auto& val) {
        auto it = std::find_if(val.modifiers.begin(), val.modifiers.end(),
                               [modifier_entity](const auto& m) { return m.first == modifier_entity; });
        if (it != val.modifiers.end()) {
            val.modifiers.erase(it);
            val.recalculate();
        }
    });
}
}  // namespace cqsp::core::systems
