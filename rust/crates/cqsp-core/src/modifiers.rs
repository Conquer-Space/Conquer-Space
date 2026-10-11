//! Modifier accumulation from `components/modifier.h` and `modifierhelper.cpp`.
use crate::read_entries;
use anyhow::{Result, ensure};
use serde::{Deserialize, Serialize};
use std::path::Path;

#[derive(Clone, Copy, Debug, PartialEq, Eq, Serialize, Deserialize)]
pub struct ModifierId(pub usize);

#[derive(Clone, Copy, Debug, PartialEq, Eq, Serialize, Deserialize)]
pub enum ModifierTarget {
    ExpertiseGain,
    MaxExpertise,
}
impl ModifierTarget {
    pub fn parse(value: &str) -> Option<Self> {
        match value {
            "expertise_gain" => Some(Self::ExpertiseGain),
            "max_expertise" => Some(Self::MaxExpertise),
            _ => None,
        }
    }
}

#[derive(Debug)]
pub struct Modifier {
    pub identifier: String,
    pub name: String,
    pub target: ModifierTarget,
    pub amount: f64,
}
pub struct ModifierCatalog {
    pub entries: Vec<Modifier>,
    pub diagnostics: Vec<String>,
}
impl ModifierCatalog {
    pub fn load(data: &Path) -> Result<Self> {
        let mut entries = Vec::new();
        let mut diagnostics = Vec::new();
        for entry in read_entries::<ModifierEntry>(&data.join("data/modifiers"))? {
            let Some(target) = ModifierTarget::parse(&entry.target) else {
                diagnostics.push(format!(
                    "{}: unknown modifier target {}; skipped",
                    entry.identifier, entry.target
                ));
                continue;
            };
            ensure!(
                entry.amount.is_finite(),
                "{}: invalid modifier amount",
                entry.identifier
            );
            entries.push(Modifier {
                name: entry.name.unwrap_or_else(|| entry.identifier.clone()),
                identifier: entry.identifier,
                target,
                amount: entry.amount,
            });
        }
        Ok(Self {
            entries,
            diagnostics,
        })
    }
}
#[derive(Deserialize)]
struct ModifierEntry {
    identifier: String,
    name: Option<String>,
    target: String,
    #[serde(default)]
    amount: f64,
}

#[derive(Clone, Copy, Debug, Serialize, Deserialize)]
pub enum ModifierOp {
    Additive,
    Multiplicative,
}
impl ModifierOp {
    fn identity(self) -> f64 {
        match self {
            Self::Additive => 0.0,
            Self::Multiplicative => 1.0,
        }
    }
}

#[derive(Clone, Debug, Serialize, Deserialize)]
pub struct ModifiableValue {
    op: ModifierOp,
    base: f64,
    sum: f64,
    value: f64,
    modifiers: Vec<(ModifierId, f64)>,
}
impl ModifiableValue {
    pub fn new(base: f64, op: ModifierOp) -> Self {
        Self {
            base,
            value: base,
            sum: op.identity(),
            op,
            modifiers: Vec::new(),
        }
    }
    pub fn value(&self) -> f64 {
        self.value
    }
    pub fn base(&self) -> f64 {
        self.base
    }
    pub fn tooltip(&self, catalog: &ModifierCatalog) -> Vec<crate::tooltips::Line> {
        use crate::{
            number_display::{float, human_with_magnitude},
            tooltips::Line,
        };
        let modifier = match self.op {
            ModifierOp::Additive => float(self.sum),
            ModifierOp::Multiplicative => format!("{}%", float((self.sum - 1.0) * 100.0)),
        };
        // The source formatter assigns abs(value) to a copy of the modifier,
        // applying its sum a second time when choosing the display suffix.
        let magnitude = match self.op {
            ModifierOp::Additive => self.value.abs() + self.sum,
            ModifierOp::Multiplicative => self.value.abs() * self.sum,
        };
        let mut lines = vec![
            Line::plain(format!(
                "Value: {}",
                human_with_magnitude(self.value, magnitude)
            )),
            Line::plain(format!("Modifier: {modifier}")),
        ];
        lines.extend(self.modifiers.iter().map(|&(id, amount)| {
            let name = catalog
                .entries
                .get(id.0)
                .map_or("Null Entity!", |modifier| modifier.name.as_str());
            Line::plain(format!(
                "{name}: {}{}",
                if amount > 0.0 { "+" } else { "" },
                float(amount)
            ))
        }));
        lines
    }
    pub fn set_base(&mut self, value: f64) {
        self.base = value;
        self.combine();
    }
    pub fn offset(&mut self, amount: f64) {
        self.set_base(self.base + amount);
    }
    pub fn scale(&mut self, coefficient: f64) {
        self.set_base(self.base * coefficient);
    }
    pub fn apply(&mut self, id: ModifierId, amount: f64) {
        // Duplicate entries are allowed; removal erases only the first, like C++.
        self.modifiers.push((id, amount));
        self.recalculate();
    }
    pub fn remove(&mut self, id: ModifierId) -> bool {
        let Some(index) = self.modifiers.iter().position(|(key, _)| *key == id) else {
            return false;
        };
        self.modifiers.remove(index);
        self.recalculate();
        true
    }
    fn recalculate(&mut self) {
        self.sum = self
            .modifiers
            .iter()
            .fold(self.op.identity(), |sum, (_, amount)| sum + amount);
        if matches!(self.op, ModifierOp::Multiplicative) {
            self.sum = self.sum.max(0.0);
        }
        self.combine();
    }
    fn combine(&mut self) {
        self.value = match self.op {
            ModifierOp::Additive => self.base + self.sum,
            ModifierOp::Multiplicative => self.base * self.sum,
        };
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::{
        catalog::{Catalog, RecipeId},
        default_data_path,
        economy::{
            Market,
            industry::{ConstructionSector, Industry, IndustryState},
        },
    };

    #[test]
    fn tooltip_suffix_selection_preserves_source_modifier_assignment() -> Result<()> {
        let catalog = ModifierCatalog::load(&default_data_path())?;
        for (base, amount, expected) in [(998.75, 1.0, "1 k"), (-500.0, -1500.0, "-2000")] {
            let mut value = ModifiableValue::new(base, ModifierOp::Additive);
            value.apply(ModifierId(0), amount);
            assert_eq!(
                value.tooltip(&catalog)[0].text,
                format!("Value: {expected}")
            );
        }
        let mut value = ModifiableValue::new(-1235.125, ModifierOp::Multiplicative);
        for _ in 0..40 {
            value.apply(ModifierId(0), 1.0);
        }
        let lines = value.tooltip(&catalog);
        assert_eq!(lines[0].text, "Value: -0.05 M");
        assert_eq!(lines.len(), 42);
        Ok(())
    }
    #[test]
    fn original_modifier_changes_industry_cap_and_removes_one_duplicate() -> Result<()> {
        let catalog = ModifierCatalog::load(&default_data_path())?;
        assert_eq!(catalog.entries.len(), 1);
        let modifier = &catalog.entries[0];
        assert_eq!(modifier.target, ModifierTarget::MaxExpertise);
        let mut industry = Industry::new(RecipeId(0), 1)?;
        industry.apply_modifier(ModifierId(0), modifier);
        industry.apply_modifier(ModifierId(0), modifier);
        assert_eq!(industry.max_expertise.value(), 3.5);
        assert!(industry.remove_modifier(ModifierId(0), modifier));
        assert_eq!(industry.max_expertise.value(), 2.5);
        assert!(industry.remove_modifier(ModifierId(0), modifier));
        assert!(!industry.remove_modifier(ModifierId(0), modifier));
        assert_eq!(industry.max_expertise.value(), 1.5);
        assert_eq!(ModifierTarget::parse("unknown"), None);
        Ok(())
    }
    #[test]
    fn gain_modifiers_survive_state_reassignment_and_clamp_only_the_multiplier() {
        let mut gain = ModifiableValue::new(0.0001, ModifierOp::Multiplicative);
        gain.apply(ModifierId(0), 0.5);
        gain.set_base(-0.0001);
        assert_eq!(gain.value(), -0.0001 * 1.5);
        gain.apply(ModifierId(1), -2.0);
        assert_eq!(gain.value(), 0.0);
        gain.remove(ModifierId(1));
        assert_eq!(gain.value(), -0.0001 * 1.5);
    }

    #[test]
    fn production_reassigns_base_gain_but_preserves_gain_and_cap_modifiers() -> Result<()> {
        let data = default_data_path();
        let catalog = Catalog::load(&data)?;
        let modifiers = ModifierCatalog::load(&data)?;
        let mut industry = Industry::new(RecipeId(0), 1)?;
        industry.apply_modifier(ModifierId(0), &modifiers.entries[0]);
        industry.apply_modifier(
            ModifierId(1),
            &Modifier {
                identifier: "gain".into(),
                name: "Gain".into(),
                target: ModifierTarget::ExpertiseGain,
                amount: 1.0,
            },
        );
        let mut market = Market::new(
            &catalog.base_prices,
            &catalog.labor_goods,
            catalog.config.market_config,
        );
        let mut construction = ConstructionSector::default();
        industry.expertise = 2.49999;
        industry.produce(&catalog, &mut market, &mut construction);
        assert_eq!(industry.expertise, 2.5);
        industry.state = IndustryState::Shrinking;
        industry.produce(&catalog, &mut market, &mut construction);
        assert_eq!(industry.expertise_gain.value(), -0.0002);
        assert_eq!(industry.expertise, 2.5 - 0.0002);
        Ok(())
    }
}
