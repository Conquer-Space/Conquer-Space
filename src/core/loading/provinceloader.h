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
#pragma once

#include <random>
#include <string_view>

#include "core/loading/hjsonloader.h"

namespace cqsp::core::loading {
/// <summary>
/// This loader has to be loaded after \ref PlanetLoader because it adds the cities to the
/// respectve planets
/// </summary>
class ProvinceLoader : public HjsonLoader {
 public:
    explicit ProvinceLoader(Universe& universe) : HjsonLoader(universe), gen(rd()), distrib(1, 10) {}

    const Hjson::Value& GetDefaultValues() override { return default_val; }
    bool LoadValue(const Hjson::Value& values, Node& node) override;
    void PostLoad(const Node& node) override;
    void ParseIndustry(const Hjson::Value& industry_hjson, Node& node, std::string_view identifier);

 private:
    Hjson::Value default_val;
    Node GetCountry(const std::string& country_identifier, const std::string& identifier);
    Node GetPlanet(const std::string& planet_identifier, const std::string& identifier);

    Node ParsePopulation(const Hjson::Value& population_hjson);

    std::random_device rd;
    std::mt19937 gen;
    std::uniform_int_distribution<> distrib;
};
}  // namespace cqsp::core::loading
