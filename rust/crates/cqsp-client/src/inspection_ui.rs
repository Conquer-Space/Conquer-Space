//! Native economy/science inspection and original factory controls.
use super::{Game, human_number, spaceport_ui, ui_button as button, ui_font, ui_row as row};
use bevy::{
    input::mouse::{AccumulatedMouseScroll, MouseScrollUnit},
    prelude::*,
};
use cqsp_core::{economy::GoodId, solar_system::BodyId};
use std::fmt::Write;

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub(super) enum Tab {
    Markets,
    Demographics,
    Industries,
    Construction,
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
    FactoryCount(i32),
    ConstructFactory,
    NextDay,
}
#[derive(Component)]
pub(super) enum Panel {
    Launcher,
    Overlay,
    FactoryActions,
    DemographicActions,
    ConstructionActions,
}
#[derive(Component)]
pub(super) enum Label {
    Scope,
    Content,
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
    recipe: usize,
    factory_count: i32,
    message: String,
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
            recipe: 0,
            factory_count: 1,
            message: String::new(),
        }
    }
    pub fn apply(&mut self, action: Action, game: &mut Game) {
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
                if self.province.is_none()
                    && Some(self.body) == simulation.solar_system.body_id("earth")
                {
                    let history = &simulation.market_history.0;
                    let _ = write!(text, "Earth price history (last eight days): ");
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
                        "Historical samples currently exist only for the Earth planetary market.",
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
    fn demographics(&self, game: &Game) -> String {
        let simulation = &game.simulation;
        let groups: Vec<_> = self
            .sites(game)
            .flat_map(|(_, site)| {
                site.populations
                    .iter()
                    .map(move |population| (site, population))
            })
            .collect();
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
            for &(good, amount) in &population.consumption {
                let quantity = if self.per_capita {
                    amount / population.size as f64
                } else {
                    amount
                };
                let value = if self.consumption_cost {
                    quantity * site.market.price[good]
                } else {
                    quantity
                };
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
}

fn cycle(index: &mut usize, count: usize, previous: bool) {
    if count > 0 {
        *index = (*index + if previous { count - 1 } else { 1 }) % count;
    }
}

pub(super) fn spawn(commands: &mut Commands, camera: Entity, font: &Handle<Font>) {
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
                    (Tab::Markets, "Markets"),
                    (Tab::Demographics, "Demographics"),
                    (Tab::Industries, "Industries"),
                    (Tab::Construction, "Construction"),
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
            panel.spawn((
                Label::Content,
                Text::default(),
                ui_font(font, 16.0),
                Node {
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
    if !inspector.open || mouse.delta.y == 0.0 {
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
            .init_resource::<ButtonInput<KeyCode>>()
            .init_resource::<AccumulatedMouseScroll>()
            .add_message::<KeyboardInput>()
            .add_systems(
                Update,
                (
                    controls,
                    spaceport_ui::controls,
                    super::super::simulation_controls,
                    super::super::selection_controls.run_if(super::super::world_input),
                    scroll,
                    spaceport_ui::update,
                    update,
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
                .market_history
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
        apply(&mut app, Action::Planet);
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
}
