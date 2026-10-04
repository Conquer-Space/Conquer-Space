//! Fast, renderer-free simulation verification. Does not claim whole-game parity.
use anyhow::{Context, Result, bail};
use cqsp_core::{World, default_data_path, economy::Market, simulation::Simulation};
use std::{
    fs::OpenOptions,
    io::{BufWriter, Write},
    path::{Path, PathBuf},
};

fn main() -> Result<()> {
    let mut data = default_data_path();
    let mut days = 30_u32;
    let mut seed = 42_u64;
    let mut market_csv = None;
    let mut industries_json = None;
    let mut science_json = None;
    let mut args = std::env::args().skip(1);
    while let Some(arg) = args.next() {
        match arg.as_str() {
            "--data" => data = PathBuf::from(args.next().context("--data needs a path")?),
            "--days" => days = args.next().context("--days needs a number")?.parse()?,
            "--seed" => seed = args.next().context("--seed needs a number")?.parse()?,
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
                    "Usage: cqsp-sim [--data PATH] [--days NUMBER] [--seed NUMBER]\n                [--market-csv NEW_PATH] [--industries-json NEW_PATH] [--science-json NEW_PATH]\nRuns the partial Earth economy and orbital propagation; prints JSON and checks every daily step. Exports never overwrite existing files."
                );
                return Ok(());
            }
            _ => bail!("Unknown argument: {arg}"),
        }
    }
    let world = World::load(&data)?;
    let mut simulation = Simulation::load(&data, &world, seed)?;
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
            "market_history_days":simulation.market_history.0.len(),
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
