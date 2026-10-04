//! Original fixed-offset time-zone definitions, not an IANA/DST calendar.
use crate::clock::StarDate;
use anyhow::{Context, Result, ensure};
use serde_json::Value;
use std::{collections::HashMap, fs, path::Path};

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct TimeZoneId(pub usize);

#[derive(Debug)]
pub struct TimeZone {
    pub identifier: String,
    pub name: Option<String>,
    pub offset: f64,
}
impl TimeZone {
    /// Preserve GetHour's signed remainder/truncation, including fractional offsets.
    pub fn local_time(&self, date: StarDate) -> (i32, u64) {
        (date.hour(self.offset), date.minute())
    }
}

pub struct TimeZones {
    pub entries: Vec<TimeZone>,
    pub diagnostics: Vec<String>,
    ids: HashMap<String, TimeZoneId>,
}
impl TimeZones {
    pub fn load(data: &Path) -> Result<Self> {
        let path = data.join("data/timezones.hjson");
        let entries = serde_hjson::from_str(&fs::read_to_string(&path)?)
            .with_context(|| format!("Parsing {}", path.display()))?;
        Self::from_entries(entries)
    }
    pub fn id(&self, identifier: &str) -> Option<TimeZoneId> {
        self.ids.get(identifier).copied()
    }
    pub fn from_entries(entries: Vec<Value>) -> Result<Self> {
        let mut zones = Self {
            entries: Vec::new(),
            diagnostics: Vec::new(),
            ids: HashMap::new(),
        };
        for entry in entries {
            let Some(identifier) = entry["identifier"].as_str() else {
                zones
                    .diagnostics
                    .push("Skipped time zone without a string identifier".into());
                continue;
            };
            let offset = match &entry["offset"] {
                Value::Null => 0.0,
                Value::Bool(value) => f64::from(*value),
                Value::Number(value) => value.as_f64().context("Invalid time-zone offset")?,
                Value::String(value) => value.parse().unwrap_or(0.0),
                _ => anyhow::bail!("{identifier}: invalid time-zone offset type"),
            };
            ensure!(
                offset.is_finite(),
                "{identifier}: non-finite time-zone offset"
            );
            if zones
                .ids
                .insert(identifier.into(), TimeZoneId(zones.entries.len()))
                .is_some()
            {
                zones.diagnostics.push(format!(
                    "Duplicate time zone {identifier}: last lookup wins"
                ));
            }
            zones.entries.push(TimeZone {
                identifier: identifier.into(),
                name: entry["name"].as_str().map(str::to_owned),
                offset,
            });
        }
        Ok(zones)
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::clock::HOUR;
    use serde_json::json;

    #[test]
    fn offsets_keep_source_conversion_and_duplicate_lookup() -> Result<()> {
        let zones = TimeZones::from_entries(vec![
            json!({"identifier":"same", "name":"First", "offset": -11}),
            json!({"identifier":"same", "name":"Last", "offset": "5.5"}),
            json!({"identifier":"missing"}),
            json!({"identifier":"invalid", "offset":"not a number"}),
            json!({"identifier":"boolean", "offset":true}),
            json!({"name":"No identifier"}),
        ])?;
        assert_eq!(zones.entries.len(), 5);
        assert_eq!(zones.id("same"), Some(TimeZoneId(1)));
        assert_eq!(zones.entries[1].name.as_deref(), Some("Last"));
        assert_eq!(zones.entries[1].offset, 5.5);
        assert_eq!(zones.entries[2].offset, 0.0);
        assert_eq!(zones.entries[3].offset, 0.0);
        assert_eq!(zones.entries[4].offset, 1.0);
        assert_eq!(zones.diagnostics.len(), 2);
        assert!(zones.id("unknown").is_none());
        assert!(TimeZones::from_entries(vec![json!({"identifier":"bad", "offset":[]})]).is_err());
        Ok(())
    }
    #[test]
    fn local_time_retains_negative_hours_and_does_not_shift_minutes() -> Result<()> {
        let zones = TimeZones::from_entries(vec![
            json!({"identifier":"negative", "offset":-3.5}),
            json!({"identifier":"fractional", "offset":5.5}),
        ])?;
        assert_eq!(
            zones.entries[0].local_time(StarDate::from_tick(1)),
            (-3, 10)
        );
        assert_eq!(
            zones.entries[1].local_time(StarDate::from_tick(23 * HOUR + 5)),
            (4, 50)
        );
        Ok(())
    }
}
