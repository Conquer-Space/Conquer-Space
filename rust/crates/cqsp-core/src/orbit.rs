//! Kepler propagation in the original inertial XY orbital plane: km, seconds, radians.
use crate::units::{SUN_MU, Unit, read_unit};
use anyhow::{Result, ensure};
use glam::{DQuat, DVec3};
use serde_json::Value;
use std::f64::consts::{PI, TAU};

#[derive(Clone, Copy, Debug, Default, PartialEq)]
pub struct StateVector {
    pub position: DVec3,
    pub velocity: DVec3,
}

#[derive(Clone, Copy, Debug)]
pub struct Orbit {
    pub semi_major_axis: f64,
    pub eccentricity: f64,
    pub inclination: f64,
    pub ascending_node: f64,
    pub periapsis_argument: f64,
    pub mean_anomaly: f64,
    pub epoch: f64,
    pub true_anomaly: f64,
    pub mu: f64,
}

impl Default for Orbit {
    fn default() -> Self {
        Self {
            semi_major_axis: 0.0,
            eccentricity: 0.0,
            inclination: 0.0,
            ascending_node: 0.0,
            periapsis_argument: 0.0,
            mean_anomaly: 0.0,
            epoch: 0.0,
            true_anomaly: 0.0,
            mu: SUN_MU,
        }
    }
}

impl Orbit {
    pub fn load(value: &Value) -> Result<Self> {
        // The source returns immediately for a missing/numeric-zero semi-major axis.
        let axis = &value["semi_major_axis"];
        if !axis.is_string() && axis.as_f64().unwrap_or(0.0) == 0.0 {
            return Ok(Self::default());
        }
        let mean_anomaly = read_unit(&value["M0"], Unit::Angle)?;
        let orbit = Self {
            semi_major_axis: read_unit(axis, Unit::Distance)?,
            eccentricity: value["eccentricity"].as_f64().unwrap_or(0.0),
            inclination: read_unit(&value["inclination"], Unit::Angle)?,
            ascending_node: read_unit(&value["LAN"], Unit::Angle)?,
            periapsis_argument: read_unit(&value["arg_periapsis"], Unit::Angle)?,
            mean_anomaly,
            // Returning std::optional invokes the source copy constructor, which sets v=M0.
            true_anomaly: mean_anomaly,
            // LoadOrbit deliberately ignores asset epoch; don't silently reinterpret it.
            ..Self::default()
        };
        orbit.validate()?;
        Ok(orbit)
    }

    pub fn validate(self) -> Result<()> {
        ensure!(
            [
                self.semi_major_axis,
                self.eccentricity,
                self.inclination,
                self.ascending_node,
                self.periapsis_argument,
                self.mean_anomaly,
                self.epoch,
                self.true_anomaly,
                self.mu,
            ]
            .into_iter()
            .all(f64::is_finite),
            "Non-finite orbital element"
        );
        ensure!(
            self.mu > 0.0 && self.eccentricity >= 0.0,
            "Invalid gravity/eccentricity"
        );
        ensure!(
            self.eccentricity != 1.0,
            "Parabolic orbits are not supported by the legacy solver"
        );
        ensure!(
            self.semi_major_axis == 0.0
                || (self.semi_major_axis > 0.0) == (self.eccentricity < 1.0),
            "Invalid conic axis sign"
        );
        Ok(())
    }

    pub fn mean_motion(self) -> f64 {
        let axis = self.semi_major_axis;
        (self.mu / (axis * axis * axis).abs()).sqrt()
    }
    pub fn period(self) -> f64 {
        let axis = self.semi_major_axis;
        TAU * (axis * axis * axis / self.mu).sqrt()
    }
    pub fn apoapsis(self) -> f64 {
        self.semi_major_axis * (1.0 + self.eccentricity)
    }
    pub fn periapsis(self) -> f64 {
        self.semi_major_axis * (1.0 - self.eccentricity)
    }
    pub fn radius_at(self, anomaly: f64) -> f64 {
        // Preserve the source's negative hyperbolic radius; Cartesian position uses p.
        self.semi_major_axis * (1.0 - self.eccentricity.powi(2)).abs()
            / (1.0 + self.eccentricity * anomaly.cos())
    }
    pub fn true_anomaly_at(self, time: f64) -> f64 {
        if self.semi_major_axis == 0.0 {
            return 0.0;
        }
        let mean = self.mean_anomaly + (time - self.epoch) * self.mean_motion();
        if self.eccentricity < 1.0 {
            let anomaly = solve_kepler(normalize_angle(mean), self.eccentricity, false);
            2.0 * ((1.0 + self.eccentricity).sqrt() * (anomaly / 2.0).sin())
                .atan2((1.0 - self.eccentricity).sqrt() * (anomaly / 2.0).cos())
        } else {
            let anomaly = solve_kepler(mean, self.eccentricity, true);
            2.0 * (((self.eccentricity + 1.0).sqrt() * (anomaly / 2.0).sinh())
                / ((self.eccentricity - 1.0).sqrt() * (anomaly / 2.0).cosh()))
            .atan()
        }
    }
    pub fn update(&mut self, time: f64) {
        self.true_anomaly = self.true_anomaly_at(time);
    }
    /// Explicit C++ Orbit copies reset v to M0; assignment does not.
    pub fn legacy_copy(self) -> Self {
        Self {
            true_anomaly: self.mean_anomaly,
            ..self
        }
    }
    pub fn state_at(self, time: f64) -> StateVector {
        self.state_at_anomaly(self.true_anomaly_at(time))
    }
    pub fn state_at_anomaly(self, anomaly: f64) -> StateVector {
        if self.semi_major_axis == 0.0 {
            return StateVector::default();
        }
        let (sin, cos) = anomaly.sin_cos();
        let parameter = self.semi_major_axis * (1.0 - self.eccentricity * self.eccentricity);
        let denominator = 1.0 + self.eccentricity * cos;
        let rotation = self.rotation();
        StateVector {
            position: parameter
                * rotate_vector(
                    rotation,
                    DVec3::new(cos / denominator, sin / denominator, 0.0),
                ),
            velocity: (self.mu / parameter).sqrt()
                * rotate_vector(rotation, DVec3::new(-sin, self.eccentricity + cos, 0.0)),
        }
    }
    pub fn speed_at(self, anomaly: f64) -> f64 {
        (self.mu * (2.0 / self.radius_at(anomaly).abs() - 1.0 / self.semi_major_axis)).sqrt()
    }
    pub fn rotation(self) -> DQuat {
        multiply_rotation(
            multiply_rotation(
                DQuat::from_rotation_z(self.ascending_node),
                DQuat::from_rotation_x(self.inclination),
            ),
            DQuat::from_rotation_z(self.periapsis_argument),
        )
    }
    pub fn time_to_anomaly(self, target: f64) -> f64 {
        let mean = |anomaly: f64| {
            if self.eccentricity > 1.0 {
                let mut eccentric = ((self.eccentricity + anomaly.cos())
                    / (1.0 + self.eccentricity * anomaly.cos()))
                .acosh();
                if anomaly > PI {
                    eccentric = -eccentric;
                }
                self.eccentricity * eccentric.sinh() - eccentric
            } else {
                let eccentric = ((1.0 - self.eccentricity.powi(2)).sqrt() * anomaly.sin())
                    .atan2(self.eccentricity + anomaly.cos());
                eccentric - self.eccentricity * eccentric.sin()
            }
        };
        let duration = (mean(target) - mean(self.true_anomaly)) / self.mean_motion();
        if self.eccentricity < 1.0 && duration < 0.0 {
            self.period() + duration
        } else {
            duration
        }
    }

    pub fn from_state(state: StateVector, mu: f64, time: f64) -> Result<Self> {
        ensure!(
            state.position.is_finite()
                && state.velocity.is_finite()
                && mu.is_finite()
                && mu > 0.0
                && time.is_finite(),
            "Invalid orbital state"
        );
        let radius = state.position.length();
        let momentum = state.position.cross(state.velocity);
        ensure!(
            radius > 0.0 && momentum.length() > 0.0,
            "Radial/zero-position orbit is unsupported"
        );
        let eccentric_vector = state.velocity.cross(momentum) / mu - state.position.normalize();
        let eccentricity = eccentric_vector.length();
        let speed = state.velocity.length();
        let axis = 1.0 / (2.0 / radius - speed * speed / mu);
        let inclination = (momentum.z / momentum.length()).clamp(-1.0, 1.0).acos();
        let node = DVec3::new(-momentum.y, momentum.x, 0.0);
        let equatorial = node == DVec3::ZERO || inclination == 0.0 || inclination == PI;
        let ascending_node = if equatorial {
            0.0
        } else {
            let angle = (node.x / node.length()).clamp(-1.0, 1.0).acos();
            if node.y < 0.0 { TAU - angle } else { angle }
        };
        let (periapsis_argument, true_anomaly) = if eccentricity == 0.0 && equatorial {
            // Only choose a stable frame where the original actually yields NaNs.
            let rotation =
                DQuat::from_rotation_z(ascending_node) * DQuat::from_rotation_x(inclination);
            let in_plane = rotation.inverse() * state.position;
            (0.0, normalize_angle(in_plane.y.atan2(in_plane.x)))
        } else if eccentricity == 0.0 {
            let angle = angle_between(node, state.position);
            (
                if state.position.z < 0.0 {
                    TAU - angle
                } else {
                    angle
                },
                0.0,
            )
        } else {
            let mut anomaly = angle_between(eccentric_vector, state.position);
            if state.position.dot(state.velocity) < 0.0 {
                anomaly = TAU - anomaly;
            }
            let periapsis = if equatorial {
                let angle = (eccentric_vector.x / eccentricity).clamp(-1.0, 1.0).acos();
                if eccentric_vector.y < 0.0 {
                    TAU - angle
                } else {
                    angle
                }
            } else {
                let angle = angle_between(node, eccentric_vector);
                if eccentric_vector.z < 0.0 {
                    TAU - angle
                } else {
                    angle
                }
            };
            (periapsis, anomaly)
        };
        let mean_anomaly = if eccentricity < 1.0 {
            let eccentric = 2.0
                * ((true_anomaly / 2.0).tan()
                    * ((1.0 - eccentricity) / (1.0 + eccentricity)).sqrt())
                .atan();
            eccentric - eccentricity * eccentric.sin()
        } else {
            let eccentric = 2.0
                * ((true_anomaly / 2.0).tan()
                    * ((eccentricity - 1.0) / (eccentricity + 1.0)).sqrt())
                .atanh();
            eccentricity * eccentric.sinh() - eccentric
        };
        let orbit = Self {
            semi_major_axis: axis,
            eccentricity,
            inclination,
            ascending_node,
            periapsis_argument: normalize_angle(periapsis_argument),
            mean_anomaly,
            epoch: time,
            true_anomaly: normalize_angle(true_anomaly),
            mu,
        };
        orbit.validate()?;
        Ok(orbit)
    }

    /// Impulse axes match ApplyImpulse: radial X, transverse Y, normal Z.
    pub fn apply_impulse(self, impulse: DVec3, time: f64) -> Result<Self> {
        let anomaly = self.true_anomaly_at(time);
        let state = self.state_at_anomaly(anomaly);
        let impulse = rotate_vector(
            multiply_rotation(self.rotation(), DQuat::from_rotation_z(anomaly)),
            impulse,
        );
        Self::from_state(
            StateVector {
                velocity: state.velocity + impulse,
                ..state
            },
            self.mu,
            time,
        )
    }
}

// GLM's operation order matters: a circularizing burn's tiny residual eccentricity
// defines the next apsis, so algebraically equivalent glam rotations change its time.
fn multiply_rotation(p: DQuat, q: DQuat) -> DQuat {
    DQuat::from_xyzw(
        p.w * q.x + p.x * q.w + p.y * q.z - p.z * q.y,
        p.w * q.y + p.y * q.w + p.z * q.x - p.x * q.z,
        p.w * q.z + p.z * q.w + p.x * q.y - p.y * q.x,
        p.w * q.w - p.x * q.x - p.y * q.y - p.z * q.z,
    )
}
fn rotate_vector(rotation: DQuat, vector: DVec3) -> DVec3 {
    let axis = DVec3::new(rotation.x, rotation.y, rotation.z);
    let cross = axis.cross(vector);
    vector + (cross * rotation.w + axis.cross(cross)) * 2.0
}

pub fn normalize_angle(angle: f64) -> f64 {
    // Match the C++20 constexpr helper rather than rem_euclid's slightly different rounding.
    let value = angle - TAU * (angle / TAU).trunc();
    if value < 0.0 { value + TAU } else { value }
}

fn angle_between(a: DVec3, b: DVec3) -> f64 {
    a.normalize().dot(b.normalize()).clamp(-1.0, 1.0).acos()
}

pub fn solve_kepler(mean: f64, eccentricity: f64, hyperbolic: bool) -> f64 {
    if eccentricity.abs() < 1e-9 {
        return mean;
    }
    let mut anomaly = mean;
    for _ in 0..200 {
        let (value, derivative) = if hyperbolic {
            (
                eccentricity * anomaly.sinh() - anomaly,
                eccentricity * anomaly.cosh() - 1.0,
            )
        } else {
            (
                anomaly - eccentricity * anomaly.sin(),
                1.0 - eccentricity * anomaly.cos(),
            )
        };
        let correction = (mean - value) / derivative;
        anomaly += correction;
        if correction.abs() <= 1e-10 {
            break;
        }
    }
    anomaly
}

#[cfg(test)]
mod tests {
    use super::*;
    use serde_json::json;

    fn sample() -> Orbit {
        Orbit {
            semi_major_axis: 12000.0,
            eccentricity: 0.25,
            inclination: 0.3,
            ascending_node: 1.1,
            periapsis_argument: 0.7,
            mean_anomaly: 0.5,
            epoch: 123.0,
            mu: 398600.44188,
            ..Orbit::default()
        }
    }

    #[test]
    fn elliptical_state_conserves_energy_and_closes_after_a_period() {
        let orbit = sample();
        for time in [0.0, 600.0, 86400.0, -600.0] {
            let state = orbit.state_at(time);
            let energy = state.velocity.length_squared() / 2.0 - orbit.mu / state.position.length();
            assert!((energy + orbit.mu / (2.0 * orbit.semi_major_axis)).abs() < 1e-10);
            assert!(
                (orbit.state_at(time + orbit.period()).position - state.position).length() < 1e-7
            );
        }
    }

    #[test]
    fn state_and_impulse_round_trip_non_degenerate_orbits() -> Result<()> {
        let orbit = sample();
        let time = 4321.0;
        let state = orbit.state_at(time);
        let recovered = Orbit::from_state(state, orbit.mu, time)?;
        assert!((recovered.semi_major_axis - orbit.semi_major_axis).abs() < 1e-8);
        assert!((recovered.state_at(time).position - state.position).length() < 1e-7);
        let impulse = DVec3::new(0.01, 0.04, -0.02);
        let burned = orbit.apply_impulse(impulse, time)?;
        let expected = state.velocity
            + orbit.rotation() * DQuat::from_rotation_z(orbit.true_anomaly_at(time)) * impulse;
        assert!((burned.state_at(time).position - state.position).length() < 1e-7);
        assert!((burned.state_at(time).velocity - expected).length() < 1e-10);
        Ok(())
    }

    #[test]
    fn explicit_legacy_copies_reset_anomaly_but_assignment_preserves_it() {
        let mut orbit = sample();
        orbit.update(4321.0);
        assert_ne!(orbit.true_anomaly, orbit.mean_anomaly);
        assert_eq!(orbit.legacy_copy().true_anomaly, orbit.mean_anomaly);
        let assigned = orbit;
        assert_eq!(assigned.true_anomaly, orbit.true_anomaly);
        assert_eq!(orbit.legacy_copy().epoch, orbit.epoch);
    }

    #[test]
    fn tiny_nonzero_eccentricity_retains_the_legacy_apsis_frame() -> Result<()> {
        let orbit = Orbit {
            eccentricity: 5e-13,
            ..sample()
        };
        let recovered = Orbit::from_state(orbit.state_at(4321.0), orbit.mu, 4321.0)?;
        assert!(recovered.eccentricity > 0.0 && recovered.eccentricity < 1e-12);
        assert!((recovered.periapsis_argument - orbit.periapsis_argument).abs() < 0.01);
        assert_ne!(recovered.periapsis_argument, 0.0);
        Ok(())
    }

    #[test]
    fn circular_and_hyperbolic_states_remain_finite() -> Result<()> {
        let circle = Orbit {
            eccentricity: 0.0,
            inclination: 0.0,
            ..sample()
        };
        let state = circle.state_at(600.0);
        let recovered = Orbit::from_state(state, circle.mu, 600.0)?;
        assert!((recovered.state_at(600.0).position - state.position).length() < 1e-7);
        let escape = Orbit {
            semi_major_axis: -12000.0,
            eccentricity: 1.5,
            ..sample()
        };
        for time in [-1000.0, 123.0, 2000.0] {
            let state = escape.state_at(time);
            assert!(state.position.is_finite() && state.velocity.is_finite());
            let recovered = Orbit::from_state(state, escape.mu, time)?;
            assert!((recovered.state_at(time).position - state.position).length() < 1e-6);
        }
        Ok(())
    }

    #[test]
    fn loader_keeps_source_epoch_zero_and_rejects_invalid_conics() -> Result<()> {
        assert_eq!(
            Orbit::load(&json!({"semi_major_axis":0}))?.state_at(0.0),
            StateVector::default()
        );
        let orbit = Orbit::load(&json!({"semi_major_axis":"7000 km", "eccentricity":0.1,
            "inclination":"30 deg", "LAN":0, "arg_periapsis":0, "M0":90, "epoch":12345}))?;
        assert_eq!(orbit.epoch, 0.0);
        assert_eq!(orbit.true_anomaly, orbit.mean_anomaly);
        assert!((orbit.mean_anomaly - PI / 2.0).abs() < 1e-15);
        assert!(
            Orbit {
                eccentricity: 1.0,
                ..orbit
            }
            .validate()
            .is_err()
        );
        assert!(
            Orbit {
                semi_major_axis: -7000.0,
                ..orbit
            }
            .validate()
            .is_err()
        );
        assert!(Orbit::from_state(StateVector::default(), 1.0, 0.0).is_err());
        Ok(())
    }
}
