//! Native economy/science inspection and original factory controls.
use super::{Game, human_number, spaceport_ui, ui_button as button, ui_font, ui_row as row};
use bevy::{
    input::mouse::{AccumulatedMouseScroll, MouseScrollUnit},
    prelude::*,
    ui::RelativeCursorPosition,
};
use cqsp_core::{
    economy::{GoodId, population::Population},
    simulation::Settlement,
    solar_system::BodyId,
};
use std::{collections::BTreeSet, fmt::Write};

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub(super) enum Tab {
    Civilization,
    Markets,
    History,
    Demographics,
    Industries,
    Construction,
    Infrastructure,
    Science,
}
#[derive(Component, Clone, Copy)]
pub(super) enum Action {
    Open,
    Close,
    Tab(Tab),
    Planet,
    PreviousProvince,
    NextProvince,
    Hovered,
    Ownership,
    Previous,
    Next,
    NextTechnology,
    NextFactory,
    Subsidy(f32),
    BoostExpertise,
    PerCapita,
    ConsumptionCost,
    ConsumptionSlice(GoodId),
    FactoryCount(i32),
    ConstructFactory,
    ChartGood,
    ChartWindow,
    ChartFit,
    NextDay,
}
#[derive(Component)]
pub(super) enum Panel {
    Launcher,
    Overlay,
    FactoryActions,
    DemographicActions,
    ConstructionActions,
    HistoryActions,
    ConsumptionPlots,
}
#[derive(Component)]
pub(super) enum Label {
    Scope,
    Content,
    ConsumptionTitle(usize),
    ConsumptionGood { segment: usize, good: GoodId },
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
enum Series {
    Gdp,
    Price,
    Supply,
    Demand,
    Population,
    Sol,
    Employment,
    EmploymentRate,
}
impl Series {
    const ALL: [Self; 8] = [
        Self::Gdp,
        Self::Price,
        Self::Supply,
        Self::Demand,
        Self::Population,
        Self::Sol,
        Self::Employment,
        Self::EmploymentRate,
    ];
    fn name(self) -> &'static str {
        match self {
            Self::Gdp => "Market GDP",
            Self::Price => "Good price",
            Self::Supply => "Good supply",
            Self::Demand => "Good demand",
            Self::Population => "Global population",
            Self::Sol => "Global standard of living",
            Self::Employment => "Global employment",
            Self::EmploymentRate => "Global employment rate (%)",
        }
    }
    fn global(self) -> bool {
        matches!(
            self,
            Self::Population | Self::Sol | Self::Employment | Self::EmploymentRate
        )
    }
}
#[derive(Clone, Copy, PartialEq)]
struct PlotKey {
    body: BodyId,
    good: usize,
    series: Series,
    window: usize,
    samples: usize,
    view: Option<super::chart::View>,
}
#[derive(Component, Default)]
pub(super) struct PlotImage {
    key: Option<PlotKey>,
}
#[derive(Component)]
pub(super) struct PieCard(usize);
#[derive(Component)]
pub(super) struct PieLegend {
    segment: usize,
    good: GoodId,
}
#[derive(Component)]
pub(super) struct PieImage {
    segment: usize,
    key: Option<Vec<(GoodId, u32, bool)>>,
}

#[derive(Resource)]
pub(super) struct Inspector {
    pub open: bool,
    pub capture_input: bool,
    tab: Tab,
    body: BodyId,
    province: Option<usize>,
    owned: bool,
    good: usize,
    page: usize,
    factory: usize,
    field: usize,
    technology: usize,
    per_capita: bool,
    consumption_cost: bool,
    hidden_consumption: BTreeSet<GoodId>,
    pie_hover: [Option<GoodId>; 2],
    recipe: usize,
    factory_count: i32,
    message: String,
    series: usize,
    plot_window: usize,
    plot_view: Option<super::chart::View>,
    plot_drag: Option<Vec2>,
    plot_cursor: Option<(f64, f64)>,
    plot_pointer_over: bool,
}
impl Inspector {
    pub fn new(game: &cqsp_core::simulation::Simulation) -> Self {
        Self {
            open: false,
            capture_input: false,
            tab: Tab::Markets,
            body: game.solar_system.body_id("earth").unwrap(),
            province: None,
            owned: true,
            good: 0,
            page: 0,
            factory: 0,
            field: 0,
            technology: 0,
            per_capita: true,
            consumption_cost: true,
            hidden_consumption: BTreeSet::new(),
            pie_hover: [None; 2],
            recipe: 0,
            factory_count: 1,
            message: String::new(),
            series: 0,
            plot_window: 0,
            plot_view: None,
            plot_drag: None,
            plot_cursor: None,
            plot_pointer_over: false,
        }
    }
    pub fn apply(&mut self, action: Action, game: &mut Game) {
        if matches!(
            action,
            Action::Tab(_)
                | Action::Planet
                | Action::ChartGood
                | Action::ChartWindow
                | Action::ChartFit
        ) || (self.tab == Tab::History && matches!(action, Action::Previous | Action::Next))
        {
            self.reset_plot();
        }
        if matches!(
            action,
            Action::Tab(_)
                | Action::Planet
                | Action::PreviousProvince
                | Action::NextProvince
                | Action::Hovered
                | Action::Ownership
        ) {
            self.factory = 0;
            self.message.clear();
        }
        match action {
            Action::Open => self.open = true,
            Action::Close => self.open = false,
            Action::Tab(tab) => {
                self.tab = tab;
                self.page = 0;
            }
            Action::Planet => {
                let bodies: Vec<_> = game.simulation.planetary_markets.keys().copied().collect();
                let index = bodies
                    .iter()
                    .position(|body| *body == self.body)
                    .unwrap_or(0);
                self.body = bodies[(index + 1) % bodies.len()];
                self.province = None;
                self.page = 0;
            }
            Action::PreviousProvince | Action::NextProvince => {
                let provinces: Vec<_> = self.provinces(game).map(|(index, _)| index).collect();
                if provinces.is_empty() {
                    self.province = None;
                    return;
                }
                let index = self
                    .province
                    .and_then(|id| provinces.iter().position(|index| *index == id));
                let next = index.map_or(0, |index| {
                    (index
                        + if matches!(action, Action::PreviousProvince) {
                            provinces.len() - 1
                        } else {
                            1
                        })
                        % provinces.len()
                });
                self.province = Some(provinces[next]);
                self.page = 0;
            }
            Action::Hovered => {
                self.province = game.hovered;
                if let Some(index) = self.province {
                    self.body = game.simulation.settlements[index].body;
                }
                self.page = 0;
            }
            Action::Ownership => {
                self.owned = !self.owned;
                self.province = None;
                self.page = 0;
            }
            Action::Previous | Action::Next => {
                let previous = matches!(action, Action::Previous);
                match self.tab {
                    Tab::Civilization => {
                        let count = game
                            .session
                            .player_country
                            .map_or(0, |country| {
                                game.simulation.countries[country]
                                    .provinces
                                    .len()
                                    .max(self.governed(game).count())
                            })
                            .div_ceil(8)
                            .max(1);
                        cycle(&mut self.page, count, previous);
                    }
                    Tab::History => cycle(&mut self.series, Series::ALL.len(), previous),
                    Tab::Markets => cycle(
                        &mut self.good,
                        game.simulation.catalog.goods.len(),
                        previous,
                    ),
                    Tab::Industries => {
                        let count = self.factories(game).count().div_ceil(4).max(1);
                        cycle(&mut self.page, count, previous);
                        self.factory = self.page * 4;
                    }
                    Tab::Demographics => {
                        let count = self
                            .sites(game)
                            .map(|(_, site)| site.populations.len())
                            .sum::<usize>()
                            .div_ceil(2)
                            .max(1);
                        cycle(&mut self.page, count, previous);
                    }
                    Tab::Construction => {
                        cycle(
                            &mut self.recipe,
                            game.simulation.catalog.recipes.len(),
                            previous,
                        );
                        self.message.clear();
                    }
                    Tab::Infrastructure => {
                        let count = self.sites(game).count().div_ceil(4).max(1);
                        cycle(&mut self.page, count, previous);
                    }
                    Tab::Science => cycle(
                        &mut self.field,
                        game.simulation.science.fields.len(),
                        previous,
                    ),
                }
            }
            Action::NextTechnology => cycle(
                &mut self.technology,
                game.simulation.science.technologies.len(),
                false,
            ),
            Action::NextFactory if self.tab == Tab::Industries => {
                let count = self.factories(game).count();
                cycle(&mut self.factory, count, false);
                self.page = self.factory / 4;
            }
            Action::Subsidy(change) if self.tab == Tab::Industries && change.is_finite() => {
                let selected = self.factories(game).nth(self.factory);
                if let Some((site, factory)) = selected {
                    let industry = &mut game.simulation.settlements[site].industries[factory];
                    // The original slider edits a float before assigning its double field.
                    industry.output_subsidy =
                        (industry.output_subsidy as f32 + change).clamp(0.0, 1.0) as f64;
                }
            }
            Action::BoostExpertise if self.tab == Tab::Industries => {
                let selected = self.factories(game).nth(self.factory);
                if let Some((site, factory)) = selected {
                    let simulation = &mut game.simulation;
                    if let Some((index, modifier)) = simulation
                        .modifiers
                        .entries
                        .iter()
                        .enumerate()
                        .find(|(_, modifier)| modifier.identifier == "higher_expertise")
                    {
                        simulation.settlements[site].industries[factory]
                            .apply_modifier(cqsp_core::modifiers::ModifierId(index), modifier);
                    }
                }
            }
            Action::NextFactory | Action::Subsidy(_) | Action::BoostExpertise => {}
            Action::PerCapita => self.per_capita = !self.per_capita,
            Action::ConsumptionCost => self.consumption_cost = !self.consumption_cost,
            Action::ConsumptionSlice(good) => {
                if !self.hidden_consumption.remove(&good) {
                    self.hidden_consumption.insert(good);
                }
            }
            Action::FactoryCount(change) => {
                self.factory_count = self.factory_count.saturating_add(change).clamp(1, 1000)
            }
            Action::ConstructFactory if self.tab == Tab::Construction => {
                self.message = self.province.map_or_else(
                    || "Select a province before constructing a factory.".into(),
                    |province| match game.simulation.construct_factory(
                        province,
                        cqsp_core::catalog::RecipeId(self.recipe),
                        self.factory_count,
                    ) {
                        Ok(_) => format!(
                            "Created {} factory units in {}.",
                            self.factory_count, game.simulation.settlements[province].name
                        ),
                        Err(error) => error.to_string(),
                    },
                );
            }
            Action::ConstructFactory => {}
            Action::ChartGood => cycle(&mut self.good, game.simulation.catalog.goods.len(), false),
            Action::ChartWindow => cycle(&mut self.plot_window, 3, false),
            Action::ChartFit => {}
            Action::NextDay => {
                game.paused = true;
                game.tick_accumulator = 0.0;
                game.simulation.advance_day();
            }
        }
    }
    fn sites<'a>(
        &self,
        game: &'a Game,
    ) -> impl Iterator<Item = (usize, &'a cqsp_core::simulation::Settlement)> {
        let selected = self.province;
        self.province
            .into_iter()
            .map(move |index| (index, &game.simulation.settlements[index]))
            .chain(self.provinces(game).filter(move |_| selected.is_none()))
    }
    fn factories(&self, game: &Game) -> impl Iterator<Item = (usize, usize)> {
        self.sites(game).flat_map(|(index, site)| {
            (0..site.industries.len()).map(move |factory| (index, factory))
        })
    }
    fn provinces<'a>(
        &self,
        game: &'a Game,
    ) -> impl Iterator<Item = (usize, &'a cqsp_core::simulation::Settlement)> {
        let (body, owned, country) = (self.body, self.owned, game.session.player_country);
        game.simulation
            .settlements
            .iter()
            .enumerate()
            .filter(move |(_, site)| {
                site.is_province() && site.body == body && (!owned || site.country == country)
            })
    }
    fn scope(&self, game: &Game) -> String {
        let site = self.province.map_or_else(
            || {
                format!(
                    "{} aggregate scope",
                    game.simulation.solar_system.bodies[self.body.0].name
                )
            },
            |index| game.simulation.settlements[index].name.clone(),
        );
        format!(
            "{} · {} · {}\n{} provinces available for cycling · filtering {}",
            game.simulation.date.display(),
            site,
            if game.paused { "Paused" } else { "Running" },
            self.provinces(game).count(),
            if self.owned {
                "owned provinces"
            } else {
                "all provinces"
            }
        )
    }
    fn content(&self, game: &Game) -> String {
        let simulation = &game.simulation;
        match self.tab {
            Tab::Civilization => self.civilization(game),
            Tab::History => self.history(game),
            Tab::Infrastructure => self.infrastructure(game),
            Tab::Markets => {
                let market = self
                    .province
                    .map_or(&simulation.planetary_markets[&self.body].market, |index| {
                        &simulation.settlements[index].market
                    });
                let mut text = format!(
                    "MARKET · GDP {} · daily deficit {} · trade balance {}\n\n",
                    human_number(market.gdp),
                    human_number(market.last_deficit),
                    human_number(market.last_trade_deficit)
                );
                let start = self.good / 4 * 4;
                for index in start..(start + 4).min(simulation.catalog.goods.len()) {
                    let good = GoodId(index);
                    let _ = writeln!(
                        text,
                        "{}{} · price {} · access {:.0}% · taxation {:.3}\nSupply {} · demand {} · production {} · consumption {}\nS/D {} · trade {} · chronic shortage {:.3}\n",
                        if index == self.good { "> " } else { "" },
                        simulation.catalog.goods[index].name,
                        human_number(market.price[good]),
                        market.market_access[good] * 100.0,
                        market.taxation[good],
                        human_number(market.supply[good]),
                        human_number(market.demand[good]),
                        human_number(market.production[good]),
                        human_number(market.consumption[good]),
                        human_number(market.sd_ratio[good]),
                        human_number(market.trade[good]),
                        market.chronic_shortages[good]
                    );
                }
                let _ = writeln!(
                    text,
                    "Selected good {}/{}: {}",
                    self.good + 1,
                    simulation.catalog.goods.len(),
                    simulation.catalog.goods[self.good].name
                );
                if self.province.is_none() {
                    let history = &simulation.planetary_markets[&self.body].history.0;
                    let _ = write!(
                        text,
                        "{} price history (last eight days): ",
                        simulation.solar_system.bodies[self.body.0].name
                    );
                    for sample in history.iter().skip(history.len().saturating_sub(8)) {
                        let _ = write!(
                            text,
                            "{}: {}  ",
                            sample.date.display(),
                            human_number(sample.price[GoodId(self.good)])
                        );
                    }
                    if history.is_empty() {
                        text.push_str("no daily samples yet");
                    }
                } else {
                    text.push_str(
                        "Historical samples exist for planetary markets, not individual provinces.",
                    );
                }
                text
            }
            Tab::Industries => {
                let sites: Vec<_> = self.sites(game).map(|(_, site)| site).collect();
                let industries: Vec<_> = sites
                    .iter()
                    .flat_map(|site| {
                        site.industries
                            .iter()
                            .map(move |industry| (*site, industry))
                    })
                    .collect();
                let population = sites
                    .iter()
                    .flat_map(|site| &site.populations)
                    .map(|population| population.size)
                    .sum::<u64>();
                let mut text = format!(
                    "INDUSTRIES · {} factories · population {} · page {}\n\n",
                    industries.len(),
                    human_number(population as f64),
                    self.page + 1
                );
                if industries.is_empty() {
                    text.push_str("No industries in this scope.\n");
                }
                for (index, (site, industry)) in
                    industries.iter().enumerate().skip(self.page * 4).take(4)
                {
                    let _ = writeln!(
                        text,
                        "{}{} · {}\n{:?} · capacity {:.1} · utilization {:.1} · expertise {:.4} / {:.1}\nRevenue {} · materials {} · wages {} · profit {}\nTax {} · construction {} · subsidy {:.0}% · power {}\n",
                        if index == self.factory { "> " } else { "" },
                        site.name,
                        simulation.catalog.recipes[industry.recipe.0].name,
                        industry.state,
                        industry.size,
                        industry.utilization,
                        industry.expertise,
                        industry.max_expertise.value(),
                        human_number(industry.revenue),
                        human_number(industry.material_costs),
                        human_number(industry.wage_cost),
                        human_number(industry.profit),
                        human_number(industry.tax_cost),
                        human_number(industry.construction_cost),
                        industry.output_subsidy * 100.0,
                        human_number(industry.power_consumption)
                    );
                }
                if let Some((site, factory)) = self.factories(game).nth(self.factory) {
                    let industry = &simulation.settlements[site].industries[factory];
                    let _ = writeln!(
                        text,
                        "Selected factory {}/{} · expertise gain {:.6} · subsidy payment {}\nWallet {} · balance change {} · maintenance {}",
                        self.factory + 1,
                        industries.len(),
                        industry.expertise_gain.value(),
                        human_number(industry.subsidy_amount),
                        human_number(industry.wallet.balance),
                        human_number(industry.wallet.change),
                        human_number(industry.maintenance)
                    );
                    for &(good, amount) in &industry.workers {
                        let _ = write!(
                            text,
                            "{}: {}  ",
                            simulation.catalog.goods[good.0].name,
                            human_number(amount)
                        );
                    }
                }
                text
            }
            Tab::Demographics => self.demographics(game),
            Tab::Construction => self.construction(game),
            Tab::Science => {
                let science = &simulation.science;
                if science.fields.is_empty() || science.technologies.is_empty() {
                    return "No science fields or technologies loaded.".into();
                }
                let field = &science.fields[self.field];
                let tech = &science.technologies[self.technology];
                let names = |ids: &[cqsp_core::science::FieldId]| {
                    ids.iter()
                        .map(|id| science.fields[id.0].name.as_str())
                        .collect::<Vec<_>>()
                        .join(", ")
                };
                format!(
                    "SCIENCE FIELD {}/{} · {}\n{}\nParents: {}\nAdjacent: {}\n\nTECHNOLOGY {}/{} · {}\n{}\nDifficulty: {} · fields: {}\nActions: {}\n\nCountry research: {}\nThe original has no automatic research progression or unlock system.\nParent/adjacent links are directed; repeated parents remain intact.",
                    self.field + 1,
                    science.fields.len(),
                    field.name,
                    field.description.as_deref().unwrap_or("No description"),
                    names(&field.parents),
                    names(&field.adjacent),
                    self.technology + 1,
                    science.technologies.len(),
                    tech.name,
                    tech.description.as_deref().unwrap_or("No description"),
                    tech.difficulty,
                    names(&tech.fields.iter().copied().collect::<Vec<_>>()),
                    tech.actions.join(", "),
                    game.session
                        .player_country
                        .map_or(0, |country| simulation.countries[country].research)
                )
            }
        }
    }
    pub(super) fn is_demographics(&self) -> bool {
        self.open && self.tab == Tab::Demographics
    }
    fn demographic_segments<'a>(
        &self,
        game: &'a Game,
    ) -> impl Iterator<Item = (&'a Settlement, &'a Population)> {
        self.sites(game).flat_map(|(_, site)| {
            site.populations
                .iter()
                .map(move |population| (site, population))
        })
    }
    fn segment<'a>(
        &self,
        game: &'a Game,
        index: usize,
    ) -> Option<(&'a Settlement, &'a Population)> {
        self.demographic_segments(game).nth(self.page * 2 + index)
    }
    fn consumption_values<'a>(
        &self,
        site: &'a Settlement,
        population: &'a Population,
    ) -> impl Iterator<Item = (GoodId, f64)> + 'a {
        let (per_capita, cost) = (self.per_capita, self.consumption_cost);
        population.consumption.iter().map(move |&(good, amount)| {
            let quantity = if per_capita {
                amount / population.size as f64
            } else {
                amount
            };
            (
                good,
                if cost {
                    quantity * site.market.price[good]
                } else {
                    quantity
                },
            )
        })
    }
    fn pie_title(&self, game: &Game, segment: usize) -> String {
        let Some((site, _)) = self.segment(game, segment) else {
            return String::new();
        };
        let values = self.pie_samples(game, segment);
        let pie = super::pie::Pie::new(&values.iter().map(|&(_, value)| value).collect::<Vec<_>>());
        let details = if let Some(pie) = pie {
            self.pie_hover[segment]
                .and_then(|good| values.iter().position(|&(id, _)| id == good))
                .map_or(
                    "Click legend: hide/show · hover a slice for its value".into(),
                    |index| {
                        format!(
                            "{}: {:.2} · {:.2}%",
                            game.simulation.catalog.goods[values[index].0.0].name,
                            values[index].1,
                            pie.fractions[index] * 100.0
                        )
                    },
                )
        } else if values.is_empty() {
            "No consumption samples yet".into()
        } else {
            "No positive finite pie; invalid/zero values remain in the ledger".into()
        };
        format!(
            "{} · {} · {}\n{}",
            site.name,
            if self.per_capita {
                "per capita"
            } else {
                "whole segment"
            },
            if self.consumption_cost {
                "cost"
            } else {
                "count"
            },
            details
        )
    }
    fn pie_legend(&self, game: &Game, segment: usize, good: GoodId) -> String {
        let Some((site, population)) = self.segment(game, segment) else {
            return String::new();
        };
        let Some((_, value)) = self
            .consumption_values(site, population)
            .find(|&(id, _)| id == good)
        else {
            return String::new();
        };
        format!(
            "[{}] {}: {:.2}",
            if self.hidden_consumption.contains(&good) {
                " "
            } else {
                "x"
            },
            game.simulation.catalog.goods[good.0].name,
            value as f32
        )
    }
    fn pie_samples(&self, game: &Game, segment: usize) -> Vec<(GoodId, f32)> {
        self.segment(game, segment)
            .map_or_else(Vec::new, |(site, population)| {
                self.consumption_values(site, population)
                    .map(|(good, value)| (good, value as f32))
                    .collect()
            })
    }
    fn demographics(&self, game: &Game) -> String {
        let simulation = &game.simulation;
        let groups: Vec<_> = self.demographic_segments(game).collect();
        let people = groups
            .iter()
            .map(|(_, population)| population.size)
            .sum::<u64>();
        let mut text = format!(
            "DEMOGRAPHICS · {} people · {} segments · page {}\n\n",
            human_number(people as f64),
            groups.len(),
            self.page + 1
        );
        if groups.is_empty() {
            text.push_str("No population segments in this scope.");
        }
        for (site, population) in groups.iter().skip(self.page * 2).take(2) {
            let income_per_capita = population.income / population.size as f64;
            let _ = writeln!(
                text,
                "{} · population {} · SOL {}\nSpending {} · per capita {}\nIncome {} · per capita {} · annual per capita {}\nEmployed {} / labor force {} · unemployment {:.2}%\nWallet {} · balance change {} · GDP per capita {}\nEducation {:.0} years · consumer confidence {}",
                site.name,
                human_number(population.size as f64),
                human_number(population.standard_of_living),
                human_number(population.spending),
                human_number(population.spending / population.size as f64),
                human_number(population.income),
                human_number(income_per_capita),
                human_number(income_per_capita * 365.0),
                human_number(population.employed as f64),
                human_number(population.labor_force as f64),
                (1.0 - population.employed as f64 / population.labor_force as f64) * 100.0,
                human_number(population.wallet.balance),
                human_number(population.wallet.change),
                human_number(population.wallet.gdp_change / population.size as f64),
                population.education,
                human_number(population.consumer_confidence)
            );
            let _ = write!(text, "Jobs: ");
            for (&job, &amount) in &population.jobs {
                let _ = write!(
                    text,
                    "{} {}  ",
                    simulation.catalog.jobs[job.0].name,
                    human_number(amount as f64)
                );
            }
            let _ = writeln!(
                text,
                "\nResource consumption · {} · {}",
                if self.per_capita {
                    "per capita"
                } else {
                    "whole segment"
                },
                if self.consumption_cost {
                    "cost"
                } else {
                    "count"
                }
            );
            for (good, value) in self.consumption_values(site, population) {
                let _ = write!(
                    text,
                    "{}: {}  ",
                    simulation.catalog.goods[good.0].name,
                    human_number(value)
                );
            }
            if population.consumption.is_empty() {
                text.push_str("no consumption samples yet");
            }
            text.push_str("\n\n");
        }
        text
    }
    fn construction(&self, game: &Game) -> String {
        let simulation = &game.simulation;
        let Some(recipe) = simulation.catalog.recipes.get(self.recipe) else {
            return "No factory recipes loaded.".into();
        };
        let market = self
            .province
            .map_or(&simulation.planetary_markets[&self.body].market, |index| {
                &simulation.settlements[index].market
            });
        let input = market.price.dot(&recipe.input);
        let capital = market.price.dot(&recipe.capital);
        let revenue = market.price[recipe.output.0] * recipe.output.1;
        let mut text = format!(
            "FACTORY CONSTRUCTION · recipe {}/{}: {}\nUnits to construct: {} (1–1000)\nWorkers per unit: {}\nInput default cost {} · capital default cost {}\nOutput revenue {} · expected profit {}\nConstruction duration {} ticks\n\n",
            self.recipe + 1,
            simulation.catalog.recipes.len(),
            recipe.name,
            self.factory_count,
            recipe
                .workers
                .iter()
                .map(|(_, amount)| *amount as u64)
                .sum::<u64>(),
            human_number(input),
            human_number(capital),
            human_number(revenue),
            human_number(revenue - input - capital),
            recipe.construction_ticks
        );
        for (title, entries) in [
            ("Input", recipe.input.as_slice()),
            ("Capital", recipe.capital.as_slice()),
            ("Output", std::slice::from_ref(&recipe.output)),
            ("Construction cost", recipe.construction_cost.as_slice()),
        ] {
            let _ = write!(text, "{title}: ");
            for &(good, amount) in entries {
                let _ = write!(
                    text,
                    "{} {}  ",
                    simulation.catalog.goods[good.0].name,
                    human_number(amount)
                );
            }
            text.push('\n');
        }
        text.push_str("\nThe original button creates a Steady factory with a zero-level construction record. It does not force Construction state or charge an up-front purchase.\n");
        if let Some(index) = self.province {
            let site = &simulation.settlements[index];
            let _ = writeln!(
                text,
                "{} · {} existing factories · construction capacity {} · used {} · cost {}",
                site.name,
                site.industries.len(),
                site.construction.capacity,
                site.construction.current,
                human_number(site.construction.cost)
            );
        } else {
            text.push_str("Select or hover a province to construct there.\n");
        }
        text.push_str(&self.message);
        text
    }
    fn infrastructure(&self, game: &Game) -> String {
        let simulation = &game.simulation;
        let mut text = format!(
            "INFRASTRUCTURE · page {}\nOriginal power/grid records; no power-production system was implemented.\n\nPOWER-PLANT DEFINITIONS\n",
            self.page + 1
        );
        for recipe in simulation.infrastructure.power_plants.values() {
            let _ = writeln!(
                text,
                "{} · capacity {} W",
                recipe.name,
                human_number(recipe.maximum_production)
            );
            for &(good, amount) in &recipe.consumption {
                let _ = writeln!(
                    text,
                    "  {} {}",
                    simulation.catalog.goods[good.0].name,
                    human_number(amount)
                );
            }
        }
        text.push_str("\nPROVINCE RECORDS\n");
        for (_, site) in self.sites(game).skip(self.page * 4).take(4) {
            let Some(infrastructure) = &site.infrastructure else {
                let _ = writeln!(
                    text,
                    "{} · no CityInfrastructure component (original city loader)",
                    site.name
                );
                continue;
            };
            let grid = infrastructure
                .power_grid
                .as_ref()
                .map_or("No grid binding", |grid| {
                    simulation.infrastructure.grids[grid].name.as_str()
                });
            let _ = writeln!(
                text,
                "{} · transport cost {} · improvement {}\nGrid: {}",
                site.name,
                human_number(infrastructure.purchase_cost),
                human_number(infrastructure.improvement),
                grid
            );
            for plant in &infrastructure.power_plants {
                let _ = writeln!(
                    text,
                    "Plant capacity {} W · production {} · fuel entries {}",
                    human_number(plant.maximum_production),
                    human_number(plant.production),
                    plant.consumption.len()
                );
            }
            for (zone, allocation) in &site.zoning {
                let _ = writeln!(
                    text,
                    "Zoning {} · allocated {} · filled {}",
                    zone, allocation.allocated, allocation.filled
                );
            }
            text.push('\n');
        }
        text.push_str("The source copies plant capacity but not its recipe's fuel consumption. Zoning is metadata; original factory creation does not enforce it.");
        text
    }
    fn governed<'a>(
        &self,
        game: &'a Game,
    ) -> impl Iterator<Item = &'a cqsp_core::simulation::Settlement> {
        let country = game.session.player_country;
        game.simulation
            .settlements
            .iter()
            .filter(move |site| country.is_some() && site.governor == country)
    }
    fn civilization(&self, game: &Game) -> String {
        let Some(country) = game.session.player_country else {
            return "Select a country first.".into();
        };
        let simulation = &game.simulation;
        let economy = &simulation.countries[country];
        let capital = economy
            .capital
            .map_or("Not defined by the original assets", |site| {
                simulation.settlements[site].name.as_str()
            });
        let mut text = format!(
            "CIVILIZATION · {}\nCapital: {}\nCurrency reserves: {} · tax income {}\nSpace-program tag: {} · research {}\nGoverned sites {} · initial provinces {} · page {}\n\nGOVERNED SITES (original City Information)\n{}\n\nPROVINCE BALANCES\n",
            game.world.countries[country].name,
            capital,
            economy
                .wallet
                .map_or("No wallet component".into(), |wallet| human_number(
                    wallet.balance
                )),
            human_number(economy.income_taxes + economy.consumption_taxes),
            economy.space_program,
            economy.research,
            self.governed(game).count(),
            economy.provinces.len(),
            self.page + 1,
            self.governed(game)
                .skip(self.page * 8)
                .take(8)
                .map(|site| site.name.as_str())
                .collect::<Vec<_>>()
                .join(", ")
        );
        for &province in economy.provinces.iter().skip(self.page * 8).take(8) {
            let site = &simulation.settlements[province];
            let market = &site.market;
            let _ = writeln!(
                text,
                "{} · deficit {} · cumulative {}\nTrade deficit {} · cumulative trade {}",
                site.name,
                human_number(market.last_deficit),
                human_number(market.deficit),
                human_number(market.last_trade_deficit),
                human_number(market.trade_deficit)
            );
        }
        text.push_str("\nMISSION QUEUE\n");
        for mission in &economy.missions {
            let mission = &simulation.missions[mission.0];
            let _ = writeln!(
                text,
                "{} -> {} · {}",
                mission.name,
                simulation.settlements[mission.province].name,
                if mission.in_progress {
                    "In progress (marker retained)"
                } else {
                    "Awaiting dispatch"
                }
            );
        }
        if economy.missions.is_empty() {
            text.push_str("No missions queued.\n");
        }
        text.push_str("\nThe original budget breakdown and research tab were unimplemented. Population history is global, not a country-only series.");
        text
    }
    pub fn is_history(&self) -> bool {
        self.open && self.tab == Tab::History
    }
    fn history_samples(&self, game: &Game) -> Vec<(usize, f64)> {
        let series = Series::ALL[self.series];
        let simulation = &game.simulation;
        let mut samples: Vec<_> = if series.global() {
            simulation
                .history
                .iter()
                .enumerate()
                .map(|(index, sample)| {
                    (
                        index,
                        match series {
                            Series::Population => sample.population as f64,
                            Series::Sol => sample.standard_of_living,
                            Series::Employment => sample.employed as f64,
                            Series::EmploymentRate => {
                                sample.employed as f64 / sample.population as f64 * 100.0
                            }
                            _ => unreachable!(),
                        },
                    )
                })
                .collect()
        } else {
            simulation.planetary_markets[&self.body]
                .history
                .0
                .iter()
                .enumerate()
                .map(|(index, sample)| {
                    (
                        index,
                        match series {
                            Series::Gdp => sample.gdp,
                            Series::Price => sample.price[GoodId(self.good)],
                            Series::Supply => sample.supply[GoodId(self.good)],
                            Series::Demand => sample.demand[GoodId(self.good)],
                            _ => unreachable!(),
                        },
                    )
                })
                .collect()
        };
        let limit = [usize::MAX, 128, 32][self.plot_window];
        if samples.len() > limit {
            samples.drain(..samples.len() - limit);
        }
        samples
    }
    fn history(&self, game: &Game) -> String {
        let samples = self.history_samples(game);
        let series = Series::ALL[self.series];
        let range =
            super::chart::bounds(&samples).map_or("No daily samples yet.".into(), |range| {
                format!(
                    "X: daily sample {}–{} · Y: {}–{}\nLatest: {}",
                    range.first,
                    range.last,
                    human_number(range.low),
                    human_number(range.high),
                    human_number(samples.last().unwrap().1)
                )
            });
        let cursor = self.plot_cursor.map_or(String::new(), |(sample, value)| {
            format!(" · cursor X {sample:.2}, Y {}", human_number(value))
        });
        let view = self.plot_view.map_or(String::new(), |view| {
            format!(
                "\nView X {:.2}–{:.2} · Y {}–{}",
                view.first,
                view.last,
                human_number(view.low),
                human_number(view.high)
            )
        });
        format!(
            "HISTORY · {}\n{} · selected good: {}\n{} samples shown · range {}\n{}{}{}\nLeft-drag: pan · wheel: cursor-centered zoom · right-click / Fit: reset\nTick means daily sample index. Non-finite samples leave gaps.\n",
            series.name(),
            if series.global() {
                "Global population history"
            } else {
                game.simulation.solar_system.bodies[self.body.0]
                    .name
                    .as_str()
            },
            game.simulation.catalog.goods[self.good].name,
            samples.len(),
            ["All", "Last 128", "Last 32"][self.plot_window],
            range,
            cursor,
            view,
        )
    }
    fn reset_plot(&mut self) {
        self.plot_view = None;
        self.plot_drag = None;
        self.plot_cursor = None;
    }
    fn chart_view(&self, game: &Game) -> Option<super::chart::View> {
        self.plot_view
            .or_else(|| super::chart::View::fit(&self.history_samples(game)))
    }
    pub(super) fn zoom_plot(&mut self, game: &Game, anchor: (f64, f64), factor: f64) {
        if let Some(view) = self.chart_view(game) {
            self.plot_view = Some(view.zoom(anchor, factor));
        }
    }
    pub(super) fn pan_plot(&mut self, game: &Game, delta: (f64, f64)) {
        if let Some(view) = self.chart_view(game) {
            self.plot_view = Some(view.pan(delta));
        }
    }
}

pub(super) fn chart_controls(
    mut inspector: ResMut<Inspector>,
    game: Res<Game>,
    buttons: Res<ButtonInput<MouseButton>>,
    scroll: Res<AccumulatedMouseScroll>,
    plots: Query<&RelativeCursorPosition, With<PlotImage>>,
) {
    if !inspector.is_history() {
        if inspector.plot_drag.is_some()
            || inspector.plot_cursor.is_some()
            || inspector.plot_pointer_over
        {
            inspector.plot_drag = None;
            inspector.plot_cursor = None;
            inspector.plot_pointer_over = false;
        }
        return;
    }
    let cursor = plots.iter().next();
    let position = cursor.and_then(|cursor| cursor.normalized).map(|point| {
        let (x, y) = super::chart::plot_position((point.x as f64 + 0.5, point.y as f64 + 0.5));
        Vec2::new(x as f32, y as f32)
    });
    let hovered = cursor.is_some_and(|cursor| cursor.cursor_over)
        && position
            .is_some_and(|point| (0.0..=1.0).contains(&point.x) && (0.0..=1.0).contains(&point.y));
    if inspector.plot_pointer_over != hovered {
        inspector.plot_pointer_over = hovered;
    }
    if hovered && buttons.just_pressed(MouseButton::Right) {
        inspector.reset_plot();
    } else if let Some(position) = position {
        if !buttons.pressed(MouseButton::Left) {
            if inspector.plot_drag.is_some() {
                inspector.plot_drag = None;
            }
        } else if buttons.just_pressed(MouseButton::Left) && hovered {
            inspector.plot_drag = Some(position);
        } else if let Some(previous) = inspector.plot_drag {
            let delta = position - previous;
            if delta != Vec2::ZERO {
                inspector.pan_plot(&game, (delta.x as f64, delta.y as f64));
                inspector.plot_drag = Some(position);
            }
        }
        if hovered && scroll.delta.y != 0.0 {
            let units = if scroll.unit == MouseScrollUnit::Line {
                scroll.delta.y
            } else {
                scroll.delta.y / 32.0
            };
            let factor = (-0.15 * units as f64).exp().clamp(0.1, 10.0);
            inspector.zoom_plot(&game, (position.x as f64, position.y as f64), factor);
        }
    }
    if !buttons.pressed(MouseButton::Left) && inspector.plot_drag.is_some() {
        inspector.plot_drag = None;
    }
    let cursor = if hovered {
        position
            .zip(inspector.chart_view(&game))
            .map(|(point, view)| view.value_at((point.x as f64, point.y as f64)))
    } else {
        None
    };
    if inspector.plot_cursor != cursor {
        inspector.plot_cursor = cursor;
    }
}

pub(super) fn update_plot(
    inspector: Res<Inspector>,
    game: Res<Game>,
    mut images: ResMut<Assets<Image>>,
    mut plots: Query<(&mut PlotImage, &mut ImageNode, &mut Node)>,
) {
    for (mut plot, mut node, mut layout) in &mut plots {
        let shown = inspector.is_history();
        let display = if shown { Display::Flex } else { Display::None };
        if layout.display != display {
            layout.display = display;
        }
        if !shown {
            continue;
        }
        let series = Series::ALL[inspector.series];
        let samples = if series.global() {
            game.simulation.history.len()
        } else {
            game.simulation.planetary_markets[&inspector.body]
                .history
                .0
                .len()
        };
        let key = PlotKey {
            body: inspector.body,
            good: inspector.good,
            series,
            window: inspector.plot_window,
            samples,
            view: inspector.plot_view,
        };
        if plot.key == Some(key) {
            continue;
        }
        let image = super::rgba_image(
            super::chart::WIDTH,
            super::chart::HEIGHT,
            super::chart::render(&inspector.history_samples(&game), inspector.plot_view),
        );
        if plot.key.is_some()
            && let Some(mut existing) = images.get_mut(&node.image)
        {
            *existing = image;
        } else {
            node.image = images.add(image);
        }
        plot.key = Some(key);
    }
}

pub(super) fn pie_controls(
    game: Res<Game>,
    mut inspector: ResMut<Inspector>,
    plots: Query<(&PieImage, &RelativeCursorPosition)>,
) {
    let mut hovered = [None; 2];
    if inspector.is_demographics() {
        for (image, cursor) in &plots {
            let Some(point) = cursor.normalized.filter(|_| cursor.cursor_over) else {
                continue;
            };
            let values = inspector.pie_samples(&game, image.segment);
            let samples: Vec<_> = values.iter().map(|&(_, value)| value).collect();
            hovered[image.segment] = super::pie::Pie::new(&samples)
                .and_then(|pie| pie.at((point.x as f64 + 0.5, point.y as f64 + 0.5)))
                .map(|index| values[index].0)
                .filter(|good| !inspector.hidden_consumption.contains(good));
        }
    }
    if inspector.pie_hover != hovered {
        inspector.pie_hover = hovered;
    }
}

pub(super) fn update_pies(
    inspector: Res<Inspector>,
    game: Res<Game>,
    mut images: ResMut<Assets<Image>>,
    mut cards: Query<(&PieCard, &mut Node), Without<PieLegend>>,
    mut legends: Query<(&PieLegend, &mut Node, &mut BackgroundColor), Without<PieCard>>,
    mut plots: Query<(&mut PieImage, &mut ImageNode)>,
) {
    let values: [Vec<(GoodId, f32)>; 2] =
        std::array::from_fn(|segment| inspector.pie_samples(&game, segment));
    for (card, mut node) in &mut cards {
        let display = if inspector.is_demographics() && inspector.segment(&game, card.0).is_some() {
            Display::Flex
        } else {
            Display::None
        };
        if node.display != display {
            node.display = display;
        }
    }
    for (legend, mut node, mut background) in &mut legends {
        let index = values[legend.segment]
            .iter()
            .position(|&(good, _)| good == legend.good);
        let display = if index.is_some() {
            Display::Flex
        } else {
            Display::None
        };
        if node.display != display {
            node.display = display;
        }
        if let Some(index) = index {
            let [r, g, b, a] = if inspector.hidden_consumption.contains(&legend.good) {
                super::pie::BACKGROUND
            } else {
                super::pie::color(index)
            };
            let color = Color::srgba_u8(r, g, b, a);
            if background.0 != color {
                background.0 = color;
            }
        }
    }
    if !inspector.is_demographics() {
        return;
    }
    for (mut plot, mut node) in &mut plots {
        if inspector.segment(&game, plot.segment).is_none() {
            continue;
        }
        let key: Vec<_> = values[plot.segment]
            .iter()
            .map(|&(good, value)| {
                (
                    good,
                    value.to_bits(),
                    inspector.hidden_consumption.contains(&good),
                )
            })
            .collect();
        if plot.key.as_ref() == Some(&key) {
            continue;
        }
        let samples: Vec<_> = values[plot.segment]
            .iter()
            .map(|&(_, value)| value)
            .collect();
        let hidden: Vec<_> = key.iter().map(|&(_, _, hidden)| hidden).collect();
        let image = super::rgba_image(
            super::pie::SIZE,
            super::pie::SIZE,
            super::pie::render(&samples, &hidden),
        );
        if plot.key.is_some()
            && let Some(mut existing) = images.get_mut(&node.image)
        {
            *existing = image;
        } else {
            node.image = images.add(image);
        }
        plot.key = Some(key);
    }
}

fn cycle(index: &mut usize, count: usize, previous: bool) {
    if count > 0 {
        *index = (*index + if previous { count - 1 } else { 1 }) % count;
    }
}

pub(super) fn spawn(commands: &mut Commands, camera: Entity, font: &Handle<Font>, goods: usize) {
    commands
        .spawn((
            Panel::Launcher,
            Action::Open,
            Button,
            UiTargetCamera(camera),
            Node {
                display: Display::None,
                position_type: PositionType::Absolute,
                left: px(24),
                bottom: px(76),
                padding: UiRect::all(px(12)),
                ..default()
            },
            BackgroundColor(Color::srgb(0.15, 0.29, 0.43)),
        ))
        .with_children(|button| {
            button.spawn((
                Text::new("Economy, construction & science (I)"),
                ui_font(font, 16.0),
            ));
        });
    commands
        .spawn((
            Panel::Overlay,
            UiTargetCamera(camera),
            GlobalZIndex(10),
            Node {
                display: Display::None,
                position_type: PositionType::Absolute,
                left: px(40),
                right: px(40),
                top: px(24),
                bottom: px(24),
                padding: UiRect::all(px(20)),
                flex_direction: FlexDirection::Column,
                row_gap: px(10),
                overflow: Overflow::scroll_y(),
                ..default()
            },
            BackgroundColor(Color::srgb(0.025, 0.05, 0.09)),
        ))
        .with_children(|panel| {
            panel.spawn((
                Text::new("NATIVE INSPECTION · LIVE SIMULATION DATA"),
                Node {
                    flex_shrink: 0.0,
                    ..default()
                },
                ui_font(font, 22.0),
                TextColor(Color::srgb(0.9, 0.76, 0.42)),
            ));
            row(panel, |bar| {
                for (tab, name) in [
                    (Tab::Civilization, "Civilization"),
                    (Tab::Markets, "Markets"),
                    (Tab::History, "History"),
                    (Tab::Demographics, "Demographics"),
                    (Tab::Industries, "Industries"),
                    (Tab::Construction, "Construction"),
                    (Tab::Infrastructure, "Infrastructure"),
                    (Tab::Science, "Science"),
                ] {
                    button(bar, font, Action::Tab(tab), name);
                }
                button(bar, font, Action::NextDay, "Advance day");
                button(bar, font, Action::Close, "Close (I / Esc)");
            });
            row(panel, |bar| {
                for (action, name) in [
                    (Action::Planet, "Next planet"),
                    (Action::PreviousProvince, "Previous province"),
                    (Action::NextProvince, "Next province"),
                    (Action::Hovered, "Hovered province"),
                    (Action::Ownership, "Owned / all"),
                ] {
                    button(bar, font, action, name);
                }
            });
            panel.spawn((
                Label::Scope,
                Text::default(),
                ui_font(font, 15.0),
                Node {
                    flex_shrink: 0.0,
                    ..default()
                },
            ));
            row(panel, |bar| {
                button(bar, font, Action::Previous, "Previous item / page");
                button(bar, font, Action::Next, "Next item / page");
                button(bar, font, Action::NextTechnology, "Next technology");
            });
            panel
                .spawn((
                    Panel::FactoryActions,
                    Node {
                        flex_direction: FlexDirection::Row,
                        flex_wrap: FlexWrap::Wrap,
                        flex_shrink: 0.0,
                        column_gap: px(10),
                        row_gap: px(6),
                        ..default()
                    },
                ))
                .with_children(|bar| {
                    button(bar, font, Action::NextFactory, "Select next factory");
                    button(bar, font, Action::Subsidy(-0.1), "Subsidy -10%");
                    button(bar, font, Action::Subsidy(0.1), "Subsidy +10%");
                    button(bar, font, Action::BoostExpertise, "Boost expertise");
                });
            panel
                .spawn((
                    Panel::DemographicActions,
                    Node {
                        flex_direction: FlexDirection::Row,
                        flex_shrink: 0.0,
                        column_gap: px(10),
                        ..default()
                    },
                ))
                .with_children(|bar| {
                    button(bar, font, Action::PerCapita, "Per capita / whole segment");
                    button(
                        bar,
                        font,
                        Action::ConsumptionCost,
                        "Consumption count / cost",
                    );
                });
            panel
                .spawn((
                    Panel::ConstructionActions,
                    Node {
                        flex_direction: FlexDirection::Row,
                        flex_wrap: FlexWrap::Wrap,
                        flex_shrink: 0.0,
                        column_gap: px(10),
                        row_gap: px(6),
                        ..default()
                    },
                ))
                .with_children(|bar| {
                    for (change, title) in [
                        (-10, "Units -10"),
                        (-1, "Units -1"),
                        (1, "Units +1"),
                        (10, "Units +10"),
                    ] {
                        button(bar, font, Action::FactoryCount(change), title);
                    }
                    button(bar, font, Action::ConstructFactory, "Construct factory");
                });
            panel
                .spawn((
                    Panel::HistoryActions,
                    Node {
                        flex_direction: FlexDirection::Row,
                        flex_shrink: 0.0,
                        column_gap: px(10),
                        ..default()
                    },
                ))
                .with_children(|bar| {
                    button(bar, font, Action::ChartGood, "Next chart good");
                    button(bar, font, Action::ChartWindow, "All / last 128 / last 32");
                    button(bar, font, Action::ChartFit, "Fit chart");
                });
            panel
                .spawn((
                    Panel::ConsumptionPlots,
                    Node {
                        display: Display::None,
                        flex_direction: FlexDirection::Row,
                        flex_wrap: FlexWrap::Wrap,
                        column_gap: px(16),
                        row_gap: px(12),
                        flex_shrink: 0.0,
                        ..default()
                    },
                ))
                .with_children(|plots| {
                    for segment in 0..2 {
                        plots
                            .spawn((
                                PieCard(segment),
                                Node {
                                    display: Display::None,
                                    flex_direction: FlexDirection::Column,
                                    row_gap: px(6),
                                    flex_basis: px(420),
                                    flex_grow: 1.0,
                                    min_width: px(0),
                                    ..default()
                                },
                            ))
                            .with_children(|card| {
                                card.spawn((
                                    Label::ConsumptionTitle(segment),
                                    Text::default(),
                                    ui_font(font, 14.0),
                                ));
                                card.spawn(Node {
                                    flex_direction: FlexDirection::Row,
                                    column_gap: px(8),
                                    ..default()
                                })
                                .with_children(|row| {
                                    row.spawn((
                                        PieImage { segment, key: None },
                                        RelativeCursorPosition::default(),
                                        ImageNode::default(),
                                        Node {
                                            width: px(200),
                                            height: px(200),
                                            flex_shrink: 0.0,
                                            ..default()
                                        },
                                    ));
                                    row.spawn(Node {
                                        flex_direction: FlexDirection::Column,
                                        row_gap: px(2),
                                        flex_grow: 1.0,
                                        min_width: px(0),
                                        ..default()
                                    })
                                    .with_children(|legend| {
                                        for index in 0..goods {
                                            let good = GoodId(index);
                                            legend
                                                .spawn((
                                                    PieLegend { segment, good },
                                                    Action::ConsumptionSlice(good),
                                                    Button,
                                                    Node {
                                                        display: Display::None,
                                                        padding: UiRect::all(px(3)),
                                                        ..default()
                                                    },
                                                    BackgroundColor(Color::BLACK),
                                                ))
                                                .with_children(|entry| {
                                                    entry.spawn((
                                                        Label::ConsumptionGood { segment, good },
                                                        Text::default(),
                                                        ui_font(font, 12.0),
                                                    ));
                                                });
                                        }
                                    });
                                });
                            });
                    }
                });
            panel.spawn((
                Label::Content,
                Text::default(),
                ui_font(font, 16.0),
                Node {
                    flex_shrink: 0.0,
                    ..default()
                },
            ));
            panel.spawn((
                PlotImage::default(),
                RelativeCursorPosition::default(),
                ImageNode::default(),
                Node {
                    display: Display::None,
                    width: percent(100),
                    height: px(super::chart::HEIGHT),
                    flex_shrink: 0.0,
                    ..default()
                },
            ));
        });
}

pub(super) fn controls(
    keyboard: Res<ButtonInput<KeyCode>>,
    buttons: Query<(&Action, &Interaction), Changed<Interaction>>,
    mut inspector: ResMut<Inspector>,
    mut ports: ResMut<spaceport_ui::SpaceportUi>,
    mut game: ResMut<Game>,
) {
    if inspector.capture_input != inspector.open {
        inspector.capture_input = inspector.open;
    }
    if game.session.player_country.is_none() {
        inspector.open = false;
        inspector.capture_input = false;
        return;
    }
    if ports.editing() {
        return;
    }
    if keyboard.just_pressed(KeyCode::KeyI) {
        inspector.open = !inspector.open;
    }
    if inspector.open && keyboard.just_pressed(KeyCode::Escape) {
        inspector.open = false;
        return;
    }
    if keyboard.just_pressed(KeyCode::KeyP) {
        inspector.open = false;
    }
    for (action, interaction) in &buttons {
        if *interaction == Interaction::Pressed
            && (inspector.open || matches!(action, Action::Open))
        {
            inspector.apply(*action, &mut game);
        }
    }
    if inspector.open {
        if !inspector.capture_input {
            inspector.capture_input = true;
        }
        if ports.open {
            ports
                .apply(spaceport_ui::Action::Close, &mut game)
                .expect("Close port overlay");
        }
    }
}

pub(super) fn scroll(
    inspector: Res<Inspector>,
    mouse: Res<AccumulatedMouseScroll>,
    mut panels: Query<(&Panel, &mut ScrollPosition)>,
) {
    if !inspector.open || inspector.plot_pointer_over || mouse.delta.y == 0.0 {
        return;
    }
    let scale = if mouse.unit == MouseScrollUnit::Line {
        32.0
    } else {
        1.0
    };
    for (panel, mut position) in &mut panels {
        if matches!(panel, Panel::Overlay) {
            position.0.y = (position.0.y - mouse.delta.y * scale).max(0.0);
        }
    }
}

pub(super) fn update(
    inspector: Res<Inspector>,
    ports: Res<spaceport_ui::SpaceportUi>,
    game: Res<Game>,
    mut panels: Query<(Entity, &Panel, &mut Node)>,
    children: Query<&Children>,
    mut texts: Query<(&mut Text, Option<&Label>)>,
) {
    for (entity, panel, mut node) in &mut panels {
        let shown = match panel {
            Panel::Overlay => inspector.open,
            Panel::FactoryActions => {
                inspector.open
                    && inspector.tab == Tab::Industries
                    && inspector.factories(&game).next().is_some()
            }
            Panel::DemographicActions => inspector.open && inspector.tab == Tab::Demographics,
            Panel::ConstructionActions => inspector.open && inspector.tab == Tab::Construction,
            Panel::HistoryActions => inspector.is_history(),
            Panel::ConsumptionPlots => {
                inspector.is_demographics() && inspector.segment(&game, 0).is_some()
            }
            Panel::Launcher => {
                game.session.player_country.is_some() && !ports.open && !inspector.open
            }
        };
        let display = if shown { Display::Flex } else { Display::None };
        if node.display != display {
            node.display = display;
        }
        if matches!(panel, Panel::Overlay)
            && inspector.open
            && (inspector.is_changed() || game.is_changed())
        {
            // Refresh sibling glyphs together, including static header/button text.
            for child in children.iter_descendants(entity) {
                if let Ok((mut text, label)) = texts.get_mut(child) {
                    if let Some(label) = label {
                        text.0 = match label {
                            Label::Scope => inspector.scope(&game),
                            Label::Content => inspector.content(&game),
                            Label::ConsumptionTitle(segment) => {
                                inspector.pie_title(&game, *segment)
                            }
                            Label::ConsumptionGood { segment, good } => {
                                inspector.pie_legend(&game, *segment, *good)
                            }
                        };
                    } else {
                        text.set_changed();
                    }
                }
            }
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use anyhow::Result;
    use bevy::input::keyboard::KeyboardInput;
    use cqsp_core::{Session, World, clock::DAY, default_data_path, simulation::Simulation};

    fn app() -> Result<App> {
        let data = default_data_path();
        let world = World::load(&data)?;
        let simulation = Simulation::load(&data, &world, 42)?;
        let mut session = Session::default();
        session.select(
            world
                .countries
                .iter()
                .position(|country| country.identifier == "usa")
                .unwrap(),
        );
        session.start();
        let mut app = App::new();
        app.insert_resource(Inspector::new(&simulation))
            .insert_resource(spaceport_ui::SpaceportUi::new(&simulation))
            .insert_resource(super::super::Navigation::new(&simulation.solar_system))
            .insert_resource(super::super::Smoke {
                enabled: false,
                stage: 0,
                elapsed: 0.0,
                captures: 0,
            })
            .insert_resource(Game {
                world,
                simulation,
                session,
                hovered: None,
                political: true,
                ordered_countries: Vec::new(),
                paused: true,
                speed_index: 0,
                tick_accumulator: 0.0,
            })
            .init_resource::<Time>()
            .init_resource::<Assets<Image>>()
            .init_resource::<ButtonInput<KeyCode>>()
            .init_resource::<ButtonInput<MouseButton>>()
            .init_resource::<AccumulatedMouseScroll>()
            .add_message::<KeyboardInput>()
            .add_systems(
                Update,
                (
                    controls,
                    chart_controls,
                    pie_controls,
                    spaceport_ui::controls,
                    super::super::simulation_controls,
                    super::super::selection_controls.run_if(super::super::world_input),
                    scroll,
                    spaceport_ui::update,
                    update,
                    update_plot,
                    update_pies,
                )
                    .chain(),
            );
        Ok(app)
    }
    fn apply(app: &mut App, action: Action) {
        app.world_mut()
            .resource_scope(|world, mut inspector: Mut<Inspector>| {
                inspector.apply(action, &mut world.resource_mut::<Game>());
            });
    }
    fn key(app: &mut App, code: KeyCode) {
        app.world_mut()
            .resource_mut::<ButtonInput<KeyCode>>()
            .press(code);
        app.update();
        app.world_mut()
            .resource_mut::<ButtonInput<KeyCode>>()
            .reset_all();
    }

    #[test]
    fn overlay_captures_shortcuts_and_closing_frame_without_leaving_game() -> Result<()> {
        let mut app = app()?;
        key(&mut app, KeyCode::KeyI);
        assert!(app.world().resource::<Inspector>().open);
        key(&mut app, KeyCode::Space);
        key(&mut app, KeyCode::KeyN);
        key(&mut app, KeyCode::Enter);
        assert!(app.world().resource::<Game>().paused);
        assert_eq!(app.world().resource::<Game>().simulation.date.tick(), 0);
        key(&mut app, KeyCode::Escape);
        assert!(!app.world().resource::<Inspector>().open);
        assert!(app.world().resource::<Inspector>().capture_input);
        assert!(
            app.world()
                .resource::<Game>()
                .session
                .player_country
                .is_some()
        );
        app.update();
        assert!(!app.world().resource::<Inspector>().capture_input);
        key(&mut app, KeyCode::Escape);
        assert!(
            app.world()
                .resource::<Game>()
                .session
                .player_country
                .is_none()
        );
        Ok(())
    }

    #[test]
    fn native_day_button_fires_once_and_refreshes_live_labels() -> Result<()> {
        let mut app = app()?;
        let content = app
            .world_mut()
            .spawn((Label::Content, Text::default()))
            .id();
        let scope = app.world_mut().spawn((Label::Scope, Text::default())).id();
        app.world_mut()
            .spawn((Panel::Overlay, Node::default()))
            .add_children(&[scope, content]);
        apply(&mut app, Action::Open);
        app.update();
        assert!(
            app.world()
                .get::<Text>(content)
                .unwrap()
                .contains("Selected good 1/56")
        );
        let before = app.world().get::<Text>(scope).unwrap().0.clone();
        app.world_mut()
            .spawn((Action::NextDay, Interaction::Pressed));
        app.update();
        app.update();
        assert_eq!(app.world().resource::<Game>().simulation.date.tick(), DAY);
        assert_eq!(
            app.world()
                .resource::<Game>()
                .simulation
                .planetary_markets
                .values()
                .next()
                .unwrap()
                .history
                .0
                .len(),
            2 // Includes the original tick-zero sample.
        );
        assert_ne!(app.world().get::<Text>(scope).unwrap().0, before);
        assert!(
            !app.world()
                .get::<Text>(content)
                .unwrap()
                .contains("no daily samples yet")
        );
        Ok(())
    }

    #[test]
    fn native_panels_are_exclusive_and_vehicle_editing_is_not_interrupted() -> Result<()> {
        use spaceport_ui::{Action as PortAction, Field, SpaceportUi};
        let mut app = app()?;
        key(&mut app, KeyCode::KeyI);
        key(&mut app, KeyCode::KeyP);
        assert!(!app.world().resource::<Inspector>().open);
        assert!(app.world().resource::<SpaceportUi>().open);
        key(&mut app, KeyCode::KeyI);
        assert!(app.world().resource::<Inspector>().open);
        assert!(!app.world().resource::<SpaceportUi>().open);
        key(&mut app, KeyCode::KeyP);
        app.world_mut()
            .resource_scope(|world, mut ports: Mut<SpaceportUi>| {
                ports
                    .apply(
                        PortAction::Edit(Field::Name),
                        &mut world.resource_mut::<Game>(),
                    )
                    .unwrap();
            });
        key(&mut app, KeyCode::KeyI);
        assert!(app.world().resource::<SpaceportUi>().editing());
        assert!(!app.world().resource::<Inspector>().open);
        Ok(())
    }

    #[test]
    fn province_filters_and_hovered_scope_expose_actual_market_and_industry_data() -> Result<()> {
        let mut app = app()?;
        apply(&mut app, Action::NextProvince);
        let province = app.world().resource::<Inspector>().province.unwrap();
        let game = app.world().resource::<Game>();
        assert_eq!(
            game.simulation.settlements[province].country,
            game.session.player_country
        );
        assert!(game.simulation.settlements[province].is_province());
        let industrial = game
            .simulation
            .settlements
            .iter()
            .position(|site| !site.industries.is_empty())
            .unwrap();
        let name = game.simulation.settlements[industrial].name.clone();
        let expected = game.simulation.settlements[industrial].market.price[GoodId(0)];
        app.world_mut().resource_mut::<Game>().hovered = Some(industrial);
        apply(&mut app, Action::Hovered);
        let inspector = app.world().resource::<Inspector>();
        let game = app.world().resource::<Game>();
        assert!(inspector.scope(game).contains(&name));
        assert!(
            inspector
                .content(game)
                .contains(&format!("price {}", human_number(expected)))
        );
        apply(&mut app, Action::Tab(Tab::Industries));
        assert!(
            app.world()
                .resource::<Inspector>()
                .content(app.world().resource::<Game>())
                .contains(&name)
        );
        apply(&mut app, Action::Ownership);
        assert!(app.world().resource::<Inspector>().province.is_none());
        assert_eq!(
            app.world()
                .resource::<Inspector>()
                .provinces(app.world().resource::<Game>())
                .count(),
            1067
        );
        let moon = app
            .world()
            .resource::<Game>()
            .simulation
            .solar_system
            .body_id("moon")
            .unwrap();
        while app.world().resource::<Inspector>().body != moon {
            apply(&mut app, Action::Planet);
        }
        assert_eq!(
            app.world()
                .resource::<Inspector>()
                .provinces(app.world().resource::<Game>())
                .count(),
            250
        );
        apply(&mut app, Action::Ownership);
        apply(&mut app, Action::NextProvince);
        assert!(app.world().resource::<Inspector>().province.is_none());
        Ok(())
    }

    #[test]
    fn every_good_field_and_technology_is_reachable_and_cycles() -> Result<()> {
        let mut app = app()?;
        for _ in 0..56 {
            apply(&mut app, Action::Next);
        }
        assert_eq!(app.world().resource::<Inspector>().good, 0);
        apply(&mut app, Action::Previous);
        assert_eq!(app.world().resource::<Inspector>().good, 55);
        assert!(
            app.world()
                .resource::<Inspector>()
                .content(app.world().resource::<Game>())
                .contains("Selected good 56/56")
        );
        apply(&mut app, Action::Tab(Tab::Science));
        for index in 0..11 {
            let inspector = app.world().resource::<Inspector>();
            let game = app.world().resource::<Game>();
            assert_eq!(inspector.field, index);
            assert!(
                inspector
                    .content(game)
                    .contains(&game.simulation.science.fields[index].name)
            );
            apply(&mut app, Action::Next);
        }
        for index in 0..7 {
            let inspector = app.world().resource::<Inspector>();
            let game = app.world().resource::<Game>();
            assert_eq!(inspector.technology, index);
            assert!(
                inspector
                    .content(game)
                    .contains(&game.simulation.science.technologies[index].name)
            );
            apply(&mut app, Action::NextTechnology);
        }
        assert_eq!(app.world().resource::<Inspector>().field, 0);
        assert_eq!(app.world().resource::<Inspector>().technology, 0);
        assert_eq!(
            app.world()
                .resource::<Game>()
                .simulation
                .countries
                .iter()
                .map(|country| country.research)
                .sum::<i32>(),
            0
        );
        Ok(())
    }

    #[test]
    fn factory_controls_preserve_original_subsidy_and_duplicate_modifier_rules() -> Result<()> {
        let mut app = app()?;
        apply(&mut app, Action::Open);
        apply(&mut app, Action::Tab(Tab::Industries));
        apply(&mut app, Action::Ownership);
        let inspector = app.world().resource::<Inspector>();
        let ids: Vec<_> = inspector
            .factories(app.world().resource::<Game>())
            .collect();
        assert_eq!(ids.len(), 20);
        let (site, factory) = ids[0];
        app.world_mut()
            .spawn((Action::BoostExpertise, Interaction::Pressed));
        app.update();
        app.update();
        assert_eq!(
            app.world().resource::<Game>().simulation.settlements[site].industries[factory]
                .max_expertise
                .value(),
            2.5
        );
        apply(&mut app, Action::BoostExpertise);
        assert_eq!(
            app.world().resource::<Game>().simulation.settlements[site].industries[factory]
                .max_expertise
                .value(),
            3.5
        );
        for _ in 0..15 {
            apply(&mut app, Action::Subsidy(0.1));
        }
        assert_eq!(
            app.world().resource::<Game>().simulation.settlements[site].industries[factory]
                .output_subsidy,
            1.0
        );
        for _ in 0..15 {
            apply(&mut app, Action::Subsidy(-0.1));
        }
        assert_eq!(
            app.world().resource::<Game>().simulation.settlements[site].industries[factory]
                .output_subsidy,
            0.0
        );
        apply(&mut app, Action::NextFactory);
        apply(&mut app, Action::Subsidy(0.1));
        let (next_site, next_factory) = ids[1];
        assert_eq!(
            app.world().resource::<Game>().simulation.settlements[next_site].industries
                [next_factory]
                .output_subsidy,
            0.1_f32 as f64
        );
        for _ in 1..20 {
            apply(&mut app, Action::NextFactory);
        }
        assert_eq!(app.world().resource::<Inspector>().factory, 0);
        apply(&mut app, Action::Tab(Tab::Science));
        apply(&mut app, Action::BoostExpertise);
        assert_eq!(
            app.world().resource::<Game>().simulation.settlements[site].industries[factory]
                .max_expertise
                .value(),
            3.5
        );
        apply(&mut app, Action::Planet);
        apply(&mut app, Action::Tab(Tab::Industries));
        apply(&mut app, Action::BoostExpertise);
        assert_eq!(
            app.world()
                .resource::<Inspector>()
                .factories(app.world().resource::<Game>())
                .count(),
            0
        );
        Ok(())
    }

    #[test]
    fn demographics_cycles_segments_and_prices_without_mutating_the_population() -> Result<()> {
        let mut app = app()?;
        apply(&mut app, Action::Open);
        apply(&mut app, Action::Tab(Tab::Demographics));
        apply(&mut app, Action::Ownership);
        apply(&mut app, Action::NextDay);
        let game = app.world().resource::<Game>();
        let inspector = app.world().resource::<Inspector>();
        let (province, _) = inspector
            .sites(game)
            .find(|(_, site)| !site.populations.is_empty())
            .unwrap();
        let pages = inspector
            .sites(game)
            .map(|(_, site)| site.populations.len())
            .sum::<usize>()
            .div_ceil(2);
        assert!(pages > 1);
        let people = game.simulation.summary(None).population;
        for _ in 0..pages {
            apply(&mut app, Action::Next);
        }
        assert_eq!(app.world().resource::<Inspector>().page, 0);
        app.world_mut().resource_mut::<Game>().hovered = Some(province);
        apply(&mut app, Action::Hovered);
        let game = app.world().resource::<Game>();
        let population = &game.simulation.settlements[province].populations[0];
        let (good, quantity) = population.consumption[0];
        let per_capita_cost = quantity / population.size as f64
            * game.simulation.settlements[province].market.price[good];
        let name = game.simulation.catalog.goods[good.0].name.clone();
        let text = app.world().resource::<Inspector>().content(game);
        assert!(text.contains("annual per capita"));
        assert!(text.contains("Education"));
        assert!(text.contains(&format!("{name}: {}", human_number(per_capita_cost))));
        apply(&mut app, Action::PerCapita);
        apply(&mut app, Action::ConsumptionCost);
        let game = app.world().resource::<Game>();
        let text = app.world().resource::<Inspector>().content(game);
        assert!(text.contains("whole segment · count"));
        assert!(text.contains(&format!("{name}: {}", human_number(quantity))));
        assert_eq!(game.simulation.summary(None).population, people);
        apply(&mut app, Action::Planet);
        assert!(
            app.world()
                .resource::<Inspector>()
                .content(app.world().resource::<Game>())
                .contains("No population segments")
        );
        Ok(())
    }

    #[test]
    fn consumption_pies_keep_source_float_values_hover_hidden_angles_and_owned_images() -> Result<()>
    {
        let mut app = app()?;
        let cards: Vec<_> = (0..2)
            .map(|segment| {
                app.world_mut()
                    .spawn((PieCard(segment), Node::default()))
                    .id()
            })
            .collect();
        let plots: Vec<_> = (0..2)
            .map(|segment| {
                app.world_mut()
                    .spawn((
                        PieImage { segment, key: None },
                        RelativeCursorPosition::default(),
                        ImageNode::default(),
                    ))
                    .id()
            })
            .collect();
        apply(&mut app, Action::Open);
        apply(&mut app, Action::Tab(Tab::Demographics));
        apply(&mut app, Action::Ownership);
        apply(&mut app, Action::NextDay);
        app.update();
        let handles: Vec<_> = plots
            .iter()
            .map(|&plot| app.world().get::<ImageNode>(plot).unwrap().image.clone())
            .collect();
        assert_eq!(app.world().resource::<Assets<Image>>().len(), 2);
        for &card in &cards {
            assert_eq!(
                app.world().get::<Node>(card).unwrap().display,
                Display::Flex
            );
        }
        let game = app.world().resource::<Game>();
        let inspector = app.world().resource::<Inspector>();
        let (site, population) = inspector.segment(game, 0).unwrap();
        let before_population = format!("{population:?}");
        let samples = inspector.pie_samples(game, 0);
        let (good, amount) = population.consumption[0];
        let price = site.market.price[good];
        let size = population.size as f64;
        assert_eq!(samples[0], (good, (amount / size * price) as f32));
        let fractions =
            crate::pie::Pie::new(&samples.iter().map(|&(_, value)| value).collect::<Vec<_>>())
                .unwrap()
                .fractions;
        let angle = std::f64::consts::FRAC_PI_2 + fractions[0] * std::f64::consts::PI;
        app.world_mut()
            .get_mut::<RelativeCursorPosition>(plots[0])
            .unwrap()
            .normalized = Some(Vec2::new(
            (angle.cos() * 0.2) as f32,
            (-angle.sin() * 0.2) as f32,
        ));
        app.world_mut()
            .get_mut::<RelativeCursorPosition>(plots[0])
            .unwrap()
            .cursor_over = true;
        app.update();
        let inspector = app.world().resource::<Inspector>();
        assert_eq!(inspector.pie_hover[0], Some(good));
        assert!(
            inspector
                .pie_title(app.world().resource::<Game>(), 0)
                .contains(&format!("{:.2}%", fractions[0] * 100.0))
        );
        let image_data = |app: &App| {
            app.world()
                .resource::<Assets<Image>>()
                .get(&handles[0])
                .unwrap()
                .data
                .clone()
        };
        let shown = image_data(&app);
        let button = app
            .world_mut()
            .spawn((Action::ConsumptionSlice(good), Interaction::Pressed))
            .id();
        app.update();
        let hidden = image_data(&app);
        assert_ne!(hidden, shown);
        assert!(
            app.world()
                .resource::<Inspector>()
                .hidden_consumption
                .contains(&good)
        );
        assert_eq!(app.world().resource::<Inspector>().pie_hover[0], None);
        app.update();
        assert_eq!(image_data(&app), hidden); // Unchanged pressed state must not toggle again.
        *app.world_mut().get_mut::<Interaction>(button).unwrap() = Interaction::None;
        app.update();
        *app.world_mut().get_mut::<Interaction>(button).unwrap() = Interaction::Pressed;
        app.update();
        assert_eq!(image_data(&app), shown);
        for (action, expected) in [
            (Action::ConsumptionCost, (amount / size) as f32),
            (Action::PerCapita, amount as f32),
            (Action::ConsumptionCost, (amount * price) as f32),
        ] {
            apply(&mut app, action);
            app.update();
            let inspector = app.world().resource::<Inspector>();
            let game = app.world().resource::<Game>();
            assert_eq!(inspector.pie_samples(game, 0)[0], (good, expected));
            assert_eq!(
                format!("{:?}", inspector.segment(game, 0).unwrap().1),
                before_population
            );
        }
        for _ in 0..20 {
            apply(&mut app, Action::Next);
            app.update();
        }
        for (index, &plot) in plots.iter().enumerate() {
            assert_eq!(
                app.world().get::<ImageNode>(plot).unwrap().image,
                handles[index]
            );
        }
        assert_eq!(app.world().resource::<Assets<Image>>().len(), 2);
        apply(&mut app, Action::Planet);
        app.update();
        for &card in &cards {
            assert_eq!(
                app.world().get::<Node>(card).unwrap().display,
                Display::None
            );
        }
        assert_eq!(app.world().resource::<Inspector>().pie_hover, [None; 2]);
        apply(&mut app, Action::Close);
        app.update();
        assert_eq!(app.world().resource::<Assets<Image>>().len(), 2);
        Ok(())
    }

    #[test]
    fn construction_button_fires_once_and_all_recipes_and_count_bounds_are_reachable() -> Result<()>
    {
        let mut app = app()?;
        apply(&mut app, Action::Open);
        apply(&mut app, Action::Tab(Tab::Construction));
        apply(&mut app, Action::ConstructFactory);
        assert!(
            app.world()
                .resource::<Inspector>()
                .message
                .contains("Select a province")
        );
        let province = app
            .world()
            .resource::<Game>()
            .simulation
            .settlements
            .iter()
            .position(|site| !site.populations.is_empty())
            .unwrap();
        app.world_mut().resource_mut::<Game>().hovered = Some(province);
        apply(&mut app, Action::Hovered);
        for recipe in 0..22 {
            let inspector = app.world().resource::<Inspector>();
            assert_eq!(inspector.recipe, recipe);
            assert!(
                inspector
                    .content(app.world().resource::<Game>())
                    .contains(&format!("recipe {}/22", recipe + 1))
            );
            apply(&mut app, Action::Next);
        }
        assert_eq!(app.world().resource::<Inspector>().recipe, 0);
        apply(&mut app, Action::FactoryCount(i32::MAX));
        assert_eq!(app.world().resource::<Inspector>().factory_count, 1000);
        apply(&mut app, Action::FactoryCount(i32::MIN));
        assert_eq!(app.world().resource::<Inspector>().factory_count, 1);
        apply(&mut app, Action::FactoryCount(26));
        let before = app.world().resource::<Game>().simulation.settlements[province]
            .industries
            .len();
        app.world_mut()
            .spawn((Action::ConstructFactory, Interaction::Pressed));
        app.update();
        app.update();
        let game = app.world().resource::<Game>();
        let industries = &game.simulation.settlements[province].industries;
        assert_eq!(industries.len(), before + 1);
        assert_eq!(industries.last().unwrap().size, 27.0);
        assert_eq!(
            industries
                .last()
                .unwrap()
                .construction
                .as_ref()
                .unwrap()
                .levels,
            0
        );
        assert!(
            app.world()
                .resource::<Inspector>()
                .message
                .contains("Created 27")
        );
        let count = industries.len();
        apply(&mut app, Action::Tab(Tab::Markets));
        apply(&mut app, Action::ConstructFactory);
        assert_eq!(
            app.world().resource::<Game>().simulation.settlements[province]
                .industries
                .len(),
            count
        );
        Ok(())
    }

    #[test]
    fn scrolling_moves_only_the_visible_overlay() -> Result<()> {
        let mut app = app()?;
        let overlay = app
            .world_mut()
            .spawn((Panel::Overlay, ScrollPosition::default()))
            .id();
        let launcher = app
            .world_mut()
            .spawn((Panel::Launcher, ScrollPosition::default()))
            .id();
        app.world_mut()
            .resource_mut::<AccumulatedMouseScroll>()
            .delta
            .y = -2.0;
        app.update();
        assert_eq!(app.world().get::<ScrollPosition>(overlay).unwrap().0.y, 0.0);
        apply(&mut app, Action::Open);
        app.update();
        assert_eq!(
            app.world().get::<ScrollPosition>(overlay).unwrap().0.y,
            64.0
        );
        assert_eq!(
            app.world().get::<ScrollPosition>(launcher).unwrap().0.y,
            0.0
        );
        Ok(())
    }

    #[test]
    fn civilization_pages_use_governors_and_initial_province_lists() -> Result<()> {
        let mut app = app()?;
        apply(&mut app, Action::Tab(Tab::Civilization));
        let game = app.world().resource::<Game>();
        let country = game.session.player_country.unwrap();
        let economy = &game.simulation.countries[country];
        let capital = economy.capital.unwrap();
        let capital_name = game.simulation.settlements[capital].name.clone();
        let text = app.world().resource::<Inspector>().content(game);
        assert!(text.contains(&format!("Capital: {capital_name}")));
        assert!(text.contains("Space-program tag: true"));
        let governed: Vec<_> = game
            .simulation
            .settlements
            .iter()
            .filter(|site| site.governor == Some(country))
            .map(|site| site.name.clone())
            .collect();
        let pages = governed.len().max(economy.provinces.len()).div_ceil(8);
        // Source colony delivery changes Province.country, not Governed or CountryCityList.
        app.world_mut()
            .resource_mut::<Game>()
            .simulation
            .settlements[capital]
            .country = None;
        let mut contents = String::new();
        for _ in 0..pages {
            contents.push_str(
                &app.world()
                    .resource::<Inspector>()
                    .content(app.world().resource::<Game>()),
            );
            apply(&mut app, Action::Next);
        }
        assert!(governed.iter().all(|name| contents.contains(name)));
        assert!(contents.contains(&format!("{capital_name} · deficit")));
        assert_eq!(app.world().resource::<Inspector>().page, 0);
        app.world_mut()
            .resource_mut::<Game>()
            .session
            .player_country = None;
        assert_eq!(
            app.world()
                .resource::<Inspector>()
                .content(app.world().resource::<Game>()),
            "Select a country first."
        );
        Ok(())
    }

    #[test]
    fn infrastructure_inspection_exposes_original_bindings_without_changing_them() -> Result<()> {
        let mut app = app()?;
        let game = app.world().resource::<Game>();
        let illinois = game
            .simulation
            .settlements
            .iter()
            .position(|site| site.identifier == "illinois")
            .unwrap();
        let city = game
            .simulation
            .settlements
            .iter()
            .position(|site| !site.is_province())
            .unwrap();
        app.world_mut().resource_mut::<Game>().hovered = Some(illinois);
        apply(&mut app, Action::Hovered);
        apply(&mut app, Action::Tab(Tab::Infrastructure));
        let text = app
            .world()
            .resource::<Inspector>()
            .content(app.world().resource::<Game>());
        assert!(text.contains("Illinois"));
        assert!(text.contains("capacity 100.00M W"));
        assert!(text.contains("allocated 100000 · filled 0"));
        assert!(text.contains("fuel entries 0"));
        apply(&mut app, Action::Next);
        assert_eq!(app.world().resource::<Inspector>().page, 0);
        app.world_mut().resource_mut::<Game>().hovered = Some(city);
        apply(&mut app, Action::Hovered);
        assert!(
            app.world()
                .resource::<Inspector>()
                .content(app.world().resource::<Game>())
                .contains("no CityInfrastructure component")
        );
        assert_eq!(
            app.world().resource::<Game>().simulation.settlements[illinois]
                .infrastructure
                .as_ref()
                .unwrap()
                .power_plants[0]
                .production,
            0.0
        );
        Ok(())
    }

    #[test]
    fn history_series_goods_bodies_and_windows_preserve_original_samples() -> Result<()> {
        use cqsp_core::{clock::StarDate, simulation::Summary};
        let mut app = app()?;
        {
            let mut game = app.world_mut().resource_mut::<Game>();
            game.simulation.history = (0..140)
                .map(|index| Summary {
                    population: 100 + index,
                    employed: 20 + index,
                    standard_of_living: index as f64 / 10.0,
                    ..default()
                })
                .collect();
            for market in game.simulation.planetary_markets.values_mut() {
                market.history.0.clear();
                for index in 0..140 {
                    market.market.gdp = index as f64 * 2.0;
                    market.market.price[GoodId(0)] = index as f64 * 3.0;
                    market.market.supply[GoodId(0)] = index as f64 * 4.0;
                    market.market.demand[GoodId(0)] = index as f64 * 5.0;
                    market
                        .history
                        .record(StarDate::from_tick(index * DAY), &market.market);
                }
            }
        }
        apply(&mut app, Action::Tab(Tab::History));
        let expected = [
            278.0,
            417.0,
            556.0,
            695.0,
            239.0,
            13.9,
            159.0,
            159.0 / 239.0 * 100.0,
        ];
        for (series, value) in expected.into_iter().enumerate() {
            let inspector = app.world().resource::<Inspector>();
            let game = app.world().resource::<Game>();
            assert_eq!(inspector.series, series);
            let samples = inspector.history_samples(game);
            assert_eq!(samples.len(), 140);
            assert_eq!(samples.last(), Some(&(139, value)));
            assert!(inspector.content(game).contains(Series::ALL[series].name()));
            apply(&mut app, Action::Next);
        }
        assert_eq!(app.world().resource::<Inspector>().series, 0);
        for (window, first, count) in [(1, 12, 128), (2, 108, 32), (0, 0, 140)] {
            apply(&mut app, Action::ChartWindow);
            let inspector = app.world().resource::<Inspector>();
            assert_eq!(inspector.plot_window, window);
            let samples = inspector.history_samples(app.world().resource::<Game>());
            assert_eq!(samples.len(), count);
            assert_eq!(samples[0].0, first);
        }
        for good in 0..56 {
            assert_eq!(app.world().resource::<Inspector>().good, good);
            apply(&mut app, Action::ChartGood);
        }
        assert_eq!(app.world().resource::<Inspector>().good, 0);
        let start = app.world().resource::<Inspector>().body;
        let mut bodies = std::collections::BTreeSet::new();
        for _ in 0..11 {
            bodies.insert(app.world().resource::<Inspector>().body);
            apply(&mut app, Action::Planet);
        }
        assert_eq!(bodies.len(), 11);
        assert_eq!(app.world().resource::<Inspector>().body, start);
        Ok(())
    }

    #[test]
    fn history_mouse_zoom_pan_fit_and_selection_reset_do_not_scroll_or_mutate_game() -> Result<()> {
        let mut app = app()?;
        apply(&mut app, Action::Open);
        apply(&mut app, Action::Tab(Tab::History));
        for _ in 0..40 {
            apply(&mut app, Action::NextDay);
        }
        let plot = app
            .world_mut()
            .spawn((
                PlotImage::default(),
                ImageNode::default(),
                Node::default(),
                RelativeCursorPosition {
                    cursor_over: true,
                    normalized: Some(Vec2::ZERO),
                },
            ))
            .id();
        let panel = app
            .world_mut()
            .spawn((
                Panel::Overlay,
                Node::default(),
                ScrollPosition(Vec2::new(0.0, 80.0)),
            ))
            .id();
        app.update();
        let handle = app.world().get::<ImageNode>(plot).unwrap().image.clone();
        let initial = {
            let inspector = app.world().resource::<Inspector>();
            super::super::chart::View::fit(
                &inspector.history_samples(app.world().resource::<Game>()),
            )
            .unwrap()
        };
        let date = app.world().resource::<Game>().simulation.date;
        app.world_mut()
            .resource_mut::<AccumulatedMouseScroll>()
            .delta
            .y = 1.0;
        app.update();
        let zoomed = app.world().resource::<Inspector>().plot_view.unwrap();
        assert!(zoomed.last - zoomed.first < initial.last - initial.first);
        assert!(
            app.world()
                .resource::<Inspector>()
                .content(app.world().resource::<Game>())
                .contains("cursor X")
        );
        assert_eq!(app.world().get::<ScrollPosition>(panel).unwrap().0.y, 80.0);
        app.world_mut()
            .resource_mut::<AccumulatedMouseScroll>()
            .delta = Vec2::ZERO;
        app.world_mut()
            .resource_mut::<ButtonInput<MouseButton>>()
            .press(MouseButton::Left);
        app.update();
        app.world_mut()
            .resource_mut::<ButtonInput<MouseButton>>()
            .clear();
        app.world_mut()
            .get_mut::<RelativeCursorPosition>(plot)
            .unwrap()
            .normalized = Some(Vec2::new(0.1, 0.1));
        app.update();
        let panned = app.world().resource::<Inspector>().plot_view.unwrap();
        assert!(panned.first < zoomed.first);
        assert!(panned.low > zoomed.low);
        app.world_mut()
            .get_mut::<RelativeCursorPosition>(plot)
            .unwrap()
            .cursor_over = false;
        app.world_mut()
            .get_mut::<RelativeCursorPosition>(plot)
            .unwrap()
            .normalized = Some(Vec2::new(0.8, 0.1));
        app.update();
        assert!(app.world().resource::<Inspector>().plot_view.unwrap().first < panned.first);
        app.world_mut()
            .resource_mut::<ButtonInput<MouseButton>>()
            .release(MouseButton::Left);
        app.update();
        assert!(app.world().resource::<Inspector>().plot_drag.is_none());
        assert!(app.world().resource::<Inspector>().plot_cursor.is_none());
        assert_eq!(
            app.world().resource::<Game>().simulation.date.tick(),
            date.tick()
        );
        assert_eq!(app.world().get::<ImageNode>(plot).unwrap().image, handle);
        assert_eq!(app.world().resource::<Assets<Image>>().len(), 1);
        app.world_mut()
            .get_mut::<RelativeCursorPosition>(plot)
            .unwrap()
            .cursor_over = true;
        app.world_mut()
            .get_mut::<RelativeCursorPosition>(plot)
            .unwrap()
            .normalized = Some(Vec2::ZERO);
        app.world_mut()
            .resource_mut::<ButtonInput<MouseButton>>()
            .press(MouseButton::Right);
        app.update();
        assert!(app.world().resource::<Inspector>().plot_view.is_none());
        app.world_mut()
            .resource_mut::<ButtonInput<MouseButton>>()
            .reset_all();
        for action in [
            Action::Next,
            Action::ChartGood,
            Action::ChartWindow,
            Action::Planet,
            Action::ChartFit,
        ] {
            app.world_mut().resource_mut::<Inspector>().plot_view = Some(zoomed);
            apply(&mut app, action);
            assert!(app.world().resource::<Inspector>().plot_view.is_none());
        }
        apply(&mut app, Action::Close);
        app.update();
        assert!(!app.world().resource::<Inspector>().plot_pointer_over);
        Ok(())
    }

    #[test]
    fn history_plot_reuses_its_owned_image_and_hides_when_closed() -> Result<()> {
        let mut app = app()?;
        let plot = app
            .world_mut()
            .spawn((PlotImage::default(), ImageNode::default(), Node::default()))
            .id();
        apply(&mut app, Action::Open);
        apply(&mut app, Action::Tab(Tab::History));
        app.update();
        let handle = app.world().get::<ImageNode>(plot).unwrap().image.clone();
        assert_eq!(app.world().resource::<Assets<Image>>().len(), 1);
        assert_eq!(
            app.world().get::<Node>(plot).unwrap().display,
            Display::Flex
        );
        let before = app
            .world()
            .resource::<Assets<Image>>()
            .get(&handle)
            .unwrap()
            .data
            .clone();
        app.update();
        assert_eq!(
            app.world()
                .resource::<Assets<Image>>()
                .get(&handle)
                .unwrap()
                .data,
            before
        );
        apply(&mut app, Action::NextDay);
        app.update();
        assert_ne!(
            app.world()
                .resource::<Assets<Image>>()
                .get(&handle)
                .unwrap()
                .data,
            before
        );
        for _ in 0..16 {
            apply(&mut app, Action::Next);
            app.update();
        }
        assert_eq!(app.world().get::<ImageNode>(plot).unwrap().image, handle);
        assert_eq!(app.world().resource::<Assets<Image>>().len(), 1);
        apply(&mut app, Action::Close);
        app.update();
        assert_eq!(
            app.world().get::<Node>(plot).unwrap().display,
            Display::None
        );
        Ok(())
    }
}
