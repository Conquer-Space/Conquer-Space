//! The functional body/province search, excluding cities and satellite-only entities.
use crate::{simulation::Simulation, solar_system::BodyId};

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum SearchTarget {
    Body(BodyId),
    Province(usize),
}
impl SearchTarget {
    pub fn name(self, simulation: &Simulation) -> &str {
        match self {
            Self::Body(id) => &simulation.solar_system.bodies[id.0].name,
            Self::Province(id) => &simulation.settlements[id].name,
        }
    }
    pub fn body(self, simulation: &Simulation) -> BodyId {
        match self {
            Self::Body(id) => id,
            Self::Province(id) => simulation.settlements[id].body,
        }
    }
}
impl Simulation {
    pub fn search(&self, query: &str) -> Vec<SearchTarget> {
        if query.is_empty() {
            return Vec::new();
        }
        let query = query.to_ascii_lowercase();
        (0..self.solar_system.bodies.len())
            .map(|id| SearchTarget::Body(BodyId(id)))
            .chain(
                self.settlements
                    .iter()
                    .enumerate()
                    .filter(|(_, site)| site.searchable())
                    .map(|(id, _)| SearchTarget::Province(id)),
            )
            .filter(|target| target.name(self).to_ascii_lowercase().contains(&query))
            .collect()
    }
}
#[cfg(test)]
mod tests {
    use super::*;
    use crate::{World, default_data_path};
    #[test]
    fn search_reaches_named_natural_bodies_and_provinces_but_not_identifier_fallbacks_cities_or_spacecraft()
    -> anyhow::Result<()> {
        let data = default_data_path();
        let simulation = Simulation::load(&data, &World::load(&data)?, 42)?;
        assert!(simulation.search("").is_empty());
        assert!(simulation.search("not-a-loaded-name").is_empty());
        for target in (0..simulation.solar_system.bodies.len())
            .map(|id| SearchTarget::Body(BodyId(id)))
            .chain(
                simulation
                    .settlements
                    .iter()
                    .enumerate()
                    .filter(|(_, site)| site.searchable())
                    .map(|(id, _)| SearchTarget::Province(id)),
            )
        {
            assert!(
                simulation
                    .search(&target.name(&simulation).to_ascii_uppercase())
                    .contains(&target)
            );
        }
        assert!(simulation.search("Kennedy Space Center").is_empty());
        assert!(simulation.search("NAVSTAR").is_empty());
        for (index, site) in simulation
            .settlements
            .iter()
            .enumerate()
            .filter(|(_, site)| site.is_province() && !site.named)
        {
            assert!(
                !simulation
                    .search(&site.name)
                    .contains(&SearchTarget::Province(index))
            );
        }
        for result in simulation.search("a") {
            if let SearchTarget::Province(id) = result {
                assert!(simulation.settlements[id].is_province());
            }
        }
        Ok(())
    }
}
