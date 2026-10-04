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
