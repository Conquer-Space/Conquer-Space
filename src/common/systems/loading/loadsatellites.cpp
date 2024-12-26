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
#include "common/systems/loading/loadsatellites.h"

#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "common/components/coordinates.h"
#include "common/components/model.h"
#include "common/components/name.h"
#include "common/components/orbit.h"
#include "common/components/ships.h"
#include "common/systems/loading/loadorbit.h"
namespace cqsp::common::systems::loading {
namespace {
std::string trim(const std::string& str, const std::string& whitespace = " \t") {
    const auto strBegin = str.find_first_not_of(whitespace);
    if (strBegin == std::string::npos) return "";  // no content

    const auto strEnd = str.find_last_not_of(whitespace);
    const auto strRange = strEnd - strBegin + 1;

    return str.substr(strBegin, strRange);
}
}  // namespace

components::types::Orbit GetOrbit(const std::string& line_one, const std::string& line_two, const double& GM) {
    // Epoch year
    double epoch_year = std::stoi(line_one.substr(18, 2));
    double epoch_time = std::stod(line_one.substr(20, 12));

    double epoch = GetEpoch(epoch_year, epoch_time);

    // If the epoch year is less than 57, then it's the 20th century
    std::stringstream ss(line_two);
    std::istream_iterator<std::string> begin(ss);
    std::istream_iterator<std::string> end;
    std::vector<std::string> vstrings(begin, end);

    components::types::Orbit orbit;

    // https://en.wikipedia.org/wiki/Two-line_element_set

    using components::types::toRadian;

    double inclination = toRadian(std::stod(vstrings[2]));
    double LAN = toRadian(std::stod(vstrings[3]));       // Longitude of the ascending node
    double e = std::stod("0." + vstrings[4]);            // eccentricity
    double w = toRadian(std::stod(vstrings[5]));         // Argument of perapsis
    double m0 = toRadian(std::stod(vstrings[6]) + 120);  // Add 180 because orbits are messed up.
                                                         // Gotta fix that somehow, but idk how

    double mean_motion = std::stod(vstrings[7]);

    double T = (24 * 3600) / mean_motion;
    double a = pow(T * T * GM / (4.0 * components::types::PI * components::types::PI),
                   1. / 3.0);  // semi_major_axis

    orbit.eccentricity = e;
    orbit.semi_major_axis = a;
    orbit.inclination = inclination;
    orbit.LAN = LAN;
    orbit.w = w;
    orbit.GM = GM;
    orbit.M0 = m0;
    orbit.epoch = epoch;

    return orbit;
}

int GetEpochYear(int year) {
    int epoch = 2000;
    if (year > 56) {
        // Then the year is 1900s
        epoch = 1900;
    }
    return epoch + year;
}

double GetEpoch(double year, double time) {
    int year_diff = GetEpochYear(year) - 2000;
    return time * 86400. + year_diff * 31557600.;
}

void LoadSatellites(Universe& universe, std::string& string) {
    // Load satellite data
    std::istringstream f(string);
    std::string line;
    int count = 0;
    entt::entity earth = universe.planets["earth"];
    const double GM = universe.get<components::bodies::Body>(earth).GM;

    // Get the next three lines or something like that
    while (std::getline(f, line)) {
        if (line.empty()) {
            break;
        }
        line = trim(line);

        std::string line_one;
        std::getline(f, line_one);
        std::string line_two;
        std::getline(f, line_two);
        entt::entity satellite = universe.create();
        universe.emplace<components::Name>(satellite, line);

        // Add to earth
        auto orbit = GetOrbit(line_one, line_two, GM);
        orbit.inclination += universe.get<components::bodies::Body>(earth).axial * cos(orbit.inclination);
        // orbit.M0 += universe.get<components::bodies::Body>(earth).axial;
        orbit.reference_body = earth;
        // The math works
        universe.get<components::bodies::OrbitalSystem>(earth).push_back(satellite);
        universe.emplace<components::types::Orbit>(satellite, orbit);
        universe.emplace<components::ships::Ship>(satellite);
    }
}

bool SatelliteLoader::LoadValue(const Hjson::Value& values, entt::entity entity) {
    std::optional<components::types::Orbit> orbit = LoadOrbit(values["orbit"]);
    if (!orbit.has_value()) {
        SPDLOG_INFO("Loaded orbit!");

        return false;
    }
    orbit->reference_body = universe.planets[values["orbit"]["reference"].to_string()];
    orbit->GM = universe.get<components::bodies::Body>(orbit->reference_body).GM;
    if (values["model"].defined()) {
        // Then we can add a model
        universe.emplace<components::WorldModel>(entity, values["model"].to_string());
    } else {
        // TODO(EhWhoAmI): We add a generic model instead
    }
    // Get name but no identifier
    universe.emplace<components::types::Orbit>(entity, *orbit);
    universe.get<components::bodies::OrbitalSystem>(orbit->reference_body).push_back(entity);
    universe.emplace<components::ships::Ship>(entity);
    SPDLOG_INFO("Loaded orbit!");
    return true;
}
}  // namespace cqsp::common::systems::loading
