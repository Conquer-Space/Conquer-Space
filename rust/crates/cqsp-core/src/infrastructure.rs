//! Original static power/grid definitions and province infrastructure bindings.
use crate::{
    catalog::Catalog,
    economy::GoodId,
    read_entries,
    units::{Unit, read_unit},
};
use anyhow::{Context, Result, ensure};
use serde::Deserialize;
use serde_json::Value;
use std::{collections::BTreeMap, path::Path};

#[derive(Debug)]
pub struct PowerPlantRecipe {
    pub name: String,
    pub maximum_production: f64,
    pub consumption: Vec<(GoodId, f64)>,
}
#[derive(Debug)]
pub struct PowerGrid {
    pub name: String,
}
pub struct InfrastructureCatalog {
    pub power_plants: BTreeMap<String, PowerPlantRecipe>,
    pub grids: BTreeMap<String, PowerGrid>,
    pub diagnostics: Vec<String>,
}
#[derive(Deserialize)]
pub struct InfrastructureDefinitions {
    power_plants: Vec<PowerPlantEntry>,
    grids: Vec<GridEntry>,
}
#[derive(Deserialize)]
struct PowerPlantEntry {
    identifier: String,
    name: Option<String>,
    output: Value,
    #[serde(default)]
    input: BTreeMap<String, f64>,
}
#[derive(Deserialize)]
struct GridEntry {
    identifier: String,
    name: Option<String>,
}
impl InfrastructureCatalog {
    pub fn load(data: &Path, catalog: &Catalog) -> Result<Self> {
        Self::from_definitions(
            InfrastructureDefinitions {
                power_plants: read_entries(&data.join("data/infrastructure/powerplant"))?,
                grids: read_entries(&data.join("data/infrastructure/powergrid"))?,
            },
            catalog,
        )
    }
    pub fn from_definitions(
        definitions: InfrastructureDefinitions,
        catalog: &Catalog,
    ) -> Result<Self> {
        let mut power_plants = BTreeMap::new();
        let mut diagnostics = Vec::new();
        for entry in definitions.power_plants {
            let maximum_production = read_unit(&entry.output, Unit::Power)?;
            let mut consumption: Vec<_> = entry
                .input
                .into_iter()
                .filter_map(|(good, amount)| {
                    if let Some(good) = catalog.good(&good) {
                        Some((good, amount))
                    } else {
                        diagnostics.push(format!(
                            "{}: unknown power-plant input {good}; skipped like the C++ loader",
                            entry.identifier
                        ));
                        None
                    }
                })
                .collect();
            consumption.sort_by_key(|&(good, _)| good);
            power_plants.insert(
                entry.identifier.clone(),
                PowerPlantRecipe {
                    name: entry.name.unwrap_or(entry.identifier),
                    maximum_production,
                    consumption,
                },
            );
        }
        let grids = definitions
            .grids
            .into_iter()
            .map(|entry| {
                (
                    entry.identifier.clone(),
                    PowerGrid {
                        name: entry.name.unwrap_or(entry.identifier),
                    },
                )
            })
            .collect();
        Ok(Self {
            power_plants,
            grids,
            diagnostics,
        })
    }
}

#[derive(Debug, Default)]
pub struct PowerPlant {
    pub production: f64,
    pub maximum_production: f64,
    pub consumption: Vec<(GoodId, f64)>,
}
#[derive(Debug)]
pub struct SurfaceInfrastructure {
    pub purchase_cost: f64,
    pub improvement: f64,
    pub power_plants: Vec<PowerPlant>,
    pub power_grid: Option<String>,
}
#[derive(Default, Deserialize)]
#[serde(default)]
struct SurfaceEntry {
    power_plants: Vec<PlantEntry>,
    power_grid: Option<String>,
}
#[derive(Deserialize)]
struct PlantEntry {
    #[serde(rename = "type")]
    kind: String,
    #[serde(default)]
    size: f64,
}
impl SurfaceInfrastructure {
    pub fn load(
        transport: Option<f64>,
        value: Option<Value>,
        catalog: &InfrastructureCatalog,
    ) -> Result<Self> {
        let mut infrastructure = Self {
            purchase_cost: transport.unwrap_or(100.0),
            improvement: 0.0,
            power_plants: Vec::new(),
            power_grid: None,
        };
        ensure!(
            infrastructure.purchase_cost.is_finite(),
            "Non-finite transport cost"
        );
        let Some(value) = value.filter(|value| {
            !value.is_null() && !value.as_object().is_some_and(|map| map.is_empty())
        }) else {
            return Ok(infrastructure);
        };
        let entry: SurfaceEntry = serde_json::from_value(value)?;
        for plant in entry.power_plants {
            let recipe = catalog
                .power_plants
                .get(&plant.kind)
                .with_context(|| format!("Unknown power-plant type {}", plant.kind))?;
            let maximum_production = recipe.maximum_production * plant.size;
            ensure!(
                maximum_production.is_finite(),
                "Non-finite power-plant capacity"
            );
            // Original ProvinceLoader copies capacity only, not recipe consumption.
            infrastructure.power_plants.push(PowerPlant {
                maximum_production,
                ..PowerPlant::default()
            });
        }
        let grid = entry
            .power_grid
            .filter(|name| !name.is_empty())
            .unwrap_or("usa".into());
        infrastructure.power_grid = catalog.grids.contains_key(&grid).then_some(grid);
        Ok(infrastructure)
    }
}

#[derive(Clone, Copy, Debug, Default, PartialEq, Eq)]
pub struct ZoningAllocation {
    pub allocated: u32,
    pub filled: u32,
}
impl ZoningAllocation {
    pub fn load(values: BTreeMap<String, i64>, catalog: &Catalog) -> BTreeMap<String, Self> {
        values
            .into_iter()
            .filter(|(zone, _)| catalog.zoning.contains_key(zone))
            .map(|(zone, amount)| {
                (
                    zone,
                    Self {
                        allocated: amount as u32,
                        filled: 0,
                    },
                )
            })
            .collect()
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::default_data_path;
    use serde_json::json;
    #[test]
    fn original_templates_bind_capacity_without_copying_fuel_or_frequency() -> Result<()> {
        let data = default_data_path();
        let catalog = Catalog::load(&data)?;
        let infrastructure = InfrastructureCatalog::load(&data, &catalog)?;
        assert_eq!(infrastructure.power_plants.len(), 1);
        assert_eq!(infrastructure.grids.len(), 1);
        let recipe = &infrastructure.power_plants["coal_power_plant"];
        assert_eq!(recipe.maximum_production, 1e8);
        assert_eq!(
            recipe.consumption,
            [(catalog.good("coal").unwrap(), 10000.0)]
        );
        for value in [None, Some(json!({}))] {
            let province = SurfaceInfrastructure::load(None, value, &infrastructure)?;
            assert_eq!(province.purchase_cost, 100.0);
            assert!(province.power_grid.is_none());
        }
        let province = SurfaceInfrastructure::load(
            Some(0.05),
            Some(json!({"power_plants":[{"type":"coal_power_plant","size":2.5}]})),
            &infrastructure,
        )?;
        assert_eq!(province.purchase_cost, 0.05);
        assert_eq!(province.power_grid.as_deref(), Some("usa"));
        assert_eq!(province.power_plants[0].maximum_production, 2.5e8);
        assert_eq!(province.power_plants[0].production, 0.0);
        assert!(province.power_plants[0].consumption.is_empty());
        assert!(
            SurfaceInfrastructure::load(
                None,
                Some(json!({"power_plants":[{"type":"missing","size":1}]})),
                &infrastructure
            )
            .is_err()
        );
        let zones = ZoningAllocation::load(
            BTreeMap::from([("undeveloped".into(), -1), ("missing".into(), 5)]),
            &catalog,
        );
        assert_eq!(zones.len(), 1);
        assert_eq!(
            zones["undeveloped"],
            ZoningAllocation {
                allocated: u32::MAX,
                filled: 0
            }
        );
        Ok(())
    }
}
