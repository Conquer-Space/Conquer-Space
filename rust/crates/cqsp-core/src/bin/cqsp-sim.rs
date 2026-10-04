//! Fast, renderer-free simulation verification. Does not claim whole-game parity.
use anyhow::{Context, Result, bail};
use cqsp_core::{
    World, default_data_path,
    economy::Market,
    save::{Metadata, SaveStore},
    simulation::Simulation,
};
use std::{
    fs::OpenOptions,
    io::{BufWriter, Write},
    path::{Path, PathBuf},
};

fn main() -> Result<()> {
    let mut data = default_data_path();
    let mut days = 30_u32;
    let mut seed = 42_u64;
    let mut print_paths = false;
    let mut market_csv = None;
    let mut industries_json = None;
    let mut science_json = None;
    let mut load_metadata = None;
    let mut save_metadata = None;
    let mut save_country = None;
    let mut list_saves = None;
    let mut args = std::env::args().skip(1);
    while let Some(arg) = args.next() {
        match arg.as_str() {
            "--data" => data = PathBuf::from(args.next().context("--data needs a path")?),
            "--days" => days = args.next().context("--days needs a number")?.parse()?,
            "--seed" => seed = args.next().context("--seed needs a number")?.parse()?,
            "--paths" => print_paths = true,
            "--load-metadata" => {
                load_metadata = Some(PathBuf::from(
                    args.next().context("--load-metadata needs a save folder")?,
                ))
            }
            "--save-metadata" => {
                save_metadata = Some(PathBuf::from(
                    args.next().context("--save-metadata needs a save root")?,
                ))
            }
            "--save-country" => {
                save_country = Some(
                    args.next()
                        .context("--save-country needs a country identifier")?,
                )
            }
            "--list-saves" => {
                list_saves = Some(PathBuf::from(
                    args.next().context("--list-saves needs a save root")?,
                ))
            }
            "--market-csv" => {
                market_csv = Some(PathBuf::from(
                    args.next().context("--market-csv needs a path")?,
                ))
            }
            "--industries-json" => {
                industries_json = Some(PathBuf::from(
                    args.next().context("--industries-json needs a path")?,
                ))
            }
            "--science-json" => {
                science_json = Some(PathBuf::from(
                    args.next().context("--science-json needs a path")?,
                ))
            }
            "--help" | "-h" => {
                println!(
                    "Usage: cqsp-sim [--data PATH] [--days NUMBER] [--seed NUMBER] [--paths]\n                [--market-csv NEW_PATH] [--industries-json NEW_PATH] [--science-json NEW_PATH]\n                [--load-metadata FOLDER] [--save-metadata ROOT --save-country COUNTRY]\n                [--list-saves ROOT]\n--paths prints the resolved data directory without running.\nRuns the partial Earth economy and orbital propagation; prints JSON and checks every daily step. Exports never overwrite existing files. Saves preserve upstream's metadata-only behavior, not a simulation/RNG snapshot. Loading changes date/UUID only; saving updates this game's metadata folder."
                );
                return Ok(());
            }
            _ => bail!("Unknown argument: {arg}"),
        }
    }
    if print_paths {
        println!("{}", serde_json::json!({"data":data}));
        return Ok(());
    }
    if let Some(root) = list_saves {
        let entries: Vec<_> = SaveStore(root)
            .list()?
            .into_iter()
            .map(|entry| serde_json::json!({"name":entry.name,"date":entry.date,"path":entry.path}))
            .collect();
        println!("{}", serde_json::to_string_pretty(&entries)?);
        return Ok(());
    }
    let world = World::load(&data)?;
    let mut simulation = Simulation::load(&data, &world, seed)?;
    if let Some(directory) = load_metadata {
        SaveStore::read(&directory)?.apply(&mut simulation.date, &mut simulation.uuid);
        eprintln!("Loaded only date/UUID metadata, not economy, orbital state or RNG.");
    }
    if save_metadata.is_some() {
        let country = save_country
            .as_deref()
            .context("--save-metadata requires --save-country")?;
        anyhow::ensure!(
            world
                .countries
                .iter()
                .any(|entry| entry.identifier == country),
            "Unknown save country"
        );
    }
    let mut csv = market_csv.as_deref().map(create_export).transpose()?;
    if let Some(output) = &mut csv {
        Market::write_csv_header(output, &simulation.catalog)?;
    }
    for diagnostic in &simulation.diagnostics {
        eprintln!("Legacy data: {diagnostic}");
    }
    for diagnostic in &simulation.solar_system.diagnostics {
        eprintln!("Legacy orbital data: {diagnostic}");
    }
    for _ in 0..days {
        simulation.advance_day();
        simulation.validate()?;
        if let Some(output) = &mut csv {
            simulation
                .earth_market()
                .write_csv_record(output, simulation.date)?;
        }
    }
    if let Some(output) = &mut csv {
        output.flush()?;
    }
    if let Some(path) = &industries_json {
        let mut output = create_export(path)?;
        serde_json::to_writer_pretty(&mut output, &simulation.industries_json())?;
        writeln!(output)?;
        output.flush()?;
    }
    if let Some(path) = &science_json {
        let mut output = create_export(path)?;
        serde_json::to_writer_pretty(&mut output, &simulation.science.definitions_json())?;
        writeln!(output)?;
        output.flush()?;
    }
    if let Some(root) = save_metadata {
        let path = SaveStore(root).write(
            save_country.as_deref().unwrap(),
            &Metadata::new(simulation.date, simulation.uuid.clone()),
        )?;
        eprintln!("Saved metadata only: {}", path.display());
    }
    let orbital_positions = ["earth", "moon", "mars"].map(|identifier| {
        let id = simulation.solar_system.body_id(identifier).unwrap();
        let body = &simulation.solar_system.bodies[id.0];
        serde_json::json!({"body": identifier, "absolute": body.kinematics.absolute_position().to_array(),
            "relative": body.kinematics.state.position.to_array()})
    });
    println!(
        "{}",
        serde_json::to_string_pretty(&serde_json::json!({
            "scope": "partial-earth-economy-and-orbits", "seed":seed, "daily_steps":days,
            "date":simulation.date.display(), "goods":simulation.catalog.goods.len(),
            "jobs":simulation.catalog.jobs.len(), "recipes":simulation.catalog.recipes.len(),
            "science_fields":simulation.science.fields.len(), "technologies":simulation.science.technologies.len(),
            "settlements":simulation.settlements.len(), "summary":simulation.summary(None),
            "celestial_bodies":simulation.solar_system.bodies.len(),
            "spacecraft":simulation.solar_system.ships.len(),
            "market_history_days":simulation.planetary_markets.values().next().map_or(0, |market| market.history.0.len()),
            "planetary_markets":simulation.planetary_markets.len(),
            "orbital_positions_km": orbital_positions,
            "legacy_data_diagnostics":simulation.diagnostics
        }))?
    );
    Ok(())
}

fn create_export(path: &Path) -> Result<BufWriter<std::fs::File>> {
    Ok(BufWriter::new(
        OpenOptions::new()
            .write(true)
            .create_new(true)
            .open(path)
            .with_context(|| {
                format!(
                    "Creating new export {}; existing files are preserved",
                    path.display()
                )
            })?,
    ))
}
