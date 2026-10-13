//! Native projections of the source entity and modifier tooltip text.
use crate::{
    catalog::Catalog,
    economy::industry::Industry,
    number_display::{fixed, float, human_float, human_unsigned},
    simulation::Settlement,
};
use serde::Serialize;

#[derive(Clone, Debug, PartialEq, Serialize)]
pub struct Line {
    pub text: String,
    pub color: [f32; 3],
}
impl Line {
    pub fn plain(text: impl Into<String>) -> Self {
        Self {
            text: text.into(),
            color: [1.0; 3],
        }
    }
}

impl Settlement {
    /// ProvinceTooltipProvider's two price-mode lines or four population/market lines.
    pub fn province_tooltip(&self, price: Option<f64>) -> Vec<Line> {
        let mut lines = vec![Line::plain(&self.name)];
        if let Some(price) = price {
            lines.push(Line::plain(format!("Price: {}", fixed(price, 2))));
            return lines;
        }
        let mut population = 0_u64;
        let mut sol = 0.0;
        for segment in &self.populations {
            population = population.wrapping_add(segment.size);
            sol += segment.standard_of_living * segment.size as f64;
        }
        lines.extend(
            [
                format!("GDP: {}", human_float(self.market.gdp)),
                format!("Population: {}", human_unsigned(population)),
                // The source does not replace empty-population 0/0 with a fake zero.
                format!("Standard of Living: {}", float(sol / population as f64)),
            ]
            .map(Line::plain),
        );
        lines
    }
}

impl Industry {
    pub fn tooltip(&self, catalog: &Catalog) -> Vec<Line> {
        let name = format!("{} Factory", catalog.recipes[self.recipe.0].name);
        let gain = self.expertise_gain.value();
        let mut lines = vec![
            name.clone(),
            name,
            format!("Wallet: {}", human_float(self.wallet.balance)),
            format!("Size: {}", float(self.size)),
            format!("Utilization: {}", float(self.utilization)),
            format!("Last Tick Difference: {}", float(self.diff)),
            format!("Material Cost: {}", human_float(self.material_costs)),
            format!("Wage Cost: {}", human_float(self.wage_cost)),
            format!(
                "Power consumption: {} W",
                human_float(self.power_consumption)
            ),
            format!("Construction Cost: {}", human_float(self.construction_cost)),
            format!("Maintenance Cost: {}", human_float(self.maintenance)),
            format!("Transport Costs: {}", human_float(self.transport)),
            format!("State: {}", self.state.name()),
            format!("Taxes: {}", human_float(self.tax_cost)),
            format!("Continous Gains: {}", self.continuous_gains),
            format!("Throughput: {}", float(self.throughput)),
        ]
        .into_iter()
        .map(Line::plain)
        .collect::<Vec<_>>();
        lines.push(Line {
            text: format!(
                "Expertise: {} ({}{} per day)",
                float(self.expertise),
                if gain > 0.0 { "+" } else { "" },
                float(gain)
            ),
            color: if gain > 0.0 {
                [0.0, 1.0, 0.0]
            } else if gain < 0.0 {
                [1.0, 0.0, 0.0]
            } else {
                [1.0; 3]
            },
        });
        lines.extend(
            [
                format!("Max Expertise: {}", float(self.max_expertise.value())),
                format!(
                    "Output Subsidy: {}% ({})",
                    float(self.output_subsidy * 100.0),
                    human_float(self.subsidy_amount)
                ),
                format!("Profit: {}", human_float(self.profit)),
                format!("Revenue: {}", human_float(self.revenue)),
            ]
            .map(Line::plain),
        );
        lines
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn province_tooltip_uses_population_weights_and_price_mode_replaces_summary() {
        let data = crate::default_data_path();
        let world = crate::World::load(&data).unwrap();
        let mut simulation =
            crate::simulation::Simulation::load_with_amenability_seed(&data, &world, 42, 7)
                .unwrap();
        let site = simulation
            .settlements
            .iter_mut()
            .find(|site| !site.populations.is_empty())
            .unwrap();
        site.populations.truncate(1);
        let second = crate::economy::population::Population::load(
            serde_json::from_value(serde_json::json!({"size":300,"sol":6.0})).unwrap(),
            &simulation.catalog,
        )
        .unwrap();
        site.populations[0].size = 100;
        site.populations[0].standard_of_living = 2.0;
        site.populations.push(second);
        site.market.gdp = 1235.0;
        let lines = site.province_tooltip(None);
        assert_eq!(lines.len(), 4);
        assert_eq!(lines[1].text, "GDP: 1.24 k");
        assert_eq!(lines[2].text, "Population: 400");
        assert_eq!(lines[3].text, "Standard of Living: 5");
        assert_eq!(site.province_tooltip(Some(12.345))[1].text, "Price: 12.35");
        assert_eq!(site.province_tooltip(Some(f64::NAN))[1].text, "Price: nan");
        assert_eq!(site.province_tooltip(Some(0.0)).len(), 2);
        site.populations.clear();
        assert_eq!(
            site.province_tooltip(None)[3].text,
            "Standard of Living: nan"
        );
    }
    #[test]
    fn factory_details_keep_source_labels_raw_values_and_gain_signs() {
        let catalog = Catalog::load(&crate::default_data_path()).unwrap();
        let mut industry = Industry::new(catalog.recipe("farming").unwrap(), 1235).unwrap();
        industry.transport = 1250.125;
        industry.throughput = 0.125;
        industry.wallet.balance = -1235.125;
        for (gain, expected, color) in [
            (0.0001, "+0.0001", [0.0, 1.0, 0.0]),
            (-0.0001, "-0.0001", [1.0, 0.0, 0.0]),
            (-0.0, "-0", [1.0; 3]),
        ] {
            industry.expertise_gain.set_base(gain);
            let lines = industry.tooltip(&catalog);
            assert_eq!(lines.len(), 21);
            assert_eq!(lines[2].text, "Wallet: -1.24 k");
            assert_eq!(lines[3].text, "Size: 1235");
            assert_eq!(lines[11].text, "Transport Costs: 1.25 k");
            assert_eq!(lines[12].text, "State: Steady State");
            assert_eq!(lines[15].text, "Throughput: 0.125");
            assert_eq!(lines[16].text, format!("Expertise: 1 ({expected} per day)"));
            assert_eq!(lines[16].color, color);
        }
    }
}
