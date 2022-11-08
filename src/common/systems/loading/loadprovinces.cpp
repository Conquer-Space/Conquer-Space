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
#include "common/systems/loading/loadprovinces.h"

#include <spdlog/spdlog.h>

#include <sstream>

#include "common/components/surface.h"
#include "common/components/name.h"
#include "common/components/organizations.h"

void cqsp::common::systems::loading::LoadProvinces(common::Universe& universe,
                                                   const std::string& text) {
    // The text has to be csv, so treat it is csv
    std::istringstream f(text);
    std::string line;
    while (std::getline(f, line)) {
        // Split and parse
        std::string token;
        std::istringstream f2(line);
        std::getline(f2, token, ',');
        std::string identifier = token;
        std::getline(f2, token, ',');
        std::string r = token;
        std::getline(f2, token, ',');
        std::string g = token;
        std::getline(f2, token, ',');
        std::string b = token;
        std::getline(f2, token, ',');
        std::string country = token;
        // Create
        entt::entity entity = universe.create();
        universe.emplace<components::Province>(entity, universe.countries[country]);
        universe.emplace<components::Identifier>(entity, identifier);
        auto& color = universe.emplace<components::ProvinceColor>(entity, std::stoi(r),
                                                    std::stoi(g), std::stoi(b));
        if (universe.provinces.find(identifier) == universe.provinces.end()) {
            universe.provinces[identifier] = entity;
        } else {
            SPDLOG_WARN(
                "Province {} conflicts with an already preexisting province",
                identifier);
        }
        // Add province to country
        universe.get_or_emplace<components::CountryCityList>(universe.countries[country])
            .province_list.push_back(entity);

        universe.province_colors[color] = entity;
    }
}
