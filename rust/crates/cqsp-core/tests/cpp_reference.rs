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
    fn visit(actual: &Value, expected: &Value, path: &str) {
        match (actual, expected) {
            (Value::Number(a), Value::Number(b)) => {
                let (a, b) = (a.as_f64().unwrap(), b.as_f64().unwrap());
                assert!(
                    (a - b).abs() <= 1e-12 * b.abs().max(1.0),
                    "{path}: Rust {a}, C++ {b}"
                );
            }
            (Value::Array(a), Value::Array(b)) => {
                assert_eq!(a.len(), b.len(), "{path}");
                for (index, (a, b)) in a.iter().zip(b).enumerate() {
                    visit(a, b, &format!("{path}[{index}]"));
                }
            }
            (Value::Object(a), Value::Object(b)) => {
                assert_eq!(
                    a.keys().collect::<Vec<_>>(),
                    b.keys().collect::<Vec<_>>(),
                    "{path}"
                );
                for (key, b) in b {
                    visit(&a[key], b, &format!("{path}.{key}"));
                }
            }
            _ => assert_eq!(actual, expected, "{path}"),
        }
    }
    visit(actual, expected, "$");
}

#[test]
fn matches_recorded_cpp_components() {
    compare(
        &snapshot(),
        &serde_json::from_str(include_str!("fixtures/cpp-components.json")).unwrap(),
    );
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
    serde_json::from_slice(&output.stdout).unwrap()
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
                false
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
#[ignore = "Run tools/build-reference.ps1 first; runs the actual C++ SysOrbit and SysSpacePort"]
fn lunar_trajectory_and_delivery_match_original_systems() {
    use cqsp_core::{flight::FlightStatus, spaceflight::ColonizationStep};
    let data = default_data_path();
    let world = World::load(&data).unwrap();
    let mut simulation = Simulation::load(&data, &world, 42).unwrap();
    let system = &simulation.solar_system;
    let sun = system.sun;
    let earth = system.body_id("earth").unwrap();
    let moon = system.body_id("moon").unwrap();
    let bodies: Vec<_> = [sun, earth, moon]
        .map(|id| {
            let body = &system.bodies[id.0];
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
        .into();
    let reference = reference_output(
        Some("--lunar-mission"),
        &json!({"bodies": bodies}).to_string(),
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
    simulation.missions[mission.0].in_progress = true;
    let radius = simulation.solar_system.bodies[earth.0].radius;
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
        if tick % 24 == 0 || tick < 10 {
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
                craft.commands.len()
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
    assert_eq!(samples.len(), expected.len());
    for (row, expected) in samples.iter().zip(expected) {
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
    use cqsp_core::{
        catalog::Catalog,
        economy::industry::{Construction, ConstructionSector, Industry, IndustryState},
    };
    let catalog = Catalog::load(&default_data_path()).unwrap();
    let id = catalog.recipe("farming").unwrap();
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
        "cases": cases, "days": days, "shortage_good": shortage_good.0,
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
            industry.advance_state(recipe, config, |mean, sd| mean + sd * 0.4);
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
