//! Daily planetary history and the original market logger's CSV column schema.
use super::{GoodId, Ledger, Market};
use crate::{catalog::Catalog, clock::StarDate};
use serde::{Deserialize, Serialize};
use std::io::{self, Write};

#[derive(Clone, Debug, Serialize, Deserialize)]
pub struct MarketSample {
    pub date: StarDate,
    pub gdp: f64,
    pub price: Ledger,
    pub sd_ratio: Ledger,
    pub supply: Ledger,
    pub demand: Ledger,
}

#[derive(Default, Clone, Debug, Serialize, Deserialize)]
pub struct MarketHistory(pub Vec<MarketSample>);

impl MarketHistory {
    pub fn record(&mut self, date: StarDate, market: &Market) {
        self.0.push(MarketSample {
            date,
            gdp: market.gdp,
            price: market.price.clone(),
            sd_ratio: market.sd_ratio.clone(),
            supply: market.supply.clone(),
            demand: market.demand.clone(),
        });
    }
}

impl Market {
    pub fn write_csv_header(output: &mut impl Write, catalog: &Catalog) -> io::Result<()> {
        write!(
            output,
            "Date,GDP,Deficit,Last Deficit,Trade Deficit,Last Trade Deficit"
        )?;
        for good in &catalog.goods {
            for field in [
                "supply",
                "demand",
                "sd_ratio",
                "volume",
                "price",
                "chronic_shortages",
                "trade",
                "taxation",
                "production",
                "consumption",
            ] {
                write!(output, ",{}_{}", good.identifier, field)?;
            }
        }
        writeln!(output)
    }
    pub fn write_csv_record(&self, output: &mut impl Write, date: StarDate) -> io::Result<()> {
        write!(
            output,
            "{},{},{},{},{},{}",
            date.display(),
            self.gdp,
            self.deficit,
            self.last_deficit,
            self.trade_deficit,
            self.last_trade_deficit
        )?;
        for index in 0..self.price.values().len() {
            let good = GoodId(index);
            for ledger in [
                &self.supply,
                &self.demand,
                &self.sd_ratio,
                &self.volume,
                &self.price,
                &self.chronic_shortages,
                &self.trade,
                &self.taxation,
                &self.production,
                &self.consumption,
            ] {
                write!(output, ",{:.6}", ledger[good])?;
            }
        }
        writeln!(output)
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::default_data_path;

    #[test]
    fn snapshots_are_independent_and_csv_keeps_original_columns() -> anyhow::Result<()> {
        let catalog = Catalog::load(&default_data_path())?;
        let mut market = Market::new(
            &catalog.base_prices,
            &catalog.labor_goods,
            catalog.config.market_config,
        );
        let mut history = MarketHistory::default();
        history.record(StarDate::from_tick(0), &market);
        market.supply[GoodId(0)] = 42.0;
        history.record(StarDate::from_tick(144), &market);
        assert_eq!(history.0[0].supply[GoodId(0)], 1.0);
        assert_eq!(history.0[1].supply[GoodId(0)], 42.0);
        let mut csv = Vec::new();
        Market::write_csv_header(&mut csv, &catalog)?;
        market.write_csv_record(&mut csv, StarDate::from_tick(0))?;
        let csv = String::from_utf8(csv)?;
        let lines: Vec<_> = csv.lines().collect();
        assert_eq!(lines[0].split(',').count(), 6 + catalog.goods.len() * 10);
        assert_eq!(lines[0].split(',').count(), lines[1].split(',').count());
        assert_eq!(
            lines[0].split(',').nth(6),
            Some(format!("{}_supply", catalog.goods[0].identifier).as_str())
        );
        assert!(lines[1].starts_with("1950-01-01,0,0,0,0,0,42.000000,"));
        Ok(())
    }
}
