//! Data projection of the original global ProductionSummary's nine columns.
use super::industry::{Industry, IndustryState};
use crate::{
    catalog::Catalog,
    number_display::{float, human_float, human_integer},
};

pub const COLUMNS: [&str; 9] = [
    "Production Type",
    "Size",
    "Utilization",
    "Utilization Delta",
    "Cash Reserves",
    "Hired Workers",
    "Wage",
    "Revenue",
    "Profit",
];

pub struct ProductionSummary {
    pub cells: [String; 9],
    pub colors: [[f32; 3]; 9],
    pub construction: Option<f32>,
    pub shortage: bool,
    pub items_sold: String,
}

impl Industry {
    pub fn summary(&self, catalog: &Catalog) -> ProductionSummary {
        use IndustryState::*;
        let state_color = match self.state {
            Steady => [1.0, 0.0, 0.0],
            Maximum => [0.0, 1.0, 0.0],
            Minimum => [0.0, 0.0, 1.0],
            Construction => [1.0, 1.0, 0.0],
            Demolishing => [1.0, 0.0, 1.0],
            Shrinking => [0.0, 1.0, 1.0],
            Expanding => [1.0, 1.0, 1.0],
            Shortage => [1.0, 0.5, 0.0],
        };
        let delta = (self.diff - 1.0) * 100.0;
        let mut colors = [[1.0; 3]; 9];
        colors[0] = state_color;
        if self.shortage {
            colors[1] = [0.75, 0.0, 0.0];
        }
        colors[3] = if delta > 0.0 {
            [0.0, 0.75, 0.0]
        } else if delta < 0.0 {
            [0.75, 0.0, 0.0]
        } else {
            [1.0; 3]
        };
        let construction = self
            .construction
            .as_ref()
            .map(|job| job.progress as f32 / job.maximum as f32);
        ProductionSummary {
            cells: [
                format!("{} Factory", catalog.recipes[self.recipe.0].name),
                human_integer(self.size as i64),
                construction.map_or_else(
                    || human_integer(self.utilization as i64),
                    |progress| format!("{:.0}%", progress.clamp(0.0, 1.0) * 100.0 + 0.01),
                ),
                format!("{delta:.2}%"),
                human_float(self.wallet.balance),
                format!(
                    "{}/{}",
                    human_integer(self.employer.population_change as i64),
                    human_integer(self.employer.population_fulfilled as i64)
                ),
                human_integer(self.wage_cost as i64),
                human_integer(self.revenue as i64),
                human_integer(self.profit as i64),
            ],
            colors,
            construction,
            shortage: self.shortage,
            items_sold: format!("Items sold: {}", float(self.amount_sold)),
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn summary_keeps_component_progress_and_employment_separate_from_state_and_demand() {
        let catalog = Catalog::load(&crate::default_data_path()).unwrap();
        let mut industry = Industry::new(catalog.recipe("farming").unwrap(), 1235).unwrap();
        industry.state = IndustryState::Shortage;
        industry.shortage = true;
        industry.construction = Some(super::super::industry::Construction {
            progress: 1,
            maximum: 3,
            levels: 2,
        });
        industry.workers = vec![(super::super::GoodId(0), 10000.0)];
        industry.diff = 1.125;
        industry.wallet.balance = -1235.125;
        industry.wage_cost = 1000.99;
        industry.revenue = 999.99;
        industry.profit = -1235.99;
        let summary = industry.summary(&catalog);
        assert_eq!(summary.cells[1], "1.24 k");
        assert_eq!(summary.cells[2], "33%");
        assert_eq!(summary.construction, Some(1.0 / 3.0));
        assert_eq!(summary.cells[3], "12.50%");
        assert_eq!(summary.cells[4], "-1.24 k");
        assert_eq!(summary.cells[5], "0/0");
        assert_eq!(summary.cells[6..], ["1 k", "999", "-1.24 k"]);
        assert_eq!(summary.colors[0], [1.0, 0.5, 0.0]);
        assert_eq!(summary.colors[1], [0.75, 0.0, 0.0]);
        assert_eq!(summary.colors[3], [0.0, 0.75, 0.0]);
    }
}
