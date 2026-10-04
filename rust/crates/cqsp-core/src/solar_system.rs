//! Natural-body hierarchy and spacecraft movement.
use crate::{
    clock::SECONDS_PER_TICK,
    flight::Spacecraft,
    orbit::{Orbit, StateVector},
    read_entries,
    units::{GRAVITATIONAL_CONSTANT_KM, Unit, read_unit},
};
use anyhow::{Context, Result, ensure};
use glam::{DVec3, Quat};
use serde::Deserialize;
use serde_json::Value;
use std::{collections::HashMap, f64::consts::TAU, path::Path};

#[derive(Clone, Copy, Debug, PartialEq, Eq, PartialOrd, Ord)]
pub struct BodyId(pub usize);

#[derive(Clone, Copy, Debug, Default)]
pub struct Kinematics {
    pub state: StateVector,
    pub center: DVec3,
    pub future_position: DVec3,
    pub future_center: DVec3,
}

impl Kinematics {
    pub fn absolute_position(self) -> DVec3 {
        self.state.position + self.center
    }
    pub fn absolute_future_position(self) -> DVec3 {
        self.future_position + self.future_center
    }
    fn update(&mut self, orbit: &mut Orbit, time: f64, parent: Self) {
        orbit.update(time);
        self.state = orbit.state_at_anomaly(orbit.true_anomaly);
        self.future_position = orbit.state_at(time + SECONDS_PER_TICK as f64).position;
        self.center = parent.absolute_position();
        self.future_center = parent.absolute_future_position();
    }
    pub(crate) fn is_finite(self) -> bool {
        self.state.position.is_finite()
            && self.state.velocity.is_finite()
            && self.center.is_finite()
            && self.future_position.is_finite()
            && self.future_center.is_finite()
    }
}

#[derive(Debug, Default, Deserialize)]
pub struct Terrain {
    pub terrain: Option<String>,
    pub normal: Option<String>,
    pub roughness: Option<String>,
    pub province_map: Option<String>,
    pub province_definitions: Option<String>,
}

#[derive(Debug)]
pub struct CelestialBody {
    pub identifier: String,
    pub name: String,
    pub parent: Option<BodyId>,
    pub children: Vec<BodyId>,
    pub is_star: bool,
    pub radius: f64,
    pub mu: f64,
    pub mass: f64,
    pub sphere_of_influence: f64,
    pub rotation_period: f64,
    pub rotation_offset: f64,
    pub axial_tilt: f64,
    pub atmosphere_scale_height: Option<f64>,
    pub terrain: Terrain,
    pub orbit: Orbit,
    pub kinematics: Kinematics,
}

impl CelestialBody {
    pub fn rotation_angle(&self, time: f64) -> Option<f64> {
        (self.rotation_period != 0.0)
            .then(|| (time / self.rotation_period - self.rotation_offset) * TAU)
    }
    /// Source GLM uses float rotation, then a double remainder and float quaternion.
    pub fn orientation(&self, time: f64) -> Quat {
        let rotation = self.rotation_angle(time).unwrap_or(0.0) as f32;
        Quat::from_rotation_x(-self.axial_tilt as f32)
            * Quat::from_rotation_z((rotation as f64 % TAU) as f32)
    }
    /// Latitude/longitude in radians, in the body's rotating frame.
    pub fn ground_track(&self, position: DVec3, time: f64) -> Option<[f64; 2]> {
        let direction = position.as_vec3().try_normalize()?;
        let rotation = self.orientation(time);
        let inverse = rotation.conjugate() / rotation.length_squared();
        let local = inverse * direction;
        let latitude = local.z.asin() as f64;
        let longitude = local.y.atan2(local.x) as f64;
        (latitude.is_finite() && longitude.is_finite()).then(|| {
            [latitude, longitude]
                .map(|angle| (angle + std::f64::consts::PI).rem_euclid(TAU) - std::f64::consts::PI)
        })
    }
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct ShipId(pub usize);

pub struct SolarSystem {
    pub bodies: Vec<CelestialBody>,
    pub ships: Vec<Spacecraft>,
    pub sun: BodyId,
    pub diagnostics: Vec<String>,
    body_ids: HashMap<String, BodyId>,
    update_order: Vec<BodyId>,
}

impl SolarSystem {
    pub fn load(data: &Path) -> Result<Self> {
        let entries = read_entries::<BodyEntry>(&data.join("data/planets"))?;
        let mut body_ids = HashMap::new();
        for (index, entry) in entries.iter().enumerate() {
            ensure!(
                body_ids
                    .insert(entry.identifier.clone(), BodyId(index))
                    .is_none(),
                "Duplicate body {}",
                entry.identifier
            );
        }
        let sun = *body_ids.get("sun").context("Missing Sun")?;
        let mut bodies = Vec::new();
        let mut diagnostics = Vec::new();
        for entry in entries {
            let parent = entry
                .reference
                .as_ref()
                .map(|name| {
                    body_ids
                        .get(name)
                        .copied()
                        .with_context(|| format!("{}: unknown parent {name}", entry.identifier))
                })
                .transpose()?;
            let radius = read_unit(&entry.radius, Unit::Distance)?;
            ensure!(
                radius > 0.0 && entry.gm.is_finite() && entry.gm > 0.0,
                "{}: invalid body",
                entry.identifier
            );
            let orbit =
                Orbit::load(&entry.orbit).with_context(|| format!("{} orbit", entry.identifier))?;
            let rotation_period = optional_unit(
                entry.day_length.as_ref(),
                Unit::Time,
                &entry.identifier,
                "day_length",
                &mut diagnostics,
            );
            let rotation_offset = optional_unit(
                entry.day_offset.as_ref(),
                Unit::Angle,
                &entry.identifier,
                "day_offset",
                &mut diagnostics,
            );
            let axial_tilt = optional_unit(
                entry.axial.as_ref(),
                Unit::Angle,
                &entry.identifier,
                "axial",
                &mut diagnostics,
            );
            let atmosphere_scale_height = entry
                .atmosphere
                .map(|atmosphere| read_unit(&atmosphere.scale_height, Unit::Distance))
                .transpose()?;
            bodies.push(CelestialBody {
                identifier: entry.identifier,
                name: entry.name,
                parent,
                children: Vec::new(),
                is_star: entry.kind.as_deref() == Some("star"),
                radius,
                mu: entry.gm,
                mass: entry.gm / GRAVITATIONAL_CONSTANT_KM,
                sphere_of_influence: f64::INFINITY,
                rotation_period,
                rotation_offset,
                axial_tilt,
                atmosphere_scale_height,
                terrain: entry.texture.unwrap_or_default(),
                orbit,
                kinematics: Kinematics::default(),
            });
        }
        ensure!(
            bodies[sun.0].parent.is_none() && bodies[sun.0].is_star,
            "Invalid Sun root"
        );
        for index in 0..bodies.len() {
            if let Some(parent) = bodies[index].parent {
                let mu = bodies[parent.0].mu;
                bodies[index].orbit.mu = mu;
                bodies[index].sphere_of_influence =
                    bodies[index].orbit.semi_major_axis * (bodies[index].mu / mu).powf(0.4);
                bodies[parent.0].children.push(BodyId(index));
            } else {
                ensure!(
                    index == sun.0,
                    "{}: additional disconnected root",
                    bodies[index].identifier
                );
            }
        }
        let update_order = parent_first_order(&bodies)?;
        let earth = *body_ids.get("earth").context("Missing Earth")?;
        let mut ships = Vec::new();
        // These HJSON definitions are registered assets; the old TLE text file is not.
        for entry in
            read_entries::<SatelliteEntry>(&data.join("data/planet_data/earth/satellites"))?
        {
            let reference = entry.orbit["reference"]
                .as_str()
                .context("Satellite missing reference")?;
            let parent = *body_ids
                .get(reference)
                .with_context(|| format!("{}: unknown parent {reference}", entry.name))?;
            let mut orbit =
                Orbit::load(&entry.orbit).with_context(|| format!("{} orbit", entry.name))?;
            orbit.mu = bodies[parent.0].mu;
            ensure!(
                orbit.semi_major_axis > 0.0,
                "{}: invalid satellite orbit",
                entry.name
            );
            let mut ship = Spacecraft::new(entry.name, parent, orbit);
            ship.model = entry.model;
            ships.push(ship);
        }
        ensure!(
            bodies[earth.0].parent == Some(sun),
            "Earth must orbit the Sun"
        );
        let mut system = Self {
            bodies,
            ships,
            sun,
            diagnostics,
            body_ids,
            update_order,
        };
        system.update(0.0);
        system.validate()?;
        Ok(system)
    }

    pub fn body_id(&self, identifier: &str) -> Option<BodyId> {
        self.body_ids.get(identifier).copied()
    }
    /// Preserve `commands::GetCommonSOI`, including its equal-depth sibling quirk.
    pub fn common_soi(&self, source: BodyId, target: BodyId) -> BodyId {
        let hierarchy = |mut id: BodyId| {
            let mut chain = vec![id];
            while let Some(parent) = self.bodies[id.0].parent {
                chain.push(parent);
                id = parent;
            }
            chain
        };
        let source = hierarchy(source);
        let target = hierarchy(target);
        let length = source.len().min(target.len());
        for index in 1..length {
            if source[source.len() - index] != target[target.len() - index] {
                return source[source.len() - index + 1];
            }
        }
        source[source.len() - length]
    }
    pub fn update(&mut self, time: f64) {
        for &id in &self.update_order {
            let parent = self.bodies[id.0]
                .parent
                .map_or(Kinematics::default(), |parent| {
                    self.bodies[parent.0].kinematics
                });
            let body = &mut self.bodies[id.0];
            body.kinematics.update(&mut body.orbit, time, parent);
        }
        for ship in &mut self.ships {
            if let Err(error) = ship.step(time, &self.bodies) {
                let diagnostic = format!("{}: spacecraft motion failed: {error:#}", ship.name);
                ship.error = Some(diagnostic.clone());
                self.diagnostics.push(diagnostic);
            }
        }
    }
    pub fn validate(&self) -> Result<()> {
        for body in &self.bodies {
            body.orbit.validate().with_context(|| body.name.clone())?;
            ensure!(
                body.kinematics.is_finite(),
                "{}: non-finite kinematics",
                body.name
            );
        }
        for ship in &self.ships {
            ensure!(
                ship.error.is_none(),
                "{}",
                ship.error.as_deref().unwrap_or("")
            );
            ship.orbit.validate().with_context(|| ship.name.clone())?;
            ensure!(
                ship.kinematics.is_finite(),
                "{}: non-finite kinematics",
                ship.name
            );
        }
        Ok(())
    }
    pub fn launch_ship(
        &mut self,
        name: String,
        parent: BodyId,
        mut orbit: Orbit,
    ) -> Result<ShipId> {
        let body = self.bodies.get(parent.0).context("Invalid launch body")?;
        orbit.mu = body.mu;
        orbit.true_anomaly = orbit.mean_anomaly;
        orbit.validate()?;
        ensure!(
            orbit.semi_major_axis != 0.0,
            "Cannot launch a zero-axis orbit"
        );
        let id = ShipId(self.ships.len());
        self.ships.push(Spacecraft::new(name, parent, orbit));
        Ok(id)
    }
    pub fn set_ship_visible(&mut self, id: ShipId, visible: bool) -> Result<()> {
        let ship = self.ships.get_mut(id.0).context("Unknown spacecraft")?;
        ensure!(
            ship.status == crate::flight::FlightStatus::Orbiting,
            "Spacecraft has no live orbit"
        );
        ship.visible_orbit = visible;
        Ok(())
    }
    pub fn set_all_ship_orbits_visible(&mut self, visible: bool) {
        for ship in &mut self.ships {
            if ship.status == crate::flight::FlightStatus::Orbiting {
                ship.visible_orbit = visible;
            }
        }
    }
}

fn optional_unit(
    value: Option<&Value>,
    unit: Unit,
    identifier: &str,
    field: &str,
    diagnostics: &mut Vec<String>,
) -> f64 {
    match value.map(|value| read_unit(value, unit)).transpose() {
        Ok(value) => value.unwrap_or(0.0),
        Err(error) => {
            diagnostics.push(format!(
                "{identifier}: invalid {field}; defaulted to zero like the C++ loader ({error})"
            ));
            0.0
        }
    }
}

fn parent_first_order(bodies: &[CelestialBody]) -> Result<Vec<BodyId>> {
    let mut order = Vec::with_capacity(bodies.len());
    let mut included = vec![false; bodies.len()];
    while order.len() < bodies.len() {
        let previous = order.len();
        for (index, body) in bodies.iter().enumerate() {
            if !included[index] && body.parent.is_none_or(|parent| included[parent.0]) {
                included[index] = true;
                order.push(BodyId(index));
            }
        }
        ensure!(order.len() > previous, "Cyclic body hierarchy");
    }
    Ok(order)
}

#[derive(Deserialize)]
struct BodyEntry {
    identifier: String,
    name: String,
    reference: Option<String>,
    #[serde(rename = "type")]
    kind: Option<String>,
    radius: Value,
    gm: f64,
    orbit: Value,
    day_length: Option<Value>,
    day_offset: Option<Value>,
    axial: Option<Value>,
    // Undefined != Null in HJSON: the source adds a zero-height atmosphere when absent.
    #[serde(default = "empty_atmosphere")]
    atmosphere: Option<AtmosphereEntry>,
    texture: Option<Terrain>,
}
#[derive(Deserialize)]
struct AtmosphereEntry {
    scale_height: Value,
}
fn empty_atmosphere() -> Option<AtmosphereEntry> {
    Some(AtmosphereEntry {
        scale_height: Value::from(0),
    })
}
#[derive(Deserialize)]
struct SatelliteEntry {
    name: String,
    orbit: Value,
    model: Option<String>,
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::default_data_path;

    #[test]
    fn orbit_filter_changes_only_live_visibility_not_physics() -> Result<()> {
        use crate::flight::FlightStatus;
        let mut system = SolarSystem::load(&default_data_path())?;
        assert!(system.ships.iter().all(|ship| !ship.visible_orbit));
        let positions: Vec<_> = system
            .ships
            .iter()
            .map(|ship| ship.kinematics.absolute_position())
            .collect();
        system.ships[0].status = FlightStatus::Docked(0);
        system.ships[1].status = FlightStatus::Crashed;
        assert!(system.set_ship_visible(ShipId(usize::MAX), true).is_err());
        assert!(system.set_ship_visible(ShipId(0), true).is_err());
        system.set_ship_visible(ShipId(2), true)?;
        assert!(system.ships[2].visible_orbit);
        system.set_all_ship_orbits_visible(false);
        assert!(system.ships.iter().all(|ship| !ship.visible_orbit));
        system.set_all_ship_orbits_visible(true);
        assert!(system.ships.iter().skip(2).all(|ship| ship.visible_orbit));
        assert!(!system.ships[0].visible_orbit && !system.ships[1].visible_orbit);
        assert_eq!(
            positions,
            system
                .ships
                .iter()
                .map(|ship| ship.kinematics.absolute_position())
                .collect::<Vec<_>>()
        );
        Ok(())
    }

    #[test]
    fn body_ground_track_uses_rotation_tilt_and_rejects_zero_position() -> Result<()> {
        let mut system = SolarSystem::load(&default_data_path())?;
        let earth_id = system.body_id("earth").unwrap();
        let earth = &mut system.bodies[earth_id.0];
        earth.rotation_period = 100.0;
        earth.rotation_offset = 0.0;
        earth.axial_tilt = 0.0;
        let [latitude, longitude] = earth.ground_track(DVec3::X, 25.0).unwrap();
        assert!(latitude.abs() < 1e-6);
        assert!((longitude + std::f64::consts::FRAC_PI_2).abs() < 1e-6);
        assert!(earth.ground_track(DVec3::ZERO, 0.0).is_none());
        earth.axial_tilt = 0.4;
        let local = glam::Vec3::new(0.4, 0.8, 0.2).normalize();
        let position = (earth.orientation(25.0) * local).as_dvec3();
        let result = earth.ground_track(position, 25.0).unwrap();
        assert!((result[0] - local.z.asin() as f64).abs() < 1e-6);
        assert!((result[1] - local.y.atan2(local.x) as f64).abs() < 1e-6);
        earth.rotation_period = 0.0;
        assert!(earth.orientation(0.0).is_finite());
        Ok(())
    }

    #[test]
    fn original_system_loads_parents_gravity_units_and_satellites() -> Result<()> {
        let system = SolarSystem::load(&default_data_path())?;
        assert_eq!(system.bodies.len(), 11);
        assert_eq!(system.ships.len(), 64);
        assert!(system.diagnostics.is_empty());
        let earth_id = system.body_id("earth").unwrap();
        let earth = &system.bodies[earth_id.0];
        let moon = &system.bodies[system.body_id("moon").unwrap().0];
        assert_eq!(moon.parent, Some(earth_id));
        assert_eq!(moon.orbit.mu, earth.mu);
        assert_eq!(earth.orbit.mu, system.bodies[system.sun.0].mu);
        assert!((earth.sphere_of_influence - 924654.0).abs() < 100.0);
        assert_eq!(earth.radius, 6371.0);
        assert_eq!(earth.atmosphere_scale_height, Some(8.5));
        assert_eq!(
            system.bodies[system.sun.0].atmosphere_scale_height,
            Some(0.0)
        );
        let iss = system
            .ships
            .iter()
            .find(|satellite| satellite.name == "ISS")
            .unwrap();
        assert_eq!(iss.parent, earth_id);
        assert_eq!(iss.model.as_deref(), Some("iss"));
        assert!((iss.orbit.semi_major_axis - 6795.667972420554).abs() < 1e-10);
        assert_eq!(iss.orbit.epoch, 0.0);
        assert!(system.bodies[system.body_id("venus").unwrap().0].rotation_period < 0.0);
        Ok(())
    }

    #[test]
    fn hierarchical_centers_and_future_positions_follow_parent_motion() -> Result<()> {
        let mut system = SolarSystem::load(&default_data_path())?;
        let earth_id = system.body_id("earth").unwrap();
        let moon_id = system.body_id("moon").unwrap();
        system.update(86400.0);
        let earth = &system.bodies[earth_id.0];
        let moon = &system.bodies[moon_id.0];
        assert_eq!(moon.kinematics.center, earth.kinematics.absolute_position());
        assert_eq!(
            moon.kinematics.future_center,
            earth.kinematics.absolute_future_position()
        );
        let future = moon.kinematics.absolute_future_position();
        for satellite in &system.ships {
            assert_eq!(
                satellite.kinematics.center,
                earth.kinematics.absolute_position()
            );
        }
        system.update(86400.0 + SECONDS_PER_TICK as f64);
        assert!((system.bodies[moon_id.0].kinematics.absolute_position() - future).length() < 1e-8);
        system.validate()?;
        Ok(())
    }

    #[test]
    fn cyclic_hierarchies_fail_instead_of_looping() -> Result<()> {
        let mut system = SolarSystem::load(&default_data_path())?;
        system.bodies[system.sun.0].parent = system.body_id("earth");
        assert!(parent_first_order(&system.bodies).is_err());
        Ok(())
    }
}
