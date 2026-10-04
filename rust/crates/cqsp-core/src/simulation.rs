//! Earth economy slice. Missing legacy systems are tracked in rust/PARITY.md.
use crate::{
    World,
    catalog::Catalog,
    clock::{DAY, StarDate, WEEK},
    economy::{
        GoodId, Market, Wallet,
        history::MarketHistory,
        industry::{ConstructionSector, Industry, IndustryEntry},
        population::{Population, PopulationEntry},
    },
    modifiers::ModifierCatalog,
    read_entries,
    solar_system::SolarSystem,
};
use anyhow::{Context, Result, ensure};
use rand::{SeedableRng, rngs::SmallRng};
use rand_distr::{Distribution, Normal};
use serde::{Deserialize, Serialize};
use serde_json::Value;
use std::{collections::HashMap, path::Path};

pub struct Settlement {
    pub identifier: String,
    pub name: String,
    pub country: Option<usize>,
    pub populations: Vec<Population>,
    pub industries: Vec<Industry>,
    pub market: Market,
    pub construction: ConstructionSector,
    pub job_demands: Vec<f64>,
}

impl Settlement {
    fn reset(&mut self) {
        self.market.reset();
        self.construction.current = 0;
        for population in &mut self.populations {
            population.wallet.reset();
        }
        for industry in &mut self.industries {
            industry.wallet.reset();
        }
    }
}

#[derive(Default)]
pub struct CountryEconomy {
    pub wallet: Option<Wallet>,
    pub income_taxes: f64,
}

#[derive(Default, Debug, Clone, Serialize)]
pub struct Summary {
    pub population: u64,
    pub employed: u64,
    pub standard_of_living: f64,
    pub gdp: f64,
    pub industries: usize,
    pub invalid_unemployment_rates: usize,
}

pub struct Simulation {
    pub date: StarDate,
    pub solar_system: SolarSystem,
    pub catalog: Catalog,
    pub modifiers: ModifierCatalog,
    /// Map provinces come first, matching World::provinces indices; then cities.
    pub settlements: Vec<Settlement>,
    pub countries: Vec<CountryEconomy>,
    pub planetary_market: Market,
    pub history: Vec<Summary>,
    pub market_history: MarketHistory,
    pub diagnostics: Vec<String>,
    autonomous: Vec<(GoodId, f64)>,
    marginal: Vec<(GoodId, f64)>,
    random: SmallRng,
}

impl Simulation {
    pub fn load(data: &Path, world: &World, seed: u64) -> Result<Self> {
        let catalog = Catalog::load(data)?;
        let country_ids: HashMap<_, _> = world
            .countries
            .iter()
            .enumerate()
            .map(|(index, country)| (country.identifier.as_str(), index))
            .collect();
        let mut countries: Vec<_> = (0..world.countries.len())
            .map(|_| CountryEconomy::default())
            .collect();
        for entry in read_entries::<CountryEntry>(&data.join("data/countries"))? {
            if let Some(balance) = entry.wallet {
                let mut wallet = Wallet::default();
                wallet.assign(balance);
                countries[country_ids[entry.identifier.as_str()]].wallet = Some(wallet);
            }
        }
        let mut diagnostics = catalog.diagnostics.clone();
        let modifiers = ModifierCatalog::load(data)?;
        diagnostics.extend(modifiers.diagnostics.iter().cloned());
        let mut settlements = Vec::new();
        let provinces =
            read_entries::<SettlementEntry>(&data.join("data/planet_data/provinces/earth"))?;
        let cities = read_entries::<SettlementEntry>(&data.join("data/cities/earth"))?;
        for entry in provinces
            .into_iter()
            .filter(|entry| !entry.is_template())
            .chain(cities)
        {
            ensure!(
                entry.planet.as_deref() == Some("earth"),
                "{}: Earth slice cannot load another planet",
                entry.identifier
            );
            let country = entry
                .country
                .as_ref()
                .and_then(|identifier| country_ids.get(identifier.as_str()).copied());
            let populations = entry
                .population
                .into_iter()
                .map(|entry| Population::load(entry, &catalog))
                .collect::<Result<Vec<_>>>()?;
            let mut industries = Vec::new();
            for industry in entry.industry {
                let Some(recipe) = catalog.recipe(&industry.recipe) else {
                    diagnostics.push(format!(
                        "{}: unknown recipe {}; skipped like the C++ loader",
                        entry.identifier, industry.recipe
                    ));
                    continue;
                };
                let mut factory =
                    Industry::new(recipe, industry.size as i32).with_context(|| {
                        format!("{}: invalid industry {}", entry.identifier, industry.recipe)
                    })?;
                factory.utilization = industry.utilization.unwrap_or(factory.utilization);
                ensure!(
                    factory.utilization.is_finite() && factory.utilization >= 0.0,
                    "Invalid utilization in {}",
                    entry.identifier
                );
                factory.revenue = match industry.revenue {
                    Some(Value::Number(value)) => value.as_f64().context("Invalid revenue")?,
                    Some(Value::String(value)) => value.parse().unwrap_or_else(|_| {
                        diagnostics.push(format!(
                            "{}: invalid numeric revenue {value}; C++ converts it to zero",
                            entry.identifier
                        ));
                        0.0
                    }),
                    None | Some(Value::Null) => 0.0,
                    _ => anyhow::bail!("{}: invalid revenue type", entry.identifier),
                };
                factory.material_costs = industry.material_costs.unwrap_or(0.0);
                factory.profit = industry.profit.unwrap_or(0.0);
                factory.continuous_gains = industry.continuous_gains.unwrap_or(0.0) as i32;
                industries.push(factory);
            }
            settlements.push(Settlement {
                name: entry.name.unwrap_or_else(|| entry.identifier.clone()),
                identifier: entry.identifier,
                country,
                populations,
                industries,
                market: Market::new(
                    &catalog.base_prices,
                    &catalog.labor_goods,
                    catalog.config.market_config,
                ),
                construction: ConstructionSector::default(),
                job_demands: vec![0.0; catalog.jobs.len()],
            });
        }
        for (index, province) in world.provinces.iter().enumerate() {
            ensure!(
                settlements
                    .get(index)
                    .is_some_and(|settlement| settlement.identifier == province.identifier
                        && settlement.country == province.country),
                "Simulation/render map disagree at province {index}"
            );
        }
        let mut autonomous = Vec::new();
        let mut marginal = Vec::new();
        for (index, good) in catalog.goods.iter().enumerate() {
            if let Some(consumption) = &good.consumption {
                let good = GoodId(index);
                autonomous.push((good, consumption.autonomous_consumption * DAY as f64));
                marginal.push((good, consumption.marginal_propensity * DAY as f64));
            }
        }
        let planetary_market = Market::new(
            &catalog.base_prices,
            &catalog.labor_goods,
            catalog.config.market_config,
        );
        Ok(Self {
            date: StarDate::default(),
            solar_system: SolarSystem::load(data)?,
            catalog,
            modifiers,
            settlements,
            countries,
            planetary_market,
            history: Vec::new(),
            market_history: MarketHistory::default(),
            diagnostics,
            autonomous,
            marginal,
            random: SmallRng::seed_from_u64(seed),
        })
    }

    pub fn tick(&mut self) {
        self.date.advance();
        self.solar_system.update(self.date.seconds());
        if !self.date.due(DAY) {
            return;
        }
        for country in &mut self.countries {
            if let Some(wallet) = &mut country.wallet {
                wallet.reset();
            }
        }
        self.planetary_market.reset();
        for settlement in &mut self.settlements {
            settlement.reset();
        }
        let default_labor = self.catalog.jobs[self.catalog.default_job.0].good;
        for settlement in &mut self.settlements {
            settlement
                .construction
                .purchase_labor(&mut settlement.market, default_labor);
        }
        // Original system order matters: weekly labor sees infrastructure demand and old ratios.
        if self.date.due(WEEK) {
            for settlement in &mut self.settlements {
                for population in &mut settlement.populations {
                    population.redistribute_labor(&settlement.market, &self.catalog);
                }
            }
        }
        for settlement in &mut self.settlements {
            for population in &mut settlement.populations {
                population.consume(&mut settlement.market, &self.autonomous, &self.marginal);
            }
        }
        for settlement in &mut self.settlements {
            for industry in &mut settlement.industries {
                industry.advance_state(
                    &self.catalog.recipes[industry.recipe.0],
                    self.catalog.config.production_config,
                    |mean, deviation| {
                        Normal::new(mean, deviation)
                            .unwrap()
                            .sample(&mut self.random)
                    },
                );
            }
        }
        for settlement in &mut self.settlements {
            if settlement.populations.is_empty() {
                continue;
            }
            settlement.market.gdp = 0.0;
            for industry in &mut settlement.industries {
                let taxes = industry.produce(
                    &self.catalog,
                    &mut settlement.market,
                    &mut settlement.construction,
                );
                if let Some(country) = settlement.country {
                    self.countries[country].income_taxes += taxes;
                }
            }
        }
        for settlement in &mut self.settlements {
            settlement
                .market
                .process(&self.catalog.base_prices, self.catalog.config.market_config);
        }
        self.planetary_market.gdp = 0.0;
        self.planetary_market.trade.clear();
        for settlement in &self.settlements {
            self.planetary_market
                .supply
                .add_ledger(&settlement.market.production);
            self.planetary_market
                .demand
                .add_ledger(&settlement.market.consumption);
            self.planetary_market.gdp += settlement.market.gdp;
        }
        // Legacy planetary pricing hard-codes 0.75 and does not update its S/D ratios.
        self.planetary_market
            .determine_prices(&self.catalog.base_prices, 0.75);
        for settlement in &mut self.settlements {
            settlement.market.trade_with(&self.planetary_market);
            for (index, job) in self.catalog.jobs.iter().enumerate() {
                settlement.job_demands[index] = (settlement.market.supply[job.good]
                    - 2.0 * settlement.market.demand[job.good])
                    / (40.0 / 7.0);
            }
        }
        self.history.push(self.summary(None));
        self.market_history
            .record(self.date, &self.planetary_market);
    }

    /// Runs real ten-minute ticks, including tick zero on the first call.
    pub fn advance_day(&mut self) {
        self.tick();
        while !self.date.due(DAY) {
            self.tick();
        }
    }

    pub fn summary(&self, country: Option<usize>) -> Summary {
        let mut summary = Summary::default();
        let mut weighted_sol = 0.0;
        for settlement in &self.settlements {
            if country.is_some() && settlement.country != country {
                continue;
            }
            summary.gdp += settlement.market.gdp;
            summary.industries += settlement.industries.len();
            for population in &settlement.populations {
                summary.population += population.size;
                summary.employed = summary.employed.saturating_add(population.employed);
                weighted_sol += population.standard_of_living * population.size as f64;
                summary.invalid_unemployment_rates +=
                    usize::from(population.unemployment_rate.is_none());
            }
        }
        if summary.population > 0 {
            summary.standard_of_living = weighted_sol / summary.population as f64;
        }
        summary
    }

    /// Production-unit fields exported by the legacy market dumper, without empty zones.
    pub fn industries_json(&self) -> Value {
        self.settlements
            .iter()
            .filter(|settlement| !settlement.industries.is_empty())
            .map(|settlement| {
                (settlement.identifier.clone(), Value::Array(settlement.industries.iter()
                .map(|industry| serde_json::json!({
                    "recipe": self.catalog.recipes[industry.recipe.0].identifier,
                    "size": industry.size, "utilization": industry.utilization,
                    "continuous_gains": industry.continuous_gains, "revenue": industry.revenue,
                    "material_costs": industry.material_costs, "profit": industry.profit,
                })).collect()))
            })
            .collect::<serde_json::Map<_, _>>()
            .into()
    }

    pub fn validate(&self) -> Result<()> {
        self.solar_system.validate()?;
        for settlement in &self.settlements {
            ensure!(
                settlement
                    .market
                    .price
                    .values()
                    .iter()
                    .all(|value| value.is_finite() && *value >= 0.0),
                "{}: invalid market price",
                settlement.identifier
            );
            ensure!(
                settlement
                    .market
                    .trade
                    .values()
                    .iter()
                    .all(|value| value.is_finite()),
                "{}: invalid trade",
                settlement.identifier
            );
            for population in &settlement.populations {
                ensure!(
                    population.wallet.balance.is_finite()
                        && population.standard_of_living.is_finite(),
                    "{}: invalid population finances",
                    settlement.identifier
                );
            }
            for industry in &settlement.industries {
                ensure!(
                    industry.profit.is_finite()
                        && industry.wallet.balance.is_finite()
                        && industry.utilization.is_finite()
                        && industry.expertise_gain.value().is_finite()
                        && industry.max_expertise.value().is_finite()
                        && industry.max_expertise.value() >= 0.0
                        && industry.size >= 1.0,
                    "{}: invalid industry",
                    settlement.identifier
                );
            }
        }
        Ok(())
    }
}

#[derive(Deserialize)]
struct CountryEntry {
    identifier: String,
    wallet: Option<f64>,
}

#[derive(Deserialize)]
struct SettlementEntry {
    identifier: String,
    name: Option<String>,
    country: Option<String>,
    planet: Option<String>,
    color: Option<Value>,
    #[serde(default)]
    population: Vec<PopulationEntry>,
    #[serde(default)]
    industry: Vec<IndustryEntry>,
}
impl SettlementEntry {
    fn is_template(&self) -> bool {
        self.identifier == "name" && self.color == Some(Value::String("color".into()))
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    fn load(seed: u64) -> Result<Simulation> {
        let data = crate::default_data_path();
        Simulation::load(&data, &World::load(&data)?, seed)
    }
    #[test]
    fn first_tick_and_daily_weekly_scheduling_use_original_intervals() -> Result<()> {
        let mut simulation = load(42)?;
        assert!(simulation.settlements.len() > 1067);
        simulation.tick();
        assert_eq!(simulation.date.tick(), 0);
        assert_eq!(simulation.history.len(), 1);
        assert_eq!(simulation.summary(None).population, 549_700_354);
        let earth = simulation.solar_system.body_id("earth").unwrap();
        let before = simulation.solar_system.bodies[earth.0]
            .kinematics
            .state
            .position;
        simulation.tick();
        assert_eq!(simulation.date.tick(), 1);
        assert_eq!(simulation.history.len(), 1);
        assert_ne!(
            simulation.solar_system.bodies[earth.0]
                .kinematics
                .state
                .position,
            before
        );
        for _ in 0..DAY - 2 {
            simulation.tick();
        }
        assert_eq!(simulation.history.len(), 1);
        simulation.tick();
        assert_eq!(simulation.history.len(), 2);
        assert_eq!(simulation.market_history.0.len(), 2);
        assert_eq!(simulation.market_history.0[1].date, simulation.date);
        assert_eq!(
            simulation.market_history.0[1].supply,
            simulation.planetary_market.supply
        );
        assert!(
            simulation
                .industries_json()
                .as_object()
                .is_some_and(|zones| !zones.is_empty())
        );
        assert_eq!(simulation.date.display(), "1950-01-02");
        simulation.validate()?;
        Ok(())
    }
    #[test]
    fn month_is_finite_and_seeded_runs_are_repeatable() -> Result<()> {
        let mut first = load(42)?;
        let mut second = load(42)?;
        for _ in 0..30 {
            first.advance_day();
            second.advance_day();
            first.validate()?;
            second.validate()?;
        }
        assert_eq!(first.date.display(), "1950-01-30");
        assert_eq!(first.history.len(), 30);
        assert_eq!(
            serde_json::to_value(&first.history)?,
            serde_json::to_value(&second.history)?
        );
        for (a, b) in first.settlements.iter().zip(&second.settlements) {
            assert_eq!(a.market.price, b.market.price);
            assert_eq!(a.market.trade, b.market.trade);
        }
        Ok(())
    }
}
