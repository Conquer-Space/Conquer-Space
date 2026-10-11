//! Native port of `SysProduction`; random samples are injected for differential tests.
use super::{GoodId, Market, ProductionConfig, Wallet};
use crate::{
    catalog::{Catalog, Recipe, RecipeId},
    clock::DAY,
    modifiers::{ModifiableValue, Modifier, ModifierId, ModifierOp, ModifierTarget},
};
use anyhow::{Result, ensure};
use serde::Deserialize;
use serde_json::Value;

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum IndustryState {
    Steady,
    Maximum,
    Minimum,
    Construction,
    Demolishing,
    Shrinking,
    Expanding,
    Shortage,
}

impl IndustryState {
    pub fn name(self) -> &'static str {
        match self {
            Self::Steady => "Steady State",
            Self::Maximum => "Maximum Production",
            Self::Minimum => "Minimum Production",
            Self::Construction => "Construction",
            Self::Demolishing => "Demolishing",
            Self::Shrinking => "Shrinking",
            Self::Expanding => "Expanding",
            Self::Shortage => "Shortage",
        }
    }
    fn expertise_gain(self) -> f64 {
        match self {
            Self::Steady | Self::Maximum | Self::Minimum | Self::Shortage => 0.0001,
            Self::Construction => 0.0,
            Self::Demolishing | Self::Shrinking | Self::Expanding => -0.0001,
        }
    }
}

#[derive(Debug)]
pub struct Construction {
    pub progress: i32,
    pub maximum: i32,
    pub levels: i32,
}

/// Separate from labor-hour demand. The original production system leaves these
/// value-initialized employer counts unchanged.
#[derive(Debug, Default)]
pub struct Employer {
    pub population_needed: i32,
    pub population_fulfilled: i32,
    pub population_change: i32,
}

#[derive(Debug)]
pub struct ConstructionSector {
    pub capacity: u32,
    pub current: u32,
    pub cost: f64,
}
impl Default for ConstructionSector {
    fn default() -> Self {
        Self {
            capacity: 1000,
            current: 0,
            cost: 1000.0,
        }
    }
}
impl ConstructionSector {
    pub fn purchase_labor(&mut self, market: &mut Market, default_labor: GoodId) {
        if self.capacity == 0 {
            return;
        }
        let hours = self.capacity as f64 * (40.0 / 7.0) * 1000.0;
        let (cost, tax) = market.purchase(&[(default_labor, hours)]);
        self.cost = (cost + tax) / self.capacity as f64;
    }
}

#[derive(Debug)]
pub struct Industry {
    pub recipe: RecipeId,
    pub size: f64,
    pub utilization: f64,
    pub diff: f64,
    pub state: IndustryState,
    pub shortage: bool,
    pub cumulative_profit_ratio: f64,
    pub continuous_gains: i32,
    pub stability: i32,
    pub expertise: f64,
    pub expertise_gain: ModifiableValue,
    pub max_expertise: ModifiableValue,
    pub revenue: f64,
    pub material_costs: f64,
    pub maintenance: f64,
    pub wage_cost: f64,
    pub tax_cost: f64,
    pub construction_cost: f64,
    pub profit: f64,
    pub power_consumption: f64,
    pub transport: f64,
    pub throughput: f64,
    pub output_subsidy: f64,
    pub subsidy_amount: f64,
    pub amount_sold: f64,
    pub workers: Vec<(GoodId, f64)>,
    pub employer: Employer,
    pub wallet: Wallet,
    pub construction: Option<Construction>,
}

impl Industry {
    pub fn new(recipe: RecipeId, size: i32) -> Result<Self> {
        ensure!(size >= 1, "Industry capacity must be at least one");
        Ok(Self {
            recipe,
            size: size as f64,
            utilization: size as f64,
            diff: 0.0,
            state: IndustryState::Steady,
            shortage: false,
            cumulative_profit_ratio: 0.0,
            continuous_gains: 0,
            stability: 0,
            expertise: 1.0,
            expertise_gain: ModifiableValue::new(0.0001, ModifierOp::Multiplicative),
            max_expertise: ModifiableValue::new(1.5, ModifierOp::Additive),
            revenue: 0.0,
            material_costs: 0.0,
            maintenance: 0.0,
            wage_cost: 0.0,
            tax_cost: 0.0,
            construction_cost: 0.0,
            profit: 0.0,
            power_consumption: 0.0,
            transport: 0.0,
            throughput: 1.0,
            output_subsidy: 0.0,
            subsidy_amount: 0.0,
            amount_sold: 0.0,
            workers: Vec::new(),
            employer: Employer::default(),
            wallet: Wallet::default(),
            construction: None,
        })
    }

    pub fn apply_modifier(&mut self, id: ModifierId, modifier: &Modifier) {
        self.modifier_value(modifier.target)
            .apply(id, modifier.amount);
    }
    pub fn remove_modifier(&mut self, id: ModifierId, modifier: &Modifier) -> bool {
        self.modifier_value(modifier.target).remove(id)
    }
    fn modifier_value(&mut self, target: ModifierTarget) -> &mut ModifiableValue {
        match target {
            ModifierTarget::ExpertiseGain => &mut self.expertise_gain,
            ModifierTarget::MaxExpertise => &mut self.max_expertise,
        }
    }

    pub fn profit_margin(&self) -> f64 {
        self.profit / (self.revenue + 0.0001)
    }

    pub fn advance_state(
        &mut self,
        recipe: &Recipe,
        config: ProductionConfig,
        mut normal: impl FnMut(f64, f64) -> f64,
    ) {
        const SUSTAINED: i32 = 30 * DAY as i32;
        self.cumulative_profit_ratio += self.profit_margin();
        if self.cumulative_profit_ratio > 5.0 {
            self.continuous_gains += DAY as i32;
        } else if self.cumulative_profit_ratio < -5.0 {
            self.continuous_gains -= DAY as i32;
        } else {
            self.stability += DAY as i32;
            self.continuous_gains = 0;
        }
        if self.shortage {
            self.state = IndustryState::Shortage;
        }
        let state = match self.state {
            IndustryState::Steady => {
                if self.continuous_gains < -SUSTAINED {
                    IndustryState::Shrinking
                } else if self.continuous_gains > SUSTAINED {
                    IndustryState::Expanding
                } else {
                    self.diff = 1.0 + normal(0.0, 0.0005);
                    self.utilization = (self.utilization * self.diff)
                        .clamp(config.factory_min_utilization * self.size, self.size)
                        .max(1.0);
                    IndustryState::Steady
                }
            }
            IndustryState::Maximum => {
                if self.continuous_gains > SUSTAINED {
                    self.construction = Some(Construction {
                        progress: 0,
                        maximum: recipe.construction_ticks * DAY as i32,
                        levels: (0.25 * self.size * self.profit_margin()) as i32,
                    });
                    IndustryState::Construction
                } else if self.continuous_gains < -SUSTAINED {
                    IndustryState::Shrinking
                } else {
                    IndustryState::Maximum
                }
            }
            IndustryState::Minimum => {
                if self.continuous_gains < -SUSTAINED {
                    IndustryState::Demolishing
                } else if self.continuous_gains > SUSTAINED {
                    IndustryState::Expanding
                } else {
                    IndustryState::Minimum
                }
            }
            IndustryState::Construction => {
                let construction = self
                    .construction
                    .as_ref()
                    .expect("Construction state requires a job");
                if construction.progress >= construction.maximum {
                    self.size += construction.levels as f64;
                    self.construction = None;
                    IndustryState::Expanding
                } else {
                    IndustryState::Construction
                }
            }
            IndustryState::Demolishing => {
                let delta = (0.25 * self.size * self.profit_margin()).clamp(-self.size * 0.25, 0.0);
                self.size = (self.size + delta).max(1.0);
                IndustryState::Shrinking
            }
            IndustryState::Shrinking => {
                let minimum = config.minimum_utilization(self.size);
                if self.continuous_gains > SUSTAINED {
                    IndustryState::Expanding
                } else if self.stability > 5 * DAY as i32 {
                    if self.utilization == minimum {
                        IndustryState::Minimum
                    } else {
                        IndustryState::Steady
                    }
                } else if self.continuous_gains < -SUSTAINED && self.utilization == minimum {
                    IndustryState::Minimum
                } else {
                    self.diff =
                        (config.utilization_diff(self.profit) + normal(0.0, 0.0005)).min(1.0);
                    self.utilization = (self.utilization * self.diff).clamp(minimum, self.size);
                    IndustryState::Shrinking
                }
            }
            IndustryState::Expanding => {
                if self.continuous_gains < -SUSTAINED {
                    IndustryState::Shrinking
                } else if self.stability > 5 * DAY as i32 {
                    if self.utilization >= self.size - 1.0 {
                        IndustryState::Maximum
                    } else {
                        IndustryState::Steady
                    }
                } else if self.continuous_gains > SUSTAINED && self.utilization >= self.size - 1.0 {
                    IndustryState::Maximum
                } else {
                    self.diff =
                        (config.utilization_diff(self.profit) + normal(0.0, 0.0005)).max(1.0);
                    self.utilization = (self.utilization * self.diff)
                        .clamp(config.minimum_utilization(self.size), self.size);
                    IndustryState::Expanding
                }
            }
            IndustryState::Shortage => {
                if !self.shortage {
                    IndustryState::Steady
                } else {
                    self.diff = normal(0.1, 0.1).max(0.02);
                    self.utilization = (self.utilization * self.diff).clamp(1.0, self.size);
                    IndustryState::Shortage
                }
            }
        };
        if state != self.state {
            self.continuous_gains = 0;
            self.cumulative_profit_ratio = 0.0;
            self.stability = 0;
        }
        self.state = state;
    }

    pub fn produce(
        &mut self,
        catalog: &Catalog,
        market: &mut Market,
        sector: &mut ConstructionSector,
    ) -> f64 {
        let recipe = &catalog.recipes[self.recipe.0];
        self.construction_cost = 0.0;
        self.tax_cost = 0.0;
        let mut tax_income = 0.0;
        if self.state == IndustryState::Construction {
            let construction = self
                .construction
                .as_mut()
                .expect("Construction state requires a job");
            let mut amount = construction.levels as f64 * DAY as f64;
            let costs = scaled(&recipe.construction_cost, construction.levels as f64);
            if costs
                .iter()
                .any(|&(good, _)| market.chronic_shortages[good] > 5.0)
            {
                amount *= 0.1;
            }
            construction.progress = (construction.progress as f64 + amount) as i32;
            sector.current = (sector.current as f64 + amount) as u32;
            let (cost, tax) = market.purchase(&costs);
            let fee = sector.cost * amount;
            self.wallet.spend(cost + tax + fee);
            self.construction_cost = cost + fee;
            self.tax_cost += tax;
            tax_income += tax;
        }
        let mut input = scaled(&recipe.input, self.utilization);
        input.extend(scaled(&recipe.capital, self.size));
        self.expertise_gain.set_base(self.state.expertise_gain());
        self.expertise =
            (self.expertise + self.expertise_gain.value()).clamp(0.0, self.max_expertise.value());
        self.amount_sold = recipe.output.1 * self.utilization * self.expertise;
        self.shortage = recipe
            .input
            .iter()
            .any(|&(good, _)| market.chronic_shortages[good] > 5.0);
        self.workers = recipe
            .workers
            .iter()
            .map(|&(job, workers)| (catalog.jobs[job.0].good, workers as f64 * self.utilization))
            .collect();
        self.shortage |= self
            .workers
            .iter()
            .any(|&(good, _)| market.chronic_shortages[good] > 5.0);
        market.production[recipe.output.0] += self.amount_sold;
        let (material_costs, taxes) = market.purchase(&input);
        self.material_costs = material_costs;
        self.power_consumption = recipe.power * self.utilization;
        let (wage_costs, income_taxes) = market.purchase(&self.workers);
        self.wage_cost = wage_costs;
        self.transport = 0.0;
        self.tax_cost += taxes + income_taxes;
        self.revenue = self.amount_sold * market.price[recipe.output.0];
        self.profit =
            self.revenue - self.maintenance - self.material_costs - self.wage_cost - self.tax_cost;
        self.subsidy_amount = if self.output_subsidy < 1.0 {
            self.profit * self.output_subsidy
        } else {
            (-self.profit).max(0.0)
        };
        self.profit += self.subsidy_amount;
        self.wallet.credit(self.profit);
        market.gdp += self.revenue - self.material_costs;
        tax_income + taxes + income_taxes - self.subsidy_amount
    }
}

fn scaled(input: &[(GoodId, f64)], scale: f64) -> Vec<(GoodId, f64)> {
    input
        .iter()
        .map(|&(good, amount)| (good, amount * scale))
        .collect()
}

#[derive(Deserialize)]
pub(crate) struct IndustryEntry {
    pub recipe: String,
    pub size: f64,
    pub utilization: Option<f64>,
    pub revenue: Option<Value>,
    pub material_costs: Option<f64>,
    pub profit: Option<f64>,
    pub continuous_gains: Option<f64>,
}

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn interrupted_construction_retains_the_job_without_progress_or_spending() -> Result<()> {
        let catalog = Catalog::load(&crate::default_data_path())?;
        let id = catalog.recipe("farming").unwrap();
        let mut industry = Industry::new(id, 32)?;
        industry.state = IndustryState::Shortage;
        industry.construction = Some(Construction {
            progress: 12,
            maximum: 1000,
            levels: 2,
        });
        let mut market = Market::new(
            &catalog.base_prices,
            &catalog.labor_goods,
            catalog.config.market_config,
        );
        let mut sector = ConstructionSector::default();
        industry.produce(&catalog, &mut market, &mut sector);
        assert_eq!(industry.construction.as_ref().unwrap().progress, 12);
        assert_eq!(industry.construction_cost, 0.0);
        assert_eq!(sector.current, 0);
        assert_eq!(industry.wallet.gdp_change, 0.0);
        Ok(())
    }
    #[test]
    fn construction_labor_costs_include_tax_and_zero_capacity_has_no_demand() {
        let prices = super::super::Ledger::filled(1, 10.0);
        let mut market = Market::new(&prices, &[], super::super::MarketConfig::default());
        market.taxation[GoodId(0)] = 0.2;
        let mut sector = ConstructionSector::default();
        sector.purchase_labor(&mut market, GoodId(0));
        let hours = 1000.0 * (40.0 / 7.0) * 1000.0;
        assert_eq!(market.consumption[GoodId(0)], hours);
        assert_eq!(sector.cost, (hours * 10.0 + hours * 2.0) / 1000.0);
        sector.capacity = 0;
        market.reset();
        sector.purchase_labor(&mut market, GoodId(0));
        assert_eq!(market.consumption.sum(), 0.0);
    }
    #[test]
    fn states_construct_complete_demolish_and_recover() -> Result<()> {
        let catalog = Catalog::load(&crate::default_data_path())?;
        let id = catalog.recipe("farming").unwrap();
        let recipe = &catalog.recipes[id.0];
        let config = catalog.config.production_config;
        let mut industry = Industry::new(id, 100)?;
        industry.state = IndustryState::Maximum;
        industry.revenue = 1000.0;
        industry.profit = 500.0;
        industry.cumulative_profit_ratio = 6.0;
        industry.continuous_gains = 30 * DAY as i32;
        industry.advance_state(recipe, config, |mean, _| mean);
        assert_eq!(industry.state, IndustryState::Construction);
        assert_eq!(industry.construction.as_ref().unwrap().levels, 12);
        assert_eq!(
            industry.construction.as_ref().unwrap().maximum,
            recipe.construction_ticks * DAY as i32
        );
        let construction = industry.construction.as_mut().unwrap();
        construction.progress = construction.maximum;
        industry.advance_state(recipe, config, |mean, _| mean);
        assert_eq!(industry.size, 112.0);
        assert_eq!(industry.state, IndustryState::Expanding);
        industry.shortage = true;
        industry.advance_state(recipe, config, |mean, _| mean);
        assert_eq!(industry.state, IndustryState::Shortage);
        assert!((industry.utilization - 10.0).abs() < 1e-12);
        industry.shortage = false;
        industry.advance_state(recipe, config, |mean, _| mean);
        assert_eq!(industry.state, IndustryState::Steady);
        industry.state = IndustryState::Demolishing;
        industry.profit = -2000.0;
        industry.advance_state(recipe, config, |mean, _| mean);
        assert_eq!(industry.size, 84.0);
        assert_eq!(industry.state, IndustryState::Shrinking);
        Ok(())
    }

    #[test]
    fn production_uses_previous_prices_and_partial_subsidies_preserve_sign() -> Result<()> {
        let catalog = Catalog::load(&crate::default_data_path())?;
        let id = catalog.recipe("farming").unwrap();
        let recipe = &catalog.recipes[id.0];
        let mut industry = Industry::new(id, 2)?;
        industry.employer.population_change = -12;
        industry.employer.population_fulfilled = 123;
        let mut market = Market::new(
            &catalog.base_prices,
            &catalog.labor_goods,
            catalog.config.market_config,
        );
        let mut sector = ConstructionSector::default();
        industry.output_subsidy = 0.5;
        let input = scaled(&recipe.input, 2.0);
        let workers: Vec<_> = recipe
            .workers
            .iter()
            .map(|&(job, amount)| (catalog.jobs[job.0].good, amount as f64 * 2.0))
            .collect();
        let expected_revenue = recipe.output.1 * 2.0 * 1.0001 * market.price[recipe.output.0];
        let expected_profit =
            (expected_revenue - market.price.dot(&input) - market.price.dot(&workers)) * 1.5;
        let taxes = industry.produce(&catalog, &mut market, &mut sector);
        assert_eq!(industry.employer.population_change, -12);
        assert_eq!(industry.employer.population_fulfilled, 123);
        assert!((industry.profit - expected_profit).abs() < 1e-8);
        assert_eq!(industry.wallet.balance, industry.profit);
        assert_eq!(taxes, -industry.subsidy_amount);
        assert_eq!(industry.power_consumption, 1e7);
        assert_eq!(market.production[recipe.output.0], 20002.0);
        Ok(())
    }

    #[test]
    fn construction_shortages_slow_progress_without_reducing_material_demand() -> Result<()> {
        let catalog = Catalog::load(&crate::default_data_path())?;
        let id = catalog.recipe("farming").unwrap();
        let recipe = &catalog.recipes[id.0];
        let mut industry = Industry::new(id, 10)?;
        industry.state = IndustryState::Construction;
        industry.construction = Some(Construction {
            progress: 0,
            maximum: 10000,
            levels: 2,
        });
        let mut market = Market::new(
            &catalog.base_prices,
            &catalog.labor_goods,
            catalog.config.market_config,
        );
        let concrete = recipe.construction_cost[0].0;
        market.chronic_shortages[concrete] = 6.0;
        market.taxation[concrete] = 0.1;
        let mut sector = ConstructionSector::default();
        let costs = scaled(&recipe.construction_cost, 2.0);
        let expected_materials = market.price.dot(&costs);
        industry.produce(&catalog, &mut market, &mut sector);
        assert_eq!(industry.construction.as_ref().unwrap().progress, 28);
        assert_eq!(sector.current, 28);
        assert_eq!(
            market.consumption[concrete],
            recipe.construction_cost[0].1 * 2.0
        );
        assert!((industry.construction_cost - expected_materials - 28800.0).abs() < 1e-8);
        assert!(
            (industry.wallet.balance - industry.profit + expected_materials * 1.1 + 28800.0).abs()
                < 1e-8
        );
        Ok(())
    }
}
