//! Bundled-asset units, following `src/core/loading/loadutil.cpp`.
use anyhow::{Context, Result, bail, ensure};
use serde_json::Value;

pub const KM_PER_AU: f64 = 149_597_870.7;
pub const GRAVITATIONAL_CONSTANT_KM: f64 = 6.6743015e-20;
pub const SUN_MU: f64 = 1.32712400188e11;

#[derive(Clone, Copy)]
pub enum Unit {
    Distance,
    Angle,
    Mass,
    Volume,
    Power,
    Time,
}

pub fn read_unit(value: &Value, unit: Unit) -> Result<f64> {
    let text = match value {
        Value::String(text) => text.clone(),
        Value::Number(value) => value.to_string(),
        _ => bail!("Invalid quantity {value}"),
    };
    // Longest numeric prefix also handles exponents and compact units (e.g. 5MW).
    let (number, suffix) = (1..=text.len())
        .rev()
        .filter(|&end| text.is_char_boundary(end))
        .find_map(|end| {
            text[..end]
                .trim()
                .parse::<f64>()
                .ok()
                .map(|number| (number, text[end..].trim()))
        })
        .with_context(|| format!("Invalid quantity {text}"))?;
    let multiplier = match (unit, suffix) {
        (Unit::Distance, "" | "km")
        | (Unit::Angle, "rad")
        | (Unit::Mass, "" | "kg")
        | (Unit::Volume, "" | "m3")
        | (Unit::Power, "w" | "W")
        | (Unit::Time, "s") => 1.0,
        (Unit::Distance, "AU" | "au") => KM_PER_AU,
        (Unit::Distance, "m") | (Unit::Mass, "g") => 0.001,
        (Unit::Angle, "" | "deg") => std::f64::consts::PI / 180.0,
        (Unit::Mass, "t") => 1000.0,
        (Unit::Power, "kW") => 1e3,
        (Unit::Power, "MW") => 1e6,
        (Unit::Power, "GW") => 1e9,
        (Unit::Power, "TW") => 1e12,
        // This unusual months-to-minutes mapping is deliberate legacy compatibility.
        (Unit::Time, "m" | "months" | "month") => 60.0,
        (Unit::Time, "h" | "hours" | "hour") => 3600.0,
        (Unit::Time, "d" | "days" | "day") => 86400.0,
        (Unit::Time, "w" | "weeks" | "week") => 604800.0,
        (Unit::Time, "y" | "years" | "year") => 31536000.0,
        _ => bail!("Unsupported unit in {text}"),
    };
    let quantity = number * multiplier;
    ensure!(quantity.is_finite(), "Non-finite quantity {text}");
    Ok(quantity)
}

#[cfg(test)]
mod tests {
    use super::*;
    use serde_json::json;

    #[test]
    fn orbital_units_include_defaults_meters_and_retrograde_days() -> Result<()> {
        assert_eq!(read_unit(&json!("1 AU"), Unit::Distance)?, KM_PER_AU);
        assert_eq!(
            read_unit(&json!("6795667.972420554 m"), Unit::Distance)?,
            6795.667972420554
        );
        assert_eq!(read_unit(&json!(180), Unit::Angle)?, std::f64::consts::PI);
        assert_eq!(read_unit(&json!("2rad"), Unit::Angle)?, 2.0);
        assert_eq!(read_unit(&json!("-2 d"), Unit::Time)?, -172800.0);
        assert!(read_unit(&json!("1 cm"), Unit::Distance).is_err());
        assert!(read_unit(&json!("1e308 AU"), Unit::Distance).is_err());
        Ok(())
    }
}
