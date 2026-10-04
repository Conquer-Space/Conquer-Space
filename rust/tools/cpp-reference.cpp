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
#include "core/actions/factoryconstructaction.h"
#include "core/components/colony.h"
#include "core/components/history.h"
#include "core/components/infrastructure.h"
#include "core/components/name.h"
#include "core/components/organizations.h"
#include "core/components/population.h"
#include "core/components/projects.h"
#include "core/components/spaceport.h"
#include "core/components/ships.h"
#include "core/components/surface.h"
#include "core/components/science.h"
#include "core/components/tags.h"
#include "core/components/market.h"
#include "core/components/modifier.h"
#include "core/components/orbit.h"
#include "core/components/pid.h"
#include "core/components/stardate.h"
#include "core/loading/loadorbit.h"
#include "core/loading/loadcities.h"
#include "core/loading/projectloader.h"
#include "core/loading/fields.h"
#include "core/loading/technology.h"
#include "core/loading/timezoneloader.h"
#include "core/systems/economy/economyconfig.h"
#include "core/systems/economy/sysspaceport.h"
#include "core/systems/economy/syslaunchvehicleproduction.h"
#include "core/systems/economy/sysinterplanetarytrade.h"
#include "core/systems/economy/sysfinance.h"
#include "core/systems/economy/sysmarketreset.h"
#include "core/systems/economy/syspopulation.h"
#include "core/systems/economy/sysproduction.h"
#include "core/systems/economy/sysmarket.h"
#include "core/systems/economy/sysplanetarytrade.h"
#include "core/systems/economy/syslabordistribution.h"
#include "core/systems/economy/syslabormarket.h"
#include "core/systems/economy/sysinfrastructure.h"
#include "core/systems/movement/sysorbit.h"
#include "core/util/orbit/groundtrack.h"
#include "core/util/random/stdrandom.h"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/null_sink.h>

using namespace cqsp::core::components;
void PrintVector(const glm::dvec3& vector);
Hjson::Value ReadInput() {
    return Hjson::Unmarshal(std::string(std::istreambuf_iterator<char>(std::cin), {}));
}
Hjson::Value Number(double value) {
    if (std::isfinite(value)) return value;
    return std::isnan(value) ? "nan" : value > 0 ? "inf" : "-inf";
}
Hjson::Value Numbers(std::initializer_list<double> values) {
    Hjson::Value row(Hjson::Type::Vector);
    for (double value : values) row.push_back(Number(value));
    return row;
}
Hjson::Value LedgerValues(const ResourceLedger& ledger, int count) {
    Hjson::Value row(Hjson::Type::Vector);
    for (int good = 0; good < count; ++good) row.push_back(Number(ledger[ToGoodEntity(good)]));
    return row;
}

// Legacy Value::to_string appends .0 after scientific exponents (e.g. 1e-05.0).
// Encode numeric output with the stream's 17-digit precision; leave upstream untouched.
void WriteJson(const Hjson::Value& value) {
    switch (value.type()) {
        case Hjson::Type::Double: std::cout << value.to_double(); break;
        case Hjson::Type::Int64: std::cout << value.to_int64(); break;
        case Hjson::Type::Vector:
            std::cout << '[';
            for (int index = 0; index < value.size(); ++index) {
                if (index > 0) std::cout << ',';
                WriteJson(value[index]);
            }
            std::cout << ']';
            break;
        case Hjson::Type::Map:
            std::cout << '{';
            for (int index = 0; index < value.size(); ++index) {
                if (index > 0) std::cout << ',';
                std::cout << Hjson::MarshalJson(Hjson::Value(value.key(index))) << ':';
                WriteJson(value[index]);
            }
            std::cout << '}';
            break;
        default: std::cout << Hjson::MarshalJson(value); break;
    }
}

// Suppress the reference Game's logging side effects, not its simulation behavior.
// This avoids original app-data log files and keeps stdout exclusively machine-readable.
namespace cqsp::core::util {
std::shared_ptr<spdlog::logger> make_logger(const std::string& name, bool) {
    return std::make_shared<spdlog::logger>(name, std::make_shared<spdlog::sinks::null_sink_mt>());
}
}

class ControlledRandom : public cqsp::core::util::IRandom {
 public:
    ControlledRandom() : IRandom(42) {}
    int GetRandomInt(int min, int) override { return min; }
    int GetRandomNormalInt(double mean, double sd) override { return static_cast<int>(std::round(GetRandomNormal(mean, sd))); }
    double GetRandomNormal(double mean, double sd) override { return mean + sd * 0.4; }
};

std::unique_ptr<cqsp::core::util::IRandom> ReferenceRandom(const Hjson::Value& seed) {
    if (seed.type() == Hjson::Type::Undefined || seed.type() == Hjson::Type::Null) {
        return std::make_unique<ControlledRandom>();
    }
    return std::make_unique<cqsp::core::util::StdRandom>(static_cast<int>(seed.to_int64()));
}

void RegisterGoods(cqsp::core::Universe& universe, const Hjson::Value& goods) {
    for (int index = 0; index < goods.size(); ++index) {
        auto entity = universe.create();
        universe.good_vector.push_back(entity);
        universe.good_map[entity] = ToGoodEntity(index);
        universe.emplace<Price>(entity, goods[index][0].to_double());
        if (goods[index][1].to_double() == 1) universe.emplace<LaborGood>(entity);
    }
}

Hjson::Value MarketValues(const Market& market, int count) {
    Hjson::Value value(Hjson::Type::Map);
    value["price"] = LedgerValues(market.price, count);
    value["supply"] = LedgerValues(market.supply, count);
    value["demand"] = LedgerValues(market.demand, count);
    value["ratio"] = LedgerValues(market.sd_ratio, count);
    value["trade"] = LedgerValues(market.trade, count);
    value["access"] = LedgerValues(market.market_access, count);
    value["shortages"] = LedgerValues(market.chronic_shortages, count);
    value["production"] = LedgerValues(market.production, count);
    value["consumption"] = LedgerValues(market.consumption, count);
    value["totals"] = Numbers({market.GDP, market.deficit, market.last_deficit, market.trade_deficit, market.last_trade_deficit});
    return value;
}

void PrintMarkets() {
    cqsp::core::Game game;
    auto& universe = game.GetUniverse();
    auto input = ReadInput();
    RegisterGoods(universe, input["goods"]);
    const int count = universe.GoodCount();
    auto& config = universe.economy_config.market_config;
    config.base_price_deviation = input["config"][0].to_double();
    config.shortage_level = input["config"][1].to_double();
    config.default_market_access = input["config"][2].to_double();
    auto planet = universe.create();
    universe.emplace<Market>(planet, count);
    universe.emplace<PlanetaryMarket>(planet, count);
    universe.emplace<Settlements>(planet);
    std::vector<entt::entity> provinces;
    for (int index = 0; index < input["provinces"].size(); ++index) {
        auto entity = universe.create(); provinces.push_back(entity);
        universe.emplace<Market>(entity, count);
        universe.get<Settlements>(planet).provinces.push_back(entity);
    }
    auto& port = universe.emplace<infrastructure::SpacePort>(provinces[0], count);
    for (int good = 0; good < count; ++good) {
        port.output_resources[ToGoodEntity(good)] = input["port"][0][good].to_double();
        port.output_resources_rate[ToGoodEntity(good)] = input["port"][1][good].to_double();
        port.demanded_resources_rate[ToGoodEntity(good)] = input["port"][2][good].to_double();
    }
    cqsp::core::systems::SysMarket local(game);
    cqsp::core::systems::SysPlanetaryTrade planetary(game);
    cqsp::core::systems::SysMarketReset reset(game);
    local.Init(); planetary.Init();
    Hjson::Value output(Hjson::Type::Map), initial(Hjson::Type::Vector), days(Hjson::Type::Vector);
    for (auto entity : provinces) initial.push_back(MarketValues(universe.get<Market>(entity), count));
    initial.push_back(MarketValues(universe.get<Market>(planet), count));
    output["initial"] = initial;
    for (int index = 0; index < provinces.size(); ++index) {
        auto& market = universe.get<Market>(provinces[index]);
        for (int good = 0; good < count; ++good) {
            market.trade[ToGoodEntity(good)] = input["provinces"][index][0][good].to_double();
            market.chronic_shortages[ToGoodEntity(good)] = input["provinces"][index][1][good].to_double();
            market.market_access[ToGoodEntity(good)] = input["provinces"][index][2][good].to_double();
        }
    }
    for (int day = 0; day < input["production"].size(); ++day) {
        reset.DoSystem();
        for (int index = 0; index < provinces.size(); ++index) {
            auto& market = universe.get<Market>(provinces[index]);
            market.GDP = 100.0 * index + day;
            for (int good = 0; good < count; ++good) {
                market.production[ToGoodEntity(good)] = input["production"][day][index][good].to_double();
                market.consumption[ToGoodEntity(good)] = input["consumption"][day][index][good].to_double();
            }
        }
        local.DoSystem();
        Hjson::Value result(Hjson::Type::Map), before(Hjson::Type::Vector), after(Hjson::Type::Vector);
        for (auto entity : provinces) before.push_back(MarketValues(universe.get<Market>(entity), count));
        result["local"] = before;
        universe.get<PlanetaryMarket>(planet).supplied_resources[ToGoodEntity(0)] = 8;
        planetary.DoSystem();
        for (auto entity : provinces) after.push_back(MarketValues(universe.get<Market>(entity), count));
        result["traded"] = after;
        result["planet"] = MarketValues(universe.get<Market>(planet), count);
        result["supplied"] = LedgerValues(universe.get<PlanetaryMarket>(planet).supplied_resources, count);
        result["port"] = Hjson::Value(Hjson::Type::Vector);
        result["port"].push_back(LedgerValues(port.output_resources, count));
        result["port"].push_back(LedgerValues(port.output_resources_rate, count));
        days.push_back(result);
    }
    output["days"] = days;
    WriteJson(output); std::cout << '\n';
}

void PrintInterplanetary() {
    cqsp::core::Game game;
    auto& universe = game.GetUniverse();
    auto input = ReadInput();
    RegisterGoods(universe, input["goods"]);
    const int count = universe.GoodCount();
    std::vector<entt::entity> bodies, ports;
    for (int index = 0; index < input["markets"].size(); ++index) {
        auto body = universe.create(); bodies.push_back(body);
        auto& market = universe.emplace<Market>(body, count);
        universe.emplace<PlanetaryMarket>(body, count);
        universe.emplace<Settlements>(body);
        for (int good = 0; good < count; ++good) {
            market.price[ToGoodEntity(good)] = input["markets"][index][0][good].to_double();
            market.production[ToGoodEntity(good)] = input["markets"][index][1][good].to_double();
            market.consumption[ToGoodEntity(good)] = input["markets"][index][2][good].to_double();
            market.chronic_shortages[ToGoodEntity(good)] = input["markets"][index][3][good].to_double();
        }
        // EnTT traverses creation in reverse: the primary port must be created last.
        for (int port_index = 0; port_index < 2; ++port_index) {
            auto port = universe.create(); ports.push_back(port);
            universe.emplace<infrastructure::SpacePort>(port, count).reference_body = body;
        }
    }
    cqsp::core::systems::SysInterplanetaryTrade system(game);
    Hjson::Value output(Hjson::Type::Vector);
    for (int day = 0; day < input["days"].to_double(); ++day) {
        system.DoSystem();
        Hjson::Value result(Hjson::Type::Map), differences(Hjson::Type::Vector), shipments(Hjson::Type::Vector);
        for (auto body : bodies) differences.push_back(LedgerValues(universe.get<PlanetaryMarket>(body).supply_difference, count));
        for (int index = 0; index < ports.size(); ++index) {
            Hjson::Value rows(Hjson::Type::Vector);
            for (const auto& [target, deliveries] : universe.get<infrastructure::SpacePort>(ports[index]).deliveries) {
                for (const auto& delivery : deliveries) rows.push_back(Numbers({
                    static_cast<double>(std::find(bodies.begin(), bodies.end(), target) - bodies.begin()),
                    static_cast<double>(universe.good_map.at(delivery.good)), delivery.amount, delivery.fulfilled, delivery.priority,
                    delivery.target_province == entt::null ? 1.0 : 0.0}));
            }
            shipments.push_back(rows);
        }
        result["differences"] = differences; result["shipments"] = shipments;
        output.push_back(result);
    }
    WriteJson(output); std::cout << '\n';
}

void PrintLabor() {
    cqsp::core::Game game;
    auto& universe = game.GetUniverse();
    auto input = ReadInput();
    RegisterGoods(universe, input["goods"]);
    const int count = universe.GoodCount();
    std::vector<entt::entity> jobs, provinces, populations;
    for (int index = 0; index < input["jobs"].size(); ++index) {
        auto entity = universe.create(); jobs.push_back(entity);
        universe.emplace<Labor>(entity).good = ToGoodEntity(input["jobs"][index].to_double());
    }
    universe.default_job = jobs[static_cast<size_t>(input["default_job"].to_double())];
    for (int index = 0; index < input["cases"].size(); ++index) {
        auto province = universe.create(); provinces.push_back(province);
        universe.emplace<Market>(province, count);
        universe.emplace<Settlement>(province);
        auto& sector = universe.emplace<infrastructure::ConstructionSector>(province);
        sector.construction_capacity = static_cast<int>(input["cases"][index][0].to_double());
        sector.construction_cost = 777.0;
        auto population = universe.create(); populations.push_back(population);
        universe.get<Settlement>(province).population.push_back(population);
        auto& segment = universe.emplace<PopulationSegment>(population);
        for (int job = 0; job < jobs.size(); ++job) segment.labor.labor_distribution[jobs[job]] = input["cases"][index][1][job].to_double();
    }
    cqsp::core::systems::SysLaborDistribution distribution(game);
    cqsp::core::systems::SysLaborMarket labor_market(game);
    cqsp::core::systems::InfrastructureSim infrastructure(game);
    cqsp::core::systems::SysMarketReset reset(game);
    distribution.Init(); labor_market.Init();
    Hjson::Value output(Hjson::Type::Vector);
    for (int week = 0; week < input["weeks"].size(); ++week) {
        reset.DoSystem();
        for (auto province : provinces) {
            auto& market = universe.get<Market>(province);
            for (int good = 0; good < count; ++good) {
                const auto& row = input["weeks"][week][good];
                market.price[ToGoodEntity(good)] = row[0].to_double();
                market.sd_ratio[ToGoodEntity(good)] = row[1].to_double();
                market.demand[ToGoodEntity(good)] = row[2].to_double();
                market.supply[ToGoodEntity(good)] = row[3].to_double();
                market.taxation[ToGoodEntity(good)] = row[4].to_double();
            }
        }
        infrastructure.DoSystem();
        // Purchases change consumption, not the controlled market-demand ledger.
        distribution.DoSystem(); labor_market.DoSystem();
        Hjson::Value rows(Hjson::Type::Vector);
        for (int index = 0; index < provinces.size(); ++index) {
            const auto& segment = universe.get<PopulationSegment>(populations[index]);
            const auto& market = universe.get<Market>(provinces[index]);
            const auto& sector = universe.get<infrastructure::ConstructionSector>(provinces[index]);
            Hjson::Value row(Hjson::Type::Map), workers(Hjson::Type::Vector), demands(Hjson::Type::Vector);
            ResourceLedger hours(count);
            for (const auto& [good, amount] : segment.labor.labor_hours) hours[good] += amount;
            double employed = 0.0, workforce = 0.0;
            for (auto job : jobs) {
                auto amount = segment.labor.labor_distribution.at(job);
                workers.push_back(amount);
                workforce += amount;
                employed += amount / market.sd_ratio[universe.get<Labor>(job).good];
                double demand = 0;
                for (const auto& [entity, value] : universe.get<Settlement>(provinces[index]).job_demands) if (entity == job) demand = value;
                demands.push_back(demand);
            }
            row["workers"] = workers; row["hours"] = LedgerValues(hours, count); row["demands"] = demands;
            row["employed"] = segment.employed_amount;
            // Negative/nonfinite float-to-uint64 conversion is undefined; never bless its result.
            double rate = (1.0 - employed) / workforce;
            row["unemployment"] = rate >= 0.0 && std::isfinite(rate) ? Hjson::Value(segment.unemployment_rate) : Hjson::Value(Hjson::Type::Null);
            row["consumption"] = LedgerValues(market.consumption, count);
            row["construction_cost"] = sector.construction_cost;
            rows.push_back(row);
        }
        output.push_back(rows);
    }
    WriteJson(output); std::cout << '\n';
}

void PrintGroundTracks() {
    auto input = ReadInput();
    Hjson::Value output(Hjson::Type::Vector);
    for (int index = 0; index < input.size(); ++index) {
        const auto& row = input[index];
        bodies::Body body{};
        body.axial = row[0].to_double(); body.rotation = row[1].to_double(); body.rotation_offset = row[2].to_double();
        auto rotation = cqsp::core::util::GetBodyRotation(row[3].to_double(), body);
        glm::vec3 position(row[4].to_double(), row[5].to_double(), row[6].to_double());
        auto ground = cqsp::core::util::GetGroundTrack(rotation, position);
        output.push_back(Numbers({rotation.x, rotation.y, rotation.z, rotation.w, ground.r_latitude(), ground.r_longitude()}));
    }
    WriteJson(output); std::cout << '\n';
}

void PrintFactoryCreation() {
    spdlog::set_level(spdlog::level::off);
    cqsp::core::Universe universe("factory-reference");
    auto input = ReadInput();
    cqsp::core::Node province(universe);
    province.emplace<IndustrialZone>();
    Hjson::Value output(Hjson::Type::Vector);
    for (int index = 0; index < input.size(); ++index) {
        const auto& row = input[index];
        cqsp::core::Node recipe(universe);
        recipe.emplace<Recipe>().type = static_cast<ProductionType>(row[2].to_int64());
        auto factory = cqsp::core::actions::CreateFactory(province, recipe, static_cast<int>(row[1].to_int64()));
        // These three arguments are the original Construction tab's attachment.
        auto& job = factory.emplace<Construction>(0, static_cast<int>(row[0].to_int64()), 0);
        const auto& production = factory.get<ProductionUnit>();
        auto& wallet = factory.get<Wallet>();
        output.push_back(Numbers({production.size, production.utilization, static_cast<double>(production.state),
            production.expertise, production.expertise_gain.value, production.max_expertise.value,
            production.revenue, production.material_costs, production.profit,
            wallet.GetBalance(), wallet.GetChange(), wallet.GetGDPChange(),
            static_cast<double>(job.progress), static_cast<double>(job.maximum), static_cast<double>(job.levels),
            static_cast<double>(province.get<IndustrialZone>().industries.size()),
            factory.all_of<Mine>() ? 1.0 : 0.0, factory.all_of<Factory>() ? 1.0 : 0.0, factory.all_of<Service>() ? 1.0 : 0.0}));
    }
    WriteJson(output); std::cout << '\n';
}

void PrintTimeZones() {
    spdlog::set_level(spdlog::level::off);
    cqsp::core::Universe universe("timezone-reference");
    auto input = ReadInput();
    cqsp::core::loading::TimezoneLoader loader(universe);
    loader.LoadHjson(input["entries"]);
    Hjson::Value output(Hjson::Type::Map), zones(Hjson::Type::Map);
    output["count"] = static_cast<int64_t>(universe.view<TimeZone>().size());
    for (const auto& [identifier, entity] : universe.time_zones) {
        auto offset = universe.get<TimeZone>(entity).time_diff;
        Hjson::Value zone(Hjson::Type::Map), times(Hjson::Type::Vector);
        zone["name"] = universe.all_of<Name>(entity) ? Hjson::Value(universe.get<Name>(entity).name) : Hjson::Value(Hjson::Type::Null);
        zone["offset"] = offset;
        for (int index = 0; index < input["ticks"].size(); ++index) {
            StarDate date;
            date.SetDate(static_cast<unsigned int>(input["ticks"][index].to_int64()));
            times.push_back(Numbers({static_cast<double>(date.GetHour(offset)), static_cast<double>(date.GetMinute())}));
        }
        zone["times"] = times;
        zones[identifier] = zone;
    }
    output["zones"] = zones;
    WriteJson(output); std::cout << '\n';
}

void PrintScience() {
    spdlog::set_level(spdlog::level::off);
    cqsp::core::Universe universe("science-reference");
    auto input = ReadInput();
    cqsp::core::loading::LoadFields(universe, input["fields"]);
    cqsp::core::loading::LoadTechnologies(universe, input["technologies"]);
    auto names = [&](const auto& entities) {
        Hjson::Value values(Hjson::Type::Vector);
        for (auto entity : entities) values.push_back(universe.get<Identifier>(entity).identifier);
        return values;
    };
    auto strings = [](const auto& items) {
        Hjson::Value values(Hjson::Type::Vector);
        for (const auto& item : items) values.push_back(item);
        return values;
    };
    auto metadata = [&](entt::entity entity) {
        Hjson::Value value(Hjson::Type::Map);
        value["name"] = universe.get<Name>(entity).name;
        value["description"] = universe.all_of<Description>(entity) ? Hjson::Value(universe.get<Description>(entity).description) : Hjson::Value(Hjson::Type::Null);
        return value;
    };
    Hjson::Value output(Hjson::Type::Map), fields(Hjson::Type::Map), technologies(Hjson::Type::Map);
    for (const auto& [identifier, entity] : universe.fields) {
        auto value = metadata(entity);
        const auto& field = universe.get<science::Field>(entity);
        value["parents"] = names(field.parents); value["adjacent"] = names(field.adjacent);
        fields[identifier] = value;
    }
    for (const auto& [identifier, entity] : universe.technologies) {
        auto value = metadata(entity);
        const auto& tech = universe.get<science::Technology>(entity);
        value["fields"] = names(tech.fields); value["actions"] = strings(tech.actions); value["difficulty"] = tech.difficulty;
        value["tags"] = universe.all_of<Tags>(entity) ? strings(universe.get<Tags>(entity).tags) : Hjson::Value(Hjson::Type::Vector);
        technologies[identifier] = value;
    }
    output["fields"] = fields; output["technologies"] = technologies;
    WriteJson(output); std::cout << '\n';
}

void PrintLunarMission() {
    spdlog::set_level(spdlog::level::off);
    cqsp::core::Game game;
    auto& universe = game.GetUniverse();
    auto input = ReadInput();
    universe.random = ReferenceRandom(input["seed"]);
    const bool seeded = input["seed"].type() != Hjson::Type::Null && input["seed"].type() != Hjson::Type::Undefined;
    std::vector<entt::entity> body_entities;
    for (int index = 0; index < input["bodies"].size(); ++index) {
        const auto& row = input["bodies"][index];
        auto entity = universe.create();
        body_entities.push_back(entity);
        auto& body = universe.emplace<bodies::Body>(entity);
        body.radius = row[0].to_double(); body.GM = row[1].to_double(); body.SOI = row[2].to_double();
        auto& orbit = universe.emplace<types::Orbit>(entity, row[3].to_double(), row[4].to_double(), row[5].to_double(),
            row[6].to_double(), row[7].to_double(), row[8].to_double(), index == 0 ? entt::null : body_entities[index == 2 ? 1 : 0]);
        orbit.GM = row[9].to_double();
        universe.emplace<types::Kinematics>(entity);
        universe.emplace<types::FuturePosition>(entity);
        universe.emplace<bodies::OrbitalSystem>(entity);
        if (index > 0) universe.get<bodies::OrbitalSystem>(orbit.reference_body).bodies.push_back(entity);
    }
    universe.sun = body_entities[0];
    universe.date.SetDate(0);
    cqsp::core::systems::SysOrbit movement(game);
    movement.DoSystem();
    auto country = universe.create();
    universe.emplace<MissionQueue>(country);
    auto& city_list = universe.emplace<CountryCityList>(country);
    auto source_province = universe.create();
    universe.emplace<Province>(source_province, country, body_entities[1]);
    auto city = universe.create();
    universe.emplace<City>(city, source_province);
    universe.emplace<infrastructure::SpacePort>(city, 0).reference_body = body_entities[1];
    city_list.space_port_list.push_back(city);
    auto target = universe.create();
    universe.emplace<Province>(target, entt::null, body_entities[2]);
    universe.emplace<ColonizationTarget>(target, country);
    universe.emplace<Settlements>(body_entities[2]).provinces.push_back(target);
    auto mission = universe.create();
    universe.emplace<Mission>(mission, entt::null, body_entities[2], target);
    universe.emplace<ColonyCoreModule>(mission);
    universe.emplace<Name>(mission, "Colony Core");
    universe.get<MissionQueue>(country).list.push_back(mission);
    cqsp::core::systems::SysSpacePort ports(game);
    entt::entity ship = entt::null;
    int landing_tick = -1;
    int unload_tick = -1;
    std::cout << "{\"samples\":[";
    bool first = true;
    for (int tick = 0; tick < StarDate::DAY * 30; ++tick) {
        universe.date.SetDate(tick);
        if (tick % ports.Interval() == 0) ports.DoSystem();
        if (ship == entt::null && !universe.view<ships::Ship>().empty()) ship = universe.view<ships::Ship>().front();
        movement.DoSystem();
        if (!universe.all_of<types::Orbit>(ship)) {
            if (landing_tick < 0) landing_tick = tick;
            if (!universe.all_of<ships::CargoHold>(ship)) { unload_tick = tick; break; }
            continue;
        }
        if (seeded || tick % 24 == 0 || tick < 10) {
            if (!first) std::cout << ',';
            const auto& orbit = universe.get<types::Orbit>(ship);
            const auto& position = universe.get<types::Kinematics>(ship);
            const auto& queue = universe.get<CommandQueue>(ship);
            std::cout << '[' << tick << ',' << (orbit.reference_body == body_entities[2] ? 2 : orbit.reference_body == body_entities[1] ? 1 : 0) << ','
                << orbit.semi_major_axis << ',' << orbit.eccentricity << ',' << orbit.inclination << ',' << orbit.LAN << ','
                << orbit.w << ',' << orbit.M0 << ',' << orbit.epoch << ',' << orbit.v << ',';
            PrintVector(position.position); std::cout << ','; PrintVector(position.velocity);
            std::cout << ',' << queue.maneuvers.size() << ',' << queue.commands.size() << ','
                << (queue.maneuvers.empty() ? -1.0 : queue.maneuvers.front().time) << ']';
            first = false;
        }
    }
    auto step = universe.get<ColonizationTarget>(target).steps;
    int core_count = universe.all_of<Colony>(target) ? universe.get<Colony>(target).components.size() : 0;
    std::cout << "],\"delivery\":[" << landing_tick << ',' << unload_tick << ',' << static_cast<int>(step) << ',' << core_count << ','
        << (universe.all_of<MissionInProgress>(mission) ? 1 : 0) << "]}\n";
}

void PrintPopulation() {
    spdlog::set_level(spdlog::level::off);
    cqsp::core::Game game;
    auto& universe = game.GetUniverse();
    auto input = ReadInput();
    int count = input["goods"].size();
    for (int index = 0; index < count; ++index) {
        auto entity = universe.create();
        universe.good_vector.push_back(entity);
        universe.good_map[entity] = ToGoodEntity(index);
        const auto& row = input["goods"][index];
        if (row[1].type() != Hjson::Type::Null) {
            universe.emplace<ConsumerGood>(entity, row[1].to_double(), row[2].to_double());
            universe.consumergoods.push_back(entity);
        }
    }
    auto province = universe.create();
    universe.emplace<Settlement>(province);
    universe.emplace<infrastructure::CityInfrastructure>(province);
    auto& market = universe.emplace<Market>(province, count);
    std::vector<entt::entity> populations;
    for (int index = 0; index < input["populations"].size(); ++index) {
        const auto& row = input["populations"][index];
        auto entity = universe.create();
        populations.push_back(entity);
        universe.get<Settlement>(province).population.push_back(entity);
        auto& segment = universe.emplace<PopulationSegment>(entity);
        segment.population = row[0].to_double(); segment.standard_of_living = row[1].to_double();
        segment.income = row[3].to_double(); segment.spending = row[4].to_double(); segment.employed_amount = row[5].to_double();
        for (int good = 0; good < count; ++good) segment.labor.labor_hours.emplace_back(ToGoodEntity(good), row[6][good].to_double());
        universe.emplace<Wallet>(entity) = row[2].to_double();
    }
    cqsp::core::systems::SysPopulationConsumption consumption(game);
    cqsp::core::systems::SysWalletReset wallets(game);
    cqsp::core::systems::SysMarketReset markets(game);
    consumption.Init();
    Hjson::Value output(Hjson::Type::Vector);
    for (int day = 0; day < input["days"].to_double(); ++day) {
        universe.date.SetDate(day * StarDate::DAY);
        wallets.DoSystem(); markets.DoSystem();
        for (int good = 0; good < count; ++good) {
            market.price[ToGoodEntity(good)] = input["goods"][good][0].to_double() * (1.0 + day * 0.05);
            market.taxation[ToGoodEntity(good)] = input["goods"][good][3].to_double();
        }
        consumption.DoSystem();
        Hjson::Value result(Hjson::Type::Map), rows(Hjson::Type::Vector);
        for (auto entity : populations) {
            const auto& segment = universe.get<PopulationSegment>(entity);
            auto& wallet = universe.get<Wallet>(entity);
            rows.push_back(Numbers({wallet.GetBalance(), wallet.GetChange(), wallet.GetGDPChange(), segment.standard_of_living,
                segment.income, segment.spending, segment.average_wage, segment.sol_pid.proportional, segment.sol_pid.integral, segment.sol_pid.derivative}));
        }
        result["populations"] = rows;
        result["production"] = LedgerValues(market.production, count);
        result["consumption"] = LedgerValues(market.consumption, count);
        const auto& history = universe.ctx().at<PopulationHistory>();
        result["history"] = Numbers({history.population.back(), history.sol.back(), history.employment.back(), history.employment_rate.back()});
        output.push_back(result);
    }
    WriteJson(output); std::cout << '\n';
}

void PrintManufacturing() {
    spdlog::set_level(spdlog::level::off);
    cqsp::core::Game game;
    auto& universe = game.GetUniverse();
    auto input = ReadInput();
    auto province = universe.create();
    universe.emplace<Market>(province, 1).price[ToGoodEntity(0)] = 10.0;
    auto project_template = universe.create();
    universe.emplace<ProjectTemplate>(project_template).cost.emplace_back(ToGoodEntity(0), 1000.0);
    std::vector<entt::entity> cities, projects, results;
    for (int index = 0; index < input["projects"].size(); ++index) {
        const auto& row = input["projects"][index];
        auto city = universe.create(); cities.push_back(city);
        universe.emplace<City>(city, row[0].to_double() == 1 ? province : entt::null);
        universe.emplace<infrastructure::SpacePort>(city, 1);
        auto result = universe.create(); results.push_back(result);
        auto project = universe.create(); projects.push_back(project);
        auto& component = universe.emplace<Project>(project);
        component.max_progress = row[1].to_double(); component.result = result;
        component.project_template = row[2].to_double() == 1 ? project_template : entt::null;
        universe.get<infrastructure::SpacePort>(city).projects.push_back(project);
    }
    cqsp::core::systems::SysLaunchVehicleProduction production(game);
    Hjson::Value output(Hjson::Type::Vector);
    for (int day = 0; day < input["days"].to_double(); ++day) {
        production.DoSystem();
        Hjson::Value rows(Hjson::Type::Vector);
        for (int index = 0; index < cities.size(); ++index) {
            auto& port = universe.get<infrastructure::SpacePort>(cities[index]);
            Hjson::Value row(Hjson::Type::Vector);
            if (universe.valid(projects[index])) row.push_back(universe.get<Project>(projects[index]).progress);
            else row.push_back(Hjson::Value(Hjson::Type::Null));
            row.push_back(port.projects.size()); row.push_back(port.stored_launch_vehicles.size());
            row.push_back(universe.get<Market>(province).consumption.GetSum());
            rows.push_back(row);
        }
        output.push_back(rows);
    }
    WriteJson(output); std::cout << '\n';
}

void PrintProduction() {
    spdlog::set_level(spdlog::level::off);
    cqsp::core::Game game;
    auto& universe = game.GetUniverse();
    auto input = ReadInput();
    universe.random = ReferenceRandom(input["seed"]);
    const int count = input["prices"].size();
    auto recipe_entity = universe.create();
    auto& recipe = universe.emplace<Recipe>(recipe_entity);
    auto read_vector = [](const Hjson::Value& values, ResourceVector& target) {
        for (int index = 0; index < values.size(); ++index) target.emplace_back(ToGoodEntity(values[index][0].to_double()), values[index][1].to_double());
    };
    const auto& recipe_input = input["recipe"];
    read_vector(recipe_input["input"], recipe.input);
    read_vector(recipe_input["capital"], recipe.capitalcost);
    recipe.output = {ToGoodEntity(recipe_input["output"][0].to_double()), recipe_input["output"][1].to_double()};
    recipe.power_cost = recipe_input["power"].to_double();
    for (int index = 0; index < recipe_input["workers"].size(); ++index) {
        auto labor = universe.create();
        universe.emplace<Labor>(labor).good = ToGoodEntity(recipe_input["workers"][index][0].to_double());
        recipe.workers.workers.emplace_back(labor, static_cast<uint32_t>(recipe_input["workers"][index][1].to_double()));
    }
    auto& cost = universe.emplace<ConstructionCost>(recipe_entity);
    cost.time = recipe_input["construction_time"].to_double();
    read_vector(recipe_input["construction_cost"], cost.cost);
    auto& config = universe.economy_config.production_config;
    config.profit_multiplier = input["config"][0].to_double();
    config.max_factory_delta = input["config"][1].to_double();
    config.factory_min_utilization = input["config"][2].to_double();
    std::vector<entt::entity> provinces, industries, countries;
    for (int index = 0; index < input["cases"].size(); ++index) {
        const auto& row = input["cases"][index];
        auto country = universe.create(); countries.push_back(country);
        universe.emplace<OrganizationIncome>(country);
        auto province = universe.create(); provinces.push_back(province);
        universe.emplace<Province>(province, country);
        universe.emplace<Settlement>(province).population.push_back(country); // Only the nonempty guard is used by this system.
        universe.emplace<infrastructure::CityInfrastructure>(province);
        auto& sector = universe.emplace<infrastructure::ConstructionSector>(province);
        sector.construction_capacity = 1000; sector.construction_cost = 2.0;
        universe.emplace<Market>(province, count);
        universe.emplace<IndustrialZone>(province);
        auto industry = universe.create(); industries.push_back(industry);
        universe.get<IndustrialZone>(province).industries.push_back(industry);
        universe.emplace<Wallet>(industry);
        universe.emplace<Employer>(industry);
        auto& production = universe.emplace<ProductionUnit>(industry);
        production.recipe = recipe_entity;
        production.state = static_cast<IndustryState>(static_cast<int>(row[0].to_double()));
        production.size = row[1].to_double(); production.utilization = row[2].to_double();
        production.profit = row[3].to_double(); production.revenue = row[4].to_double();
        production.continuous_gains = row[5].to_double(); production.stability = row[6].to_double();
        production.cumulative_pr = row[7].to_double(); production.shortage = row[8].to_double() == 1;
        production.output_subsidy = row[9].to_double();
        if (row[10].type() != Hjson::Type::Null) universe.emplace<Construction>(industry,
            static_cast<int>(row[10][0].to_double()), static_cast<int>(row[10][1].to_double()), static_cast<int>(row[10][2].to_double()));
    }
    cqsp::core::systems::SysProduction production(game);
    cqsp::core::systems::SysWalletReset wallets(game);
    cqsp::core::systems::SysMarketReset markets(game);
    Hjson::Value output(Hjson::Type::Vector);
    for (int day = 0; day < input["days"].to_double(); ++day) {
        wallets.DoSystem(); markets.DoSystem();
        for (auto province : provinces) {
            auto& market = universe.get<Market>(province);
            for (int good = 0; good < count; ++good) {
                market.price[ToGoodEntity(good)] = input["prices"][good].to_double() * (1.0 + day * 0.01);
                market.taxation[ToGoodEntity(good)] = 0.05;
                market.chronic_shortages[ToGoodEntity(good)] = good == input["shortage_good"].to_double() && day >= 2 && day <= 4 ? 7.0 : 0.0;
            }
        }
        production.DoSystem();
        Hjson::Value rows(Hjson::Type::Vector);
        for (int index = 0; index < industries.size(); ++index) {
            auto industry = industries[index];
            const auto& value = universe.get<ProductionUnit>(industry);
            auto& wallet = universe.get<Wallet>(industry);
            const auto& market = universe.get<Market>(provinces[index]);
            Hjson::Value row(Hjson::Type::Map);
            row["industry"] = Numbers({static_cast<double>(value.state), value.size, value.utilization, value.diff,
                value.shortage ? 1.0 : 0.0, value.cumulative_pr, static_cast<double>(value.continuous_gains), static_cast<double>(value.stability),
                value.expertise, value.revenue, value.material_costs, value.wage_cost, value.tax_cost, value.construction_cost,
                value.profit, value.output_subsidy_amount, value.amount_sold, value.power_consumption,
                wallet.GetBalance(), wallet.GetChange(), wallet.GetGDPChange()});
            row["construction"] = Hjson::Value(Hjson::Type::Null);
            if (universe.all_of<Construction>(industry)) {
                const auto& job = universe.get<Construction>(industry);
                row["construction"] = Numbers({static_cast<double>(job.progress), static_cast<double>(job.maximum), static_cast<double>(job.levels)});
            }
            row["production"] = LedgerValues(market.production, count);
            row["consumption"] = LedgerValues(market.consumption, count);
            row["totals"] = Numbers({market.GDP, universe.get<OrganizationIncome>(countries[index]).income_taxes,
                static_cast<double>(universe.get<infrastructure::ConstructionSector>(provinces[index]).current_construction)});
            rows.push_back(row);
        }
        output.push_back(rows);
    }
    WriteJson(output); std::cout << '\n';
}

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
    cqsp::core::loading::TimezoneLoader timezone_loader(universe);
    std::ifstream timezone_stream(data / "data/timezones.hjson");
    timezone_loader.LoadHjson(Hjson::Unmarshal(std::string(std::istreambuf_iterator<char>(timezone_stream), {})));
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
            << universe.all_of<Settlement>(city) << ',' << universe.all_of<IndustrialZone>(city) << ',';
        auto* zone = universe.try_get<CityTimeZone>(city);
        if (zone && universe.all_of<TimeZone>(zone->time_zone)) std::cout << std::quoted(universe.get<Identifier>(zone->time_zone).identifier);
        else std::cout << "null";
        std::cout << ']';
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

void PrintRandom() {
    auto input = ReadInput();
    Hjson::Value result(Hjson::Type::Vector);
    for (int index = 0; index < input["seeds"].size(); ++index) {
        auto seed = static_cast<uint32_t>(input["seeds"][index].to_int64());
        std::mt19937 engine(seed);
        cqsp::core::util::StdRandom random(static_cast<int>(seed));
        Hjson::Value row(Hjson::Type::Map);
        row["words"] = Hjson::Value(Hjson::Type::Vector);
        for (int sample = 0; sample < input["words"].to_int64(); ++sample) {
            row["words"].push_back(static_cast<int64_t>(engine()));
        }
        row["canonical"] = Hjson::Value(Hjson::Type::Vector);
        for (int sample = 0; sample < 1000; ++sample) {
            row["canonical"].push_back(std::generate_canonical<double, 53>(engine));
        }
        row["mixed"] = Hjson::Value(Hjson::Type::Vector);
        for (int sample = 0; sample < input["samples"].to_int64(); ++sample) {
            Hjson::Value values(Hjson::Type::Vector);
            values.push_back(random.GetRandomNormal(0, 1));
            values.push_back(random.GetRandomNormal(0, 0.000005));
            values.push_back(random.GetRandomNormal(6900, 60));
            values.push_back(random.GetRandomNormalInt(-5.5, 20));
            values.push_back(random.GetRandomInt(-13, 27));
            values.push_back(random.GetRandomInt(INT_MIN, INT_MAX));
            values.push_back(random.GetRandomInt(-5, -5));
            // Large odd range exercises rejection in the integer distribution.
            values.push_back(random.GetRandomInt(INT_MIN, 1));
            row["mixed"].push_back(values);
        }
        result.push_back(row);
    }
    WriteJson(result);
}

int main(int argc, char** argv) {
    std::cout << std::setprecision(17);
    if (argc == 2 && std::string(argv[1]) == "--factory-creation") { PrintFactoryCreation(); return 0; }
    if (argc == 2 && std::string(argv[1]) == "--timezones") { PrintTimeZones(); return 0; }
    if (argc == 2 && std::string(argv[1]) == "--random") { PrintRandom(); return 0; }
    if (argc == 2 && std::string(argv[1]) == "--markets") { PrintMarkets(); return 0; }
    if (argc == 2 && std::string(argv[1]) == "--interplanetary") { PrintInterplanetary(); return 0; }
    if (argc == 2 && std::string(argv[1]) == "--labor") { PrintLabor(); return 0; }
    if (argc == 2 && std::string(argv[1]) == "--ground-tracks") { PrintGroundTracks(); return 0; }
    if (argc == 2 && std::string(argv[1]) == "--science") { PrintScience(); return 0; }
    if (argc == 2 && std::string(argv[1]) == "--population") { PrintPopulation(); return 0; }
    if (argc == 2 && std::string(argv[1]) == "--manufacturing") { PrintManufacturing(); return 0; }
    if (argc == 2 && std::string(argv[1]) == "--production") { PrintProduction(); return 0; }
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
    if (argc == 2 && std::string(argv[1]) == "--lunar-mission") {
        PrintLunarMission();
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
