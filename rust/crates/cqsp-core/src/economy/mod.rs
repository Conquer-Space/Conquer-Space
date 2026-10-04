//! Ports of the original market, wallet, PID, and production configuration rules.
pub mod history;
pub mod industry;
pub mod population;
use serde::{Deserialize, Serialize};
use std::ops::{Index, IndexMut};

#[derive(Clone, Copy, Debug, PartialEq, Eq, PartialOrd, Ord, Hash, Serialize, Deserialize)]
pub struct GoodId(pub usize);

#[derive(Clone, Debug, PartialEq, Serialize, Deserialize)]
pub struct Ledger(Vec<f64>);

impl Ledger {
    pub fn new(count: usize) -> Self {
        Self(vec![0.0; count])
    }
    pub fn filled(count: usize, value: f64) -> Self {
        Self(vec![value; count])
    }
    pub fn values(&self) -> &[f64] {
        &self.0
    }
    pub fn clear(&mut self) {
        self.0.fill(0.0);
    }
    pub fn sum(&self) -> f64 {
        self.0.iter().sum()
    }
    pub fn add(&mut self, entries: &[(GoodId, f64)]) {
        for &(good, amount) in entries {
            self[good] += amount;
        }
    }
    pub fn add_ledger(&mut self, other: &Self) {
        assert_eq!(self.0.len(), other.0.len(), "Ledger sizes differ");
        for (target, value) in self.0.iter_mut().zip(&other.0) {
            *target += value;
        }
    }
    pub fn dot(&self, entries: &[(GoodId, f64)]) -> f64 {
        entries
            .iter()
            .map(|&(good, amount)| self[good] * amount)
            .sum()
    }
    pub fn dot_ledger(&self, other: &Self) -> f64 {
        assert_eq!(self.0.len(), other.0.len(), "Ledger sizes differ");
        self.0.iter().zip(&other.0).map(|(a, b)| a * b).sum()
    }
    pub fn safe_division(&self, other: &Self, zero: f64) -> Self {
        assert_eq!(self.0.len(), other.0.len(), "Ledger sizes differ");
        Self(
            self.0
                .iter()
                .zip(&other.0)
                .map(|(a, b)| if *b == 0.0 { zero } else { a / b })
                .collect(),
        )
    }
}

impl Index<GoodId> for Ledger {
    type Output = f64;
    fn index(&self, good: GoodId) -> &f64 {
        &self.0[good.0]
    }
}
impl IndexMut<GoodId> for Ledger {
    fn index_mut(&mut self, good: GoodId) -> &mut f64 {
        &mut self.0[good.0]
    }
}

#[derive(Default, Clone, Copy, Debug, PartialEq, Serialize, Deserialize)]
pub struct Wallet {
    pub balance: f64,
    pub change: f64,
    pub gdp_change: f64,
}

impl Wallet {
    pub fn credit(&mut self, amount: f64) {
        self.balance += amount;
        self.change += amount;
    }
    pub fn spend(&mut self, amount: f64) {
        self.balance -= amount;
        self.change -= amount;
        self.gdp_change += amount;
    }
    pub fn assign(&mut self, balance: f64) {
        let difference = balance - self.balance;
        self.change += difference;
        // Original assignment and -= have opposite GDP-delta signs; preserve this.
        if difference < 0.0 {
            self.gdp_change += difference;
        }
        self.balance = balance;
    }
    pub fn scale(&mut self, coefficient: f64) {
        let difference = self.balance * coefficient - self.balance;
        if difference > 0.0 {
            self.credit(difference);
        } else if difference < 0.0 {
            self.spend(-difference);
        }
    }
    pub fn reset(&mut self) {
        self.change = 0.0;
        self.gdp_change = 0.0;
    }
}

#[derive(Default, Clone, Copy, Debug, PartialEq, Serialize, Deserialize)]
pub struct Pid {
    pub proportional: f64,
    pub integral: f64,
    pub derivative: f64,
}

impl Pid {
    pub fn update(&mut self, value: f64) {
        // Deliberately previous - current, not the conventional derivative sign.
        self.derivative = self.proportional - value;
        self.proportional = value;
        self.integral += value;
    }
    pub fn value(self, [kp, ki, kd]: [f64; 3]) -> f64 {
        self.proportional * kp + self.integral * ki + self.derivative * kd
    }
}

#[derive(Clone, Copy, Debug, Serialize, Deserialize)]
#[serde(default)]
pub struct MarketConfig {
    pub base_price_deviation: f64,
    pub shortage_level: f64,
    pub default_market_access: f64,
}
impl Default for MarketConfig {
    fn default() -> Self {
        Self {
            base_price_deviation: 0.75,
            shortage_level: 0.8,
            default_market_access: 0.8,
        }
    }
}

#[derive(Clone, Copy, Debug, Serialize, Deserialize)]
#[serde(default)]
pub struct ProductionConfig {
    pub profit_multiplier: f64,
    pub max_factory_delta: f64,
    pub factory_min_utilization: f64,
    pub construction_limit: i32,
    pub underutilization_limit: f64,
}
impl Default for ProductionConfig {
    fn default() -> Self {
        Self {
            profit_multiplier: 0.001,
            max_factory_delta: 0.01,
            factory_min_utilization: 0.05,
            construction_limit: 90,
            underutilization_limit: 30.0,
        }
    }
}
impl ProductionConfig {
    pub fn minimum_utilization(self, size: f64) -> f64 {
        (self.factory_min_utilization * size).max(1.0)
    }
    pub fn utilization_diff(self, profit: f64) -> f64 {
        1.0 + self.max_factory_delta / (1.0 + (-profit * self.profit_multiplier).exp())
            - self.max_factory_delta / 2.0
    }
}

#[derive(Default, Clone, Copy, Debug, Serialize, Deserialize)]
#[serde(default)]
pub struct EconomyConfig {
    pub market_config: MarketConfig,
    pub production_config: ProductionConfig,
}

#[derive(Clone, Debug, Serialize, Deserialize)]
pub struct Market {
    pub demand: Ledger,
    pub supply: Ledger,
    pub sd_ratio: Ledger,
    pub volume: Ledger,
    pub price: Ledger,
    pub chronic_shortages: Ledger,
    pub trade: Ledger,
    pub taxation: Ledger,
    pub production: Ledger,
    pub consumption: Ledger,
    pub market_access: Ledger,
    pub gdp: f64,
    pub deficit: f64,
    pub last_deficit: f64,
    pub trade_deficit: f64,
    pub last_trade_deficit: f64,
}

impl Market {
    pub fn new(base_prices: &Ledger, labor: &[GoodId], config: MarketConfig) -> Self {
        let count = base_prices.values().len();
        let mut access = Ledger::filled(count, config.default_market_access);
        for &good in labor {
            access[good] = 0.0;
        }
        Self {
            demand: Ledger::filled(count, 1.0),
            supply: Ledger::filled(count, 1.0),
            sd_ratio: Ledger::filled(count, 1.0),
            volume: Ledger::new(count),
            price: base_prices.clone(),
            chronic_shortages: Ledger::new(count),
            trade: Ledger::new(count),
            taxation: Ledger::new(count),
            production: Ledger::new(count),
            consumption: Ledger::new(count),
            market_access: access,
            gdp: 0.0,
            deficit: 0.0,
            last_deficit: 0.0,
            trade_deficit: 0.0,
            last_trade_deficit: 0.0,
        }
    }

    pub fn reset(&mut self) {
        self.demand.clear();
        self.supply.clear();
        self.production.clear();
        self.consumption.clear();
        // Prices, shortages and last tick's trade are intentionally retained.
    }

    pub fn purchase(&mut self, input: &[(GoodId, f64)]) -> (f64, f64) {
        let cost = self.price.dot(input);
        let tax = input
            .iter()
            .map(|&(good, amount)| self.taxation[good] * self.price[good] * amount)
            .sum();
        self.consumption.add(input);
        (cost, tax)
    }

    pub fn labor_demand(&self, good: GoodId) -> f64 {
        (self.supply[good] - 2.0 * self.demand[good]) / (40.0 / 7.0)
    }

    pub fn determine_prices(&mut self, base_prices: &Ledger, deviation: f64) {
        for index in 0..base_prices.values().len() {
            let good = GoodId(index);
            let (demand, supply) = (self.demand[good], self.supply[good]);
            let pressure = ((demand - supply) / demand.min(supply).max(0.001)).clamp(-1.0, 1.0);
            self.price[good] = base_prices[good] * (1.0 + deviation * pressure);
        }
    }

    pub fn process(&mut self, base_prices: &Ledger, config: MarketConfig) {
        self.supply = self.production.clone();
        self.demand = self.consumption.clone();
        for index in 0..base_prices.values().len() {
            let good = GoodId(index);
            let trade = self.trade[good];
            if trade > 0.0 {
                self.supply[good] += trade;
            } else if trade < 0.0 {
                self.demand[good] -= trade;
            }
        }
        self.sd_ratio = self.supply.safe_division(&self.demand, f64::INFINITY);
        self.determine_prices(base_prices, config.base_price_deviation);
        let mut deficit = 0.0;
        for index in 0..base_prices.values().len() {
            let good = GoodId(index);
            let (demand, supply) = (self.demand[good], self.supply[good]);
            deficit += (demand - supply) * self.price[good];
            let shortage = if demand == 0.0 {
                0.0
            } else {
                (demand - supply) / demand
            };
            if shortage > config.shortage_level {
                self.chronic_shortages[good] += shortage;
            } else if shortage < 0.0 {
                self.chronic_shortages[good] -=
                    (self.chronic_shortages[good] - (1.0 - shortage)).max(0.0);
            }
        }
        self.last_deficit = deficit;
        self.deficit += deficit;
    }

    /// `SysPlanetaryTrade::DetermineTrade`: price interpolation, weighted imports/exports.
    pub fn trade_with(&mut self, planetary: &Self) {
        self.trade.clear();
        for index in 0..self.price.values().len() {
            let good = GoodId(index);
            let access = self.market_access[good];
            self.price[good] = planetary.price[good] * access + (1.0 - access) * self.price[good];
            let change = self.price[good] / planetary.price[good];
            if planetary.supply[good] != 0.0 {
                let exports = (self.production[good] / planetary.supply[good]
                    * planetary.demand[good]
                    - self.consumption[good])
                    .max(0.0);
                self.trade[good] -= exports * change * access;
            }
            if planetary.demand[good] != 0.0 {
                let imports = (self.consumption[good] / planetary.demand[good]
                    * planetary.supply[good]
                    - self.production[good])
                    .max(0.0);
                self.trade[good] += imports * change * access;
            }
        }
        self.last_trade_deficit = -self.price.dot_ledger(&self.trade);
        self.trade_deficit += self.last_trade_deficit;
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    fn base() -> Ledger {
        Ledger(vec![10.0, 20.0])
    }

    #[test]
    fn zero_demand_ratio_is_infinity_even_with_zero_supply() {
        assert!(
            Ledger::new(1).safe_division(&Ledger::new(1), f64::INFINITY)[GoodId(0)].is_infinite()
        );
    }

    #[test]
    fn purchase_registers_demand_not_supply_and_returns_tax_separately() {
        let mut market = Market::new(&base(), &[], MarketConfig::default());
        market.taxation[GoodId(0)] = 0.1;
        assert_eq!(
            market.purchase(&[(GoodId(0), 2.0), (GoodId(1), 3.0)]),
            (80.0, 2.0)
        );
        assert_eq!(market.consumption.values(), &[2.0, 3.0]);
        assert_eq!(market.production.sum(), 0.0);
    }

    #[test]
    fn reset_preserves_prices_shortages_and_trade() {
        let mut market = Market::new(&base(), &[], MarketConfig::default());
        market.trade[GoodId(0)] = 9.0;
        market.chronic_shortages[GoodId(0)] = 7.0;
        market.purchase(&[(GoodId(0), 2.0)]);
        market.reset();
        assert_eq!(market.consumption.sum(), 0.0);
        assert_eq!(market.demand.sum(), 0.0);
        assert_eq!(market.price, base());
        assert_eq!(market.trade[GoodId(0)], 9.0);
        assert_eq!(market.chronic_shortages[GoodId(0)], 7.0);
    }

    #[test]
    fn market_caps_prices_and_preserves_shortage_recovery_rule() {
        let mut market = Market::new(&base(), &[], MarketConfig::default());
        market.consumption[GoodId(0)] = 100.0;
        market.process(&base(), MarketConfig::default());
        assert_eq!(market.price.values(), &[17.5, 20.0]);
        assert_eq!(market.chronic_shortages[GoodId(0)], 1.0);
        market.chronic_shortages[GoodId(0)] = 8.0;
        market.production[GoodId(0)] = 200.0;
        market.process(&base(), MarketConfig::default());
        assert_eq!(market.price[GoodId(0)], 2.5);
        assert_eq!(market.chronic_shortages[GoodId(0)], 2.0);
    }

    #[test]
    fn wallet_matches_legacy_assignment_gdp_sign() {
        let mut wallet = Wallet::default();
        wallet.assign(100.0);
        wallet.reset();
        wallet.spend(25.0);
        wallet.assign(50.0);
        assert_eq!(wallet.balance, 50.0);
        assert_eq!(wallet.change, -50.0);
        assert_eq!(wallet.gdp_change, 0.0);
    }

    #[test]
    fn trade_distributes_exports_and_imports_but_labor_is_not_tradable() {
        let config = MarketConfig::default();
        let mut planetary = Market::new(&base(), &[GoodId(1)], config);
        planetary.supply[GoodId(0)] = 100.0;
        planetary.demand[GoodId(0)] = 100.0;
        let mut exporter = Market::new(&base(), &[GoodId(1)], config);
        exporter.production[GoodId(0)] = 100.0;
        exporter.consumption[GoodId(0)] = 20.0;
        let mut importer = Market::new(&base(), &[GoodId(1)], config);
        importer.consumption[GoodId(0)] = 80.0;
        importer.consumption[GoodId(1)] = 80.0;
        importer.price[GoodId(1)] = 35.0;
        exporter.trade_with(&planetary);
        importer.trade_with(&planetary);
        assert_eq!(exporter.trade[GoodId(0)], -64.0);
        assert_eq!(importer.trade[GoodId(0)], 64.0);
        assert_eq!(exporter.last_trade_deficit, 640.0);
        assert_eq!(importer.last_trade_deficit, -640.0);
        assert_eq!(importer.trade[GoodId(1)], 0.0);
        assert_eq!(importer.price[GoodId(1)], 35.0);
        importer.process(&base(), config);
        assert_eq!(importer.supply[GoodId(0)], 64.0);
        assert_eq!(importer.sd_ratio[GoodId(0)], 0.8);
    }
}
