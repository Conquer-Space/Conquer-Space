//! Typed legacy economy assets; symbol references resolve once, during loading.
use crate::{
    clock::{StarDate, WEEK},
    economy::{EconomyConfig, GoodId, Ledger},
    read_entries,
    units::{Unit, read_unit},
};
use anyhow::{Context, Result, ensure};
use serde::Deserialize;
use serde_json::Value;
use std::{
    collections::{BTreeMap, HashMap},
    fs,
    path::Path,
};

#[derive(Debug, Deserialize)]
pub struct Consumption {
    pub autonomous_consumption: f64,
    pub marginal_propensity: f64,
}

#[derive(Debug)]
pub struct Good {
    pub identifier: String,
    pub name: String,
    pub price: f64,
    pub tags: Vec<String>,
    pub consumption: Option<Consumption>,
    pub mass: Option<f64>,
    pub volume: Option<f64>,
    pub energy: Option<f64>,
    pub unit: Option<String>,
}

#[derive(Clone, Copy, Debug, PartialEq, Eq, PartialOrd, Ord)]
pub struct JobId(pub usize);

#[derive(Debug)]
pub struct Job {
    pub identifier: String,
    pub name: String,
    pub good: GoodId,
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct RecipeId(pub usize);

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum ProductionType {
    Mine,
    Factory,
    Service,
    Unspecified,
}

#[derive(Debug)]
pub struct Recipe {
    pub identifier: String,
    pub name: String,
    pub input: Vec<(GoodId, f64)>,
    pub output: (GoodId, f64),
    pub capital: Vec<(GoodId, f64)>,
    pub workers: Vec<(JobId, u32)>,
    pub power: f64,
    pub kind: ProductionType,
    pub fixed_cost: Vec<(GoodId, f64)>,
    pub scaling_cost: Vec<(GoodId, f64)>,
    pub construction_ticks: i32,
    pub construction_cost: Vec<(GoodId, f64)>,
    pub zoning: BTreeMap<String, i32>,
}

pub struct Catalog {
    pub goods: Vec<Good>,
    pub jobs: Vec<Job>,
    pub recipes: Vec<Recipe>,
    pub default_job: JobId,
    pub labor_goods: Vec<GoodId>,
    pub base_prices: Ledger,
    pub config: EconomyConfig,
    pub diagnostics: Vec<String>,
    good_ids: HashMap<String, GoodId>,
    job_ids: HashMap<String, JobId>,
    recipe_ids: HashMap<String, RecipeId>,
}

impl Catalog {
    pub fn load(data: &Path) -> Result<Self> {
        let config_path = data.join("data/economy_config.hjson");
        let value: Value = serde_hjson::from_str(&fs::read_to_string(&config_path)?)?;
        let config = serde_json::from_value(value).context("Reading economy configuration")?;
        let mut goods = Vec::new();
        let mut good_ids = HashMap::new();
        let mut diagnostics = Vec::new();
        for entry in read_entries::<GoodEntry>(&data.join("data/goods"))? {
            ensure!(
                !good_ids.contains_key(&entry.identifier),
                "Duplicate good {}",
                entry.identifier
            );
            let physical = match (&entry.mass, &entry.volume) {
                (Some(mass), Some(volume)) => {
                    match (read_unit(mass, Unit::Mass), read_unit(volume, Unit::Volume)) {
                        (Ok(mass), Ok(volume)) => Some((mass, volume)),
                        _ => {
                            diagnostics.push(format!(
                                "{}: invalid mass/volume; skipped like the C++ loader",
                                entry.identifier
                            ));
                            continue;
                        }
                    }
                }
                _ => None,
            };
            good_ids.insert(entry.identifier.clone(), GoodId(goods.len()));
            goods.push(Good {
                name: entry.name.unwrap_or_else(|| entry.identifier.clone()),
                identifier: entry.identifier,
                price: entry.price,
                tags: entry.tags,
                consumption: entry.consumption,
                mass: physical.map(|pair| pair.0),
                volume: physical.map(|pair| pair.1),
                energy: entry.energy,
                unit: entry.unit,
            });
        }
        let mut jobs = Vec::new();
        let mut job_ids = HashMap::new();
        let mut default_job = None;
        for entry in read_entries::<JobEntry>(&data.join("data/labor"))? {
            ensure!(
                !job_ids.contains_key(&entry.identifier),
                "Duplicate job {}",
                entry.identifier
            );
            let good = *good_ids
                .get(&entry.good)
                .with_context(|| format!("Unknown labor good {}", entry.good))?;
            let id = JobId(jobs.len());
            if entry.tags.iter().any(|tag| tag == "default") {
                default_job = Some(id);
            }
            job_ids.insert(entry.identifier.clone(), id);
            jobs.push(Job {
                name: entry.name.unwrap_or_else(|| entry.identifier.clone()),
                identifier: entry.identifier,
                good,
            });
        }
        let mut base_prices = Ledger::new(goods.len());
        let mut labor_goods = Vec::new();
        for (index, good) in goods.iter().enumerate() {
            base_prices[GoodId(index)] = good.price;
            if good.tags.iter().any(|tag| tag == "labor") {
                labor_goods.push(GoodId(index));
            }
        }
        let mut catalog = Self {
            goods,
            jobs,
            recipes: Vec::new(),
            default_job: default_job.context("No default job")?,
            labor_goods,
            base_prices,
            config,
            diagnostics,
            good_ids,
            job_ids,
            recipe_ids: HashMap::new(),
        };
        for entry in read_entries::<RecipeEntry>(&data.join("data/recipes"))? {
            ensure!(
                !catalog.recipe_ids.contains_key(&entry.identifier),
                "Duplicate recipe {}",
                entry.identifier
            );
            ensure!(
                entry.output.len() == 1,
                "{}: expected exactly one recipe output",
                entry.identifier
            );
            let (output, amount) = entry.output.into_iter().next().unwrap();
            let output = catalog
                .good(&output)
                .with_context(|| format!("{}: unknown recipe output {output}", entry.identifier))?;
            let input = catalog.resources(&entry.input, &entry.identifier);
            let capital = catalog.resources(&entry.cost.capital, &entry.identifier);
            let fixed_cost = catalog.resources(&entry.cost.fixed, &entry.identifier);
            let scaling_cost = catalog.resources(&entry.cost.scaling, &entry.identifier);
            let workers = entry
                .cost
                .labor
                .iter()
                .map(|(job, amount)| {
                    Ok((
                        catalog
                            .job(job)
                            .with_context(|| format!("{}: unknown job {job}", entry.identifier))?,
                        *amount,
                    ))
                })
                .collect::<Result<_>>()?;
            let power = entry
                .cost
                .power
                .as_ref()
                .map(|value| read_unit(value, Unit::Power))
                .transpose()?
                .unwrap_or(0.0);
            let (construction_ticks, construction_cost, zoning) =
                if let Some(construction) = entry.construction {
                    let seconds = construction
                        .time
                        .as_ref()
                        .and_then(|value| read_unit(value, Unit::Time).ok())
                        .unwrap_or(100.0);
                    ensure!(
                        seconds > 0.0,
                        "{}: invalid construction duration",
                        entry.identifier
                    );
                    let costs = catalog
                        .resources(&construction.cost, &entry.identifier)
                        .into_iter()
                        .map(|(good, amount)| (good, amount / seconds))
                        .collect();
                    (
                        StarDate::ticks_from_seconds(seconds as i32 as f64),
                        costs,
                        construction.zoning,
                    )
                } else {
                    (12 * WEEK as i32, Vec::new(), BTreeMap::new())
                };
            let kind = entry
                .tags
                .iter()
                .fold(ProductionType::Unspecified, |kind, tag| {
                    match tag.as_str() {
                        "raw" => ProductionType::Mine,
                        "factory" => ProductionType::Factory,
                        "service" => ProductionType::Service,
                        _ => kind,
                    }
                });
            catalog
                .recipe_ids
                .insert(entry.identifier.clone(), RecipeId(catalog.recipes.len()));
            catalog.recipes.push(Recipe {
                name: entry.name.unwrap_or_else(|| entry.identifier.clone()),
                identifier: entry.identifier,
                input,
                output: (output, amount),
                capital,
                workers,
                power,
                kind,
                fixed_cost,
                scaling_cost,
                construction_ticks,
                construction_cost,
                zoning,
            });
        }
        Ok(catalog)
    }

    pub fn good(&self, identifier: &str) -> Option<GoodId> {
        self.good_ids.get(identifier).copied()
    }
    pub fn job(&self, identifier: &str) -> Option<JobId> {
        self.job_ids.get(identifier).copied()
    }
    pub fn recipe(&self, identifier: &str) -> Option<RecipeId> {
        self.recipe_ids.get(identifier).copied()
    }

    fn resources(&mut self, values: &BTreeMap<String, f64>, owner: &str) -> Vec<(GoodId, f64)> {
        let mut resources = Vec::new();
        for (identifier, &amount) in values {
            if let Some(good) = self.good(identifier) {
                resources.push((good, amount));
            } else {
                self.diagnostics.push(format!(
                    "{owner}: unknown good {identifier}; skipped like the C++ loader"
                ));
            }
        }
        resources.sort_by_key(|&(good, _)| good);
        resources
    }
}

#[derive(Deserialize)]
struct GoodEntry {
    identifier: String,
    name: Option<String>,
    #[serde(default = "one")]
    price: f64,
    #[serde(default)]
    tags: Vec<String>,
    consumption: Option<Consumption>,
    mass: Option<Value>,
    volume: Option<Value>,
    energy: Option<f64>,
    unit: Option<String>,
}
fn one() -> f64 {
    1.0
}

#[derive(Deserialize)]
struct JobEntry {
    identifier: String,
    name: Option<String>,
    good: String,
    #[serde(default)]
    tags: Vec<String>,
}

#[derive(Default, Deserialize)]
#[serde(default)]
struct RecipeCost {
    capital: BTreeMap<String, f64>,
    labor: BTreeMap<String, u32>,
    fixed: BTreeMap<String, f64>,
    scaling: BTreeMap<String, f64>,
    power: Option<Value>,
}
#[derive(Deserialize)]
struct ConstructionEntry {
    time: Option<Value>,
    #[serde(default)]
    cost: BTreeMap<String, f64>,
    #[serde(default)]
    zoning: BTreeMap<String, i32>,
}
#[derive(Deserialize)]
struct RecipeEntry {
    identifier: String,
    name: Option<String>,
    #[serde(default)]
    input: BTreeMap<String, f64>,
    output: BTreeMap<String, f64>,
    #[serde(default)]
    cost: RecipeCost,
    construction: Option<ConstructionEntry>,
    #[serde(default)]
    tags: Vec<String>,
}

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn original_catalog_and_config_load() -> Result<()> {
        let catalog = Catalog::load(&crate::default_data_path())?;
        assert!(catalog.goods.len() > 50);
        assert_eq!(catalog.jobs.len(), 5);
        assert_eq!(catalog.recipes.len(), 22);
        assert_eq!(catalog.jobs[catalog.default_job.0].identifier, "laborer");
        assert_eq!(catalog.config.production_config.profit_multiplier, 0.0005);
        assert_eq!(catalog.config.production_config.max_factory_delta, 0.05);
        let recipe = &catalog.recipes[catalog.recipe("farming").unwrap().0];
        assert_eq!(recipe.power, 5e6);
        assert_eq!(recipe.construction_ticks, 4032);
        assert_eq!(
            recipe.output,
            (catalog.good("cereal_grains").unwrap(), 10000.0)
        );
        assert_eq!(
            recipe
                .workers
                .iter()
                .map(|&(_, workers)| workers)
                .sum::<u32>(),
            1500
        );
        assert!((recipe.construction_cost[0].1 - 2e6 / (4.0 * 604800.0)).abs() < 1e-12);
        Ok(())
    }
    #[test]
    fn quantities_preserve_legacy_units() -> Result<()> {
        assert_eq!(read_unit(&Value::String("5MW".into()), Unit::Power)?, 5e6);
        assert_eq!(
            read_unit(&Value::String("1.5e3 kg".into()), Unit::Mass)?,
            1500.0
        );
        assert_eq!(
            read_unit(&Value::String("2 months".into()), Unit::Time)?,
            120.0
        );
        assert!(read_unit(&Value::String("2".into()), Unit::Time).is_err());
        Ok(())
    }
}
