//! Spaceport queues, vehicle projects, missions and the original cargo lifecycle.
use crate::{
    catalog::Catalog,
    clock::DAY,
    economy::{GoodId, Ledger, Market},
    flight::{Burn, Command, FlightStatus, Trigger},
    orbit::Orbit,
    read_entries,
    simulation::{Simulation, SurfaceKind},
    solar_system::{BodyId, ShipId},
    units::{Unit, read_unit},
};
use anyhow::{Context, Result, ensure};
use glam::DVec3;
use serde::Deserialize;
use serde_json::Value;
use std::{collections::BTreeMap, path::Path};

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct VehicleId(pub usize);
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct TemplateId(pub usize);
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct MissionId(pub usize);

#[derive(Debug)]
pub struct LaunchVehicle {
    pub name: String,
    pub delta_v: f64,
    pub reliability: f64,
    pub fairing_size: f64,
    pub mass_to_orbit: f64,
    pub manufacture_time: u64,
}
impl LaunchVehicle {
    /// These conversions intentionally match the original design window, not its labels.
    pub fn design(name: String, reliability: f64, fairing: f64, mass: f64) -> Result<Self> {
        ensure!(
            reliability.is_finite()
                && (0.0..=100.0).contains(&reliability)
                && fairing.is_finite()
                && fairing >= 0.0
                && mass.is_finite()
                && mass >= 0.0,
            "Invalid launch vehicle design"
        );
        Ok(Self {
            name,
            delta_v: 0.0,
            reliability,
            fairing_size: fairing / 100.0,
            mass_to_orbit: mass,
            manufacture_time: 90 * DAY,
        })
    }
}

#[derive(Debug)]
pub struct ProjectTemplate {
    pub identifier: String,
    pub cost: Vec<(GoodId, f64)>,
    /// The source stores seconds here, although production advances this by ticks.
    pub max_progress: i32,
    pub crew: Option<u16>,
}
impl ProjectTemplate {
    pub fn load(data: &Path, catalog: &Catalog) -> Result<Vec<Self>> {
        read_entries::<TemplateEntry>(&data.join("data/projects"))?
            .into_iter()
            .map(|entry| {
                let time = read_unit(&entry.construction.time, Unit::Time).unwrap_or(100.0);
                ensure!(
                    time.is_finite() && (i32::MIN as f64..=i32::MAX as f64).contains(&time),
                    "{}: project duration outside C++ int range",
                    entry.identifier
                );
                let cost = entry
                    .construction
                    .cost
                    .into_iter()
                    .map(|(name, amount)| {
                        Ok((
                            catalog
                                .good(&name)
                                .with_context(|| format!("Unknown project good {name}"))?,
                            amount,
                        ))
                    })
                    .collect::<Result<_>>()?;
                Ok(Self {
                    identifier: entry.identifier,
                    cost,
                    max_progress: time as i32,
                    crew: entry
                        .crew
                        .and_then(|value| value.as_i64())
                        .map(|value| value as u16),
                })
            })
            .collect()
    }
}
#[derive(Deserialize)]
struct TemplateEntry {
    identifier: String,
    construction: TemplateConstruction,
    crew: Option<Value>,
}
#[derive(Deserialize)]
struct TemplateConstruction {
    time: Value,
    cost: BTreeMap<String, f64>,
}

#[derive(Debug)]
pub struct ManufacturingProject {
    pub result: VehicleId,
    pub template: Option<TemplateId>,
    pub progress: u64,
    pub max_progress: u64,
    pub total_cost: f64,
    pub last_cost: f64,
}
impl ManufacturingProject {
    pub fn new(result: VehicleId, max_progress: u64) -> Self {
        Self {
            result,
            template: None,
            progress: 0,
            max_progress,
            total_cost: 0.0,
            last_cost: 0.0,
        }
    }
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum ColonizationStep {
    Surveying,
    Preparation,
    InitialBase,
    HumanSettlement,
    PermanentSettlement,
}
impl ColonizationStep {
    pub fn next(self) -> Option<Self> {
        match self {
            Self::Surveying => Some(Self::Preparation),
            Self::Preparation => Some(Self::InitialBase),
            Self::InitialBase => Some(Self::HumanSettlement),
            Self::HumanSettlement => Some(Self::PermanentSettlement),
            Self::PermanentSettlement => None,
        }
    }
    pub fn name(self) -> &'static str {
        match self {
            Self::Surveying => "Surveying",
            Self::Preparation => "Preparation",
            Self::InitialBase => "Initial Base",
            Self::HumanSettlement => "Human Settlement",
            Self::PermanentSettlement => "Permanent Settlement",
        }
    }
}
#[derive(Debug)]
pub struct ColonizationTarget {
    pub colonizer: usize,
    pub step: ColonizationStep,
    pub progress: i32,
}
#[derive(Debug, Default)]
pub struct Colony {
    pub population: u16,
    pub max_population: u16,
    pub power: u16,
    pub communications_power: f32,
    pub components: Vec<MissionId>,
}
#[derive(Debug)]
pub struct Mission {
    pub name: String,
    pub project: Option<TemplateId>,
    pub target_body: BodyId,
    pub province: usize,
    pub in_progress: bool,
    pub colony_core: bool,
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum Shipment {
    Good(GoodId),
    Mission(MissionId),
}
#[derive(Clone, Copy, Debug)]
pub struct TransportedGood {
    pub item: Shipment,
    pub target_province: Option<usize>,
    pub amount: f64,
    pub fulfilled: f64,
    pub priority: f64,
}
impl TransportedGood {
    pub fn new(item: Shipment, amount: f64, target_province: Option<usize>) -> Self {
        Self {
            item,
            amount,
            target_province,
            fulfilled: 0.0,
            priority: 0.0,
        }
    }
}
#[derive(Debug)]
pub struct MarketOrder {
    pub target: BodyId,
    pub amount: f64,
    pub price: f64,
}
pub struct PlanetaryMarket {
    pub market: Market,
    pub supply_difference: Ledger,
    pub demands: BTreeMap<GoodId, Vec<MarketOrder>>,
}
impl PlanetaryMarket {
    pub fn new(catalog: &Catalog) -> Self {
        Self {
            market: Market::new(
                &catalog.base_prices,
                &catalog.labor_goods,
                catalog.config.market_config,
            ),
            supply_difference: Ledger::new(catalog.goods.len()),
            demands: BTreeMap::new(),
        }
    }
    pub fn update_difference(&mut self) {
        for index in 0..self.supply_difference.values().len() {
            let good = GoodId(index);
            self.supply_difference[good] =
                self.market.consumption[good] - self.market.production[good];
        }
    }
    pub fn orders_for(&self, buyer: &Self, target: BodyId) -> Vec<(GoodId, MarketOrder)> {
        (0..self.supply_difference.values().len())
            .filter_map(|index| {
                let good = GoodId(index);
                (buyer.supply_difference[good] > 0.0
                    && buyer.market.price[good] > self.market.price[good]
                    && self.market.chronic_shortages[good] <= 0.0)
                    .then_some((
                        good,
                        MarketOrder {
                            target,
                            amount: buyer.supply_difference[good],
                            price: buyer.market.price[good],
                        },
                    ))
            })
            .collect()
    }
}

#[derive(Debug)]
pub struct SpacePort {
    pub reference_body: BodyId,
    pub launch_cadence: f64,
    pub launchpads: i32,
    pub deliveries: BTreeMap<BodyId, Vec<TransportedGood>>,
    pub demanded_resources: Ledger,
    pub demanded_resources_rate: Ledger,
    pub output_resources: Ledger,
    pub output_resources_rate: Ledger,
    pub resource_stockpile: Ledger,
    pub projects: Vec<ManufacturingProject>,
    pub stored_launch_vehicles: Vec<VehicleId>,
}
impl SpacePort {
    pub fn new(reference_body: BodyId, goods: usize) -> Self {
        Self {
            reference_body,
            launch_cadence: 0.0,
            launchpads: 0,
            deliveries: BTreeMap::new(),
            demanded_resources: Ledger::new(goods),
            demanded_resources_rate: Ledger::new(goods),
            output_resources: Ledger::new(goods),
            output_resources_rate: Ledger::new(goods),
            resource_stockpile: Ledger::new(goods),
            projects: Vec::new(),
            stored_launch_vehicles: Vec::new(),
        }
    }
    pub fn manufacture(&mut self) {
        for project in &mut self.projects {
            project.progress += DAY;
            // SysLaunchVehicleProduction skips costs for both missing AND valid typed templates.
            if project.progress >= project.max_progress {
                self.stored_launch_vehicles.push(project.result);
            }
        }
        self.projects
            .retain(|project| project.progress < project.max_progress);
    }
    pub fn process_market(&mut self, market: &mut Market) {
        // SysMarket subsequently overwrites supply from production. Still deplete the stockpile.
        market.supply.add_ledger(&self.output_resources_rate);
        for index in 0..self.output_resources.values().len() {
            let good = GoodId(index);
            self.output_resources[good] -= self.output_resources_rate[good];
            if self.output_resources[good] < 0.0 {
                self.output_resources_rate[good] = 0.0;
            }
        }
    }
    pub fn take_orders(&mut self, planetary: &mut PlanetaryMarket) {
        for (&good, orders) in &mut planetary.demands {
            for order in orders.iter_mut() {
                self.deliveries
                    .entry(order.target)
                    .or_default()
                    .push(TransportedGood::new(
                        Shipment::Good(good),
                        order.amount,
                        None,
                    ));
                order.amount = 0.0;
            }
            orders.retain(|order| order.amount > 0.0);
        }
    }
}

impl Simulation {
    pub fn design_vehicle(&mut self, country: usize, vehicle: LaunchVehicle) -> Result<VehicleId> {
        let country = self
            .countries
            .get_mut(country)
            .context("Invalid vehicle owner")?;
        let id = VehicleId(self.vehicles.len());
        self.vehicles.push(vehicle);
        country.vehicles.push(id);
        Ok(id)
    }
    pub fn manufacture_vehicle(&mut self, city: usize, vehicle: VehicleId) -> Result<()> {
        let vehicle_data = self
            .vehicles
            .get(vehicle.0)
            .context("Unknown launch vehicle")?;
        let settlement = self
            .settlements
            .get_mut(city)
            .context("Unknown launch site")?;
        ensure!(
            matches!(settlement.kind, SurfaceKind::City { .. }),
            "Manufacturing needs a city"
        );
        settlement
            .spaceport
            .as_mut()
            .context("City has no spaceport")?
            .projects
            .push(ManufacturingProject::new(
                vehicle,
                vehicle_data.manufacture_time,
            ));
        Ok(())
    }
    pub fn launch_vehicle(
        &mut self,
        city: usize,
        mut orbit: Orbit,
        azimuth: f64,
    ) -> Result<ShipId> {
        ensure!(azimuth.is_finite(), "Invalid launch azimuth");
        let settlement = self.settlements.get(city).context("Unknown launch site")?;
        let SurfaceKind::City { latitude, .. } = settlement.kind else {
            anyhow::bail!("Launch needs a city");
        };
        let port = settlement
            .spaceport
            .as_ref()
            .context("City has no spaceport")?;
        let vehicle = *port
            .stored_launch_vehicles
            .last()
            .context("No manufactured launch vehicles")?;
        orbit.inclination = (latitude.cos() * azimuth.sin()).acos();
        let parent = settlement.body;
        let name = self
            .vehicles
            .get(vehicle.0)
            .context("Unknown stored vehicle")?
            .name
            .clone();
        let ship = self.solar_system.launch_ship(name, parent, orbit)?;
        self.settlements[city]
            .spaceport
            .as_mut()
            .unwrap()
            .stored_launch_vehicles
            .pop();
        Ok(ship)
    }
    pub fn begin_colonization(&mut self, province: usize, country: usize) -> Result<()> {
        ensure!(country < self.countries.len(), "Unknown colonizer");
        let target = self
            .settlements
            .get_mut(province)
            .context("Unknown target province")?;
        ensure!(target.is_province(), "Colonization needs a province");
        if target.colonization.is_none() {
            target.colonization = Some(ColonizationTarget {
                colonizer: country,
                step: ColonizationStep::Surveying,
                progress: 0,
            });
        }
        Ok(())
    }
    pub fn queue_colonization_mission(
        &mut self,
        province: usize,
        country: usize,
        colony_core: bool,
    ) -> Result<MissionId> {
        ensure!(country < self.countries.len(), "Unknown mission owner");
        let target = self
            .settlements
            .get(province)
            .context("Unknown target province")?;
        ensure!(target.is_province(), "Mission needs a province");
        let step = target
            .colonization
            .as_ref()
            .context("Province is not a colonization target")?
            .step;
        let id = MissionId(self.missions.len());
        self.missions.push(Mission {
            name: if colony_core {
                "Colony Core"
            } else {
                step.name()
            }
            .into(),
            project: None,
            target_body: target.body,
            province,
            in_progress: false,
            colony_core,
        });
        self.countries[country].missions.push(id);
        Ok(id)
    }
    pub fn dispatch_mission(&mut self, city: usize, id: MissionId) -> Result<()> {
        let mission = self.missions.get(id.0).context("Unknown mission")?;
        ensure!(!mission.in_progress, "Mission is already in progress");
        let delivery = TransportedGood::new(Shipment::Mission(id), 0.0, Some(mission.province));
        self.settlements
            .get_mut(city)
            .context("Unknown launch site")?
            .spaceport
            .as_mut()
            .context("Launch site has no spaceport")?
            .deliveries
            .entry(mission.target_body)
            .or_default()
            .push(delivery);
        self.missions[id.0].in_progress = true;
        Ok(())
    }
    pub fn queue_goods(
        &mut self,
        city: usize,
        target: BodyId,
        good: GoodId,
        amount: f64,
        landing: Option<usize>,
    ) -> Result<()> {
        ensure!(
            good.0 < self.catalog.goods.len() && amount.is_finite() && amount >= 0.0,
            "Invalid shipment"
        );
        ensure!(
            target.0 < self.solar_system.bodies.len(),
            "Unknown shipment body"
        );
        if let Some(landing) = landing {
            ensure!(
                self.settlements
                    .get(landing)
                    .is_some_and(|site| site.body == target),
                "Landing site is not on the target body"
            );
        }
        self.settlements
            .get_mut(city)
            .context("Unknown launch site")?
            .spaceport
            .as_mut()
            .context("Launch site has no spaceport")?
            .deliveries
            .entry(target)
            .or_default()
            .push(TransportedGood::new(Shipment::Good(good), amount, landing));
        Ok(())
    }
    pub(crate) fn process_spaceports(&mut self) {
        let pending: Vec<_> = self
            .countries
            .iter()
            .enumerate()
            .flat_map(|(country, economy)| {
                let city = self.settlements.iter().position(|settlement| {
                    settlement.country == Some(country) && settlement.spaceport.is_some()
                });
                economy
                    .missions
                    .iter()
                    .filter(|id| !self.missions[id.0].in_progress)
                    .filter_map(move |&id| city.map(|city| (city, id)))
            })
            .collect();
        for (city, mission) in pending {
            if let Err(error) = self.dispatch_mission(city, mission) {
                self.diagnostics.push(error.to_string());
            }
        }
        let mut shipments = Vec::new();
        for settlement in &mut self.settlements {
            if let Some(port) = &mut settlement.spaceport {
                for (&target, queue) in &mut port.deliveries {
                    while let Some(delivery) = queue.pop() {
                        shipments.push((port.reference_body, target, delivery));
                    }
                }
            }
        }
        for (source, target, delivery) in shipments {
            if let Err(error) = self.ship_delivery(source, target, delivery) {
                self.diagnostics.push(format!(
                    "Delivery {:?} to {:?} failed: {error:#}",
                    delivery.item, target
                ));
            }
        }
        for index in 0..self.solar_system.ships.len() {
            let FlightStatus::Docked(site) = self.solar_system.ships[index].status else {
                continue;
            };
            let Some(settlement) = self.settlements.get_mut(site) else {
                continue;
            };
            let ship = &mut self.solar_system.ships[index];
            if let Some(port) = &mut settlement.spaceport {
                if let Some(stockpile) = ship.stockpile.take() {
                    port.output_resources.add_ledger(&stockpile);
                }
                if ship.cargo.is_some()
                    && let Some(province) = settlement.province()
                {
                    self.settlements[province].advance_colonization(false);
                }
            } else if settlement.is_province()
                && settlement.colonization.is_some()
                && let Some(cargo) = ship.cargo.take()
            {
                if settlement.colony.is_none()
                    && let Some(&core) = cargo.iter().find(|id| self.missions[id.0].colony_core)
                {
                    settlement.colony = Some(Colony {
                        components: vec![core],
                        ..Colony::default()
                    });
                }
                settlement.advance_colonization(true);
            }
        }
    }
    fn ship_delivery(
        &mut self,
        source: BodyId,
        target: BodyId,
        delivery: TransportedGood,
    ) -> Result<ShipId> {
        let common = self.solar_system.common_soi(source, target);
        ensure!(
            common == source || common == target,
            "Interplanetary transfers not supported by the source"
        );
        let landing = delivery
            .target_province
            .or_else(|| {
                self.settlements
                    .iter()
                    .position(|settlement| !settlement.is_province() && settlement.body == target)
            })
            .context("Source expects a destination city, but none exists")?;
        ensure!(
            self.settlements
                .get(landing)
                .is_some_and(|settlement| settlement.body == target),
            "Invalid landing site"
        );
        let name = match delivery.item {
            Shipment::Good(good) => self
                .catalog
                .goods
                .get(good.0)
                .context("Invalid cargo good")?
                .name
                .as_str(),
            Shipment::Mission(id) => self
                .missions
                .get(id.0)
                .context("Invalid cargo mission")?
                .name
                .as_str(),
        };
        let name = format!("{name} Transport Vehicle");
        let radius = self.solar_system.bodies[source.0].radius;
        let eccentricity = self.normal(0.0, 0.000005).abs();
        let semi_major_axis = self.normal(radius * 1.08, radius * 0.01);
        let inclination = self.normal(0.0, 0.05);
        let id = self.solar_system.launch_ship(
            name,
            source,
            Orbit {
                semi_major_axis,
                eccentricity,
                inclination,
                periapsis_argument: 0.1,
                ..Orbit::default()
            },
        )?;
        let time = self.date.seconds();
        let body = &self.solar_system.bodies[target.0];
        let ship = &mut self.solar_system.ships[id.0];
        if common == target {
            ship.queue_command(Trigger::Maneuver, Command::ExitSoi(body.radius * 5.0));
            ship.queue_burn(
                Burn {
                    delta_v: DVec3::ZERO,
                    delay: 1000.0,
                },
                time,
            )?;
            ship.queue_command(Trigger::ExitSoi, Command::CircularizeAtPeriapsis);
            ship.queue_command(Trigger::Maneuver, Command::SetPeriapsis(body.radius * 0.9));
            ship.queue_command(Trigger::Crash, Command::Land(landing));
        } else {
            ship.land_on_moon(target, body, landing, time)?;
        }
        match delivery.item {
            Shipment::Good(good) => {
                let mut stockpile = Ledger::new(self.catalog.goods.len());
                stockpile[good] = delivery.amount;
                ship.stockpile = Some(stockpile);
            }
            Shipment::Mission(id) => ship.cargo = Some(vec![id]),
        }
        Ok(id)
    }
    pub(crate) fn process_interplanetary_trade(&mut self) {
        for planetary in self.planetary_markets.values_mut() {
            planetary.update_difference();
        }
        let mut orders = Vec::new();
        for (&seller_id, seller) in &self.planetary_markets {
            for (&buyer_id, buyer) in &self.planetary_markets {
                if seller_id != buyer_id {
                    orders.extend(
                        seller
                            .orders_for(buyer, buyer_id)
                            .into_iter()
                            .map(|order| (seller_id, order)),
                    );
                }
            }
        }
        for (seller, (good, order)) in orders {
            self.planetary_markets
                .get_mut(&seller)
                .unwrap()
                .demands
                .entry(good)
                .or_default()
                .push(order);
        }
        for settlement in &mut self.settlements {
            if let Some(port) = &mut settlement.spaceport {
                port.take_orders(
                    self.planetary_markets
                        .get_mut(&port.reference_body)
                        .unwrap(),
                );
            }
        }
    }
}

impl crate::simulation::Settlement {
    fn advance_colonization(&mut self, transfer_ownership: bool) {
        let Some(target) = &mut self.colonization else {
            return;
        };
        if let Some(next) = target.step.next() {
            target.step = next;
        } else {
            if transfer_ownership {
                self.country = Some(target.colonizer);
            }
            self.colonization = None;
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::{World, default_data_path, economy::MarketConfig};
    use std::f64::consts::FRAC_PI_2;

    fn load() -> Result<Simulation> {
        let data = default_data_path();
        Simulation::load(&data, &World::load(&data)?, 42)
    }
    fn cape(simulation: &Simulation) -> usize {
        simulation
            .settlements
            .iter()
            .position(|site| site.identifier == "cape_canaveral")
            .unwrap()
    }
    fn moon_province(simulation: &Simulation) -> usize {
        let moon = simulation.solar_system.body_id("moon").unwrap();
        simulation
            .settlements
            .iter()
            .position(|site| site.body == moon && site.is_province())
            .unwrap()
    }
    #[test]
    fn original_surface_assets_and_project_templates_keep_loader_semantics() -> Result<()> {
        let simulation = load()?;
        assert_eq!(simulation.settlements.len(), 1341);
        assert_eq!(
            simulation
                .settlements
                .iter()
                .filter(|site| site.is_province())
                .count(),
            1317
        );
        assert_eq!(simulation.summary(None).population, 360_099_455);
        assert_eq!(simulation.summary(None).industries, 20);
        assert_eq!(simulation.planetary_markets.len(), 2);
        let site = &simulation.settlements[cape(&simulation)];
        assert_eq!(
            simulation.settlements[site.province().unwrap()].identifier,
            "florida"
        );
        let port = site.spaceport.as_ref().unwrap();
        assert_eq!(port.launchpads, 0);
        assert_eq!(port.launch_cadence, 0.0);
        assert_eq!(simulation.project_templates.len(), 2);
        assert!(
            simulation
                .project_templates
                .iter()
                .all(|template| template.max_progress == 2_419_200)
        );
        let habitation = simulation
            .project_templates
            .iter()
            .find(|template| template.identifier == "habitation_module")
            .unwrap();
        assert_eq!(habitation.crew, Some(10));
        assert_eq!(habitation.cost.len(), 2);
        Ok(())
    }
    #[test]
    fn manufacturing_finishes_at_boundary_without_charging_template_costs() -> Result<()> {
        let mut simulation = load()?;
        let city = cape(&simulation);
        let owner = simulation.settlements[city].country.unwrap();
        let vehicle = simulation.design_vehicle(
            owner,
            LaunchVehicle::design("Test rocket".into(), 95.0, 10.0, 5.0)?,
        )?;
        assert_eq!(simulation.vehicles[vehicle.0].reliability, 95.0);
        assert_eq!(simulation.vehicles[vehicle.0].fairing_size, 0.1);
        assert!(
            simulation
                .launch_vehicle(
                    city,
                    Orbit {
                        semi_major_axis: 7000.0,
                        ..Orbit::default()
                    },
                    FRAC_PI_2
                )
                .is_err()
        );
        simulation.manufacture_vehicle(city, vehicle)?;
        let port = simulation.settlements[city].spaceport.as_mut().unwrap();
        port.projects[0].template = Some(TemplateId(0));
        for _ in 0..89 {
            port.manufacture();
        }
        assert!(port.stored_launch_vehicles.is_empty());
        assert_eq!(port.projects[0].progress, 89 * DAY);
        assert_eq!(port.projects[0].last_cost, 0.0);
        port.manufacture();
        assert!(port.projects.is_empty());
        assert_eq!(port.stored_launch_vehicles, [vehicle]);
        let invalid = Orbit {
            semi_major_axis: 0.0,
            ..Orbit::default()
        };
        assert!(simulation.launch_vehicle(city, invalid, FRAC_PI_2).is_err());
        assert_eq!(
            simulation.settlements[city]
                .spaceport
                .as_ref()
                .unwrap()
                .stored_launch_vehicles
                .len(),
            1
        );
        let id = simulation.launch_vehicle(
            city,
            Orbit {
                semi_major_axis: 7000.0,
                eccentricity: 0.01,
                ..Orbit::default()
            },
            FRAC_PI_2,
        )?;
        let ship = &simulation.solar_system.ships[id.0];
        assert!((ship.orbit.inclination.to_degrees() - 28.46).abs() < 1e-10);
        assert!(
            simulation.settlements[city]
                .spaceport
                .as_ref()
                .unwrap()
                .stored_launch_vehicles
                .is_empty()
        );
        simulation.solar_system.update(0.0);
        simulation.validate()?;
        Ok(())
    }
    #[test]
    fn market_output_depletion_can_go_negative_and_supply_is_overwritten() {
        let base = Ledger::filled(1, 10.0);
        let config = MarketConfig::default();
        let mut market = Market::new(&base, &[], config);
        let mut port = SpacePort::new(BodyId(0), 1);
        port.output_resources[GoodId(0)] = 3.0;
        port.output_resources_rate[GoodId(0)] = 5.0;
        port.process_market(&mut market);
        assert_eq!(port.output_resources[GoodId(0)], -2.0);
        assert_eq!(port.output_resources_rate[GoodId(0)], 0.0);
        assert_eq!(market.supply[GoodId(0)], 6.0);
        market.process(&base, config);
        assert_eq!(market.supply[GoodId(0)], 0.0);
    }
    #[test]
    fn trade_uses_production_consumption_not_aggregates_and_first_port_takes_orders() -> Result<()>
    {
        let simulation = load()?;
        let catalog = &simulation.catalog;
        let mut seller = PlanetaryMarket::new(catalog);
        let mut buyer = PlanetaryMarket::new(catalog);
        let good = GoodId(0);
        buyer.market.demand[good] = 100.0;
        buyer.market.price[good] = seller.market.price[good] * 2.0;
        buyer.update_difference();
        assert!(seller.orders_for(&buyer, BodyId(1)).is_empty());
        buyer.market.consumption[good] = 20.0;
        buyer.market.production[good] = 5.0;
        buyer.update_difference();
        let orders = seller.orders_for(&buyer, BodyId(1));
        assert_eq!(orders.len(), 1);
        assert_eq!(orders[0].1.amount, 15.0);
        seller
            .demands
            .insert(good, orders.into_iter().map(|(_, order)| order).collect());
        let mut first = SpacePort::new(BodyId(0), catalog.goods.len());
        let mut second = SpacePort::new(BodyId(0), catalog.goods.len());
        first.take_orders(&mut seller);
        second.take_orders(&mut seller);
        assert_eq!(first.deliveries[&BodyId(1)][0].amount, 15.0);
        assert!(first.deliveries[&BodyId(1)][0].target_province.is_none());
        assert!(second.deliveries.is_empty());
        seller.market.chronic_shortages[good] = 0.01;
        assert!(seller.orders_for(&buyer, BodyId(1)).is_empty());
        Ok(())
    }
    #[test]
    fn queue_dispatch_is_lifo_and_does_not_consume_manufactured_vehicles() -> Result<()> {
        let mut simulation = load()?;
        let city = cape(&simulation);
        let province = moon_province(&simulation);
        let moon = simulation.settlements[province].body;
        let good = simulation.catalog.good("steel").unwrap();
        let count = simulation.solar_system.ships.len();
        simulation.queue_goods(city, moon, good, 10.0, Some(province))?;
        simulation.queue_goods(city, moon, good, 20.0, Some(province))?;
        simulation.tick();
        let ships = &simulation.solar_system.ships[count..];
        assert_eq!(ships.len(), 2);
        assert_eq!(ships[0].stockpile.as_ref().unwrap()[good], 20.0);
        assert_eq!(ships[1].stockpile.as_ref().unwrap()[good], 10.0);
        assert!(
            simulation.settlements[city]
                .spaceport
                .as_ref()
                .unwrap()
                .deliveries[&moon]
                .is_empty()
        );
        simulation.validate()?;
        Ok(())
    }
    #[test]
    fn docked_province_cargo_advances_once_and_final_delivery_transfers_ownership() -> Result<()> {
        let mut simulation = load()?;
        let city = cape(&simulation);
        let country = simulation.settlements[city].country.unwrap();
        let province = moon_province(&simulation);
        simulation.begin_colonization(province, country)?;
        for delivery in 0..5 {
            let mission =
                simulation.queue_colonization_mission(province, country, delivery == 0)?;
            let moon = simulation.settlements[province].body;
            let ship = simulation.solar_system.launch_ship(
                "Docked cargo".into(),
                moon,
                Orbit {
                    semi_major_axis: 3500.0,
                    ..Orbit::default()
                },
            )?;
            simulation.solar_system.ships[ship.0].status = FlightStatus::Docked(province);
            simulation.solar_system.ships[ship.0].cargo = Some(vec![mission]);
            simulation.missions[mission.0].in_progress = true;
            simulation.process_spaceports();
            assert!(simulation.solar_system.ships[ship.0].cargo.is_none());
            let step = simulation.settlements[province]
                .colonization
                .as_ref()
                .map(|target| target.step);
            simulation.process_spaceports();
            assert_eq!(
                simulation.settlements[province]
                    .colonization
                    .as_ref()
                    .map(|target| target.step),
                step
            );
        }
        let province = &simulation.settlements[province];
        assert!(province.colonization.is_none());
        assert_eq!(province.country, Some(country));
        assert_eq!(province.colony.as_ref().unwrap().components.len(), 1);
        Ok(())
    }
    #[test]
    fn city_docking_unloads_goods_once_but_retains_cargo_for_repeated_stage_progress() -> Result<()>
    {
        let mut simulation = load()?;
        let city = cape(&simulation);
        let province = simulation.settlements[city].province().unwrap();
        let country = simulation.settlements[city].country.unwrap();
        simulation.begin_colonization(province, country)?;
        let mission = simulation.queue_colonization_mission(province, country, false)?;
        simulation.missions[mission.0].in_progress = true;
        let earth = simulation.settlements[city].body;
        let ship = simulation.solar_system.launch_ship(
            "Docked cargo".into(),
            earth,
            Orbit {
                semi_major_axis: 7000.0,
                ..Orbit::default()
            },
        )?;
        let good = simulation.catalog.good("steel").unwrap();
        let mut stockpile = Ledger::new(simulation.catalog.goods.len());
        stockpile[good] = 20.0;
        simulation.solar_system.ships[ship.0].status = FlightStatus::Docked(city);
        simulation.solar_system.ships[ship.0].stockpile = Some(stockpile);
        simulation.solar_system.ships[ship.0].cargo = Some(vec![mission]);
        for _ in 0..5 {
            simulation.process_spaceports();
        }
        assert_eq!(
            simulation.settlements[city]
                .spaceport
                .as_ref()
                .unwrap()
                .output_resources[good],
            20.0
        );
        assert!(simulation.solar_system.ships[ship.0].stockpile.is_none());
        assert!(simulation.solar_system.ships[ship.0].cargo.is_some());
        assert!(simulation.settlements[province].colonization.is_none());
        Ok(())
    }
    #[test]
    fn full_lunar_mission_dispatches_lands_and_unloads_on_next_economic_tick() -> Result<()> {
        let mut simulation = load()?;
        let city = cape(&simulation);
        let country = simulation.settlements[city].country.unwrap();
        let province = moon_province(&simulation);
        simulation.begin_colonization(province, country)?;
        let mission = simulation.queue_colonization_mission(province, country, true)?;
        let ship = ShipId(simulation.solar_system.ships.len());
        simulation.tick();
        assert!(simulation.missions[mission.0].in_progress);
        assert!(simulation.dispatch_mission(city, mission).is_err());
        for _ in 0..DAY * 30 {
            simulation.tick();
            simulation.validate()?;
            if simulation.solar_system.ships[ship.0].status != FlightStatus::Orbiting {
                break;
            }
        }
        let craft = &simulation.solar_system.ships[ship.0];
        assert_eq!(
            craft.status,
            FlightStatus::Docked(province),
            "Orbit: {:?}; commands: {:?}",
            craft.orbit,
            craft.commands
        );
        assert!(craft.cargo.is_some());
        assert_eq!(
            simulation.settlements[province]
                .colonization
                .as_ref()
                .unwrap()
                .step,
            ColonizationStep::Surveying
        );
        simulation.advance_day();
        simulation.validate()?;
        assert_eq!(
            simulation.settlements[province]
                .colonization
                .as_ref()
                .unwrap()
                .step,
            ColonizationStep::Preparation
        );
        assert_eq!(
            simulation.settlements[province]
                .colony
                .as_ref()
                .unwrap()
                .components,
            [mission]
        );
        assert!(simulation.solar_system.ships[ship.0].cargo.is_none());
        Ok(())
    }
}
