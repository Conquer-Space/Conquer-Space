//! Native economic scheduling and surface/spaceflight integration.
use crate::{
    World,
    catalog::{Catalog, RecipeId},
    clock::{DAY, StarDate, WEEK},
    economy::{
        GoodId, Market, Wallet,
        industry::{Construction, ConstructionSector, Industry, IndustryEntry},
        population::{Population, PopulationEntry},
    },
    infrastructure::{InfrastructureCatalog, SurfaceInfrastructure, ZoningAllocation},
    modifiers::ModifierCatalog,
    random::LegacyRandom,
    read_entries,
    science::ScienceCatalog,
    solar_system::{BodyId, SolarSystem},
    spaceflight::{
        ColonizationTarget, Colony, LaunchVehicle, Mission, MissionId, PlanetaryMarket,
        ProjectTemplate, SpacePort, VehicleId,
    },
    timezones::{TimeZoneId, TimeZones},
};
use anyhow::{Context, Result, ensure};
use serde::{Deserialize, Serialize};
use serde_json::Value;
use std::{
    collections::{BTreeMap, HashMap},
    path::Path,
};

#[derive(Clone, Copy, Debug)]
pub enum SurfaceKind {
    Province,
    City {
        province: Option<usize>,
        latitude: f64,
        longitude: f64,
    },
}

pub struct Settlement {
    pub identifier: String,
    pub name: String,
    pub country: Option<usize>,
    /// Governed is separate from Province.country; colony delivery changes only the latter.
    pub governor: Option<usize>,
    pub body: BodyId,
    pub kind: SurfaceKind,
    pub time_zone: Option<TimeZoneId>,
    pub is_capital: bool,
    pub zoning: BTreeMap<String, ZoningAllocation>,
    pub infrastructure: Option<SurfaceInfrastructure>,
    pub spaceport: Option<SpacePort>,
    pub colonization: Option<ColonizationTarget>,
    pub colony: Option<Colony>,
    pub populations: Vec<Population>,
    pub industries: Vec<Industry>,
    pub market: Market,
    pub construction: ConstructionSector,
    pub job_demands: Vec<f64>,
}

impl Settlement {
    pub fn is_province(&self) -> bool {
        matches!(self.kind, SurfaceKind::Province)
    }
    pub fn province(&self) -> Option<usize> {
        match self.kind {
            SurfaceKind::Province => None,
            SurfaceKind::City { province, .. } => province,
        }
    }
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
    pub consumption_taxes: f64,
    pub capital: Option<usize>,
    pub space_program: bool,
    pub provinces: Vec<usize>,
    pub cities: Vec<usize>,
    pub spaceports: Vec<usize>,
    pub vehicles: Vec<VehicleId>,
    pub missions: Vec<MissionId>,
    pub research: i32,
    pub subsidies: BTreeMap<RecipeId, f64>,
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
    pub uuid: String,
    pub solar_system: SolarSystem,
    pub catalog: Catalog,
    pub infrastructure: InfrastructureCatalog,
    pub modifiers: ModifierCatalog,
    pub science: ScienceCatalog,
    pub time_zones: TimeZones,
    /// Map provinces come first, matching World::provinces indices; then cities.
    pub settlements: Vec<Settlement>,
    pub countries: Vec<CountryEconomy>,
    pub planetary_markets: BTreeMap<BodyId, PlanetaryMarket>,
    pub vehicles: Vec<LaunchVehicle>,
    pub project_templates: Vec<ProjectTemplate>,
    pub missions: Vec<Mission>,
    pub history: Vec<Summary>,
    pub diagnostics: Vec<String>,
    autonomous: Vec<(GoodId, f64)>,
    marginal: Vec<(GoodId, f64)>,
    random: LegacyRandom,
}

impl Simulation {
    pub fn load(data: &Path, world: &World, seed: u64) -> Result<Self> {
        let catalog = Catalog::load(data)?;
        let infrastructure = InfrastructureCatalog::load(data, &catalog)?;
        let solar_system = SolarSystem::load(data)?;
        let project_templates = ProjectTemplate::load(data, &catalog)?;
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
            let country = &mut countries[country_ids[entry.identifier.as_str()]];
            country.space_program = entry.tags.iter().any(|tag| tag == "space_program");
            if let Some(balance) = entry.wallet {
                let mut wallet = Wallet::default();
                wallet.assign(balance);
                country.wallet = Some(wallet);
            }
        }
        let mut diagnostics = catalog.diagnostics.clone();
        diagnostics.extend(infrastructure.diagnostics.iter().cloned());
        let modifiers = ModifierCatalog::load(data)?;
        diagnostics.extend(modifiers.diagnostics.iter().cloned());
        let science = ScienceCatalog::load(data)?;
        diagnostics.extend(science.diagnostics.iter().cloned());
        let time_zones = TimeZones::load(data)?;
        diagnostics.extend(time_zones.diagnostics.iter().cloned());
        let mut settlements: Vec<Settlement> = Vec::new();
        let provinces = read_entries::<SettlementEntry>(&data.join("data/planet_data/provinces"))?;
        let cities = read_entries::<SettlementEntry>(&data.join("data/cities"))?;
        let mut city_provinces = Vec::new();
        for (entry, city) in provinces
            .into_iter()
            .filter(|entry| !entry.is_template())
            .map(|entry| (entry, false))
            .chain(cities.into_iter().map(|entry| (entry, true)))
        {
            let body = entry
                .planet
                .as_deref()
                .and_then(|name| solar_system.body_id(name))
                .with_context(|| format!("{}: unknown settlement body", entry.identifier))?;
            let country = entry
                .country
                .as_ref()
                .and_then(|identifier| country_ids.get(identifier.as_str()).copied());
            let time_zone = entry
                .timezone
                .as_deref()
                .filter(|name| !name.is_empty())
                .and_then(|name| {
                    let zone = time_zones.id(name);
                    if zone.is_none() {
                        diagnostics.push(format!("{}: unknown time zone {name}", entry.identifier));
                    }
                    zone
                });
            let populations = entry
                .population
                .into_iter()
                .filter(|_| !city) // CityLoader ignores economic asset fields.
                .map(|entry| Population::load(entry, &catalog))
                .collect::<Result<Vec<_>>>()?;
            let mut industries = Vec::new();
            for industry in entry.industry.into_iter().filter(|_| !city) {
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
            let kind = if city {
                city_provinces.push((settlements.len(), entry.province));
                let coordinates = entry.coordinates.unwrap_or_default();
                SurfaceKind::City {
                    province: None,
                    latitude: coordinates.latitude.to_radians(),
                    longitude: coordinates.longitude.to_radians(),
                }
            } else {
                SurfaceKind::Province
            };
            let is_capital = !city && entry.tags.iter().any(|tag| tag == "capital");
            if is_capital && let Some(country) = country {
                let previous = countries[country].capital.replace(settlements.len());
                if let Some(previous) = previous {
                    settlements[previous].is_capital = false;
                }
            }
            let surface_infrastructure = if city {
                None
            } else {
                Some(
                    SurfaceInfrastructure::load(
                        entry.transport,
                        entry.infrastructure,
                        &infrastructure,
                    )
                    .with_context(|| format!("{}: province infrastructure", entry.identifier))?,
                )
            };
            settlements.push(Settlement {
                name: entry.name.unwrap_or_else(|| entry.identifier.clone()),
                identifier: entry.identifier,
                country,
                governor: country,
                body,
                kind,
                time_zone,
                is_capital,
                zoning: if city {
                    BTreeMap::new()
                } else {
                    ZoningAllocation::load(entry.zoning, &catalog)
                },
                infrastructure: surface_infrastructure,
                spaceport: (city
                    && entry.spaceport.is_some_and(|value| {
                        value != Value::Null
                            && !value.as_object().is_some_and(|object| object.is_empty())
                    }))
                .then(|| SpacePort::new(body, catalog.goods.len())),
                colonization: None,
                colony: None,
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
            if let Some(country) = country {
                let index = settlements.len() - 1;
                if city {
                    countries[country].cities.push(index);
                    if settlements[index].spaceport.is_some() {
                        countries[country].spaceports.push(index);
                    }
                } else {
                    countries[country].provinces.push(index);
                }
            }
        }
        for (city, identifier) in city_provinces {
            let province = identifier.as_ref().and_then(|identifier| {
                settlements.iter().position(|settlement| {
                    settlement.is_province() && settlement.identifier == *identifier
                })
            });
            if identifier.is_some() && province.is_none() {
                diagnostics.push(format!(
                    "{}: unknown city province {:?}",
                    settlements[city].identifier, identifier
                ));
            }
            if let SurfaceKind::City {
                province: parent, ..
            } = &mut settlements[city].kind
            {
                *parent = province;
            }
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
        let planetary_markets = (0..solar_system.bodies.len())
            .map(BodyId)
            .map(|body| {
                (
                    body,
                    PlanetaryMarket::new(
                        &catalog,
                        settlements.iter().any(|site| site.body == body),
                    ),
                )
            })
            .collect();
        Ok(Self {
            date: StarDate::default(),
            uuid: crate::save::new_uuid(),
            solar_system,
            catalog,
            infrastructure,
            modifiers,
            science,
            time_zones,
            settlements,
            countries,
            planetary_markets,
            vehicles: Vec::new(),
            project_templates,
            missions: Vec::new(),
            history: Vec::new(),
            diagnostics,
            autonomous,
            marginal,
            random: LegacyRandom::new(seed),
        })
    }

    pub fn tick(&mut self) {
        self.date.advance();
        if self.date.due(DAY) {
            self.economic_tick();
        }
        // SysOrbit is last: arrivals are unloaded at the following economic tick.
        self.solar_system.update(self.date.seconds());
    }

    fn economic_tick(&mut self) {
        for country in &mut self.countries {
            if let Some(wallet) = &mut country.wallet {
                wallet.reset();
            }
        }
        for planetary in self.planetary_markets.values_mut() {
            planetary.market.reset();
        }
        for settlement in &mut self.settlements {
            settlement.reset();
        }
        self.process_spaceports();
        let default_labor = self.catalog.jobs[self.catalog.default_job.0].good;
        for settlement in self
            .settlements
            .iter_mut()
            .filter(|settlement| settlement.is_province())
        {
            settlement
                .construction
                .purchase_labor(&mut settlement.market, default_labor);
        }
        // Weekly labor sees old ratios, but reset demand: purchases only change consumption.
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
                    |mean, deviation| self.random.normal(mean, deviation),
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
            if settlement.province().is_some()
                && let Some(port) = &mut settlement.spaceport
            {
                port.manufacture();
            }
        }
        for settlement in self
            .settlements
            .iter_mut()
            .filter(|settlement| settlement.is_province())
        {
            if let Some(port) = &mut settlement.spaceport {
                port.process_market(&mut settlement.market);
            }
            settlement
                .market
                .process(&self.catalog.base_prices, self.catalog.config.market_config);
        }
        for (&body, planetary) in &mut self.planetary_markets {
            if !planetary.has_surface_sites {
                continue;
            }
            planetary.process(
                &self.catalog.base_prices,
                self.settlements
                    .iter()
                    .filter(|site| site.is_province() && site.body == body)
                    .map(|site| (&site.market, site.spaceport.as_ref())),
            );
        }
        for settlement in self
            .settlements
            .iter_mut()
            .filter(|settlement| settlement.is_province())
        {
            settlement
                .market
                .trade_with(&self.planetary_markets[&settlement.body].market);
        }
        self.process_interplanetary_trade();
        for settlement in &mut self.settlements {
            for (index, job) in self.catalog.jobs.iter().enumerate() {
                settlement.job_demands[index] = settlement.market.labor_demand(job.good);
            }
        }
        self.history.push(self.summary(None));
        for planetary in self.planetary_markets.values_mut() {
            planetary.history.record(self.date, &planetary.market);
        }
    }

    pub fn earth_market(&self) -> &Market {
        &self.planetary_markets[&self.solar_system.body_id("earth").expect("Loaded Earth")].market
    }
    /// The original tax UI applies an absolute float amount to every IndustrialZone market.
    pub fn set_good_tax(&mut self, good: GoodId, amount: f32) -> Result<usize> {
        ensure!(good.0 < self.catalog.goods.len(), "Unknown taxable good");
        ensure!(amount.is_finite(), "Tax must be finite");
        let mut updated = 0;
        for site in self
            .settlements
            .iter_mut()
            .filter(|site| site.is_province())
        {
            site.market.taxation[good] = f64::from(amount);
            updated += 1;
        }
        Ok(updated)
    }
    /// ApplySubsidy uses the initial CountryCityList, not current province ownership.
    pub fn apply_subsidy(
        &mut self,
        country: usize,
        recipe: RecipeId,
        amount: f32,
    ) -> Result<usize> {
        ensure!(
            recipe.0 < self.catalog.recipes.len(),
            "Unknown subsidy recipe"
        );
        ensure!(amount.is_finite(), "Subsidy must be finite");
        let country = self
            .countries
            .get_mut(country)
            .context("Unknown subsidy country")?;
        ensure!(
            country.provinces.iter().all(|&province| self
                .settlements
                .get(province)
                .is_some_and(Settlement::is_province)),
            "Invalid subsidy province list"
        );
        country.subsidies.insert(recipe, f64::from(amount));
        let mut updated = 0;
        for &province in &country.provinces {
            for industry in &mut self.settlements[province].industries {
                if industry.recipe == recipe {
                    industry.output_subsidy = f64::from(amount);
                    updated += 1;
                }
            }
        }
        Ok(updated)
    }
    /// The original delete button erases only the policy row; factory values survive.
    pub fn remove_subsidy(&mut self, country: usize, recipe: RecipeId) -> Result<bool> {
        ensure!(
            recipe.0 < self.catalog.recipes.len(),
            "Unknown subsidy recipe"
        );
        Ok(self
            .countries
            .get_mut(country)
            .context("Unknown subsidy country")?
            .subsidies
            .remove(&recipe)
            .is_some())
    }
    /// Native equivalent of the original province Construction tab's button.
    pub fn construct_factory(
        &mut self,
        province: usize,
        recipe: RecipeId,
        count: i32,
    ) -> Result<usize> {
        ensure!(
            (1..=1000).contains(&count),
            "Factory construction count must be 1–1000"
        );
        let recipe_data = self
            .catalog
            .recipes
            .get(recipe.0)
            .context("Unknown factory recipe")?;
        let site = self
            .settlements
            .get_mut(province)
            .context("Unknown factory province")?;
        ensure!(
            site.is_province(),
            "Cities have no IndustrialZone in the original loader"
        );
        let mut industry = Industry::new(recipe, count)?;
        // The original UI attaches a zero-level job without setting Construction state.
        industry.construction = Some(Construction {
            progress: 0,
            maximum: recipe_data.construction_ticks,
            levels: 0,
        });
        let index = site.industries.len();
        site.industries.push(industry);
        Ok(index)
    }
    /// ConstructionMenu painting is distinct from the province tab's construction job.
    pub fn paint_factory(
        &mut self,
        country: usize,
        province: usize,
        recipe: RecipeId,
        count: i32,
    ) -> Result<Option<usize>> {
        ensure!(
            country < self.countries.len(),
            "Unknown construction country"
        );
        ensure!(count > 0, "Factory painting count must be positive");
        let Some(site) = self.settlements.get_mut(province) else {
            return Ok(None);
        };
        if !site.is_province() || site.country != Some(country) {
            return Ok(None);
        }
        let index = if let Some(industry) = site.industries.last_mut() {
            // Upstream ignores the selected recipe and changes size, not utilization or the job.
            industry.size += f64::from(count);
            site.industries.len() - 1
        } else {
            ensure!(
                recipe.0 < self.catalog.recipes.len(),
                "Unknown factory recipe"
            );
            site.industries.push(Industry::new(recipe, count)?);
            0
        };
        Ok(Some(index))
    }
    pub(crate) fn normal(&mut self, mean: f64, deviation: f64) -> f64 {
        self.random.normal(mean, deviation)
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
    #[serde(default)]
    tags: Vec<String>,
}

#[derive(Deserialize)]
struct SettlementEntry {
    identifier: String,
    name: Option<String>,
    country: Option<String>,
    planet: Option<String>,
    color: Option<Value>,
    province: Option<String>,
    coordinates: Option<CoordinatesEntry>,
    timezone: Option<String>,
    #[serde(default)]
    tags: Vec<String>,
    #[serde(rename = "space-port")]
    spaceport: Option<Value>,
    #[serde(default)]
    population: Vec<PopulationEntry>,
    #[serde(default)]
    industry: Vec<IndustryEntry>,
    #[serde(default)]
    zoning: BTreeMap<String, i64>,
    transport: Option<f64>,
    infrastructure: Option<Value>,
}
#[derive(Deserialize, Default)]
struct CoordinatesEntry {
    #[serde(default)]
    latitude: f64,
    #[serde(default)]
    longitude: f64,
}
impl SettlementEntry {
    fn is_template(&self) -> bool {
        self.identifier == "name" && self.color == Some(Value::String("color".into()))
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn province_infrastructure_does_not_invent_city_records_or_power_production() -> Result<()> {
        let data = crate::default_data_path();
        let simulation = Simulation::load(&data, &World::load(&data)?, 42)?;
        let illinois = simulation
            .settlements
            .iter()
            .find(|site| site.identifier == "illinois")
            .unwrap();
        assert_eq!(illinois.zoning.len(), 3);
        assert_eq!(illinois.zoning["undeveloped"].allocated, 100000);
        let infrastructure = illinois.infrastructure.as_ref().unwrap();
        assert_eq!(infrastructure.power_grid.as_deref(), Some("usa"));
        assert_eq!(infrastructure.power_plants.len(), 1);
        assert_eq!(infrastructure.power_plants[0].maximum_production, 1e8);
        assert_eq!(infrastructure.power_plants[0].production, 0.0);
        assert!(
            simulation
                .settlements
                .iter()
                .filter(|site| !site.is_province())
                .all(|site| site.infrastructure.is_none() && site.zoning.is_empty())
        );
        let new_york = simulation
            .settlements
            .iter()
            .find(|site| site.identifier == "new_york" && site.is_province())
            .unwrap();
        assert_eq!(
            new_york.infrastructure.as_ref().unwrap().purchase_cost,
            0.05
        );
        Ok(())
    }
    fn load(seed: u64) -> Result<Simulation> {
        let data = crate::default_data_path();
        Simulation::load(&data, &World::load(&data)?, seed)
    }
    #[test]
    fn national_subsidies_use_initial_lists_and_do_not_inherit_or_clear_factory_values()
    -> Result<()> {
        let mut simulation = load(42)?;
        let country = simulation
            .countries
            .iter()
            .position(|country| !country.provinces.is_empty())
            .unwrap();
        let province = simulation.countries[country].provinces[0];
        let other = simulation
            .settlements
            .iter()
            .position(|site| site.is_province() && site.country != Some(country))
            .unwrap();
        simulation.countries[country].provinces = vec![province];
        simulation.settlements[province].country = simulation.settlements[other].country;
        let recipe = RecipeId(0);
        let factory = simulation.construct_factory(province, recipe, 1)?;
        let different = simulation.construct_factory(province, RecipeId(1), 1)?;
        let foreign = simulation.construct_factory(other, recipe, 1)?;
        let matches = simulation.settlements[province]
            .industries
            .iter()
            .filter(|industry| industry.recipe == recipe)
            .count();
        assert_eq!(simulation.apply_subsidy(country, recipe, 0.2)?, matches);
        assert_eq!(
            simulation.countries[country].subsidies[&recipe],
            f64::from(0.2_f32)
        );
        assert_eq!(
            simulation.settlements[province].industries[factory].output_subsidy,
            f64::from(0.2_f32)
        );
        assert_eq!(
            simulation.settlements[province].industries[different].output_subsidy,
            0.0
        );
        assert_eq!(
            simulation.settlements[other].industries[foreign].output_subsidy,
            0.0
        );
        let future = simulation.construct_factory(province, recipe, 1)?;
        assert_eq!(
            simulation.settlements[province].industries[future].output_subsidy,
            0.0
        );
        assert!(simulation.remove_subsidy(country, recipe)?);
        assert!(!simulation.remove_subsidy(country, recipe)?);
        assert_eq!(
            simulation.settlements[province].industries[factory].output_subsidy,
            f64::from(0.2_f32)
        );
        for (country, recipe, amount) in [
            (usize::MAX, recipe, 1.0),
            (country, RecipeId(usize::MAX), 1.0),
            (country, recipe, f32::NAN),
            (country, recipe, f32::INFINITY),
        ] {
            assert!(simulation.apply_subsidy(country, recipe, amount).is_err());
        }
        assert!(simulation.countries[country].subsidies.is_empty());
        simulation.countries[country].provinces.push(usize::MAX);
        assert!(simulation.apply_subsidy(country, recipe, 1.0).is_err());
        assert!(simulation.countries[country].subsidies.is_empty());
        assert!(simulation.remove_subsidy(usize::MAX, recipe).is_err());
        assert!(
            simulation
                .remove_subsidy(country, RecipeId(usize::MAX))
                .is_err()
        );
        Ok(())
    }
    #[test]
    fn absolute_good_taxes_are_global_to_provinces_only() -> Result<()> {
        let mut simulation = load(42)?;
        let good = GoodId(0);
        let provinces = simulation
            .settlements
            .iter()
            .filter(|site| site.is_province())
            .count();
        assert_eq!(simulation.set_good_tax(good, 2.3)?, provinces);
        assert_eq!(provinces, 1317);
        for site in &simulation.settlements {
            assert_eq!(
                site.market.taxation[good],
                if site.is_province() {
                    f64::from(2.3_f32)
                } else {
                    0.0
                }
            );
            assert!(
                site.market.taxation.values()[1..]
                    .iter()
                    .all(|&tax| tax == 0.0)
            );
        }
        assert!(
            simulation
                .planetary_markets
                .values()
                .all(|planet| planet.market.taxation.sum() == 0.0)
        );
        for (good, amount) in [
            (GoodId(usize::MAX), 0.0),
            (good, f32::NAN),
            (good, f32::NEG_INFINITY),
        ] {
            assert!(simulation.set_good_tax(good, amount).is_err());
        }
        assert!(
            simulation
                .settlements
                .iter()
                .filter(|site| site.is_province())
                .all(|site| site.market.taxation[good] == f64::from(2.3_f32))
        );
        // Core actions preserve finite raw values; slider bounds belong to the UI.
        simulation.set_good_tax(good, -0.5)?;
        assert!(
            simulation
                .settlements
                .iter()
                .filter(|site| site.is_province())
                .all(|site| site.market.taxation[good] == -0.5)
        );
        Ok(())
    }
    #[test]
    fn civilization_metadata_and_all_body_histories_preserve_source_scope() -> Result<()> {
        let data = crate::default_data_path();
        let world = World::load(&data)?;
        let mut simulation = Simulation::load(&data, &world, 42)?;
        let usa = world
            .countries
            .iter()
            .position(|country| country.identifier == "usa")
            .unwrap();
        let capital = simulation.countries[usa].capital.unwrap();
        assert_eq!(
            simulation.settlements[capital].identifier,
            "district_of_columbia"
        );
        assert!(simulation.settlements[capital].is_capital);
        assert!(simulation.settlements[capital].is_province());
        assert_eq!(
            simulation.countries[usa].wallet.unwrap().balance,
            100_000_000.0
        );
        assert_eq!(
            simulation
                .countries
                .iter()
                .filter(|country| country.space_program)
                .count(),
            1
        );
        assert!(
            simulation
                .settlements
                .iter()
                .filter(|site| !site.is_province())
                .all(|site| !site.is_capital)
        );
        simulation.advance_day();
        simulation.advance_day();
        assert_eq!(simulation.planetary_markets.len(), 11);
        for market in simulation.planetary_markets.values() {
            assert_eq!(market.history.0.len(), 2);
            assert_eq!(market.history.0[1].date, simulation.date);
            if !market.has_surface_sites {
                assert_eq!(market.market.price, simulation.catalog.base_prices);
                assert_eq!(
                    market.market.sd_ratio.values(),
                    vec![1.0; simulation.catalog.goods.len()]
                );
                assert_eq!(market.market.production.sum(), 0.0);
                assert!(market.demands.is_empty());
            }
        }
        simulation.validate()?;
        Ok(())
    }
    #[test]
    fn manual_factory_creation_keeps_the_original_zero_level_steady_record() -> Result<()> {
        use crate::economy::industry::IndustryState;
        let mut simulation = load(42)?;
        let province = simulation
            .settlements
            .iter()
            .position(|site| !site.populations.is_empty())
            .unwrap();
        let recipe = simulation.catalog.recipe("farming").unwrap();
        let before = simulation.settlements[province].industries.len();
        for (site, recipe, count) in [
            (usize::MAX, recipe, 1),
            (province, RecipeId(usize::MAX), 1),
            (province, recipe, 0),
            (province, recipe, 1001),
        ] {
            assert!(simulation.construct_factory(site, recipe, count).is_err());
        }
        let city = simulation
            .settlements
            .iter()
            .position(|site| !site.is_province())
            .unwrap();
        assert!(simulation.construct_factory(city, recipe, 1).is_err());
        assert!(simulation.settlements[city].industries.is_empty());
        assert_eq!(simulation.settlements[province].industries.len(), before);
        let factory = simulation.construct_factory(province, recipe, 27)?;
        let industry = &simulation.settlements[province].industries[factory];
        assert_eq!(
            (industry.size, industry.utilization, industry.wallet.balance),
            (27.0, 27.0, 0.0)
        );
        assert_eq!(industry.state, IndustryState::Steady);
        let job = industry.construction.as_ref().unwrap();
        assert_eq!(
            (job.progress, job.maximum, job.levels),
            (
                0,
                simulation.catalog.recipes[recipe.0].construction_ticks,
                0
            )
        );
        simulation.tick();
        let industry = &simulation.settlements[province].industries[factory];
        assert!(industry.amount_sold > 0.0);
        assert_eq!(industry.construction_cost, 0.0);
        assert_eq!(industry.construction.as_ref().unwrap().progress, 0);
        simulation.validate()?;
        Ok(())
    }
    #[test]
    fn map_factory_painting_checks_ownership_and_enlarges_only_the_last_size() -> Result<()> {
        let mut simulation = load(42)?;
        let province = simulation
            .settlements
            .iter()
            .position(|site| site.is_province() && site.country.is_some())
            .unwrap();
        let country = simulation.settlements[province].country.unwrap();
        let city = simulation
            .settlements
            .iter()
            .position(|site| !site.is_province())
            .unwrap();
        let foreign = (country + 1) % simulation.countries.len();
        simulation.settlements[province].industries.clear();
        for (owner, site) in [(foreign, province), (country, city), (country, usize::MAX)] {
            assert_eq!(simulation.paint_factory(owner, site, RecipeId(0), 5)?, None);
        }
        for recipe in 0..simulation.catalog.recipes.len() {
            simulation.settlements[province].industries.clear();
            assert!(
                simulation
                    .paint_factory(country, province, RecipeId(usize::MAX), 1)
                    .is_err()
            );
            assert!(
                simulation
                    .paint_factory(country, province, RecipeId(recipe), 0)
                    .is_err()
            );
            assert_eq!(
                simulation.paint_factory(country, province, RecipeId(recipe), 5)?,
                Some(0)
            );
            let industry = &simulation.settlements[province].industries[0];
            assert_eq!(
                (industry.recipe, industry.size, industry.utilization),
                (RecipeId(recipe), 5.0, 5.0)
            );
            assert!(industry.construction.is_none());
            // Even a nonexistent selection is ignored when enlarging the existing last factory.
            assert_eq!(
                simulation.paint_factory(country, province, RecipeId(usize::MAX), 10)?,
                Some(0)
            );
            let industry = &simulation.settlements[province].industries[0];
            assert_eq!(
                (industry.recipe, industry.size, industry.utilization),
                (RecipeId(recipe), 15.0, 5.0)
            );
            assert!(industry.construction.is_none());
            let second = simulation.construct_factory(
                province,
                RecipeId((recipe + 1) % simulation.catalog.recipes.len()),
                7,
            )?;
            assert_eq!(
                simulation.paint_factory(country, province, RecipeId(recipe), 1)?,
                Some(second)
            );
            let industries = &simulation.settlements[province].industries;
            assert_eq!(industries[0].size, 15.0);
            assert_eq!(
                (industries[second].size, industries[second].utilization),
                (8.0, 7.0)
            );
            assert_eq!(industries[second].construction.as_ref().unwrap().levels, 0);
        }
        Ok(())
    }
    #[test]
    fn bundled_time_zones_bind_cities_without_inventing_province_offsets() -> Result<()> {
        let simulation = load(42)?;
        assert_eq!(simulation.time_zones.entries.len(), 107);
        assert!(!simulation.time_zones.diagnostics.is_empty());
        for (city, identifier) in [("london", "GMT"), ("tokyo", "JST"), ("beijing", "CST")] {
            let site = simulation
                .settlements
                .iter()
                .find(|site| !site.is_province() && site.identifier == city)
                .unwrap();
            let zone = &simulation.time_zones.entries[site.time_zone.unwrap().0];
            assert_eq!(zone.identifier, identifier);
            assert_eq!(site.time_zone, simulation.time_zones.id(identifier));
        }
        assert!(
            simulation
                .settlements
                .iter()
                .filter(|site| site.is_province())
                .all(|site| site.time_zone.is_none())
        );
        Ok(())
    }
    #[test]
    fn first_tick_and_daily_weekly_scheduling_use_original_intervals() -> Result<()> {
        let mut simulation = load(42)?;
        assert!(simulation.settlements.len() > 1067);
        simulation.tick();
        assert_eq!(simulation.date.tick(), 0);
        assert_eq!(simulation.history.len(), 1);
        // Cities have no Settlement/Market/PopulationSegment in the original loader.
        assert!(
            simulation
                .settlements
                .iter()
                .filter(|settlement| !settlement.is_province())
                .all(|city| city.populations.is_empty() && city.industries.is_empty())
        );
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
        let history = &simulation.planetary_markets[&earth].history.0;
        assert_eq!(history.len(), 2);
        assert_eq!(history[1].date, simulation.date);
        assert_eq!(history[1].supply, simulation.earth_market().supply);
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
