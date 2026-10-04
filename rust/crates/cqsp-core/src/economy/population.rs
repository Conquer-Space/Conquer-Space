//! `ProvinceLoader::ParsePopulation`, `SysPopulationConsumption`, and labor rules.
use super::{GoodId, Market, Pid, Wallet};
use crate::catalog::{Catalog, JobId};
use anyhow::{Context, Result};
use serde::Deserialize;
use std::collections::BTreeMap;

#[derive(Debug)]
pub struct Population {
    pub size: u64,
    pub labor_force: u64,
    pub employed: u64,
    /// Undefined C++ float-to-unsigned conversions are represented as None.
    pub unemployment_rate: Option<u64>,
    pub standard_of_living: f64,
    pub education: f64,
    pub consumer_confidence: f64,
    pub wallet: Wallet,
    pub income: f64,
    pub spending: f64,
    pub average_wage: f64,
    pub jobs: BTreeMap<JobId, u64>,
    pub labor_hours: Vec<(GoodId, f64)>,
    pub consumption: Vec<(GoodId, f64)>,
    pub sol_pid: Pid,
}

impl Population {
    pub(crate) fn load(entry: PopulationEntry, catalog: &Catalog) -> Result<Self> {
        let jobs = entry
            .job_distribution
            .iter()
            .map(|(identifier, &amount)| {
                Ok((
                    catalog
                        .job(identifier)
                        .with_context(|| format!("Unknown population job {identifier}"))?,
                    amount,
                ))
            })
            .collect::<Result<_>>()?;
        let mut wallet = Wallet::default();
        wallet.assign(entry.balance);
        Ok(Self {
            size: entry.size,
            labor_force: entry.labor_force.unwrap_or(entry.size / 2),
            employed: 0,
            unemployment_rate: Some(0),
            standard_of_living: entry.sol,
            education: entry.education.map_or(12.0, |value| value as i64 as f64),
            consumer_confidence: entry
                .consumer_confidence
                .map_or(1.0, |value| value as i64 as f64),
            wallet,
            income: 0.0,
            spending: 0.0,
            average_wage: 0.0,
            jobs,
            labor_hours: Vec::new(),
            consumption: Vec::new(),
            sol_pid: Pid::default(),
        })
    }

    pub fn consume(
        &mut self,
        market: &mut Market,
        autonomous: &[(GoodId, f64)],
        marginal: &[(GoodId, f64)],
    ) {
        self.consumption = autonomous
            .iter()
            .map(|&(good, amount)| (good, amount * self.size as f64))
            .collect();
        let mut cost = market.price.dot(&self.consumption);
        if self.wallet.balance > 0.0 {
            cost += market.price.dot(marginal) * self.standard_of_living;
            // Consumers overlap both vectors; add by good so demand isn't order-dependent.
            for &(good, amount) in marginal {
                let amount = amount * self.standard_of_living;
                if let Some((_, current)) = self.consumption.iter_mut().find(|(id, _)| *id == good)
                {
                    *current += amount;
                } else {
                    self.consumption.push((good, amount));
                }
            }
        }
        let ratio = if self.income > 0.0 {
            (self.income - self.spending) / self.income
        } else {
            -1.0
        };
        self.sol_pid.update(ratio.clamp(-2.0, 2.0));
        let limit = self.standard_of_living * 0.1;
        let change = (200.0 * self.sol_pid.value([0.01, 0.1, 0.01])).clamp(-limit, limit);
        self.standard_of_living = (self.standard_of_living + change).max(1.0);
        self.average_wage = self.income / (self.employed as f64 + 1.0);
        self.spending = cost;
        market.purchase(&self.consumption);
        self.income = market.price.dot(&self.labor_hours);
        self.wallet.spend(cost);
        market.production.add(&self.labor_hours);
        // The original registers population consumption twice and doesn't credit wages.
        market.consumption.add(&self.consumption);
    }

    pub fn redistribute_labor(&mut self, market: &Market, catalog: &Catalog) {
        let mut drift = BTreeMap::<JobId, i32>::new();
        for (&job, &workers) in &self.jobs {
            let good = catalog.jobs[job.0].good;
            if job != catalog.default_job && market.sd_ratio[good] > 2.0 {
                let delta = (market.sd_ratio[good] - 2.0).min(10.0);
                let difference = workers as f64 * 0.01 * delta;
                add_drift(&mut drift, catalog.default_job, difference);
                add_drift(&mut drift, job, -difference);
            }
            for (index, destination) in catalog.jobs.iter().enumerate() {
                let higher = market.price[destination.good];
                let current = market.price[good];
                if higher - current > current * 0.1 {
                    let destination = JobId(index);
                    let delta = ((higher - current) / current).min(1.0);
                    let cap = (self.jobs.get(&destination).copied().unwrap_or(0) as f64 * 0.001)
                        .max(100.0);
                    let difference = (workers as f64 * delta * 0.00001).min(cap);
                    add_drift(&mut drift, destination, difference);
                    add_drift(&mut drift, job, -difference);
                }
            }
        }
        for (job, drift) in drift {
            let workers = self.jobs.entry(job).or_default();
            let limit = ((*workers as f64 * 0.01) as i32).max(1000);
            let delta = drift.clamp(-limit, limit);
            if delta > 0 && market.demand[catalog.jobs[job.0].good] == 0.0 {
                continue;
            }
            // Match the original defined unsigned arithmetic, including wrapping.
            *workers = workers.wrapping_add_signed(delta as i64);
        }
        self.labor_hours.clear();
        let mut workforce = 0.0;
        let mut employed = 0.0;
        for (&job, &workers) in &self.jobs {
            let good = catalog.jobs[job.0].good;
            self.labor_hours.push((good, 40.0 * workers as f64));
            workforce += workers as f64;
            employed += workers as f64 * (1.0 / market.sd_ratio[good]);
        }
        self.employed = employed as u64;
        let rate = (1.0 - employed) / workforce;
        self.unemployment_rate =
            (rate.is_finite() && rate >= 0.0 && rate < u64::MAX as f64).then_some(rate as u64);
    }
}

fn add_drift(drift: &mut BTreeMap<JobId, i32>, job: JobId, difference: f64) {
    let value = drift.entry(job).or_default();
    // C++ compound assignment truncates after every addition, not after accumulation.
    *value = (*value as f64 + difference) as i32;
}

#[derive(Deserialize)]
pub(crate) struct PopulationEntry {
    pub size: u64,
    pub labor_force: Option<u64>,
    #[serde(default)]
    pub sol: f64,
    #[serde(default)]
    pub balance: f64,
    pub education: Option<f64>,
    pub consumer_confidence: Option<f64>,
    #[serde(default)]
    pub job_distribution: BTreeMap<String, u64>,
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::economy::{Ledger, MarketConfig};
    #[test]
    fn consumption_uses_old_income_and_sol_and_registers_double_demand() -> Result<()> {
        let catalog = Catalog::load(&crate::default_data_path())?;
        let entry =
            serde_json::from_value(serde_json::json!({"size":100,"sol":10,"balance":1000}))?;
        let mut population = Population::load(entry, &catalog)?;
        population.income = 100.0;
        population.spending = 80.0;
        population.employed = 4;
        population.labor_hours.push((GoodId(1), 4.0));
        let mut prices = Ledger::new(2);
        prices[GoodId(0)] = 2.0;
        prices[GoodId(1)] = 10.0;
        let mut market = Market::new(&prices, &[], MarketConfig::default());
        population.consume(&mut market, &[(GoodId(0), 1.0)], &[(GoodId(0), 0.5)]);
        assert_eq!(population.standard_of_living, 11.0);
        assert_eq!(population.average_wage, 20.0);
        assert_eq!(population.spending, 210.0);
        assert_eq!(population.income, 40.0);
        assert_eq!(population.wallet.balance, 790.0);
        assert_eq!(market.consumption[GoodId(0)], 210.0);
        assert_eq!(market.production[GoodId(1)], 4.0);
        Ok(())
    }
    #[test]
    fn job_drift_truncates_each_operation() {
        let mut drift = BTreeMap::new();
        add_drift(&mut drift, JobId(0), 0.75);
        add_drift(&mut drift, JobId(0), 0.75);
        assert_eq!(drift[&JobId(0)], 0);
    }
}
