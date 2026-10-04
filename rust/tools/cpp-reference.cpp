// Runs the original C++ components, not a second implementation of their formulas.
#include <cmath>
#include <iomanip>
#include <fstream>
#include <filesystem>
#include <initializer_list>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>
#include "core/actions/maneuver/basicmaneuver.h"
#include "core/actions/maneuver/hohmann.h"
#include "core/actions/maneuver/rendezvous.h"
#include "core/actions/maneuver/commands.h"
#include "core/components/colony.h"
#include "core/components/name.h"
#include "core/components/organizations.h"
#include "core/components/population.h"
#include "core/components/projects.h"
#include "core/components/spaceport.h"
#include "core/components/surface.h"
#include "core/components/market.h"
#include "core/components/modifier.h"
#include "core/components/orbit.h"
#include "core/components/pid.h"
#include "core/components/stardate.h"
#include "core/loading/loadorbit.h"
#include "core/loading/loadcities.h"
#include "core/loading/projectloader.h"
#include "core/systems/economy/economyconfig.h"
#include <spdlog/spdlog.h>

using namespace cqsp::core::components;

std::vector<Hjson::Value> ReadEntries(const std::filesystem::path& directory) {
    std::vector<std::filesystem::path> paths;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(directory)) {
        if (entry.is_regular_file() && entry.path().extension() == ".hjson") paths.push_back(entry.path());
    }
    std::sort(paths.begin(), paths.end());
    std::vector<Hjson::Value> entries;
    for (const auto& path : paths) {
        std::ifstream input(path);
        auto value = Hjson::Unmarshal(std::string(std::istreambuf_iterator<char>(input), {}));
        for (int index = 0; index < value.size(); ++index) entries.push_back(value[index]);
    }
    return entries;
}

void PrintSurfaceAssets(const std::filesystem::path& data) {
    // Prerequisite reference maps only; city/project behavior is executed by the actual loaders.
    cqsp::core::Universe universe("surface-reference");
    spdlog::set_level(spdlog::level::off);
    auto register_entries = [&](const auto& path, auto& map) {
        for (const auto& entry : ReadEntries(path)) {
            auto identifier = entry["identifier"].to_string();
            if (map.contains(identifier)) continue;
            auto entity = universe.create();
            universe.emplace<Identifier>(entity, identifier);
            map.emplace(identifier, entity);
        }
    };
    register_entries(data / "data/planets", universe.planets);
    register_entries(data / "data/countries", universe.countries);
    register_entries(data / "data/planet_data/provinces", universe.provinces);
    register_entries(data / "data/goods", universe.goods);
    for (const auto& [identifier, planet] : universe.planets) universe.emplace<types::Orbit>(planet);
    auto sun = universe.planets.at("sun");
    for (const auto& [identifier, planet] : universe.planets) {
        universe.get<types::Orbit>(planet).reference_body = identifier == "sun" ? entt::null
            : identifier == "moon" ? universe.planets.at("earth") : sun;
    }
    for (const auto& [identifier, province] : universe.provinces) universe.emplace<Province>(province);
    for (const auto& [identifier, country] : universe.countries) universe.emplace<CountryCityList>(country);
    for (const auto& [identifier, good] : universe.goods) {
        universe.good_map[good] = ToGoodEntity(static_cast<uint32_t>(universe.good_vector.size()));
        universe.good_vector.push_back(good);
    }
    cqsp::core::loading::CityLoader city_loader(universe);
    for (const auto& entry : ReadEntries(data / "data/cities")) {
        cqsp::core::Node node(universe);
        universe.emplace<Identifier>(node, entry["identifier"].to_string());
        if (!city_loader.LoadValue(entry, node)) throw std::runtime_error("City loader rejected an asset");
    }
    std::cout << std::boolalpha << "{\"cities\":[";
    bool first = true;
    for (const auto& [identifier, city] : universe.cities) {
        if (!first) std::cout << ',';
        auto coordinate = universe.get<types::SurfaceCoordinate>(city);
        auto province = universe.get<City>(city).province;
        std::cout << '[' << std::quoted(identifier) << ',' << std::quoted(universe.get<Identifier>(coordinate.planet).identifier) << ',';
        if (province == entt::null) std::cout << "null";
        else std::cout << std::quoted(universe.get<Identifier>(province).identifier);
        std::cout << ',' << coordinate.r_latitude() << ',' << coordinate.r_longitude() << ','
            << universe.all_of<infrastructure::SpacePort>(city) << ',' << universe.all_of<Market>(city) << ','
            << universe.all_of<Settlement>(city) << ',' << universe.all_of<IndustrialZone>(city) << ']';
        first = false;
    }
    cqsp::core::loading::ProjectLoader project_loader(universe);
    std::cout << "],\"projects\":[";
    first = true;
    for (const auto& entry : ReadEntries(data / "data/projects")) {
        cqsp::core::Node node(universe);
        if (!project_loader.LoadValue(entry, node)) throw std::runtime_error("Project loader rejected an asset");
        if (!first) std::cout << ',';
        auto& project = node.get<ProjectTemplate>();
        std::cout << '[' << std::quoted(entry["identifier"].to_string()) << ',' << project.max_progress << ',';
        if (node.all_of<HabitationModule>()) std::cout << node.get<HabitationModule>().crew;
        else std::cout << "null";
        std::cout << ",{";
        bool first_good = true;
        for (const auto& [good, amount] : project.cost) {
            if (!first_good) std::cout << ',';
            std::cout << std::quoted(universe.get<Identifier>(universe.GetGood(good)).identifier) << ':' << amount;
            first_good = false;
        }
        std::cout << "}]";
        first = false;
    }
    std::cout << "],\"common_soi\":[";
    first = true;
    for (auto [source, target] : { std::pair{"earth", "moon"}, {"moon", "earth"}, {"moon", "mars"},
            {"earth", "mars"}, {"earth", "earth"}, {"sun", "earth"} }) {
        if (!first) std::cout << ',';
        auto common = cqsp::core::systems::commands::GetCommonSOI(universe, universe.planets.at(source), universe.planets.at(target));
        std::cout << std::quoted(universe.get<Identifier>(common).identifier);
        first = false;
    }
    std::cout << "]}\n";
}

void PrintVector(const glm::dvec3& vector) {
    std::cout << vector.x << ',' << vector.y << ',' << vector.z;
}

void PrintOrbit(const char* name, types::Orbit orbit, std::initializer_list<double> times) {
    std::cout << '"' << name << "\":[";
    bool first = true;
    for (double time : times) {
        types::UpdateOrbit(orbit, time);
        if (!first) std::cout << ',';
        std::cout << orbit.v << ',' << orbit.GetOrbitingRadius() << ',' << orbit.OrbitalVelocity() << ',';
        PrintVector(types::toVec3(orbit));
        std::cout << ',';
        PrintVector(types::OrbitVelocityToVec3(orbit));
        first = false;
    }
    std::cout << "],";
}

void PrintElements(const char* name, const types::Orbit& orbit) {
    std::cout << '"' << name << "\":[" << orbit.semi_major_axis << ',' << orbit.eccentricity << ','
        << orbit.inclination << ',' << orbit.LAN << ',' << orbit.w << ',' << orbit.M0 << ','
        << orbit.epoch << ',' << orbit.v << ',' << orbit.GM << "],";
}

void PrintOrbitGrid() {
    struct Sample { double time; types::Orbit orbit; };
    std::vector<Sample> samples;
    Sample sample;
    // Read everything before writing, so redirected input/output cannot deadlock.
    while (std::cin >> sample.time >> sample.orbit.semi_major_axis >> sample.orbit.eccentricity
        >> sample.orbit.inclination >> sample.orbit.LAN >> sample.orbit.w >> sample.orbit.M0
        >> sample.orbit.epoch >> sample.orbit.GM) {
        samples.push_back(sample);
    }
    std::cout << "{\"grid\":[";
    bool first = true;
    for (auto& sample : samples) {
        types::UpdateOrbit(sample.orbit, sample.time);
        if (!first) std::cout << ',';
        std::cout << sample.orbit.v << ',' << sample.orbit.GetOrbitingRadius() << ','
            << sample.orbit.OrbitalVelocity() << ',';
        PrintVector(types::toVec3(sample.orbit));
        std::cout << ',';
        PrintVector(types::OrbitVelocityToVec3(sample.orbit));
        first = false;
    }
    std::cout << "]}\n";
}

void PrintLoadedOrbits() {
    std::vector<std::string> paths;
    std::string path;
    while (std::getline(std::cin, path)) {
        paths.push_back(path);
    }
    std::cout << "{\"loaded\":[";
    bool first = true;
    for (const auto& path : paths) {
        std::ifstream file(path);
        if (!file) throw std::runtime_error("Could not open orbit asset");
        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        auto entries = Hjson::Unmarshal(content);
        for (int index = 0; index < entries.size(); ++index) {
            auto orbit = cqsp::core::loading::LoadOrbit(entries[index]["orbit"]);
            if (!orbit) throw std::runtime_error("Source rejected an orbit asset");
            if (!first) std::cout << ',';
            std::cout << orbit->semi_major_axis << ',' << orbit->eccentricity << ',' << orbit->inclination
                << ',' << orbit->LAN << ',' << orbit->w << ',' << orbit->M0 << ',' << orbit->epoch
                << ',' << orbit->v << ',' << orbit->GM;
            first = false;
        }
    }
    std::cout << "]}\n";
}

int main(int argc, char** argv) {
    std::cout << std::setprecision(17);
    if (argc == 2 && std::string(argv[1]) == "--orbit-grid") {
        PrintOrbitGrid();
        return 0;
    }
    if (argc == 2 && std::string(argv[1]) == "--load-orbits") {
        PrintLoadedOrbits();
        return 0;
    }
    if (argc == 3 && std::string(argv[1]) == "--surface-assets") {
        PrintSurfaceAssets(argv[2]);
        return 0;
    }
    Wallet wallet;
    wallet = 100;
    wallet.Reset();
    wallet -= 25;
    wallet = 50;
    wallet *= 0.5;
    std::cout << "{\"wallet\":[" << wallet.GetBalance() << ',' << wallet.GetChange() << ',' << wallet.GetGDPChange() << "],";

    AdditiveModifier additive(1.5);
    additive.modifiers.emplace_back(static_cast<entt::entity>(0), 1.0);
    additive.modifiers.emplace_back(static_cast<entt::entity>(0), 1.0);
    additive.recalculate();
    std::cout << "\"modifiers\":[" << additive.value << ',';
    additive.modifiers.erase(additive.modifiers.begin());
    additive.recalculate();
    std::cout << additive.value << ',';
    additive = 2.0;
    std::cout << additive.value << ',';
    additive += 0.25;
    std::cout << additive.value << ',';
    additive *= 2.0;
    std::cout << additive.value << ',';
    MultiplicativeModifier multiplicative(0.0001);
    multiplicative.modifiers.emplace_back(static_cast<entt::entity>(0), -0.8);
    multiplicative.modifiers.emplace_back(static_cast<entt::entity>(1), -0.8);
    multiplicative.recalculate();
    std::cout << multiplicative.value << ',';
    multiplicative = 0.01;
    std::cout << multiplicative.value << ',';
    multiplicative.modifiers.erase(multiplicative.modifiers.begin());
    multiplicative.recalculate();
    std::cout << multiplicative.value << ',';
    multiplicative *= 0.5;
    std::cout << multiplicative.value << ',';
    multiplicative = -0.02;
    std::cout << multiplicative.value << "],";

    PID pid;
    std::cout << "\"pid\":[";
    bool first = true;
    for (double error : {0.4, -0.8, 1.2, -2.0, 0.0}) {
        pid.Update(error);
        if (!first) std::cout << ',';
        std::cout << pid.GetValue({0.01, 0.1, 0.01});
        first = false;
    }
    std::cout << "],";

    Market market(3);
    market.price[ToGoodEntity(0)] = 10;
    market.price[ToGoodEntity(1)] = 20;
    market.price[ToGoodEntity(2)] = 5;
    market.taxation[ToGoodEntity(0)] = 0.1;
    market.taxation[ToGoodEntity(2)] = 0.25;
    ResourceVector input;
    input.emplace_back(ToGoodEntity(0), 2);
    input.emplace_back(ToGoodEntity(1), 3);
    input.emplace_back(ToGoodEntity(2), 4);
    auto [cost, tax] = market.PurchaseFromMarket(input);
    std::cout << "\"purchase\":[" << cost << ',' << tax << ',' << market.consumption.GetSum() << "],";
    market.trade[ToGoodEntity(0)] = -3;
    market.chronic_shortages[ToGoodEntity(0)] = 7;
    market.ResetLedgers();
    std::cout << "\"reset\":[" << market.consumption.GetSum() << ',' << market.trade[ToGoodEntity(0)] << ','
        << market.chronic_shortages[ToGoodEntity(0)] << ',' << market.price[ToGoodEntity(0)] << "],";

    ResourceLedger supply(3), demand(3);
    supply[ToGoodEntity(0)] = 10;
    demand[ToGoodEntity(0)] = 5;
    supply[ToGoodEntity(1)] = 2;
    auto ratios = supply.SafeDivision(demand);
    std::cout << "\"ratios\":[" << ratios[ToGoodEntity(0)] << ',' << std::isinf(ratios[ToGoodEntity(1)]) << ','
        << std::isinf(ratios[ToGoodEntity(2)]) << "],";

    cqsp::core::systems::EconomyConfig::ProductionConfig production;
    std::cout << "\"utilization\":[" << production.GetFactoryMinUtilization(10) << ','
        << production.GetFactoryMinUtilization(100) << ',' << production.GetFactoryUtilizationDiff(-500) << ','
        << production.GetFactoryUtilizationDiff(0) << ',' << production.GetFactoryUtilizationDiff(500) << "],";

    StarDate date;
    date.IncrementDate();
    std::cout << "\"date\":[" << date.GetDate() << ',' << date.ToSecond() << ',' << date.ToDay() << ',';
    date.SetDate(144);
    std::cout << date.ToSecond() << ',' << date.ToDay() << ',' << date.ToTicks(3600) << "],";
    date.SetDate(143);
    std::cout << "\"time\":[" << date.GetHour() << ',' << date.GetMinute() << ',' << date.GetHour(-24) << "],";
    date.SetDate((365 * 2 + 59) * StarDate::DAY);
    std::cout << "\"calendar\":[" << date.GetYear() << ',' << date.GetMonth() << ',' << date.GetDay() << "],";

    types::Orbit orbit;
    orbit.semi_major_axis = 12000;
    orbit.eccentricity = 0.25;
    orbit.inclination = 0.3;
    orbit.LAN = 1.1;
    orbit.w = 0.7;
    orbit.M0 = 0.5;
    orbit.epoch = 123;
    orbit.GM = 398600.44188;
    PrintOrbit("orbit_elliptic", orbit, {0, 123, 600, 86400, -600});
    auto circle = orbit;
    circle.eccentricity = 0;
    PrintOrbit("orbit_circular", circle, {0, 600, 86400});
    auto escape = orbit;
    escape.semi_major_axis = -12000;
    escape.eccentricity = 1.5;
    PrintOrbit("orbit_hyperbolic", escape, {-1000, 123, 2000});
    auto state = types::OrbitTimeToVec3(orbit, 4321);
    auto velocity = types::OrbitTimeToVelocityVec3(orbit, 4321);
    PrintElements("orbit_recovered", types::Vec3ToOrbit(state, velocity, orbit.GM, 4321));
    PrintElements("orbit_impulse", types::ApplyImpulse(orbit, glm::dvec3(0.01, 0.04, -0.02), 4321));
    types::UpdateOrbit(orbit, 600);
    std::cout << "\"orbit_timing\":[" << orbit.T() << ',' << orbit.nu() << ',' << orbit.GetApoapsis()
        << ',' << orbit.GetPeriapsis() << ',' << orbit.TimeToTrueAnomaly(2.5) << "],";
    auto circular = orbit;
    circular.eccentricity = 0;
    types::UpdateOrbit(circular, 600);
    auto target = orbit;
    target.semi_major_axis = 18000;
    target.eccentricity = 0.1;
    target.inclination = 0.6;
    target.LAN = 1.4;
    target.w = 0.3;
    target.M0 = 1.4;
    types::UpdateOrbit(target, 600);
    using namespace cqsp::core::systems;
    auto transfer = UnsafeHohmannTransfer(circular, 22000);
    auto intercept = CoplanarIntercept(circular, target, 600);
    std::cout << "\"maneuvers\":[";
    bool first_maneuver = true;
    for (auto maneuver : {CircularizeAtApoapsis(orbit), CircularizeAtPeriapsis(orbit),
        SetApoapsis(orbit, 22000), SetPeriapsis(orbit, 7000), SetInclination(orbit, 0.6),
        SetCircularInclination(circular, 0.6), MatchPlanes(orbit, target),
        transfer.first, transfer.second, intercept.first, intercept.second, SetPeriapsis(escape, 7000)}) {
        if (!first_maneuver) std::cout << ',';
        PrintVector(maneuver.first);
        std::cout << ',' << maneuver.second;
        first_maneuver = false;
    }
    std::cout << "]}" << '\n';
}
