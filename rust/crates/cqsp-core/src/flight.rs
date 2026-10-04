//! Native spacecraft actions and the ten-minute movement/command loop.
use crate::{
    clock::SECONDS_PER_TICK,
    economy::Ledger,
    orbit::{Orbit, StateVector, normalize_angle},
    solar_system::{BodyId, CelestialBody, Kinematics},
    spaceflight::MissionId,
};
use anyhow::{Result, ensure};
use glam::DVec3;
use std::{
    collections::VecDeque,
    f64::consts::{PI, TAU},
};

#[derive(Clone, Copy, Debug)]
pub struct Burn {
    pub delta_v: DVec3,
    /// Seconds from the time this plan is queued, matching Maneuver_t.
    pub delay: f64,
}

impl Burn {
    pub fn at(self, current_time: f64) -> Result<Maneuver> {
        let maneuver = Maneuver {
            delta_v: self.delta_v,
            time: current_time + self.delay,
        };
        ensure!(
            maneuver.delta_v.is_finite() && maneuver.time.is_finite(),
            "Non-finite maneuver"
        );
        Ok(maneuver)
    }
}

#[derive(Clone, Copy, Debug)]
pub struct Maneuver {
    pub delta_v: DVec3,
    pub time: f64,
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum Trigger {
    EnterSoi,
    ExitSoi,
    Maneuver,
    Crash,
}

#[derive(Clone, Copy, Debug)]
pub enum Command {
    CircularizeAtApoapsis,
    CircularizeAtPeriapsis,
    SetApoapsis(f64),
    SetPeriapsis(f64),
    SetInclination(f64),
    MatchPlanes(Orbit),
    ForceMatchPlanes(Orbit),
    CoplanarIntercept(Orbit),
    CoplanarInterceptAndTransfer(Orbit),
    ExitSoi(f64),
    Land(usize),
    SelfDestruct,
    /// These two commands also do nothing in the original switch.
    Impulse,
    InterceptAndCircularizeBody,
}

#[derive(Clone, Copy, Debug)]
pub struct TriggeredCommand {
    pub trigger: Trigger,
    pub command: Command,
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum FlightStatus {
    Orbiting,
    Crashed,
    Docked(usize),
}

#[derive(Clone, Copy, Debug, PartialEq)]
pub enum FlightEvent {
    Maneuver { time: f64, delta_v: DVec3 },
    LeftSoi { from: BodyId, to: BodyId },
    EnteredSoi { from: BodyId, to: BodyId },
    Crashed(BodyId),
    Docked(usize),
}

#[derive(Debug)]
pub struct Spacecraft {
    pub name: String,
    pub parent: BodyId,
    pub model: Option<String>,
    pub orbit: Orbit,
    pub kinematics: Kinematics,
    pub status: FlightStatus,
    pub maneuvers: VecDeque<Maneuver>,
    pub commands: VecDeque<TriggeredCommand>,
    pub events: Vec<FlightEvent>,
    pub error: Option<String>,
    pub cargo: Option<Vec<MissionId>>,
    pub stockpile: Option<Ledger>,
}

impl Spacecraft {
    pub fn new(name: String, parent: BodyId, orbit: Orbit) -> Self {
        Self {
            name,
            parent,
            model: None,
            orbit,
            kinematics: Kinematics::default(),
            status: FlightStatus::Orbiting,
            maneuvers: VecDeque::new(),
            commands: VecDeque::new(),
            events: Vec::new(),
            error: None,
            cargo: None,
            stockpile: None,
        }
    }
    pub fn queue_burn(&mut self, burn: Burn, current_time: f64) -> Result<()> {
        self.maneuvers.push_back(burn.at(current_time)?);
        Ok(())
    }
    pub fn queue_command(&mut self, trigger: Trigger, command: Command) {
        self.commands
            .push_back(TriggeredCommand { trigger, command });
    }
    pub fn step(&mut self, time: f64, bodies: &[CelestialBody]) -> Result<()> {
        self.events.clear();
        if self.status != FlightStatus::Orbiting || self.error.is_some() {
            return Ok(());
        }
        ensure!(
            time.is_finite() && self.parent.0 < bodies.len(),
            "Invalid spacecraft time/parent"
        );
        // Like CalculatePosition, retain the starting parent for the rest of this tick.
        let starting_parent = self.parent;
        let parent = &bodies[starting_parent.0];
        self.orbit.update(time);
        if let Some(maneuver) = self
            .maneuvers
            .front()
            .copied()
            .filter(|maneuver| maneuver.time <= time)
        {
            self.orbit = self.orbit.apply_impulse(maneuver.delta_v, maneuver.time)?;
            self.maneuvers.pop_front();
            self.events.push(FlightEvent::Maneuver {
                time: maneuver.time,
                delta_v: maneuver.delta_v,
            });
            self.process_trigger(Trigger::Maneuver, time, bodies)?;
        }
        if matches!(self.status, FlightStatus::Docked(_)) {
            return Ok(());
        }
        // A late maneuver leaves v at its own epoch for this tick in the source.
        self.kinematics.state = self.orbit.state_at_anomaly(self.orbit.true_anomaly);
        if self.kinematics.state.position.length() > parent.sphere_of_influence
            && let Some(grandparent) = parent.parent
        {
            let state = StateVector {
                position: self.kinematics.state.position + parent.kinematics.state.position,
                velocity: self.kinematics.state.velocity + parent.kinematics.state.velocity,
            };
            self.orbit = Orbit::from_state(state, bodies[grandparent.0].mu, time)?;
            self.parent = grandparent;
            self.kinematics.state = state;
            self.events.push(FlightEvent::LeftSoi {
                from: starting_parent,
                to: grandparent,
            });
            self.process_trigger(Trigger::ExitSoi, time, bodies)?;
        }
        if self.status == FlightStatus::Crashed {
            self.kinematics.state.position = DVec3::ZERO;
            self.maneuvers.clear();
            self.commands.clear();
            return Ok(());
        }
        if matches!(self.status, FlightStatus::Docked(_)) {
            return Ok(());
        }
        if self.kinematics.state.position.length() <= parent.radius {
            if !self.process_trigger(Trigger::Crash, time, bodies)? {
                self.status = FlightStatus::Crashed;
                self.kinematics.state.position = DVec3::ZERO;
                self.orbit.semi_major_axis = 0.0;
                self.events.push(FlightEvent::Crashed(self.parent));
            }
            return Ok(());
        }
        self.kinematics.center = parent.kinematics.absolute_position();
        let future_center = parent.kinematics.absolute_future_position();
        for &child in &parent.children {
            let target = &bodies[child.0];
            if self
                .kinematics
                .state
                .position
                .distance(target.kinematics.state.position)
                > target.sphere_of_influence
            {
                continue;
            }
            let state = StateVector {
                position: self.kinematics.state.position - target.kinematics.state.position,
                velocity: self.kinematics.state.velocity - target.kinematics.state.velocity,
            };
            self.orbit = Orbit::from_state(state, target.mu, time)?;
            self.parent = child;
            self.kinematics.state = self.orbit.state_at_anomaly(self.orbit.true_anomaly);
            self.kinematics.center = target.kinematics.absolute_position();
            self.events.push(FlightEvent::EnteredSoi {
                from: starting_parent,
                to: child,
            });
            self.process_trigger(Trigger::EnterSoi, time, bodies)?;
            break;
        }
        // SelfDestruct on SOI entry is late: the source still writes this tick's
        // future position before excluding the craft from subsequent updates.
        if !matches!(self.status, FlightStatus::Docked(_)) {
            self.kinematics.future_position =
                self.orbit.state_at(time + SECONDS_PER_TICK as f64).position;
            // The original loop uses the starting parent's future center for one tick.
            self.kinematics.future_center = future_center;
        }
        Ok(())
    }

    fn process_trigger(
        &mut self,
        trigger: Trigger,
        time: f64,
        bodies: &[CelestialBody],
    ) -> Result<bool> {
        let Some(command) = self
            .commands
            .front()
            .copied()
            .filter(|command| command.trigger == trigger)
        else {
            return Ok(false);
        };
        let orbit = self.orbit;
        let burns: Vec<Burn> = match command.command {
            Command::CircularizeAtApoapsis => vec![orbit.circularize_at_apoapsis()],
            Command::CircularizeAtPeriapsis => vec![orbit.circularize_at_periapsis()],
            Command::SetApoapsis(radius) => vec![orbit.set_apoapsis(radius)],
            Command::SetPeriapsis(radius) => vec![orbit.set_periapsis(radius)],
            Command::SetInclination(inclination) => vec![orbit.set_inclination(inclination)],
            Command::MatchPlanes(target) => vec![orbit.match_planes(target)],
            Command::CoplanarIntercept(target) => vec![orbit.coplanar_intercept(target, time)[0]],
            Command::CoplanarInterceptAndTransfer(target) => {
                orbit.coplanar_intercept(target, time).into()
            }
            Command::ExitSoi(radius) => {
                vec![orbit.depart_body(self.kinematics.state, &bodies[self.parent.0], radius)]
            }
            Command::ForceMatchPlanes(target) => {
                self.orbit.ascending_node = target.ascending_node;
                self.orbit.inclination = target.inclination;
                vec![Burn {
                    delta_v: DVec3::ZERO,
                    delay: 100.0,
                }]
            }
            Command::Land(settlement) => {
                self.status = FlightStatus::Docked(settlement);
                self.events.push(FlightEvent::Docked(settlement));
                Vec::new()
            }
            Command::SelfDestruct => {
                self.status = FlightStatus::Crashed;
                Vec::new()
            }
            Command::Impulse | Command::InterceptAndCircularizeBody => Vec::new(),
        };
        // Validate the entire command before appending either half of a transfer.
        let maneuvers = burns
            .into_iter()
            .map(|burn| burn.at(time))
            .collect::<Result<Vec<_>>>()?;
        self.maneuvers.extend(maneuvers);
        self.commands.pop_front();
        Ok(true)
    }

    pub fn transfer_to_moon(&mut self, target: Orbit, time: f64) -> Result<()> {
        let target = target.legacy_copy();
        let orbit = self.orbit.legacy_copy();
        if orbit.eccentricity > 0.00001 {
            self.queue_burn(orbit.circularize_at_periapsis(), time)?;
            self.queue_command(Trigger::Maneuver, Command::MatchPlanes(target));
        } else {
            self.queue_burn(orbit.match_planes(target), time)?;
        }
        self.queue_command(Trigger::Maneuver, Command::ForceMatchPlanes(target));
        self.queue_command(Trigger::Maneuver, Command::CoplanarIntercept(target));
        self.queue_command(Trigger::EnterSoi, Command::CircularizeAtPeriapsis);
        Ok(())
    }
    pub fn land_on_moon(
        &mut self,
        target_id: BodyId,
        target: &CelestialBody,
        settlement: usize,
        time: f64,
    ) -> Result<()> {
        if self.parent != target_id {
            self.transfer_to_moon(target.orbit, time)?;
        }
        self.queue_command(
            Trigger::Maneuver,
            Command::SetPeriapsis(target.radius * 0.9),
        );
        self.queue_command(Trigger::Crash, Command::Land(settlement));
        Ok(())
    }
}

impl Orbit {
    pub fn circularize_at_apoapsis(self) -> Burn {
        self.change_apsis_speed(self.apoapsis(), (self.mu / self.apoapsis()).sqrt(), PI)
    }
    pub fn circularize_at_periapsis(self) -> Burn {
        self.change_apsis_speed(self.periapsis(), (self.mu / self.periapsis()).sqrt(), 0.0)
    }
    pub fn set_apoapsis(self, radius: f64) -> Burn {
        let axis = (self.periapsis() + radius) / 2.0;
        self.change_apsis_speed(
            self.periapsis(),
            speed_at_radius(self.mu, axis, self.periapsis()),
            0.0,
        )
    }
    pub fn set_periapsis(self, radius: f64) -> Burn {
        if self.eccentricity > 1.0 {
            return Burn {
                delta_v: DVec3::ZERO,
                delay: 0.0,
            };
        }
        let axis = (self.apoapsis() + radius) / 2.0;
        self.change_apsis_speed(
            self.apoapsis(),
            speed_at_radius(self.mu, axis, self.apoapsis()),
            PI,
        )
    }
    fn change_apsis_speed(self, radius: f64, speed: f64, anomaly: f64) -> Burn {
        Burn {
            delta_v: DVec3::new(
                0.0,
                speed - speed_at_radius(self.mu, self.semi_major_axis, radius),
                0.0,
            ),
            delay: self.time_to_anomaly(anomaly),
        }
    }
    pub fn flight_path_angle(self, anomaly: f64) -> f64 {
        // Preserve the minus sign used by the legacy helper.
        (self.eccentricity * anomaly.sin()).atan2(1.0 - self.eccentricity * anomaly.cos())
    }
    pub fn set_inclination(self, inclination: f64) -> Burn {
        let change = -(inclination - self.inclination);
        let cost = |anomaly: f64| {
            (2.0 * self.speed_at(anomaly)
                * self.flight_path_angle(anomaly).cos()
                * (change / 2.0).sin())
            .abs()
        };
        let first = PI - self.periapsis_argument;
        let second = TAU - self.periapsis_argument;
        let anomaly = if cost(first) > cost(second) {
            second
        } else {
            first
        };
        plane_burn(
            self.speed_at(anomaly),
            change,
            self.time_to_anomaly(anomaly),
        )
    }
    pub fn set_circular_inclination(self, inclination: f64) -> Burn {
        let mut anomaly = PI - self.periapsis_argument;
        let mut sign = -1.0;
        if self.true_anomaly > anomaly {
            anomaly = TAU - self.periapsis_argument;
            sign = 1.0;
        }
        if self.periapsis_argument > PI {
            sign = -sign;
        }
        plane_burn(
            (self.mu / self.semi_major_axis).sqrt(),
            (inclination - self.inclination) * sign,
            self.time_to_anomaly(anomaly),
        )
    }
    pub fn normal(self) -> DVec3 {
        // Source normal uses current position but velocity at v=0, not the current velocity.
        self.state_at_anomaly(self.true_anomaly)
            .position
            .normalize()
            .cross(self.state_at_anomaly(0.0).velocity.normalize())
            .normalize()
    }
    pub fn anomaly_from_vector(self, vector: DVec3) -> f64 {
        let periapsis = self.state_at_anomaly(0.0).position;
        let angle = vector
            .normalize()
            .dot(periapsis.normalize())
            .clamp(-1.0, 1.0)
            .acos();
        // GLM angle's dot is intentionally not normalized in this second test.
        let half = vector
            .dot(self.normal().cross(periapsis))
            .clamp(-1.0, 1.0)
            .acos();
        if half.abs() < PI / 2.0 {
            angle
        } else {
            TAU - angle
        }
    }
    pub fn match_planes(self, target: Self) -> Burn {
        let anomaly =
            normalize_angle(self.anomaly_from_vector(self.normal().cross(target.normal())));
        let angle = self
            .normal()
            .normalize()
            .dot(target.normal().normalize())
            .clamp(-1.0, 1.0)
            .acos();
        plane_burn(self.speed_at(anomaly), angle, self.time_to_anomaly(anomaly))
    }
    pub fn hohmann_transfer(self, radius: f64) -> Option<[Burn; 2]> {
        (self.eccentricity <= 1e-5).then(|| self.unsafe_hohmann_transfer(radius))
    }
    pub fn unsafe_hohmann_transfer(self, radius: f64) -> [Burn; 2] {
        let axis = (self.semi_major_axis + radius) / 2.0;
        [
            Burn {
                delta_v: DVec3::new(
                    0.0,
                    speed_at_radius(self.mu, axis, self.semi_major_axis)
                        - (self.mu / self.semi_major_axis).sqrt(),
                    0.0,
                ),
                delay: 0.0,
            },
            Burn {
                delta_v: DVec3::new(
                    0.0,
                    (self.mu / radius).sqrt() - speed_at_radius(self.mu, axis, radius),
                    0.0,
                ),
                delay: PI * (axis * axis * axis / self.mu).sqrt(),
            },
        ]
    }
    pub fn coplanar_intercept(self, target: Self, time: f64) -> [Burn; 2] {
        let axis = (self.semi_major_axis + target.semi_major_axis) / 2.0;
        let transfer_angle = PI - PI * (axis * axis * axis / self.mu).sqrt() * target.mean_motion();
        let phase = target.true_anomaly_at(time) + target.periapsis_argument
            - (self.true_anomaly_at(time) + self.periapsis_argument);
        let relative_motion = self.mean_motion() - target.mean_motion();
        let mut wait = (phase - transfer_angle) / relative_motion;
        if wait < 0.0 {
            wait = (phase - transfer_angle + TAU) / relative_motion;
        }
        let mut burns = self.unsafe_hohmann_transfer(target.semi_major_axis);
        for burn in &mut burns {
            burn.delay += wait;
        }
        burns
    }
    pub fn depart_body(self, state: StateVector, body: &CelestialBody, radius: f64) -> Burn {
        let parent_orbit = body.orbit;
        let parent_radius = parent_orbit.radius_at(parent_orbit.true_anomaly);
        let axis = (parent_radius + radius) / 2.0;
        let eccentricity = (parent_radius - radius) / (parent_radius + radius);
        let intended_radius = axis * (1.0 - eccentricity * eccentricity) / (1.0 - eccentricity);
        let intended_speed = speed_at_radius(parent_orbit.mu, axis, intended_radius.abs());
        let excess = parent_orbit.speed_at(parent_orbit.true_anomaly) - intended_speed;
        let current_radius = self.radius_at(self.true_anomaly);
        let escape_eccentricity = 1.0 + current_radius * excess * excess / self.mu;
        let turn = (-1.0 / escape_eccentricity).acos();
        let speed = (excess * excess + 2.0 * self.mu / current_radius).sqrt();
        let normal = self.normal();
        let forward = body.kinematics.state.velocity.normalize();
        let projected = forward - forward.dot(normal) / normal.length_squared() * normal;
        let eccentric = state.velocity.cross(state.position.cross(state.velocity)) / self.mu
            - state.position.normalize();
        let mut anomaly = if eccentric.length() == 0.0 {
            0.0
        } else {
            eccentric
                .normalize()
                .dot(projected.normalize())
                .clamp(-1.0, 1.0)
                .acos()
        };
        if eccentric
            .normalize()
            .cross(projected.normalize())
            .dot(state.position.cross(state.velocity).normalize())
            < 0.0
        {
            anomaly = TAU - anomaly;
        }
        let target = anomaly - turn + PI;
        Burn {
            delta_v: DVec3::new(0.0, speed - self.speed_at(target), 0.0),
            delay: self.time_to_anomaly(target),
        }
    }
}

fn speed_at_radius(mu: f64, axis: f64, radius: f64) -> f64 {
    (mu * (2.0 / radius - 1.0 / axis)).sqrt()
}
fn plane_burn(speed: f64, angle: f64, delay: f64) -> Burn {
    Burn {
        delta_v: DVec3::new(0.0, speed * (angle.cos() - 1.0), speed * angle.sin()),
        delay,
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::{
        default_data_path,
        solar_system::{ShipId, SolarSystem},
    };

    fn launch(parent_name: &str, axis: f64, eccentricity: f64) -> Result<(SolarSystem, ShipId)> {
        let mut system = SolarSystem::load(&default_data_path())?;
        let parent = system.body_id(parent_name).unwrap();
        let orbit = Orbit {
            semi_major_axis: axis,
            eccentricity,
            inclination: 0.3,
            ascending_node: 1.1,
            periapsis_argument: 0.7,
            mean_anomaly: 0.5,
            ..Orbit::default()
        };
        let ship = system.launch_ship("Test vehicle".into(), parent, orbit)?;
        Ok((system, ship))
    }

    #[test]
    fn one_overdue_maneuver_runs_per_tick_at_its_own_epoch() -> Result<()> {
        let (mut system, id) = launch("earth", 12000.0, 0.25)?;
        let original = system.ships[id.0].orbit;
        let first = Burn {
            delta_v: DVec3::new(0.0, 0.01, 0.0),
            delay: 10.0,
        };
        system.ships[id.0].queue_burn(first, 0.0)?;
        system.ships[id.0].queue_burn(
            Burn {
                delay: 20.0,
                ..first
            },
            0.0,
        )?;
        system.update(600.0);
        let ship = &system.ships[id.0];
        assert_eq!(ship.maneuvers.len(), 1);
        assert_eq!(ship.events.len(), 1);
        assert_eq!(ship.orbit.epoch, 10.0);
        let expected = original.apply_impulse(first.delta_v, 10.0)?.state_at(10.0);
        assert!((ship.kinematics.state.position - expected.position).length() < 1e-7);
        system.update(1200.0);
        assert!(system.ships[id.0].maneuvers.is_empty());
        assert_eq!(system.ships[id.0].orbit.epoch, 20.0);
        system.validate()?;
        Ok(())
    }

    #[test]
    fn trigger_queue_does_not_skip_a_mismatching_front_command() -> Result<()> {
        let (mut system, id) = launch("earth", 12000.0, 0.25)?;
        let ship = &mut system.ships[id.0];
        ship.queue_burn(
            Burn {
                delta_v: DVec3::ZERO,
                delay: 0.0,
            },
            0.0,
        )?;
        ship.queue_command(Trigger::ExitSoi, Command::SelfDestruct);
        ship.queue_command(Trigger::Maneuver, Command::SelfDestruct);
        system.update(0.0);
        assert_eq!(system.ships[id.0].commands.len(), 2);
        assert_eq!(system.ships[id.0].status, FlightStatus::Orbiting);
        system.validate()?;
        Ok(())
    }

    #[test]
    fn collision_crashes_or_docks_then_stops_orbital_updates() -> Result<()> {
        let (mut system, crashed) = launch("earth", 5000.0, 0.05)?;
        let parent = system.ships[crashed.0].parent;
        let docked = system.launch_ship("Lander".into(), parent, system.ships[crashed.0].orbit)?;
        system.ships[docked.0].queue_command(Trigger::Crash, Command::Land(123));
        system.update(0.0);
        assert_eq!(system.ships[crashed.0].status, FlightStatus::Crashed);
        assert_eq!(system.ships[crashed.0].orbit.semi_major_axis, 0.0);
        assert_eq!(
            system.ships[crashed.0].kinematics.state.position,
            DVec3::ZERO
        );
        assert_eq!(system.ships[docked.0].status, FlightStatus::Docked(123));
        let landed = system.ships[docked.0].kinematics.state;
        system.update(600.0);
        assert_eq!(system.ships[docked.0].kinematics.state, landed);
        system.validate()?;
        Ok(())
    }

    #[test]
    fn entering_moon_soi_converts_the_state_and_fires_its_trigger() -> Result<()> {
        let mut system = SolarSystem::load(&default_data_path())?;
        let earth = system.body_id("earth").unwrap();
        let moon = system.body_id("moon").unwrap();
        let moon_state = system.bodies[moon.0].kinematics.state;
        let offset = DVec3::new(10000.0, 0.0, 0.0);
        let relative_velocity = DVec3::new(0.1, 0.3, 0.02);
        let state = StateVector {
            position: moon_state.position + offset,
            velocity: moon_state.velocity + relative_velocity,
        };
        let orbit = Orbit::from_state(state, system.bodies[earth.0].mu, 0.0)?;
        let id = system.launch_ship("Moon arrival".into(), earth, orbit)?;
        system.ships[id.0].queue_command(Trigger::EnterSoi, Command::CircularizeAtPeriapsis);
        system.update(0.0);
        let ship = &system.ships[id.0];
        assert_eq!(ship.parent, moon);
        assert!(
            matches!(ship.events.last(), Some(FlightEvent::EnteredSoi { from, to }) if *from == earth && *to == moon)
        );
        assert!((ship.kinematics.state.position - offset).length() < 1e-6);
        assert!((ship.kinematics.state.velocity - relative_velocity).length() < 1e-9);
        assert_eq!(ship.maneuvers.len(), 1);
        assert_eq!(
            ship.kinematics.center,
            system.bodies[moon.0].kinematics.absolute_position()
        );
        assert_eq!(
            ship.kinematics.future_center,
            system.bodies[earth.0].kinematics.absolute_future_position()
        );
        system.validate()?;
        Ok(())
    }

    #[test]
    fn self_destruct_on_soi_entry_finishes_this_ticks_future_position() -> Result<()> {
        let mut system = SolarSystem::load(&default_data_path())?;
        let earth = system.body_id("earth").unwrap();
        let moon = system.body_id("moon").unwrap();
        let moon_state = system.bodies[moon.0].kinematics.state;
        let orbit = Orbit::from_state(
            StateVector {
                position: moon_state.position + DVec3::new(10000.0, 0.0, 0.0),
                velocity: moon_state.velocity + DVec3::new(0.1, 0.3, 0.02),
            },
            system.bodies[earth.0].mu,
            0.0,
        )?;
        let id = system.launch_ship("Self destruct arrival".into(), earth, orbit)?;
        system.ships[id.0].queue_command(Trigger::EnterSoi, Command::SelfDestruct);
        system.update(0.0);
        let ship = &system.ships[id.0];
        assert_eq!(ship.status, FlightStatus::Crashed);
        assert_eq!(ship.parent, moon);
        assert_eq!(
            ship.kinematics.future_position,
            ship.orbit.state_at(600.0).position
        );
        assert_eq!(
            ship.kinematics.future_center,
            system.bodies[earth.0].kinematics.absolute_future_position()
        );
        let retained = ship.kinematics;
        system.update(600.0);
        assert_eq!(system.ships[id.0].kinematics.state, retained.state);
        assert_eq!(
            system.ships[id.0].kinematics.future_position,
            retained.future_position
        );
        system.validate()?;
        Ok(())
    }

    #[test]
    fn leaving_moon_soi_preserves_source_starting_center_for_one_tick() -> Result<()> {
        let (mut system, id) = launch("moon", 100000.0, 0.1)?;
        let moon = system.body_id("moon").unwrap();
        let earth = system.body_id("earth").unwrap();
        let state = system.ships[id.0].orbit.state_at(0.0);
        let expected = state.position + system.bodies[moon.0].kinematics.state.position;
        system.update(0.0);
        let ship = &system.ships[id.0];
        assert_eq!(ship.parent, earth);
        assert!(
            matches!(ship.events.first(), Some(FlightEvent::LeftSoi { from, to }) if *from == moon && *to == earth)
        );
        assert!((ship.kinematics.state.position - expected).length() < 1e-6);
        assert_eq!(
            ship.kinematics.center,
            system.bodies[moon.0].kinematics.absolute_position()
        );
        system.update(600.0);
        assert_eq!(
            system.ships[id.0].kinematics.center,
            system.bodies[earth.0].kinematics.absolute_position()
        );
        system.validate()?;
        Ok(())
    }

    #[test]
    fn hohmann_burn_pair_reaches_a_circular_destination_orbit() -> Result<()> {
        let (mut system, id) = launch("earth", 7000.0, 0.0)?;
        let burns = system.ships[id.0].orbit.hohmann_transfer(14000.0).unwrap();
        for burn in burns {
            system.ships[id.0].queue_burn(burn, 0.0)?;
        }
        system.update(0.0);
        assert!((system.ships[id.0].orbit.apoapsis() - 14000.0).abs() < 1e-7);
        system.update(burns[1].delay);
        let orbit = system.ships[id.0].orbit;
        assert!(orbit.eccentricity < 1e-10);
        assert!((orbit.semi_major_axis - 14000.0).abs() < 1e-7);
        system.validate()?;
        Ok(())
    }

    #[test]
    fn lunar_deorbit_and_landing_complete_without_a_renderer() -> Result<()> {
        let (mut system, id) = launch("moon", 3500.0, 0.05)?;
        let moon = system.body_id("moon").unwrap();
        system.ships[id.0].queue_burn(
            Burn {
                delta_v: DVec3::ZERO,
                delay: 0.0,
            },
            0.0,
        )?;
        let target = &system.bodies[moon.0];
        system.ships[id.0].land_on_moon(moon, target, 456, 0.0)?;
        for tick in 0..120 {
            system.update(tick as f64 * SECONDS_PER_TICK as f64);
            system.validate()?;
            if system.ships[id.0].status != FlightStatus::Orbiting {
                break;
            }
        }
        assert_eq!(system.ships[id.0].status, FlightStatus::Docked(456));
        Ok(())
    }

    #[test]
    fn non_finite_maneuvers_are_rejected_before_queueing() -> Result<()> {
        let (mut system, id) = launch("earth", 7000.0, 0.0)?;
        assert!(
            system.ships[id.0]
                .queue_burn(
                    Burn {
                        delta_v: DVec3::ZERO,
                        delay: f64::INFINITY
                    },
                    0.0
                )
                .is_err()
        );
        assert!(system.ships[id.0].maneuvers.is_empty());
        assert!(system.ships[id.0].orbit.hohmann_transfer(14000.0).is_some());
        Ok(())
    }
}
