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
#include "core/systems/economy/syslabordistribution.h"

#include <map>

#include <tracy/Tracy.hpp>

#include "core/components/market.h"
#include "core/components/population.h"
#include "core/components/surface.h"

namespace cqsp::core::systems {
void SysLaborDistribution::DoSystem() {
    ZoneScoped;
    for (auto&& [entity, settlement, market] :
         GetUniverse().view<components::Settlement, components::Market>().each()) {
        // Now compute the values
        if (settlement.population.empty()) {
            continue;
        }
        // Then compute our job distribution
        // Sort through population...
        for (entt::entity population_segment : settlement.population) {
            HandleJob(GetUniverse().get<components::PopulationSegment>(population_segment), market);
        }
    }
}
void SysLaborDistribution::HandleJob(components::PopulationSegment& segment, components::Market& market) {
    ZoneScoped;
    static const double tick_hours = (40. / static_cast<double>(components::StarDate::WEEK)) * Interval();
    segment.labor.labor_hours.clear();
    int workforce = 0;
    double hours_sum = 0;
    double employment_rate_sum = 0;

    std::map<entt::entity, int> job_drift;
    // Compute job drift
    for (auto& [labor, workers] : segment.labor.labor_distribution) {
        // Now sort through
        auto& labor_comp = GetUniverse().get<components::Labor>(labor);
        // Check how much job we should cut and stuff
        // So we need to loop through our market and see our possible jobs
        // In the future we should make a graph or something so that we don't need to check everything
        if (labor != GetUniverse().default_job && market.sd_ratio[labor_comp.good] > 2) {
            // Then we should probably start cutting
            // Find a ratio for amount we should cut...
            double delta = std::min((market.sd_ratio[labor_comp.good] - 2), 10.);
            double difference = workers * 0.01 * delta;
            job_drift[GetUniverse().default_job] += difference;
            job_drift[labor] -= difference;
        }
        // Also seek for more jobs
        for (const auto good : labor_goods) {
            if (market.price[good] - market.price[labor_comp.good] > market.price[labor_comp.good] * 0.1) {
                // It's enough money for the pop segment to want to shift their jobs
                // Like the thing is it has to be capped, it can't just swarm it...
                // Also cap by the amount of available good labors...
                double delta =
                    std::min((market.price[good] - market.price[labor_comp.good]) / market.price[labor_comp.good], 1.);
                double difference = workers * delta * 0.00001;
                double max_diff = std::max(segment.labor.labor_distribution[good_labors[good]] * 0.001, 100.);
                difference = std::min(max_diff, difference);
                job_drift[good_labors[good]] += difference;
                job_drift[labor] -= difference;
            }
        }
    }
    // Add the job drift
    for (auto& [labor, drift] : job_drift) {
        auto labor_count = std::max(1000, static_cast<int>(segment.labor.labor_distribution[labor] * 0.01));
        int drift_delta = std::clamp(drift, static_cast<int>(-labor_count), static_cast<int>(labor_count));
        if (market.demand[entity_to_good[labor]] == 0 && drift_delta > 0) {
            // then drift should be zero
            drift_delta = 0;
        }
        segment.labor.labor_distribution[labor] += drift_delta;
        // Cap our labor distribution as well...
    }

    for (auto& [labor, workers] : segment.labor.labor_distribution) {
        // Get the jobs that we are over and then figure out why
        auto& labor_comp = GetUniverse().get<components::Labor>(labor);
        // Check if we are over
        // Shift our jobs a little towards jobs that need it and pay more...
        segment.labor.labor_hours.emplace_back(labor_comp.good, tick_hours * workers);
        workforce += workers;
        hours_sum += tick_hours * workers;
        double unemployment_rate = 1 / market.sd_ratio[labor_comp.good];

        // Then we should do something about it
        // If we are way over we are also overemployed...
        // Check if we are way over and if we are way over we should dump jobs
        // Also check job drift to higher paying jobs
        employment_rate_sum += workers * unemployment_rate;
    }

    // Now redistribute our workers
    segment.employed_amount = employment_rate_sum;
    segment.unemployment_rate = (1 - employment_rate_sum) / workforce;
}

void SysLaborDistribution::Init() {
    auto labor_view = GetUniverse().view<components::Labor>();
    for (auto&& [entity, comp] : labor_view.each()) {
        labor_goods.push_back(comp.good);
        entity_to_good[entity] = comp.good;
        good_labors[comp.good] = entity;
    }
}
}  // namespace cqsp::core::systems
