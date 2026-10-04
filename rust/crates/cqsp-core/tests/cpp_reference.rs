//! Differential checks against the actual legacy components (upstream 74b678f).
use cqsp_core::{
    World,
    clock::{DAY, StarDate},
    default_data_path,
    economy::{GoodId, Ledger, Market, MarketConfig, Pid, ProductionConfig, Wallet},
    modifiers::{ModifiableValue, ModifierId, ModifierOp},
    orbit::Orbit,
    simulation::{Simulation, SurfaceKind},
    solar_system::SolarSystem,
};
use glam::DVec3;
use serde_json::{Value, json};
use std::{
    fmt::Write as _,
    fs,
    io::Write as _,
    path::Path,
    process::{Command, Stdio},
};

#[test]
#[ignore = "Run tools/build-reference.ps1 first; compares actual goods/labor/zoning/recipe/config loaders"]
fn economic_catalog_and_defaults_match_original_loaders() {
    use cqsp_core::{
        catalog::{Catalog, ProductionType},
        read_entries,
    };
    let data = default_data_path();
    let config: Value =
        serde_hjson::from_str(&fs::read_to_string(data.join("data/economy_config.hjson")).unwrap())
            .unwrap();
    let zoning: Value =
        serde_hjson::from_str(&fs::read_to_string(data.join("data/real_estate.hjson")).unwrap())
            .unwrap();
    let bundled = json!({
        "config": config, "zoning": zoning,
        "goods": read_entries::<Value>(&data.join("data/goods")).unwrap(),
        "jobs": read_entries::<Value>(&data.join("data/labor")).unwrap(),
        "recipes": read_entries::<Value>(&data.join("data/recipes")).unwrap(),
    });
    let controlled = serde_json::from_str(include_str!("fixtures/catalog.json")).unwrap();
    for input in [bundled, controlled] {
        let catalog = Catalog::from_definitions(
            serde_json::from_value(input.clone()).unwrap(),
            serde_json::from_value(input["config"].clone()).unwrap(),
        )
        .unwrap();
        let goods: Vec<_> = catalog.goods.iter().map(|good| {
            let consumption = good.consumption.as_ref().map(|value| [value.autonomous_consumption, value.marginal_propensity]);
            let kinds: Vec<_> = ["mineral", "raw", "capital", "labor", "fluid", "intermediate"].map(|tag| usize::from(good.tags.iter().any(|value| value == tag))).into();
            json!({"identifier":good.identifier, "name":good.name, "price":good.price,
                "physical":[good.mass.unwrap_or(0.0),good.volume.unwrap_or(0.0)],
                "energy":good.energy, "unit":good.unit, "consumption":consumption, "tags":good.tags, "kinds":kinds})
        }).collect();
        let jobs: Vec<_> = catalog.jobs.iter().map(|job| json!({"identifier":job.identifier, "name":job.name, "good":catalog.goods[job.good.0].identifier})).collect();
        let resources = |values: &[(GoodId, f64)]| -> serde_json::Map<String, Value> {
            values
                .iter()
                .map(|&(good, amount)| (catalog.goods[good.0].identifier.clone(), json!(amount)))
                .collect()
        };
        let recipes: serde_json::Map<_, _> = catalog.recipes.iter().map(|recipe| {
            let workers: serde_json::Map<_, _> = recipe.workers.iter().map(|&(job, count)| (catalog.jobs[job.0].identifier.clone(), json!(count))).collect();
            let kind = match recipe.kind { ProductionType::Factory => 0, ProductionType::Mine => 1, ProductionType::Service => 2 };
            (recipe.identifier.clone(), json!({"name":recipe.name,"input":resources(&recipe.input),"output":[recipe.output.0.0 as f64,recipe.output.1],
                "capital":resources(&recipe.capital),"power":recipe.power,"kind":kind,"fixed":resources(&recipe.fixed_cost),"scaling":resources(&recipe.scaling_cost),
                "workers":workers,"construction_ticks":recipe.construction_ticks,"construction_cost":resources(&recipe.construction_cost),"zoning":recipe.zoning}))
        }).collect();
        let good_ids: serde_json::Map<_, _> = catalog
            .goods
            .iter()
            .map(|good| {
                (
                    good.identifier.clone(),
                    json!(catalog.good(&good.identifier).unwrap().0),
                )
            })
            .collect();
        let market = catalog.config.market_config;
        let production = catalog.config.production_config;
        compare(
            &json!({"goods":goods,"good_ids":good_ids,"jobs":jobs,"recipes":recipes,"zoning":catalog.zoning,"default_job":catalog.jobs[catalog.default_job.0].identifier,
            "config":[market.base_price_deviation,market.shortage_level,market.default_market_access,production.profit_multiplier,production.max_factory_delta,production.factory_min_utilization,production.construction_limit as f64,production.underutilization_limit]}),
            &reference_output(Some("--catalog"), &input.to_string()),
        );
    }
}

#[test]
#[ignore = "Run tools/build-reference.ps1 first; compares actual power/grid/province infrastructure loaders"]
fn province_zoning_transport_power_and_grid_match_original_loaders() {
    use cqsp_core::{
        catalog::Catalog,
        infrastructure::{InfrastructureCatalog, SurfaceInfrastructure, ZoningAllocation},
        read_entries,
    };
    let data = default_data_path();
    let catalog = Catalog::load(&data).unwrap();
    let mut provinces = read_entries::<Value>(&data.join("data/planet_data/provinces")).unwrap();
    provinces.retain(|row| !(row["identifier"] == "name" && row["color"] == "color"));
    let controlled = [
        json!({"identifier":"empty","planet":"earth","infrastructure":{},"zoning":{"undeveloped":-1,"missing":7}}),
        json!({"identifier":"default_grid","planet":"earth","transport":0,"infrastructure":{"power_plants":[{"type":"coal_power_plant","size":2.5},{"type":"coal_power_plant","size":0}]}}),
        json!({"identifier":"missing_grid","planet":"moon","transport":-0.5,"infrastructure":{"power_grid":"missing","power_plants":[{"type":"coal_power_plant","size":-1}]}}),
        json!({"identifier":"wide_zoning","planet":"earth","zoning":{"commercial":4294967297_i64}}),
    ];
    provinces.extend(controlled);
    // Isolate infrastructure from unrelated population, ownership, color and random amenability.
    for province in &mut provinces {
        let object = province.as_object_mut().unwrap();
        object.retain(|key, _| {
            [
                "identifier",
                "name",
                "planet",
                "infrastructure",
                "transport",
                "zoning",
            ]
            .contains(&key.as_str())
        });
        object.insert("country".into(), json!(""));
        object.insert("color".into(), json!([0, 0, 0]));
    }
    let input = json!({"goods":read_entries::<Value>(&data.join("data/goods")).unwrap(),
        "zoning":serde_hjson::from_str::<Value>(&fs::read_to_string(data.join("data/real_estate.hjson")).unwrap()).unwrap(),
        "power_plants":read_entries::<Value>(&data.join("data/infrastructure/powerplant")).unwrap(),
        "grids":read_entries::<Value>(&data.join("data/infrastructure/powergrid")).unwrap(),"bodies":["earth","moon"],"provinces":provinces});
    let infrastructure = InfrastructureCatalog::from_definitions(
        serde_json::from_value(input.clone()).unwrap(),
        &catalog,
    )
    .unwrap();
    let recipes: serde_json::Map<_, _> = infrastructure.power_plants.iter().map(|(identifier, recipe)| {
        let consumption: serde_json::Map<_, _> = recipe.consumption.iter().map(|&(good, amount)| (catalog.goods[good.0].identifier.clone(), json!(amount))).collect();
        (identifier.clone(), json!({"name":recipe.name,"capacity":recipe.maximum_production,"consumption":consumption}))
    }).collect();
    let grids: serde_json::Map<_, _> = infrastructure
        .grids
        .iter()
        .map(|(identifier, grid)| (identifier.clone(), json!(grid.name)))
        .collect();
    let sites: Vec<_> = input["provinces"].as_array().unwrap().iter().map(|province| {
        let transport = province["transport"].as_f64();
        let value = province.get("infrastructure").cloned();
        let site = SurfaceInfrastructure::load(transport, value, &infrastructure).unwrap();
        let plants: Vec<_> = site.power_plants.iter().map(|plant| [plant.production, plant.maximum_production, plant.consumption.len() as f64]).collect();
        let allocations = ZoningAllocation::load(serde_json::from_value(province.get("zoning").cloned().unwrap_or(json!({}))).unwrap(), &catalog);
        let zoning: serde_json::Map<_, _> = allocations.into_iter().map(|(zone, area)| (zone,json!([area.allocated,area.filled]))).collect();
        json!({"transport":[site.purchase_cost,site.improvement],"grid":site.power_grid,"plants":plants,"zoning":zoning})
    }).collect();
    compare(
        &json!({"recipes":recipes,"grids":grids,"provinces":sites}),
        &reference_output(Some("--infrastructure"), &input.to_string()),
    );
}

#[test]
#[ignore = "Run tools/build-reference.ps1 first; compares original time-zone loader and clock"]
fn timezone_definitions_and_clock_match_original() {
    use cqsp_core::timezones::TimeZones;
    let entries: Vec<Value> = serde_hjson::from_str(
        &fs::read_to_string(default_data_path().join("data/timezones.hjson")).unwrap(),
    )
    .unwrap();
    let controlled = vec![
        json!({"identifier":"same", "name":"First", "offset":-11}),
        json!({"identifier":"same", "name":"Last", "offset":"5.5"}),
        json!({"identifier":"negative", "offset":-3.5}),
        json!({"identifier":"boolean", "offset":true}),
        json!({"identifier":"missing"}),
        json!({"identifier":"null", "offset":null}),
        json!({"identifier":"invalid", "offset":"2e+05.0"}),
        json!({"name":"No identifier"}),
    ];
    let ticks = [0, 1, 143, 144, 145, 1008, 1_000_000, u32::MAX as u64];
    for entries in [entries, controlled] {
        let zones = TimeZones::from_entries(entries.clone()).unwrap();
        let definitions: serde_json::Map<_, _> = zones
            .entries
            .iter()
            .map(|zone| {
                let times: Vec<_> = ticks
                    .iter()
                    .map(|&tick| {
                        let (hour, minute) = zone.local_time(StarDate::from_tick(tick));
                        json!([hour, minute])
                    })
                    .collect();
                (
                    zone.identifier.clone(),
                    json!({"name":zone.name,"offset":zone.offset,"times":times}),
                )
            })
            .collect();
        compare(
            &json!({"count":zones.entries.len(),"zones":definitions}),
            &reference_output(
                Some("--timezones"),
                &json!({"entries":entries,"ticks":ticks}).to_string(),
            ),
        );
    }
}

#[test]
#[ignore = "Run tools/build-reference.ps1 first; compares actual CreateFactory and original UI attachment"]
fn manual_factory_creation_matches_original_action() {
    use cqsp_core::catalog::{ProductionType, RecipeId};
    let data = default_data_path();
    let mut simulation = Simulation::load(&data, &World::load(&data).unwrap(), 42).unwrap();
    let province = simulation
        .settlements
        .iter()
        .position(|site| site.is_province())
        .unwrap();
    simulation.settlements[province].industries.clear();
    let mut input = Vec::new();
    let mut output = Vec::new();
    for recipe in 0..simulation.catalog.recipes.len() {
        for count in [1, 27, 1000] {
            let definition = &simulation.catalog.recipes[recipe];
            let kind = match definition.kind {
                ProductionType::Mine => 1,
                ProductionType::Service => 2,
                _ => 0,
            };
            input.push(json!([definition.construction_ticks, count, kind]));
            let factory = simulation
                .construct_factory(province, RecipeId(recipe), count)
                .unwrap();
            let site = &simulation.settlements[province];
            let industry = &site.industries[factory];
            let job = industry.construction.as_ref().unwrap();
            output.push(json!([
                industry.size,
                industry.utilization,
                industry.state as usize,
                industry.expertise,
                industry.expertise_gain.value(),
                industry.max_expertise.value(),
                industry.revenue,
                industry.material_costs,
                industry.profit,
                industry.wallet.balance,
                industry.wallet.change,
                industry.wallet.gdp_change,
                job.progress,
                job.maximum,
                job.levels,
                site.industries.len(),
                i32::from(kind == 1),
                i32::from(kind == 0),
                i32::from(kind == 2)
            ]));
        }
    }
    compare(
        &json!(output),
        &reference_output(Some("--factory-creation"), &json!(input).to_string()),
    );
}

#[test]
#[ignore = "Run tools/build-reference.ps1 first; compares actual ApplySubsidy and UI ledger deletion"]
fn national_recipe_subsidies_match_original_action_and_removal() {
    use cqsp_core::catalog::RecipeId;
    let data = default_data_path();
    let mut simulation = Simulation::load(&data, &World::load(&data).unwrap(), 42).unwrap();
    let count = simulation.catalog.recipes.len();
    for site in 0..3 {
        simulation.settlements[site].industries.clear();
        for recipe in 0..count {
            simulation
                .construct_factory(site, RecipeId(recipe), 1)
                .unwrap();
        }
    }
    simulation.countries[0].provinces = vec![0, 2];
    simulation.countries[1].provinces = vec![1];
    simulation.countries[2].provinces.clear();
    simulation.settlements[0].country = Some(1);
    let mut actions = Vec::new();
    for recipe in 0..count {
        for amount in [0.0_f32, 0.2, 0.333_333_34, 1.0, -0.25, 1.5] {
            actions.push(json!({"kind":"apply","country":0,"recipe":recipe,"amount":amount}));
        }
        actions.push(json!({"kind":"create","country":0,"recipe":recipe,"site":0}));
        actions.push(json!({"kind":"remove","country":0,"recipe":recipe}));
        actions.push(json!({"kind":"apply","country":1,"recipe":recipe,"amount":0.7}));
        actions.push(json!({"kind":"apply","country":2,"recipe":recipe,"amount":0.9}));
        actions.push(json!({"kind":"apply","country":0,"recipe":recipe,"amount":0.4}));
        actions.push(json!({"kind":"remove","country":0,"recipe":recipe}));
        actions.push(json!({"kind":"remove","country":0,"recipe":recipe}));
    }
    let mut output = Vec::new();
    for action in &actions {
        let recipe = RecipeId(action["recipe"].as_u64().unwrap() as usize);
        let country = action["country"].as_u64().unwrap() as usize;
        match action["kind"].as_str().unwrap() {
            "apply" => {
                simulation
                    .apply_subsidy(country, recipe, action["amount"].as_f64().unwrap() as f32)
                    .unwrap();
            }
            "remove" => {
                simulation.remove_subsidy(country, recipe).unwrap();
            }
            "create" => {
                simulation
                    .construct_factory(action["site"].as_u64().unwrap() as usize, recipe, 1)
                    .unwrap();
            }
            _ => unreachable!(),
        }
        let policies: Vec<_> = simulation.countries[..3]
            .iter()
            .map(|country| {
                (0..count)
                    .map(|recipe| country.subsidies.get(&RecipeId(recipe)).copied())
                    .collect::<Vec<_>>()
            })
            .collect();
        let factories: Vec<_> = simulation.settlements[..3]
            .iter()
            .map(|site| {
                site.industries
                    .iter()
                    .map(|industry| industry.output_subsidy)
                    .collect::<Vec<_>>()
            })
            .collect();
        output.push(json!({"policies":policies,"factories":factories}));
    }
    compare(
        &json!(output),
        &reference_output(
            Some("--subsidies"),
            &json!({"recipes":count,"actions":actions}).to_string(),
        ),
    );
}

#[test]
#[ignore = "Run tools/build-reference.ps1 first; compares actual GetMetadata/LoadMetadata and empty SaveGame"]
fn save_metadata_only_matches_original_and_ignores_versions() {
    use cqsp_core::save::Metadata;
    let input = [
        json!({}),
        json!({"date":0,"uuid":"abcdef"}),
        json!({"date":143,"uuid":"012345","version":"old"}),
        json!({"date":144,"uuid":"new","version":"future"}),
        json!({"date":143.75,"uuid":null}),
        json!({"date":-1,"uuid":true}),
        json!({"date":4294967295_u64,"uuid":123}),
        json!({"date":4294967297_u64}),
        json!({"date":"144","uuid":"abc"}),
        json!({"date":"1e3","uuid":"abc"}),
        json!({"date":"invalid","uuid":"abc"}),
        json!({"date":true,"uuid":"abc"}),
        json!({"date":null,"uuid":"abc"}),
    ];
    let output: Vec<_> = input.iter().map(|value| {
            let Ok(metadata) = Metadata::parse(value) else { return json!({"error":true}); };
        let mut date = StarDate::default();
        let mut uuid = String::new();
        metadata.apply(&mut date, &mut uuid);
        json!({"metadata":Metadata::new(date, uuid), "empty_game":false,"path":Path::new("fixture").join("meta.hjson")})
    }).collect();
    compare(
        &json!(output),
        &reference_output(Some("--save-metadata"), &json!(input).to_string()),
    );
}

#[test]
#[ignore = "Run tools/build-reference.ps1 first; reads actual clientctx tick_speeds and PauseOptions"]
fn playback_settings_match_original_client_context() {
    use cqsp_core::clock::PlaybackSpeed;
    let speeds: Vec<_> = PlaybackSpeed::ALL
        .into_iter()
        .map(PlaybackSpeed::milliseconds)
        .collect();
    compare(
        &json!({"speeds":speeds,"default_index":PlaybackSpeed::DEFAULT_INDEX,"running":false,"tick_once":false}),
        &reference_output(Some("--playback-settings"), ""),
    );
}

#[test]
#[ignore = "Run tools/build-reference.ps1 first; compares actual country/province/city metadata loaders"]
fn civilization_wallet_capital_capability_and_owned_lists_match_original() {
    let data = default_data_path();
    let world = World::load(&data).unwrap();
    let simulation = Simulation::load(&data, &world, 42).unwrap();
    let countries = cqsp_core::read_entries::<Value>(&data.join("data/countries")).unwrap();
    let provinces: Vec<_> = cqsp_core::read_entries::<Value>(&data.join("data/planet_data/provinces")).unwrap().into_iter()
        .filter(|row| !(row["identifier"] == "name" && row["color"] == "color"))
        .map(|row| json!({"identifier":row["identifier"],"name":row["name"].as_str().unwrap_or(row["identifier"].as_str().unwrap()),"planet":row["planet"],"country":row["country"],"tags":row["tags"],"color":[0,0,0]}))
        .collect();
    let cities = cqsp_core::read_entries::<Value>(&data.join("data/cities")).unwrap();
    let bodies: Vec<_> = simulation
        .solar_system
        .bodies
        .iter()
        .map(|body| &body.identifier)
        .collect();
    let output: serde_json::Map<_, _> = world.countries.iter().enumerate().map(|(index, country)| {
        let economy = &simulation.countries[index];
        let sites: Vec<_> = simulation.settlements.iter().filter(|site| site.governor == Some(index)).collect();
        (country.identifier.clone(), json!({
            "capital":economy.capital.map(|site| &simulation.settlements[site].identifier),
            "wallet":economy.wallet.map(|wallet| [wallet.balance,wallet.change,wallet.gdp_change]),
            "space_program":economy.space_program,"taxes":[economy.income_taxes,economy.consumption_taxes],"research":economy.research,
            "provinces":sites.iter().filter(|site| site.is_province()).map(|site| &site.identifier).collect::<Vec<_>>(),
            "cities":sites.iter().filter(|site| !site.is_province()).map(|site| &site.identifier).collect::<Vec<_>>(),
        }))
    }).collect();
    compare(
        &json!(output),
        &reference_output(
            Some("--civilizations"),
            &json!({"countries":countries,"provinces":provinces,"cities":cities,"bodies":bodies})
                .to_string(),
        ),
    );
}

#[test]
#[ignore = "Run tools/build-reference.ps1 first; compares all-body market history and inactive trade exclusion"]
fn all_body_market_histories_match_original_daily_system() {
    use cqsp_core::{catalog::Catalog, spaceflight::PlanetaryMarket};
    let catalog = Catalog::load(&default_data_path()).unwrap();
    let sites: Vec<_> = (0..11).map(|body| body == 0 || body == 4).collect();
    let mut bodies: Vec<_> = sites
        .iter()
        .map(|&sites| PlanetaryMarket::new(&catalog, sites))
        .collect();
    let mut provinces: Vec<_> = sites
        .iter()
        .map(|&sites| {
            sites.then(|| {
                Market::new(
                    &catalog.base_prices,
                    &catalog.labor_goods,
                    catalog.config.market_config,
                )
            })
        })
        .collect();
    let mut output = Vec::new();
    for day in 0..12 {
        for (body, (planetary, province)) in bodies.iter_mut().zip(&mut provinces).enumerate() {
            planetary.market.reset();
            if let Some(province) = province {
                province.reset();
                province.gdp = (day * 100 + body) as f64;
                for good in 0..catalog.goods.len() {
                    province.production[GoodId(good)] = (((day + body + good) % 4) * 3) as f64;
                    province.consumption[GoodId(good)] = (((day + good) % 3) * 2) as f64;
                }
                province.process(&catalog.base_prices, catalog.config.market_config);
                planetary.process(&catalog.base_prices, [(province as &Market, None)]);
                province.trade_with(&planetary.market);
            }
            planetary
                .history
                .record(StarDate::from_tick(day as u64 * DAY), &planetary.market);
        }
        output.push(bodies.iter().map(|body| {
            let sample = body.history.0.last().unwrap();
            json!({"market":market_values(&body.market),"history":{"gdp":sample.gdp,"length":body.history.0.len(),
                "price":ledger_values(&sample.price),"supply":ledger_values(&sample.supply),"demand":ledger_values(&sample.demand),"ratio":ledger_values(&sample.sd_ratio)}})
        }).collect::<Vec<_>>());
    }
    let goods: Vec<_> = catalog
        .goods
        .iter()
        .map(|good| {
            json!([
                good.price,
                i32::from(good.tags.iter().any(|tag| tag == "labor"))
            ])
        })
        .collect();
    compare(
        &json!(output),
        &reference_output(
            Some("--planet-histories"),
            &json!({"goods":goods,"sites":sites,"days":12}).to_string(),
        ),
    );
}

fn snapshot() -> Value {
    let mut wallet = Wallet::default();
    wallet.assign(100.0);
    wallet.reset();
    wallet.spend(25.0);
    wallet.assign(50.0);
    wallet.scale(0.5);
    let mut additive = ModifiableValue::new(1.5, ModifierOp::Additive);
    additive.apply(ModifierId(0), 1.0);
    additive.apply(ModifierId(0), 1.0);
    let mut modifiers = vec![additive.value()];
    additive.remove(ModifierId(0));
    modifiers.push(additive.value());
    additive.set_base(2.0);
    modifiers.push(additive.value());
    additive.offset(0.25);
    modifiers.push(additive.value());
    additive.scale(2.0);
    modifiers.push(additive.value());
    let mut multiplicative = ModifiableValue::new(0.0001, ModifierOp::Multiplicative);
    multiplicative.apply(ModifierId(0), -0.8);
    multiplicative.apply(ModifierId(1), -0.8);
    modifiers.push(multiplicative.value());
    multiplicative.set_base(0.01);
    modifiers.push(multiplicative.value());
    multiplicative.remove(ModifierId(0));
    modifiers.push(multiplicative.value());
    multiplicative.scale(0.5);
    modifiers.push(multiplicative.value());
    multiplicative.set_base(-0.02);
    modifiers.push(multiplicative.value());
    let mut pid = Pid::default();
    let responses: Vec<_> = [0.4, -0.8, 1.2, -2.0, 0.0]
        .map(|error| {
            pid.update(error);
            pid.value([0.01, 0.1, 0.01])
        })
        .into();
    let mut base = Ledger::new(3);
    for (index, price) in [10.0, 20.0, 5.0].into_iter().enumerate() {
        base[GoodId(index)] = price;
    }
    let mut market = Market::new(&base, &[], MarketConfig::default());
    market.taxation[GoodId(0)] = 0.1;
    market.taxation[GoodId(2)] = 0.25;
    let (cost, tax) = market.purchase(&[(GoodId(0), 2.0), (GoodId(1), 3.0), (GoodId(2), 4.0)]);
    let purchase = [cost, tax, market.consumption.sum()];
    market.trade[GoodId(0)] = -3.0;
    market.chronic_shortages[GoodId(0)] = 7.0;
    market.reset();
    let mut supply = Ledger::new(3);
    let mut demand = Ledger::new(3);
    supply[GoodId(0)] = 10.0;
    supply[GoodId(1)] = 2.0;
    demand[GoodId(0)] = 5.0;
    let ratios = supply.safe_division(&demand, f64::INFINITY);
    let config = ProductionConfig::default();
    let mut date = StarDate::default();
    date.advance();
    let first = [date.tick() as f64, date.seconds(), date.days()];
    date = StarDate::from_tick(144);
    let time = StarDate::from_tick(143);
    let (year, month, day) = StarDate::from_tick((365 * 2 + 59) * DAY).calendar();
    let orbit = Orbit {
        semi_major_axis: 12000.0,
        eccentricity: 0.25,
        inclination: 0.3,
        ascending_node: 1.1,
        periapsis_argument: 0.7,
        mean_anomaly: 0.5,
        epoch: 123.0,
        mu: 398600.44188,
        ..Orbit::default()
    };
    let state = orbit.state_at(4321.0);
    let recovered = Orbit::from_state(state, orbit.mu, 4321.0).unwrap();
    let impulse = orbit
        .apply_impulse(DVec3::new(0.01, 0.04, -0.02), 4321.0)
        .unwrap();
    let mut timed = orbit;
    timed.update(600.0);
    let mut circular = Orbit {
        eccentricity: 0.0,
        ..orbit
    };
    circular.update(600.0);
    let mut target = Orbit {
        semi_major_axis: 18000.0,
        eccentricity: 0.1,
        inclination: 0.6,
        ascending_node: 1.4,
        periapsis_argument: 0.3,
        mean_anomaly: 1.4,
        ..orbit
    };
    target.update(600.0);
    let transfer = circular.unsafe_hohmann_transfer(22000.0);
    let intercept = circular.coplanar_intercept(target, 600.0);
    let maneuvers: Vec<_> = [
        timed.circularize_at_apoapsis(),
        timed.circularize_at_periapsis(),
        timed.set_apoapsis(22000.0),
        timed.set_periapsis(7000.0),
        timed.set_inclination(0.6),
        circular.set_circular_inclination(0.6),
        timed.match_planes(target),
        transfer[0],
        transfer[1],
        intercept[0],
        intercept[1],
        Orbit {
            semi_major_axis: -12000.0,
            eccentricity: 1.5,
            ..orbit
        }
        .set_periapsis(7000.0),
    ]
    .into_iter()
    .flat_map(|burn| [burn.delta_v.x, burn.delta_v.y, burn.delta_v.z, burn.delay])
    .collect();
    json!({
        "wallet": [wallet.balance, wallet.change, wallet.gdp_change],
        "modifiers": modifiers,
        "pid": responses,
        "purchase": purchase,
        "reset": [market.consumption.sum(), market.trade[GoodId(0)], market.chronic_shortages[GoodId(0)], market.price[GoodId(0)]],
        "ratios": [ratios[GoodId(0)], f64::from(ratios[GoodId(1)].is_infinite()), f64::from(ratios[GoodId(2)].is_infinite())],
        "utilization": [config.minimum_utilization(10.0), config.minimum_utilization(100.0), config.utilization_diff(-500.0), config.utilization_diff(0.0), config.utilization_diff(500.0)],
        "date": [first[0], first[1], first[2], date.seconds(), date.days(), StarDate::ticks_from_seconds(3600.0) as f64],
        "time": [time.hour(0.0), time.minute() as i32, time.hour(-24.0)],
        "calendar": [year, month, day],
        "orbit_elliptic": orbit_samples(orbit, &[0.0, 123.0, 600.0, 86400.0, -600.0]),
        "orbit_circular": orbit_samples(Orbit { eccentricity: 0.0, ..orbit }, &[0.0, 600.0, 86400.0]),
        "orbit_hyperbolic": orbit_samples(Orbit { semi_major_axis: -12000.0, eccentricity: 1.5, ..orbit }, &[-1000.0, 123.0, 2000.0]),
        "orbit_recovered": orbit_elements(recovered),
        "orbit_impulse": orbit_elements(impulse),
        "orbit_timing": [timed.period(), timed.mean_motion(), timed.apoapsis(), timed.periapsis(), timed.time_to_anomaly(2.5)],
        "maneuvers": maneuvers
    })
}

fn orbit_samples(orbit: Orbit, times: &[f64]) -> Vec<f64> {
    times
        .iter()
        .flat_map(|&time| {
            let anomaly = orbit.true_anomaly_at(time);
            let state = orbit.state_at_anomaly(anomaly);
            [
                anomaly,
                orbit.radius_at(anomaly),
                orbit.speed_at(anomaly),
                state.position.x,
                state.position.y,
                state.position.z,
                state.velocity.x,
                state.velocity.y,
                state.velocity.z,
            ]
        })
        .collect()
}

fn orbit_elements(orbit: Orbit) -> [f64; 9] {
    [
        orbit.semi_major_axis,
        orbit.eccentricity,
        orbit.inclination,
        orbit.ascending_node,
        orbit.periapsis_argument,
        orbit.mean_anomaly,
        orbit.epoch,
        orbit.true_anomaly,
        orbit.mu,
    ]
}

fn compare(actual: &Value, expected: &Value) {
    compare_with_tolerance(actual, expected, 1e-12);
}

#[test]
#[ignore = "Run tools/build-reference.ps1 first; compares actual StdRandom and MSVC distributions"]
fn random_stream_and_mixed_distributions_match_original_msvc() {
    use cqsp_core::random::LegacyRandom;
    let seeds = [0_u64, 1, 42, 5489, i32::MAX as u64, u32::MAX as u64];
    let words = 2000;
    let samples = 5000;
    let reference = reference_output(
        Some("--random"),
        &json!({"seeds": seeds, "words": words, "samples": samples}).to_string(),
    );
    for (index, seed) in seeds.into_iter().enumerate() {
        let mut engine = LegacyRandom::new(seed);
        let raw: Vec<_> = (0..words).map(|_| engine.next_u32()).collect();
        assert_eq!(
            json!(raw),
            reference[index]["words"],
            "MT19937 words at seed {seed}"
        );
        let canonical: Vec<_> = (0..1000).map(|_| engine.canonical()).collect();
        compare(&json!(canonical), &reference[index]["canonical"]);
        let mut random = LegacyRandom::new(seed);
        let mixed: Vec<_> = (0..samples)
            .map(|_| {
                json!([
                    random.normal(0.0, 1.0),
                    random.normal(0.0, 0.000005),
                    random.normal(6900.0, 60.0),
                    random.normal_int(-5.5, 20.0),
                    random.int(-13, 27),
                    random.int(i32::MIN, i32::MAX),
                    random.int(-5, -5),
                    random.int(i32::MIN, 1),
                ])
            })
            .collect();
        compare(&json!(mixed), &reference[index]["mixed"]);
    }
}

fn compare_with_tolerance(actual: &Value, expected: &Value, tolerance: f64) {
    compare_at(actual, expected, "$", tolerance);
}

fn compare_at(actual: &Value, expected: &Value, path: &str, tolerance: f64) {
    match (actual, expected) {
        (Value::Number(a), Value::Number(b)) => {
            let (a, b) = (a.as_f64().unwrap(), b.as_f64().unwrap());
            assert!(
                (a - b).abs() <= tolerance * b.abs().max(1.0),
                "{path}: Rust {a}, C++ {b}"
            );
        }
        (Value::Array(a), Value::Array(b)) => {
            assert_eq!(a.len(), b.len(), "{path}");
            for (index, (a, b)) in a.iter().zip(b).enumerate() {
                compare_at(a, b, &format!("{path}[{index}]"), tolerance);
            }
        }
        (Value::Object(a), Value::Object(b)) => {
            assert_eq!(
                a.keys().collect::<Vec<_>>(),
                b.keys().collect::<Vec<_>>(),
                "{path}"
            );
            for (key, b) in b {
                compare_at(&a[key], b, &format!("{path}.{key}"), tolerance);
            }
        }
        _ => assert_eq!(actual, expected, "{path}"),
    }
}

#[test]
#[ignore = "Run tools/build-reference.ps1 first; compares original float body rotation/ground track"]
fn body_rotations_and_ground_tracks_match_original_float_helpers() {
    let mut system = SolarSystem::load(&default_data_path()).unwrap();
    system.bodies[system.sun.0].rotation_period = 0.0;
    let mut inputs = Vec::new();
    let mut samples = Vec::new();
    for body in &system.bodies {
        for time in [-86400.0, 0.0, 600.0, 86400.0, 365.0 * 86400.0] {
            for position in [
                DVec3::X * 7000.0,
                DVec3::Y * 7000.0,
                DVec3::new(4000.0, -5000.0, 3000.0),
            ] {
                inputs.push([
                    body.axial_tilt,
                    body.rotation_period,
                    body.rotation_offset,
                    time,
                    position.x,
                    position.y,
                    position.z,
                ]);
                let rotation = body.orientation(time);
                let [latitude, longitude] = body.ground_track(position, time).unwrap();
                samples.push([
                    rotation.x as f64,
                    rotation.y as f64,
                    rotation.z as f64,
                    rotation.w as f64,
                    latitude,
                    longitude,
                ]);
            }
        }
    }
    assert_eq!(samples.len(), 165);
    // Original helpers are float GLM operations, unlike double orbital comparisons.
    compare_with_tolerance(
        &json!(samples),
        &reference_output(Some("--ground-tracks"), &json!(inputs).to_string()),
        2e-6,
    );
}

#[test]
fn matches_recorded_cpp_components() {
    compare(
        &snapshot(),
        &serde_json::from_str(include_str!("fixtures/cpp-components.json")).unwrap(),
    );
}

#[test]
#[ignore = "Run tools/build-reference.ps1 first; compares actual science-field/technology loaders"]
fn science_definitions_and_reference_rules_match_original_loaders() {
    use cqsp_core::science::ScienceCatalog;
    let data = default_data_path();
    let read = |folder: &str| {
        let mut files: Vec<_> = fs::read_dir(data.join(folder))
            .unwrap()
            .map(|entry| entry.unwrap().path())
            .collect();
        files.sort();
        files
            .into_iter()
            .filter(|path| {
                path.extension()
                    .is_some_and(|extension| extension == "hjson")
            })
            .flat_map(|path| {
                serde_hjson::from_str::<Vec<Value>>(&fs::read_to_string(path).unwrap()).unwrap()
            })
            .collect::<Vec<_>>()
    };
    let scenarios = [
        (read("data/science/fields"), read("data/science/technology")),
        (
            vec![
                json!({"name":"A", "identifier":"a", "parent":["b","b","a","missing"]}),
                json!({"name":"B", "identifier":"b", "adjacent":["a"]}),
                json!({"identifier":"skipped"}),
            ],
            vec![
                json!({"name":"T", "identifier":"t", "fields":["a","a","b"], "difficulty":10.9, "tags":["test"]}),
                json!({"name":"Defaults", "identifier":"defaults"}),
            ],
        ),
    ];
    for (fields, technologies) in scenarios {
        let science = ScienceCatalog::from_entries(&fields, &technologies).unwrap();
        compare(
            &science.definitions_json(),
            &reference_output(
                Some("--science"),
                &json!({"fields":fields, "technologies":technologies}).to_string(),
            ),
        );
    }
}

#[test]
#[ignore = "Run tools/build-reference.ps1 first; requires the legacy C++ dependencies and Visual Studio"]
fn matches_live_cpp_components() {
    compare(&snapshot(), &reference_output(None, ""));
}

fn reference_output(mode: Option<&str>, input: &str) -> Value {
    let executable =
        Path::new(env!("CARGO_MANIFEST_DIR")).join("../../target/cpp-reference/cpp-reference.exe");
    let mut command = Command::new(executable);
    if let Some(mode) = mode {
        command.arg(mode);
        if mode == "--surface-assets" {
            command.arg(default_data_path());
        }
    }
    let mut reference = command
        .stdin(Stdio::piped())
        .stdout(Stdio::piped())
        .stderr(Stdio::piped())
        .spawn()
        .expect("Build the C++ reference harness first");
    reference
        .stdin
        .take()
        .unwrap()
        .write_all(input.as_bytes())
        .unwrap();
    let output = reference.wait_with_output().unwrap();
    assert!(
        output.status.success(),
        "{}",
        String::from_utf8_lossy(&output.stderr)
    );
    serde_json::from_slice(&output.stdout).unwrap_or_else(|error| {
        let text = String::from_utf8_lossy(&output.stdout);
        let context = text
            .lines()
            .nth(error.line().saturating_sub(1))
            .unwrap_or("");
        panic!("Invalid reference JSON: {error}: {context}")
    })
}

fn ledger_values(ledger: &Ledger) -> Value {
    Value::Array(
        ledger
            .values()
            .iter()
            .map(|&value| {
                if value.is_finite() {
                    json!(value)
                } else {
                    json!(if value.is_nan() {
                        "nan"
                    } else if value > 0.0 {
                        "inf"
                    } else {
                        "-inf"
                    })
                }
            })
            .collect(),
    )
}

fn market_values(market: &Market) -> Value {
    json!({
        "price": ledger_values(&market.price), "supply": ledger_values(&market.supply),
        "demand": ledger_values(&market.demand), "ratio": ledger_values(&market.sd_ratio),
        "trade": ledger_values(&market.trade), "access": ledger_values(&market.market_access),
        "shortages": ledger_values(&market.chronic_shortages),
        "production": ledger_values(&market.production), "consumption": ledger_values(&market.consumption),
        "totals": [market.gdp, market.deficit, market.last_deficit, market.trade_deficit, market.last_trade_deficit]
    })
}

#[test]
#[ignore = "Run tools/build-reference.ps1 first; compares original weekly labor and construction demand"]
fn labor_drift_hours_demands_and_construction_costs_match_original_systems() {
    use cqsp_core::{catalog::JobId, economy::industry::ConstructionSector};
    let data = default_data_path();
    let mut simulation = Simulation::load(&data, &World::load(&data).unwrap(), 42).unwrap();
    let cases = [
        (0, [1_000_000, 2_000_000, 3_000_000, 4_000_000, 5_000_000]),
        (1, [100, 250, 5, 0, 1000]),
        (10, [0, 0, 0, 0, 1]),
        (
            1000,
            [
                200_000_000,
                100_000_000,
                100_000_000,
                100_000_000,
                100_000_000,
            ],
        ),
        (0, [1001, 999, 1001, 1, 0]),
        (0, [0; 5]),
    ];
    let catalog = &simulation.catalog;
    let count = catalog.goods.len();
    assert_eq!(catalog.jobs.len(), 5);
    let mut populations: Vec<_> = simulation
        .settlements
        .iter_mut()
        .flat_map(|site| std::mem::take(&mut site.populations))
        .take(cases.len())
        .collect();
    assert_eq!(populations.len(), cases.len());
    for (population, (_, workers)) in populations.iter_mut().zip(cases) {
        population.jobs = workers
            .into_iter()
            .enumerate()
            .map(|(index, amount)| (JobId(index), amount))
            .collect();
    }
    let mut sectors: Vec<_> = cases
        .iter()
        .map(|&(capacity, _)| ConstructionSector {
            capacity,
            current: 0,
            cost: 777.0,
        })
        .collect();
    let mut markets: Vec<_> = cases
        .iter()
        .map(|_| {
            Market::new(
                &catalog.base_prices,
                &catalog.labor_goods,
                catalog.config.market_config,
            )
        })
        .collect();
    let default_labor = catalog.jobs[catalog.default_job.0].good;
    let mut weeks = Vec::new();
    let mut samples = Vec::new();
    for week in 0..12 {
        let mut values: Vec<_> = catalog
            .base_prices
            .values()
            .iter()
            .map(|&price| [price, 1.0, 0.0, 0.0, 0.0])
            .collect();
        for (index, job) in catalog.jobs.iter().enumerate() {
            values[job.good.0] = [
                if week >= 4 && JobId(index) == catalog.default_job {
                    20.0
                } else {
                    10.0
                },
                if week == 8 {
                    1e8
                } else {
                    [1.0, 2.0, 4.0, 12.0, 2.5][index]
                },
                if week == 0 || week >= 9 { 0.0 } else { 1000.0 },
                (index * 7 + week) as f64,
                0.05 + week as f64 * 0.01,
            ];
        }
        for market in &mut markets {
            market.reset();
            for (index, &[price, ratio, demand, supply, tax]) in values.iter().enumerate() {
                let good = GoodId(index);
                market.price[good] = price;
                market.sd_ratio[good] = ratio;
                market.demand[good] = demand;
                market.supply[good] = supply;
                market.taxation[good] = tax;
            }
        }
        for (sector, market) in sectors.iter_mut().zip(&mut markets) {
            sector.purchase_labor(market, default_labor);
        }
        weeks.push(values);
        let mut rows = Vec::new();
        for (index, population) in populations.iter_mut().enumerate() {
            let market = &markets[index];
            population.redistribute_labor(market, catalog);
            let mut hours = Ledger::new(count);
            hours.add(&population.labor_hours);
            let workers: Vec<_> = (0..catalog.jobs.len())
                .map(|job| population.jobs[&JobId(job)])
                .collect();
            let demands: Vec<_> = catalog
                .jobs
                .iter()
                .map(|job| market.labor_demand(job.good))
                .collect();
            rows.push(json!({"workers": workers, "hours": hours.values(), "employed": population.employed,
                "unemployment": population.unemployment_rate, "demands": demands,
                "consumption": market.consumption.values(), "construction_cost": sectors[index].cost}));
        }
        samples.push(rows);
    }
    let goods: Vec<_> = catalog
        .base_prices
        .values()
        .iter()
        .enumerate()
        .map(|(index, price)| {
            json!([
                price,
                usize::from(catalog.labor_goods.contains(&GoodId(index)))
            ])
        })
        .collect();
    let jobs: Vec<_> = catalog.jobs.iter().map(|job| job.good.0).collect();
    compare(
        &json!(samples),
        &reference_output(
            Some("--labor"),
            &json!({
        "goods": goods, "jobs": jobs, "default_job": catalog.default_job.0, "cases": cases,
        "weeks": weeks})
            .to_string(),
        ),
    );
}

#[test]
#[ignore = "Run tools/build-reference.ps1 first; compares actual local and planetary trade systems"]
fn local_markets_and_planetary_trade_match_original_systems() {
    use cqsp_core::{
        catalog::Catalog,
        solar_system::BodyId,
        spaceflight::{PlanetaryMarket, SpacePort},
    };
    let mut catalog = Catalog::load(&default_data_path()).unwrap();
    let config = MarketConfig {
        base_price_deviation: 0.4,
        shortage_level: 0.8,
        default_market_access: 0.3,
    };
    catalog.config.market_config = config;
    catalog.labor_goods = vec![GoodId(5)];
    for (index, price) in [10.0, 20.0, 30.0, 5.0, 100.0, 2.0].into_iter().enumerate() {
        catalog.base_prices[GoodId(index)] = price;
    }
    let count = catalog.goods.len();
    let ledger = |values: &[f64]| {
        let mut ledger = Ledger::new(count);
        for (index, &value) in values.iter().enumerate() {
            ledger[GoodId(index)] = value;
        }
        ledger
    };
    let mut planetary = PlanetaryMarket::new(&catalog, true);
    let mut markets: Vec<_> = (0..3)
        .map(|_| Market::new(&catalog.base_prices, &catalog.labor_goods, config))
        .collect();
    let initial: Vec<_> = markets
        .iter()
        .chain([&planetary.market])
        .map(market_values)
        .collect();
    let provinces: Vec<_> = markets
        .iter_mut()
        .enumerate()
        .map(|(index, market)| {
            market.trade = ledger(&[-3.0, 9.0, 0.0, 2.0]);
            market.chronic_shortages = ledger(&[8.0, 8.0]);
            for good in 0..count {
                market.market_access[GoodId(good)] = if good == 5 {
                    0.0
                } else {
                    [0.0, 0.25, 1.0][index]
                };
            }
            json!([
                market.trade.values(),
                market.chronic_shortages.values(),
                market.market_access.values()
            ])
        })
        .collect();
    let mut port = SpacePort::new(BodyId(0), count);
    port.output_resources = ledger(&[9.0, 0.5]);
    port.output_resources_rate = ledger(&[3.0, 1.0]);
    port.demanded_resources_rate = ledger(&[0.0, 2.0]);
    let port_input = json!([
        port.output_resources.values(),
        port.output_resources_rate.values(),
        port.demanded_resources_rate.values()
    ]);
    let production_base = [
        [0.0, 10.0, 0.0, 20.0, 0.001, 40.0],
        [100.0, 0.0, 20.0, 5.0, 0.0, 10.0],
        [5.0, 20.0, 0.0, 1.0, 0.0, 5.0],
    ];
    let consumption_base = [
        [10.0, 0.0, 0.0, 100.0, 0.0, 40.0],
        [20.0, 30.0, 0.0, 2.0, 0.001, 10.0],
        [10.0, 5.0, 0.0, 1.0, 0.0, 5.0],
    ];
    let mut production = Vec::new();
    let mut consumption = Vec::new();
    let mut days = Vec::new();
    for day in 0..12 {
        let (supply_scale, demand_scale) =
            [(1.0, 1.0), (0.01, 1.0), (3.0, 1.0), (0.0, 0.0)][day % 4];
        planetary.market.reset();
        for (index, market) in markets.iter_mut().enumerate() {
            market.reset();
            market.gdp = (100 * index + day) as f64;
            market.production = ledger(&production_base[index].map(|value| value * supply_scale));
            market.consumption = ledger(&consumption_base[index].map(|value| value * demand_scale));
        }
        production.push(
            markets
                .iter()
                .map(|market| market.production.values().to_vec())
                .collect::<Vec<_>>(),
        );
        consumption.push(
            markets
                .iter()
                .map(|market| market.consumption.values().to_vec())
                .collect::<Vec<_>>(),
        );
        port.process_market(&mut markets[0]);
        for market in &mut markets {
            market.process(&catalog.base_prices, config);
        }
        let local: Vec<_> = markets.iter().map(market_values).collect();
        planetary.supplied_resources[GoodId(0)] = 8.0;
        planetary.process(
            &catalog.base_prices,
            markets
                .iter()
                .enumerate()
                .map(|(index, market)| (market, (index == 0).then_some(&port))),
        );
        for market in &mut markets {
            market.trade_with(&planetary.market);
        }
        let traded: Vec<_> = markets.iter().map(market_values).collect();
        days.push(json!({"local": local, "traded": traded, "planet": market_values(&planetary.market),
            "supplied": planetary.supplied_resources.values(), "port": [port.output_resources.values(), port.output_resources_rate.values()]}));
    }
    let goods: Vec<_> = catalog
        .base_prices
        .values()
        .iter()
        .enumerate()
        .map(|(index, price)| json!([price, usize::from(index == 5)]))
        .collect();
    let reference = reference_output(Some("--markets"), &json!({"goods": goods,
        "config": [config.base_price_deviation, config.shortage_level, config.default_market_access],
        "provinces": provinces, "port": port_input, "production": production, "consumption": consumption}).to_string());
    compare(&json!({"initial": initial, "days": days}), &reference);
}

#[test]
#[ignore = "Run tools/build-reference.ps1 first; compares actual interplanetary order resolution"]
fn interplanetary_orders_and_first_port_allocation_match_original_system() {
    use cqsp_core::{
        catalog::Catalog,
        solar_system::BodyId,
        spaceflight::{PlanetaryMarket, Shipment, SpacePort},
    };
    let catalog = Catalog::load(&default_data_path()).unwrap();
    let count = catalog.goods.len();
    let inputs = [
        [
            [10.0, 30.0, 30.0],
            [100.0, 0.0, 0.0],
            [0.0, 0.0, 0.0],
            [0.0, 0.0, 0.0],
        ],
        [
            [20.0, 40.0, 40.0],
            [0.0, 0.0, 0.0],
            [0.0, 0.0, 7.0],
            [0.0, 1.0, 0.0],
        ],
        [
            [30.0, 50.0, 20.0],
            [2.0, 1.0, 0.0],
            [5.0, 3.0, 0.0],
            [-1.0, 0.0, 0.0],
        ],
    ];
    let mut markets: Vec<_> = inputs
        .iter()
        .map(|values| {
            let mut planetary = PlanetaryMarket::new(&catalog, true);
            for (index, (&price, (&production, (&consumption, &shortage)))) in values[0]
                .iter()
                .zip(values[1].iter().zip(values[2].iter().zip(&values[3])))
                .enumerate()
            {
                let good = GoodId(index);
                planetary.market.price[good] = price;
                planetary.market.production[good] = production;
                planetary.market.consumption[good] = consumption;
                planetary.market.chronic_shortages[good] = shortage;
            }
            planetary
        })
        .collect();
    let market_input: Vec<_> = markets
        .iter()
        .map(|planetary| {
            json!([
                planetary.market.price.values(),
                planetary.market.production.values(),
                planetary.market.consumption.values(),
                planetary.market.chronic_shortages.values()
            ])
        })
        .collect();
    // Same controlled EnTT order as the fixture: secondary created first, primary last.
    let mut ports: Vec<_> = (0..markets.len())
        .flat_map(|body| (0..2).map(move |_| SpacePort::new(BodyId(body), count)))
        .collect();
    let mut days = Vec::new();
    for _ in 0..3 {
        for market in &mut markets {
            market.update_difference();
        }
        let mut orders = Vec::new();
        for (seller, market) in markets.iter().enumerate() {
            for (buyer, target) in markets.iter().enumerate() {
                if seller != buyer {
                    orders.extend(
                        market
                            .orders_for(target, BodyId(buyer))
                            .into_iter()
                            .map(|order| (seller, order)),
                    );
                }
            }
        }
        for (seller, (good, order)) in orders {
            markets[seller].demands.entry(good).or_default().push(order);
        }
        for port in ports.iter_mut().rev() {
            port.take_orders(&mut markets[port.reference_body.0]);
        }
        let differences: Vec<_> = markets
            .iter()
            .map(|market| market.supply_difference.values())
            .collect();
        let shipments: Vec<_> = ports
            .iter()
            .map(|port| {
                port.deliveries
                    .iter()
                    .flat_map(|(target, deliveries)| {
                        deliveries.iter().map(|delivery| {
                            let Shipment::Good(good) = delivery.item else {
                                panic!("Unexpected mission")
                            };
                            [
                                target.0 as f64,
                                good.0 as f64,
                                delivery.amount,
                                delivery.fulfilled,
                                delivery.priority,
                                f64::from(delivery.target_province.is_none()),
                            ]
                        })
                    })
                    .collect::<Vec<_>>()
            })
            .collect();
        assert!(
            ports
                .iter()
                .step_by(2)
                .all(|port| port.deliveries.is_empty())
        );
        assert!(
            markets
                .iter()
                .all(|market| market.demands.values().all(Vec::is_empty))
        );
        days.push(json!({"differences": differences, "shipments": shipments}));
    }
    let goods: Vec<_> = catalog
        .base_prices
        .values()
        .iter()
        .map(|price| json!([price, 0]))
        .collect();
    compare(
        &json!(days),
        &reference_output(
            Some("--interplanetary"),
            &json!({"goods": goods, "markets": market_input, "days": 3}).to_string(),
        ),
    );
}

#[test]
#[ignore = "Run tools/build-reference.ps1 first; compares the actual C++ city/project loaders"]
fn surface_assets_match_live_cpp_loaders_and_common_soi() {
    let data = default_data_path();
    let simulation = Simulation::load(&data, &World::load(&data).unwrap(), 42).unwrap();
    let mut cities: Vec<_> = simulation
        .settlements
        .iter()
        .filter_map(|site| {
            let SurfaceKind::City {
                province,
                latitude,
                longitude,
            } = site.kind
            else {
                return None;
            };
            Some(json!([
                site.identifier,
                simulation.solar_system.bodies[site.body.0].identifier,
                province.map(|index| &simulation.settlements[index].identifier),
                latitude,
                longitude,
                site.spaceport.is_some(),
                false,
                false,
                false,
                site.time_zone
                    .map(|id| &simulation.time_zones.entries[id.0].identifier)
            ]))
        })
        .collect();
    cities.sort_by(|a, b| a[0].as_str().cmp(&b[0].as_str()));
    let projects: Vec<_> = simulation
        .project_templates
        .iter()
        .map(|template| {
            let cost = template
                .cost
                .iter()
                .map(|(good, amount)| {
                    (
                        simulation.catalog.goods[good.0].identifier.clone(),
                        json!(amount),
                    )
                })
                .collect::<serde_json::Map<_, _>>();
            json!([
                template.identifier,
                template.max_progress,
                template.crew,
                cost
            ])
        })
        .collect();
    let system = &simulation.solar_system;
    let common: Vec<_> = [
        ("earth", "moon"),
        ("moon", "earth"),
        ("moon", "mars"),
        ("earth", "mars"),
        ("earth", "earth"),
        ("sun", "earth"),
    ]
    .into_iter()
    .map(|(source, target)| {
        let id = system.common_soi(
            system.body_id(source).unwrap(),
            system.body_id(target).unwrap(),
        );
        system.bodies[id.0].identifier.as_str()
    })
    .collect();
    compare(
        &json!({"cities": cities, "projects": projects, "common_soi": common}),
        &reference_output(Some("--surface-assets"), ""),
    );
}

#[test]
#[ignore = "Run tools/build-reference.ps1 first; compares original movement, SOI transitions and command boundaries"]
fn flight_commands_soi_boundaries_and_return_trajectories_match_original() {
    use cqsp_core::{
        clock::SECONDS_PER_TICK,
        flight::{Burn, Command as FlightCommand, FlightStatus, Trigger, TriggeredCommand},
        orbit::StateVector,
    };
    struct Case {
        name: String,
        parent: &'static str,
        orbit: Orbit,
        burns: Vec<Burn>,
        commands: Vec<TriggeredCommand>,
        ticks: Vec<u64>,
    }
    impl Case {
        fn new(name: &str, parent: &'static str, orbit: Orbit, tick: u64, steps: u64) -> Self {
            Self {
                name: format!("{name} at tick {tick}"),
                parent,
                orbit,
                burns: Vec::new(),
                commands: Vec::new(),
                ticks: (tick..tick + steps).collect(),
            }
        }
        fn command(mut self, trigger: Trigger, command: FlightCommand) -> Self {
            self.commands.push(TriggeredCommand { trigger, command });
            self
        }
        fn burn(mut self, delay: f64) -> Self {
            self.burns.push(Burn {
                delta_v: DVec3::ZERO,
                delay,
            });
            self
        }
    }
    let mut system = SolarSystem::load(&default_data_path()).unwrap();
    system.ships.clear();
    let earth = system.body_id("earth").unwrap();
    let moon = system.body_id("moon").unwrap();
    let body_number = |parent| {
        if parent == moon {
            2
        } else if parent == earth {
            1
        } else {
            0
        }
    };
    let base = Orbit {
        semi_major_axis: 12000.0,
        eccentricity: 0.25,
        inclination: 0.3,
        ascending_node: 1.1,
        periapsis_argument: 0.7,
        mean_anomaly: 0.5,
        true_anomaly: 0.5,
        mu: system.bodies[earth.0].mu,
        ..Orbit::default()
    };
    let target = Orbit {
        semi_major_axis: 24000.0,
        eccentricity: 0.05,
        inclination: 0.7,
        ascending_node: 0.2,
        ..base
    };
    let mut cases = Vec::new();
    for tick in [0, DAY, 10000] {
        let time = (tick * SECONDS_PER_TICK) as f64;
        let orbit = Orbit {
            epoch: time,
            ..base
        };
        cases.push(
            Case::new("overdue burns", "earth", orbit, tick, 16)
                .burn(10.0)
                .burn(20.0)
                .command(Trigger::Maneuver, FlightCommand::Impulse)
                .command(
                    Trigger::Maneuver,
                    FlightCommand::InterceptAndCircularizeBody,
                ),
        );
        cases.push(
            Case::new("blocked front trigger", "earth", orbit, tick, 16)
                .burn(0.0)
                .command(Trigger::ExitSoi, FlightCommand::SelfDestruct)
                .command(Trigger::Maneuver, FlightCommand::SelfDestruct),
        );
        for (name, command) in [
            ("circularize apoapsis", FlightCommand::CircularizeAtApoapsis),
            (
                "circularize periapsis",
                FlightCommand::CircularizeAtPeriapsis,
            ),
            ("raise apoapsis", FlightCommand::SetApoapsis(24000.0)),
            ("lower periapsis", FlightCommand::SetPeriapsis(8000.0)),
            ("inclination", FlightCommand::SetInclination(0.6)),
            ("plane matching", FlightCommand::MatchPlanes(target)),
            ("forced planes", FlightCommand::ForceMatchPlanes(target)),
            ("interception", FlightCommand::CoplanarIntercept(target)),
            (
                "interception and transfer",
                FlightCommand::CoplanarInterceptAndTransfer(target),
            ),
            ("maneuver self destruct", FlightCommand::SelfDestruct),
        ] {
            cases.push(
                Case::new(name, "earth", orbit, tick, 96)
                    .burn(0.0)
                    .command(Trigger::Maneuver, command),
            );
        }
        let collision = Orbit {
            semi_major_axis: 5000.0,
            eccentricity: 0.05,
            ..orbit
        };
        cases.push(Case::new("natural crash", "earth", collision, tick, 3).burn(10000.0));
        cases.push(
            Case::new("crash docking", "earth", collision, tick, 3)
                .command(Trigger::Crash, FlightCommand::Land(123)),
        );
        cases.push(
            Case::new("crash no-op", "earth", collision, tick, 3)
                .command(Trigger::Crash, FlightCommand::Impulse)
                .command(Trigger::Crash, FlightCommand::SelfDestruct),
        );
        let lunar = Orbit {
            semi_major_axis: 3500.0,
            eccentricity: 0.05,
            mu: system.bodies[moon.0].mu,
            ..orbit
        };
        let escaping = Orbit {
            semi_major_axis: -15000.0,
            eccentricity: 1.5,
            ..lunar
        };
        cases.push(Case::new("lunar escape", "moon", escaping, tick, DAY * 10));
        cases.push(
            Case::new("lunar return command", "moon", lunar, tick, DAY * 20)
                .burn(10.0)
                .command(Trigger::Maneuver, FlightCommand::ExitSoi(8000.0))
                .command(
                    Trigger::ExitSoi,
                    FlightCommand::SetPeriapsis(system.bodies[earth.0].radius * 0.9),
                )
                .command(Trigger::Crash, FlightCommand::Land(123)),
        );
        system.update(time);
        let moon_state = system.bodies[moon.0].kinematics.state;
        for offset in [-0.01, 0.0, 0.01] {
            let state = StateVector {
                position: moon_state.position
                    + DVec3::new(system.bodies[moon.0].sphere_of_influence + offset, 0.0, 0.0),
                velocity: moon_state.velocity + DVec3::new(0.1, 0.3, 0.02),
            };
            let arrival = Orbit::from_state(state, system.bodies[earth.0].mu, time).unwrap();
            cases.push(
                Case::new(&format!("SOI boundary {offset}"), "earth", arrival, tick, 3)
                    .command(Trigger::EnterSoi, FlightCommand::Impulse),
            );
        }
        let arrival = Orbit::from_state(
            StateVector {
                position: moon_state.position + DVec3::new(10000.0, 0.0, 0.0),
                velocity: moon_state.velocity + DVec3::new(0.1, 0.3, 0.02),
            },
            system.bodies[earth.0].mu,
            time,
        )
        .unwrap();
        cases.push(
            Case::new("arrival self destruct", "earth", arrival, tick, 3)
                .command(Trigger::EnterSoi, FlightCommand::SelfDestruct),
        );
        let outside = Orbit {
            semi_major_axis: 100000.0,
            eccentricity: 0.1,
            ..lunar
        };
        cases.push(
            Case::new("exit self destruct", "moon", outside, tick, 3)
                .command(Trigger::ExitSoi, FlightCommand::SelfDestruct),
        );
    }
    let command_row = |command: TriggeredCommand| {
        let trigger = match command.trigger {
            Trigger::EnterSoi => 0,
            Trigger::ExitSoi => 1,
            Trigger::Maneuver => 2,
            Trigger::Crash => 3,
        };
        let (kind, payload) = match command.command {
            FlightCommand::CircularizeAtApoapsis => (0, json!(0)),
            FlightCommand::CircularizeAtPeriapsis => (1, json!(0)),
            FlightCommand::SetApoapsis(value) => (2, json!(value)),
            FlightCommand::SetPeriapsis(value) => (3, json!(value)),
            FlightCommand::SetInclination(value) => (4, json!(value)),
            FlightCommand::MatchPlanes(orbit) => (5, orbit_row(orbit)),
            FlightCommand::CoplanarIntercept(orbit) => (6, orbit_row(orbit)),
            FlightCommand::CoplanarInterceptAndTransfer(orbit) => (7, orbit_row(orbit)),
            FlightCommand::Impulse => (8, json!(0)),
            FlightCommand::Land(value) => (9, json!(value)),
            FlightCommand::InterceptAndCircularizeBody => (10, json!(0)),
            FlightCommand::ForceMatchPlanes(orbit) => (11, orbit_row(orbit)),
            FlightCommand::ExitSoi(value) => (12, json!(value)),
            FlightCommand::SelfDestruct => (13, json!(0)),
        };
        json!([trigger, kind, payload])
    };
    let inputs: Vec<_> = cases.iter().map(|case| {
        let parent = system.body_id(case.parent).unwrap();
        let time = (case.ticks[0] * SECONDS_PER_TICK) as f64;
        let burns: Vec<_> = case.burns.iter().map(|burn| json!([burn.delta_v.x, burn.delta_v.y, burn.delta_v.z, time + burn.delay])).collect();
        let commands: Vec<_> = case.commands.iter().copied().map(command_row).collect();
        json!({"parent":body_number(parent), "orbit":orbit_row(case.orbit), "burns":burns, "commands":commands, "ticks":case.ticks})
    }).collect();
    let reference = reference_output(
        Some("--flight-boundaries"),
        &json!({"bodies":flight_body_rows(&system),"cases":inputs}).to_string(),
    );
    assert_eq!(reference.as_array().unwrap().len(), cases.len());
    for (case, expected) in cases.iter().zip(reference.as_array().unwrap()) {
        system.ships.clear();
        let parent = system.body_id(case.parent).unwrap();
        let ship = system
            .launch_ship(case.name.clone(), parent, case.orbit)
            .unwrap();
        let craft = &mut system.ships[ship.0];
        craft.orbit = case.orbit;
        craft.kinematics.state = case.orbit.state_at_anomaly(case.orbit.true_anomaly);
        let time = (case.ticks[0] * SECONDS_PER_TICK) as f64;
        for &burn in &case.burns {
            craft.queue_burn(burn, time).unwrap();
        }
        for &command in &case.commands {
            craft.queue_command(command.trigger, command.command);
        }
        assert_eq!(expected.as_array().unwrap().len(), case.ticks.len());
        let mut visited_earth = false;
        for (&tick, expected) in case.ticks.iter().zip(expected.as_array().unwrap()) {
            system.update((tick * SECONDS_PER_TICK) as f64);
            system.validate().unwrap();
            let craft = &system.ships[ship.0];
            visited_earth |= craft.parent == earth;
            let docked = matches!(craft.status, FlightStatus::Docked(_));
            let status = match craft.status {
                FlightStatus::Orbiting => 0,
                FlightStatus::Crashed => 1,
                FlightStatus::Docked(_) => 2,
            };
            let burns: Vec<_> = craft
                .maneuvers
                .iter()
                .map(|burn| json!([burn.delta_v.x, burn.delta_v.y, burn.delta_v.z, burn.time]))
                .collect();
            let commands: Vec<_> = craft
                .commands
                .iter()
                .copied()
                .map(|command| {
                    let row = command_row(command);
                    json!([row[0], row[1]])
                })
                .collect();
            let mut actual = json!({"status":status,"burns":burns,"commands":commands});
            if !docked {
                let k = craft.kinematics;
                actual["parent"] = json!(body_number(craft.parent));
                actual["orbit"] = orbit_row(craft.orbit);
                actual["motion"] = json!([
                    k.state.position.x,
                    k.state.position.y,
                    k.state.position.z,
                    k.state.velocity.x,
                    k.state.velocity.y,
                    k.state.velocity.z,
                    k.center.x,
                    k.center.y,
                    k.center.z,
                    k.future_position.x,
                    k.future_position.y,
                    k.future_position.z,
                    k.future_center.x,
                    k.future_center.y,
                    k.future_center.z
                ]);
            }
            compare_at(
                &actual,
                expected,
                &format!("{} tick {tick}", case.name),
                1e-12,
            );
        }
        if case.name.starts_with("lunar escape") || case.name.starts_with("lunar return command") {
            assert!(visited_earth, "{} never returned to Earth's SOI", case.name);
        }
        if case.name.starts_with("lunar return command") {
            assert_eq!(
                system.ships[ship.0].status,
                FlightStatus::Docked(123),
                "{}",
                case.name
            );
        }
    }
}

fn orbit_row(orbit: Orbit) -> Value {
    json!([
        orbit.semi_major_axis,
        orbit.eccentricity,
        orbit.inclination,
        orbit.ascending_node,
        orbit.periapsis_argument,
        orbit.mean_anomaly,
        orbit.epoch,
        orbit.true_anomaly,
        orbit.mu
    ])
}

fn flight_body_rows(system: &SolarSystem) -> Value {
    ["sun", "earth", "moon"]
        .map(|name| {
            let body = &system.bodies[system.body_id(name).unwrap().0];
            let orbit = body.orbit;
            json!([
                body.radius,
                body.mu,
                if body.sphere_of_influence.is_finite() {
                    body.sphere_of_influence
                } else {
                    0.0
                },
                orbit.semi_major_axis,
                orbit.eccentricity,
                orbit.inclination,
                orbit.ascending_node,
                orbit.periapsis_argument,
                orbit.mean_anomaly,
                orbit.mu
            ])
        })
        .into()
}

#[test]
#[ignore = "Run tools/build-reference.ps1 first; runs the actual C++ SysOrbit and SysSpacePort"]
fn lunar_trajectory_and_delivery_match_original_systems() {
    lunar_mission_case(None);
}

#[test]
#[ignore = "Run tools/build-reference.ps1 first; runs seeded C++ SysOrbit and SysSpacePort"]
fn seeded_lunar_launch_trajectories_and_delivery_match_original_systems() {
    for seed in [0, 1, 42, 5489, u32::MAX as u64] {
        lunar_mission_case(Some(seed));
    }
}

fn lunar_mission_case(seed: Option<u64>) {
    use cqsp_core::{flight::FlightStatus, spaceflight::ColonizationStep};
    let data = default_data_path();
    let world = World::load(&data).unwrap();
    let mut simulation = Simulation::load(&data, &world, seed.unwrap_or(42)).unwrap();
    let system = &simulation.solar_system;
    let earth = system.body_id("earth").unwrap();
    let moon = system.body_id("moon").unwrap();
    let reference = reference_output(
        Some("--lunar-mission"),
        &json!({"bodies": flight_body_rows(system), "seed": seed}).to_string(),
    );
    let country = world
        .countries
        .iter()
        .position(|country| country.identifier == "usa")
        .unwrap();
    let target = simulation
        .settlements
        .iter()
        .position(|site| site.body == moon && site.is_province())
        .unwrap();
    simulation.begin_colonization(target, country).unwrap();
    let mission = simulation
        .queue_colonization_mission(target, country, true)
        .unwrap();
    let radius = simulation.solar_system.bodies[earth.0].radius;
    let ship = if seed.is_some() {
        // The first daily step automatically dispatches through the actual port system.
        cqsp_core::solar_system::ShipId(simulation.solar_system.ships.len())
    } else {
        simulation.missions[mission.0].in_progress = true;
        let ship = simulation
            .solar_system
            .launch_ship(
                "Colony Core Transport Vehicle".into(),
                earth,
                Orbit {
                    semi_major_axis: radius * 1.08 + radius * 0.01 * 0.4,
                    eccentricity: 0.000005 * 0.4,
                    inclination: 0.05 * 0.4,
                    periapsis_argument: 0.1,
                    ..Orbit::default()
                },
            )
            .unwrap();
        simulation.solar_system.ships[ship.0]
            .land_on_moon(moon, &simulation.solar_system.bodies[moon.0], target, 0.0)
            .unwrap();
        simulation.solar_system.ships[ship.0].cargo = Some(vec![mission]);
        ship
    };
    let mut samples = Vec::new();
    let mut landing_tick = None;
    let mut unload_tick = None;
    for tick in 0..DAY * 30 {
        simulation.tick();
        simulation.validate().unwrap();
        let craft = &simulation.solar_system.ships[ship.0];
        if craft.status == FlightStatus::Docked(target) {
            landing_tick.get_or_insert(tick);
            if craft.cargo.is_none() {
                unload_tick = Some(tick);
                break;
            }
            continue;
        }
        if tick % if seed.is_some() { 1 } else { 24 } == 0 || tick < 10 {
            let orbit = craft.orbit;
            let p = craft.kinematics.state.position;
            let v = craft.kinematics.state.velocity;
            samples.push(json!([
                tick,
                if craft.parent == moon {
                    2
                } else if craft.parent == earth {
                    1
                } else {
                    0
                },
                orbit.semi_major_axis,
                orbit.eccentricity,
                orbit.inclination,
                orbit.ascending_node,
                orbit.periapsis_argument,
                orbit.mean_anomaly,
                orbit.epoch,
                orbit.true_anomaly,
                p.x,
                p.y,
                p.z,
                v.x,
                v.y,
                v.z,
                craft.maneuvers.len(),
                craft.commands.len(),
                craft.maneuvers.front().map_or(-1.0, |burn| burn.time)
            ]));
        }
    }
    assert_eq!(
        simulation.settlements[target]
            .colonization
            .as_ref()
            .unwrap()
            .step,
        ColonizationStep::Preparation
    );
    let delivery = json!([
        landing_tick,
        unload_tick,
        1,
        simulation.settlements[target]
            .colony
            .as_ref()
            .unwrap()
            .components
            .len(),
        usize::from(simulation.missions[mission.0].in_progress)
    ]);
    let expected = reference["samples"].as_array().unwrap();
    assert_eq!(
        samples.len(),
        expected.len(),
        "Lunar trajectory seed {seed:?}; landing Rust {landing_tick:?}, C++ {}",
        reference["delivery"][0]
    );
    for (row, expected) in samples.iter().zip(expected) {
        if seed.is_some() && cfg!(all(windows, target_env = "msvc")) {
            // The installed MSVC runtime is the exact seeded-flight target,
            // including the tiny residual apsis and the next maneuver's time.
            for column in 0..19 {
                assert_eq!(
                    row[column].as_f64().unwrap().to_bits(),
                    expected[column].as_f64().unwrap().to_bits(),
                    "Lunar trajectory seed {seed:?}, tick {}, column {column}: Rust {}, C++ {}",
                    row[0],
                    row[column],
                    expected[column]
                );
            }
            continue;
        }
        if expected[3].as_f64().unwrap() < 1e-12 {
            // A circular orbit has no defined apsis: compiler rounding chooses
            // different w/M0/v for residual e~1e-13. Compare its physical state
            // (within 1 m / 1 mm/s), not that arbitrary element representation.
            for index in [0, 1, 2, 3, 4, 5, 8, 16, 17] {
                compare(&row[index], &expected[index]);
            }
            let vector = |row: &Value, start| {
                DVec3::new(
                    row[start].as_f64().unwrap(),
                    row[start + 1].as_f64().unwrap(),
                    row[start + 2].as_f64().unwrap(),
                )
            };
            assert!(vector(row, 10).distance(vector(expected, 10)) <= 0.001);
            assert!(vector(row, 13).distance(vector(expected, 13)) <= 0.000001);
        } else {
            compare(row, expected);
        }
    }
    compare(&delivery, &reference["delivery"]);
}

#[test]
#[ignore = "Run tools/build-reference.ps1 first; runs original population and wallet/market reset systems"]
fn population_consumption_and_history_match_original_daily_systems() {
    let data = default_data_path();
    let mut simulation = Simulation::load(&data, &World::load(&data).unwrap(), 42).unwrap();
    let mut populations: Vec<_> = simulation
        .settlements
        .iter_mut()
        .filter_map(|site| site.populations.pop())
        .take(2)
        .collect();
    assert_eq!(populations.len(), 2);
    let catalog = &simulation.catalog;
    let good_count = catalog.goods.len();
    for (index, population) in populations.iter_mut().enumerate() {
        population.size = [100, 250][index];
        population.standard_of_living = [10.0, 1.0][index];
        population.wallet.assign([10000.0, -10.0][index]);
        population.income = [100.0, 0.0][index];
        population.spending = [80.0, 0.0][index];
        population.employed = [4, 0][index];
        population.labor_hours = vec![(catalog.labor_goods[index], [4.0, 0.25][index])];
    }
    let input_populations: Vec<_> = populations
        .iter()
        .map(|population| {
            let mut hours = Ledger::new(good_count);
            hours.add(&population.labor_hours);
            json!([
                population.size,
                population.standard_of_living,
                population.wallet.balance,
                population.income,
                population.spending,
                population.employed,
                hours.values()
            ])
        })
        .collect();
    let goods: Vec<_> = catalog
        .goods
        .iter()
        .enumerate()
        .map(|(index, good)| {
            json!([
                good.price,
                good.consumption
                    .as_ref()
                    .map(|value| value.autonomous_consumption),
                good.consumption
                    .as_ref()
                    .map(|value| value.marginal_propensity),
                index as f64 * 0.001
            ])
        })
        .collect();
    let days = 30;
    let reference = reference_output(
        Some("--population"),
        &json!({"days": days, "goods": goods, "populations": input_populations}).to_string(),
    );
    let autonomous: Vec<_> = catalog
        .goods
        .iter()
        .enumerate()
        .filter_map(|(index, good)| {
            good.consumption
                .as_ref()
                .map(|value| (GoodId(index), value.autonomous_consumption * DAY as f64))
        })
        .collect();
    let marginal: Vec<_> = catalog
        .goods
        .iter()
        .enumerate()
        .filter_map(|(index, good)| {
            good.consumption
                .as_ref()
                .map(|value| (GoodId(index), value.marginal_propensity * DAY as f64))
        })
        .collect();
    let mut market = Market::new(
        &catalog.base_prices,
        &catalog.labor_goods,
        catalog.config.market_config,
    );
    let mut samples = Vec::new();
    for day in 0..days {
        market.reset();
        for (index, good) in catalog.goods.iter().enumerate() {
            market.price[GoodId(index)] = good.price * (1.0 + day as f64 * 0.05);
            market.taxation[GoodId(index)] = index as f64 * 0.001;
        }
        for population in &mut populations {
            population.wallet.reset();
            population.consume(&mut market, &autonomous, &marginal);
        }
        let rows: Vec<_> = populations
            .iter()
            .map(|population| {
                json!([
                    population.wallet.balance,
                    population.wallet.change,
                    population.wallet.gdp_change,
                    population.standard_of_living,
                    population.income,
                    population.spending,
                    population.average_wage,
                    population.sol_pid.proportional,
                    population.sol_pid.integral,
                    population.sol_pid.derivative
                ])
            })
            .collect();
        let total: u64 = populations.iter().map(|population| population.size).sum();
        let employed: u64 = populations
            .iter()
            .map(|population| population.employed)
            .sum();
        let sol: f64 = populations
            .iter()
            .map(|population| population.standard_of_living * population.size as f64)
            .sum();
        samples.push(json!({"populations": rows, "production": market.production.values(),
            "consumption": market.consumption.values(), "history": [total as f64, sol / total as f64, employed as f64, employed as f64 / total as f64 * 100.0]}));
    }
    compare(&json!(samples), &reference);
}

#[test]
#[ignore = "Run tools/build-reference.ps1 first; compares the actual combined daily economy and weekly labor schedule"]
fn combined_economy_ticks_match_original_ordered_systems() {
    use cqsp_core::{
        catalog::{Catalog, JobId},
        clock::WEEK,
        economy::industry::{Construction, ConstructionSector, Industry, IndustryState},
        spaceflight::PlanetaryMarket,
    };
    let data = default_data_path();
    let world = World::load(&data).unwrap();
    let recipes: Vec<_> = Catalog::load(&data)
        .unwrap()
        .recipes
        .into_iter()
        .map(|recipe| recipe.identifier)
        .collect();
    for identifier in recipes {
        for (seed, state, capacity, subsidy) in [
            (0, IndustryState::Steady, 0, 0.0),
            (42, IndustryState::Steady, 1, 0.2),
            (5489, IndustryState::Maximum, 10, 1.0),
            (1, IndustryState::Minimum, 1000, 0.0),
            (42, IndustryState::Construction, 10, 0.0),
        ] {
            let mut simulation = Simulation::load(&data, &world, seed).unwrap();
            let recipe_id = simulation.catalog.recipe(&identifier).unwrap();
            // One job/industry isolates scheduling from registry/hash traversal, using Simulation::tick.
            let job = simulation
                .catalog
                .jobs
                .remove(simulation.catalog.default_job.0);
            let labor_good = job.good;
            simulation.catalog.jobs = vec![job];
            simulation.catalog.default_job = JobId(0);
            simulation.catalog.recipes[recipe_id.0].workers = vec![(JobId(0), 12)];
            simulation
                .settlements
                .retain(|site| !site.populations.is_empty());
            simulation.settlements.truncate(1);
            let site = &mut simulation.settlements[0];
            site.spaceport = None;
            site.colonization = None;
            site.colony = None;
            site.populations.truncate(1);
            let population = &mut site.populations[0];
            population.size = 10_000;
            population.standard_of_living = 5.0;
            population.wallet.assign(10_000.0);
            population.income = 100.0;
            population.spending = 80.0;
            population.jobs = [(JobId(0), 5000)].into();
            let mut industry = Industry::new(recipe_id, 32).unwrap();
            industry.state = state;
            industry.output_subsidy = subsidy;
            if state == IndustryState::Construction {
                industry.construction = Some(Construction {
                    progress: 0,
                    maximum: 3 * DAY as i32,
                    levels: 2,
                });
            }
            site.industries = vec![industry];
            site.construction = ConstructionSector {
                capacity,
                current: 0,
                cost: 777.0,
            };
            site.job_demands = vec![0.0];
            for index in 0..simulation.catalog.goods.len() {
                site.market.taxation[GoodId(index)] = index as f64 * 0.001;
            }
            let body = site.body;
            let country = site.country.unwrap();
            for (&id, market) in &mut simulation.planetary_markets {
                *market = PlanetaryMarket::new(&simulation.catalog, id == body);
            }
            let recipe = &simulation.catalog.recipes[recipe_id.0];
            let market_config = simulation.catalog.config.market_config;
            let production_config = simulation.catalog.config.production_config;
            let goods: Vec<_> = simulation
                .catalog
                .goods
                .iter()
                .enumerate()
                .map(|(index, good)| {
                    json!([
                        good.price,
                        usize::from(simulation.catalog.labor_goods.contains(&GoodId(index))),
                        good.consumption
                            .as_ref()
                            .map(|value| value.autonomous_consumption),
                        good.consumption
                            .as_ref()
                            .map(|value| value.marginal_propensity),
                    ])
                })
                .collect();
            let days = 43;
            let reference = reference_output(Some("--combined-economy"), &json!({
            "seed":seed, "days":days, "goods":goods, "labor_good":labor_good.0,
            "market_config":[market_config.base_price_deviation, market_config.shortage_level, market_config.default_market_access],
            "production_config":[production_config.profit_multiplier, production_config.max_factory_delta,
                production_config.factory_min_utilization, production_config.construction_limit, production_config.underutilization_limit],
            "population":[10_000,5.0,10_000.0,100.0,80.0,5000], "sector":[capacity,777.0],
            "recipe":{"input":recipe.input,"capital":recipe.capital,"output":recipe.output,"power":recipe.power,
                "workers":12,"construction_time":recipe.construction_ticks,"construction_cost":recipe.construction_cost},
            "industry":[state as usize,32.0,32.0,subsidy,simulation.settlements[0].industries[0].construction.as_ref().map(|job| [job.progress,job.maximum,job.levels])],
            "taxation":simulation.settlements[0].market.taxation.values(),
        }).to_string());
            let mut comparisons = 0;
            let mut weekly_updates = 0;
            for tick in 0..=((days - 1) * DAY) {
                simulation.tick();
                assert_eq!(simulation.date.tick(), tick);
                if simulation.date.due(WEEK) {
                    weekly_updates += 1;
                }
                if !simulation.date.due(DAY) {
                    continue;
                }
                let site = &simulation.settlements[0];
                let population = &site.populations[0];
                let industry = &site.industries[0];
                let history = simulation.history.last().unwrap();
                let row = json!({
                    "population":[population.wallet.balance,population.wallet.change,population.wallet.gdp_change,
                        population.standard_of_living,population.income,population.spending,population.average_wage,
                        population.sol_pid.proportional,population.sol_pid.integral,population.sol_pid.derivative,
                        population.employed as f64,population.jobs[&JobId(0)] as f64],
                    "unemployment":population.unemployment_rate, "market":market_values(&site.market),
                    "planet":market_values(&simulation.planetary_markets[&body].market),
                    "industry":[industry.state as usize as f64,industry.size,industry.utilization,industry.diff,f64::from(industry.shortage),
                        industry.cumulative_profit_ratio,industry.continuous_gains as f64,industry.stability as f64,industry.expertise,
                        industry.revenue,industry.material_costs,industry.wage_cost,industry.tax_cost,industry.construction_cost,industry.profit,
                        industry.subsidy_amount,industry.amount_sold,industry.power_consumption,industry.wallet.balance,industry.wallet.change,industry.wallet.gdp_change],
                    "construction":industry.construction.as_ref().map(|job| [job.progress,job.maximum,job.levels]),
                    "sector":[site.construction.capacity as f64,site.construction.current as f64,site.construction.cost],
                    "taxes":simulation.countries[country].income_taxes,
                    "history":[history.population as f64,history.standard_of_living,history.employed as f64,history.employed as f64 / history.population as f64 * 100.0],
                    "job_demand":site.job_demands[0], "market_history_length":simulation.planetary_markets[&body].history.0.len(),
                });
                compare_at(
                    &row,
                    &reference[comparisons],
                    &format!("{identifier} / seed {seed} / {state:?} / tick {tick}"),
                    1e-12,
                );
                comparisons += 1;
            }
            assert_eq!(comparisons, days as usize);
            assert_eq!(weekly_updates, 7);
        }
    }
}

#[test]
#[ignore = "Run tools/build-reference.ps1 first; runs actual launch vehicle production boundaries"]
fn manufacturing_boundaries_match_original_system() {
    use cqsp_core::{
        solar_system::BodyId,
        spaceflight::{ManufacturingProject, SpacePort, TemplateId, VehicleId},
    };
    let cases = [
        (true, 0, false),
        (true, DAY, true),
        (true, DAY + 1, true),
        (true, 90 * DAY, false),
        (false, DAY, true),
    ];
    let input: Vec<_> = cases
        .map(|(parent, duration, template)| {
            json!([usize::from(parent), duration, usize::from(template)])
        })
        .into();
    let days = 92;
    let reference = reference_output(
        Some("--manufacturing"),
        &json!({"days": days, "projects": input}).to_string(),
    );
    let mut ports: Vec<_> = cases
        .iter()
        .enumerate()
        .map(|(index, &(_, duration, template))| {
            let mut port = SpacePort::new(BodyId(0), 1);
            let mut project = ManufacturingProject::new(VehicleId(index), duration);
            project.template = template.then_some(TemplateId(0));
            port.projects.push(project);
            port
        })
        .collect();
    let mut samples = Vec::new();
    for _ in 0..days {
        let rows: Vec<_> = ports
            .iter_mut()
            .zip(cases)
            .map(|(port, (valid_parent, _, _))| {
                if valid_parent {
                    port.manufacture();
                }
                assert!(
                    port.projects
                        .iter()
                        .all(|project| project.total_cost == 0.0 && project.last_cost == 0.0)
                );
                json!([
                    port.projects.first().map(|project| project.progress),
                    port.projects.len(),
                    port.stored_launch_vehicles.len(),
                    0.0
                ])
            })
            .collect();
        samples.push(rows);
    }
    compare(&json!(samples), &reference);
}

#[test]
#[ignore = "Run tools/build-reference.ps1 first; runs the actual eight-state production system with controlled noise"]
fn industry_states_construction_taxes_and_subsidies_match_original_system() {
    production_case("farming", None);
}

#[test]
#[ignore = "Run tools/build-reference.ps1 first; runs original seeded production for all 22 recipes"]
fn every_recipe_with_original_random_stream_matches_production_system() {
    let catalog = cqsp_core::catalog::Catalog::load(&default_data_path()).unwrap();
    assert_eq!(catalog.recipes.len(), 22);
    for recipe in &catalog.recipes {
        for seed in [42, u32::MAX as u64] {
            production_case(&recipe.identifier, Some(seed));
        }
    }
}

fn production_case(identifier: &str, seed: Option<u64>) {
    use cqsp_core::{
        catalog::Catalog,
        economy::industry::{Construction, ConstructionSector, Industry, IndustryState},
    };
    let catalog = Catalog::load(&default_data_path()).unwrap();
    let id = catalog.recipe(identifier).unwrap();
    let recipe = &catalog.recipes[id.0];
    let config = catalog.config.production_config;
    let states = [
        IndustryState::Steady,
        IndustryState::Maximum,
        IndustryState::Minimum,
        IndustryState::Construction,
        IndustryState::Demolishing,
        IndustryState::Shrinking,
        IndustryState::Expanding,
        IndustryState::Shortage,
        IndustryState::Construction,
    ];
    let mut industries: Vec<_> = states
        .iter()
        .enumerate()
        .map(|(index, &state)| {
            let mut industry = Industry::new(id, 32).unwrap();
            industry.state = state;
            industry.utilization = if state == IndustryState::Shrinking {
                20.0
            } else {
                32.0
            };
            industry.revenue = 1000.0;
            industry.profit = if matches!(
                state,
                IndustryState::Minimum | IndustryState::Demolishing | IndustryState::Shrinking
            ) {
                -500.0
            } else {
                500.0
            };
            industry.continuous_gains = if industry.profit < 0.0 {
                -((30 * DAY) as i32)
            } else {
                (30 * DAY) as i32
            };
            industry.cumulative_profit_ratio = if industry.profit < 0.0 { -6.0 } else { 6.0 };
            industry.shortage = state == IndustryState::Shortage || index == 8;
            industry.output_subsidy = [0.0, 0.2, 1.0][index % 3];
            if state == IndustryState::Construction {
                industry.construction = Some(Construction {
                    progress: 0,
                    maximum: 3 * DAY as i32,
                    levels: 2,
                });
            }
            industry
        })
        .collect();
    let cases: Vec<_> = industries
        .iter()
        .map(|industry| {
            json!([
                industry.state as usize,
                industry.size,
                industry.utilization,
                industry.profit,
                industry.revenue,
                industry.continuous_gains,
                industry.stability,
                industry.cumulative_profit_ratio,
                usize::from(industry.shortage),
                industry.output_subsidy,
                industry
                    .construction
                    .as_ref()
                    .map(|job| [job.progress, job.maximum, job.levels])
            ])
        })
        .collect();
    let entries = |values: &[(GoodId, f64)]| {
        values
            .iter()
            .map(|&(good, amount)| json!([good.0, amount]))
            .collect::<Vec<_>>()
    };
    let workers: Vec<_> = recipe
        .workers
        .iter()
        .map(|&(job, amount)| json!([catalog.jobs[job.0].good.0, amount]))
        .collect();
    let shortage_good = recipe
        .input
        .first()
        .map_or(catalog.labor_goods[0], |&(good, _)| good);
    let days = 12;
    let reference = reference_output(Some("--production"), &json!({
        "prices": catalog.base_prices.values(), "config": [config.profit_multiplier, config.max_factory_delta, config.factory_min_utilization],
        "cases": cases, "days": days, "shortage_good": shortage_good.0, "seed": seed,
        "recipe": {"input": entries(&recipe.input), "capital": entries(&recipe.capital), "output": [recipe.output.0.0 as f64, recipe.output.1],
            "power": recipe.power, "workers": workers, "construction_time": recipe.construction_ticks, "construction_cost": entries(&recipe.construction_cost)}
    }).to_string());
    let mut markets: Vec<_> = (0..states.len())
        .map(|_| {
            Market::new(
                &catalog.base_prices,
                &catalog.labor_goods,
                catalog.config.market_config,
            )
        })
        .collect();
    let mut sectors: Vec<_> = (0..states.len())
        .map(|_| ConstructionSector {
            cost: 2.0,
            ..ConstructionSector::default()
        })
        .collect();
    let mut taxes = vec![0.0; states.len()];
    let mut samples = Vec::new();
    let mut random = cqsp_core::random::LegacyRandom::new(seed.unwrap_or(42));
    for day in 0..days {
        for (market, (industry, sector)) in markets
            .iter_mut()
            .zip(industries.iter_mut().zip(&mut sectors))
        {
            market.reset();
            market.gdp = 0.0;
            industry.wallet.reset();
            sector.current = 0;
            for (index, good) in catalog.goods.iter().enumerate() {
                market.price[GoodId(index)] = good.price * (1.0 + day as f64 * 0.01);
                market.taxation[GoodId(index)] = 0.05;
                market.chronic_shortages[GoodId(index)] =
                    if GoodId(index) == shortage_good && (2..=4).contains(&day) {
                        7.0
                    } else {
                        0.0
                    };
            }
        }
        // EnTT traverses the fixture's ProductionUnit pool in reverse insertion order.
        // This controls subsystem RNG assignment, not whole-game registry parity.
        for industry in industries.iter_mut().rev() {
            industry.advance_state(recipe, config, |mean, sd| {
                seed.map_or_else(|| mean + sd * 0.4, |_| random.normal(mean, sd))
            });
        }
        let rows: Vec<_> = industries.iter_mut().enumerate().map(|(index, industry)| {
            taxes[index] += industry.produce(&catalog, &mut markets[index], &mut sectors[index]);
            json!({"industry": [industry.state as usize as f64, industry.size, industry.utilization, industry.diff,
                f64::from(industry.shortage), industry.cumulative_profit_ratio, industry.continuous_gains as f64, industry.stability as f64,
                industry.expertise, industry.revenue, industry.material_costs, industry.wage_cost, industry.tax_cost, industry.construction_cost,
                industry.profit, industry.subsidy_amount, industry.amount_sold, industry.power_consumption,
                industry.wallet.balance, industry.wallet.change, industry.wallet.gdp_change],
                "construction": industry.construction.as_ref().map(|job| [job.progress, job.maximum, job.levels]),
                "production": markets[index].production.values(), "consumption": markets[index].consumption.values(),
                "totals": [markets[index].gdp, taxes[index], sectors[index].current as f64]})
        }).collect();
        samples.push(rows);
    }
    compare(&json!(samples), &reference);
}

#[test]
#[ignore = "Run tools/build-reference.ps1 first; requires Visual Studio and the legacy headers"]
fn all_loaded_orbits_match_live_cpp_across_fifty_years() {
    let system = SolarSystem::load(&default_data_path()).unwrap();
    let times = [
        -86400.0,
        0.0,
        600.0,
        86400.0,
        31.0 * 86400.0,
        365.0 * 86400.0,
        50.0 * 365.0 * 86400.0,
    ];
    let orbits: Vec<_> = system
        .bodies
        .iter()
        .filter(|body| body.parent.is_some())
        .map(|body| body.orbit)
        .chain(system.ships.iter().map(|satellite| satellite.orbit))
        .collect();
    assert_eq!(orbits.len(), 74);
    let mut input = String::new();
    let mut expected = Vec::new();
    for orbit in orbits {
        for time in times {
            writeln!(
                input,
                "{time} {} {} {} {} {} {} {} {}",
                orbit.semi_major_axis,
                orbit.eccentricity,
                orbit.inclination,
                orbit.ascending_node,
                orbit.periapsis_argument,
                orbit.mean_anomaly,
                orbit.epoch,
                orbit.mu
            )
            .unwrap();
        }
        expected.extend(orbit_samples(orbit, &times));
    }
    compare(
        &json!({"grid": expected}),
        &reference_output(Some("--orbit-grid"), &input),
    );
}

#[test]
#[ignore = "Run tools/build-reference.ps1 first; requires Visual Studio and the legacy headers"]
fn original_orbit_assets_match_cpp_hjson_loader() {
    let data = default_data_path();
    let mut paths = Vec::new();
    for folder in ["data/planets", "data/planet_data/earth/satellites"] {
        let mut files: Vec<_> = fs::read_dir(data.join(folder))
            .unwrap()
            .map(|entry| entry.unwrap().path())
            .filter(|path| {
                path.extension()
                    .is_some_and(|extension| extension == "hjson")
            })
            .collect();
        files.sort();
        paths.extend(files);
    }
    let mut input = String::new();
    let mut expected = Vec::new();
    for path in paths {
        writeln!(input, "{}", path.display()).unwrap();
        let values: Vec<Value> = serde_hjson::from_str(&fs::read_to_string(path).unwrap()).unwrap();
        for value in values {
            expected.extend(orbit_elements(Orbit::load(&value["orbit"]).unwrap()));
        }
    }
    assert_eq!(expected.len(), 75 * 9);
    compare(
        &json!({"loaded": expected}),
        &reference_output(Some("--load-orbits"), &input),
    );
}
