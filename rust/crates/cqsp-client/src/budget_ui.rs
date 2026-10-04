//! Native economy subsidy/labor windows and the separate absolute-tax table.
use super::{Action as Inspect, Game, Inspector, Panel, Tab, button, ui_font};
use bevy::{prelude::*, ui::RelativeCursorPosition};
use cqsp_core::{catalog::RecipeId, economy::GoodId, number_display::float};
use std::fmt::Write;

pub(super) const TAX_PAGE_SIZE: usize = 8;
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq)]
pub(crate) enum View {
    #[default]
    Subsidies,
    Labor,
}
#[derive(Clone, Copy)]
pub(crate) enum Action {
    View(View),
    Add,
    Close,
    Recipe(RecipeId),
    Full,
    Apply,
    Remove(RecipeId),
}
#[derive(Clone, Copy, PartialEq, Eq)]
enum Policy {
    Tax(usize),
    Subsidy,
}
impl Policy {
    fn maximum(self) -> f32 {
        if self == Self::Subsidy { 1.0 } else { 5.0 }
    }
}
#[derive(Component)]
pub(crate) struct Slider(Policy);
#[derive(Component)]
pub(crate) struct Fill(Policy);
#[derive(Component)]
pub(crate) struct ValueLabel(Policy);
#[derive(Component)]
pub(crate) enum Section {
    Tabs,
    Subsidies,
    Editor,
    TaxRow(usize),
    SubsidyRow(RecipeId),
}
pub(super) struct Budget {
    taxes: Vec<f32>,
    amount: f32,
    full: bool,
    view: View,
    adding: bool,
    selected: Option<RecipeId>,
    drag: Option<Policy>,
    message: String,
}
impl Budget {
    pub fn new(goods: usize) -> Self {
        Self {
            taxes: vec![0.0; goods],
            amount: 0.0,
            full: false,
            view: View::default(),
            adding: false,
            selected: None,
            drag: None,
            message: String::new(),
        }
    }
    pub fn release(&mut self) {
        self.drag = None;
    }
    pub fn close(&mut self) {
        self.adding = false;
        self.release();
    }
    fn value(&self, policy: Policy, page: usize) -> f32 {
        match policy {
            Policy::Tax(slot) => self
                .taxes
                .get(page * TAX_PAGE_SIZE + slot)
                .copied()
                .unwrap_or(0.0),
            Policy::Subsidy => self.amount,
        }
    }
    fn available(&self, policy: Policy, tab: Tab, page: usize) -> bool {
        match policy {
            Policy::Tax(slot) => {
                tab == Tab::Budget && page * TAX_PAGE_SIZE + slot < self.taxes.len()
            }
            Policy::Subsidy => tab == Tab::Economy && self.adding && !self.full,
        }
    }
    fn subsidy(&self) -> f32 {
        if self.full { 1.0 } else { self.amount }
    }
    fn set(&mut self, policy: Policy, position: f32, game: &mut Game, page: usize) {
        if !position.is_finite() {
            return;
        }
        let amount = position.clamp(0.0, 1.0) * policy.maximum();
        match policy {
            Policy::Tax(slot) => self.set_tax(amount, game, page * TAX_PAGE_SIZE + slot),
            Policy::Subsidy if !self.full => self.amount = amount,
            Policy::Subsidy => {}
        }
    }
    pub fn set_tax(&mut self, amount: f32, game: &mut Game, good: usize) {
        if !amount.is_finite() || good >= self.taxes.len() {
            return;
        }
        let amount = amount.clamp(0.0, 5.0);
        if self.taxes[good] == amount {
            return;
        }
        self.message = match game.simulation.set_good_tax(GoodId(good), amount) {
            Ok(count) => {
                self.taxes[good] = amount;
                format!("Updated {count} province markets.")
            }
            Err(error) => error.to_string(),
        };
    }
    pub fn apply(&mut self, action: Action, game: &mut Game) {
        match action {
            Action::View(view) => {
                if self.view != view {
                    self.close();
                }
                self.view = view;
                return;
            }
            Action::Add if self.view == View::Subsidies => {
                self.adding = !self.adding;
                self.release();
                return;
            }
            Action::Close => {
                self.close();
                return;
            }
            Action::Recipe(recipe)
                if self.adding && recipe.0 < game.simulation.catalog.recipes.len() =>
            {
                self.selected = Some(recipe);
                return;
            }
            Action::Full if self.adding => {
                self.full = !self.full;
                self.release();
                return;
            }
            _ => {}
        }
        let Some(country) = game.session.player_country else {
            return;
        };
        self.message = match action {
            Action::Apply if self.adding => {
                self.close();
                let Some(recipe) = self.selected else {
                    return;
                };
                match game
                    .simulation
                    .apply_subsidy(country, recipe, self.subsidy())
                {
                    Ok(count) => format!(
                        "Applied to {count} existing factories in the initial province list."
                    ),
                    Err(error) => error.to_string(),
                }
            }
            Action::Remove(recipe) if !self.adding && self.view == View::Subsidies => {
                match game.simulation.remove_subsidy(country, recipe) {
                    Ok(true) => {
                        "Removed policy row; existing factory values are retained, as upstream."
                            .into()
                    }
                    Ok(false) => "No recorded subsidy for this recipe.".into(),
                    Err(error) => error.to_string(),
                }
            }
            _ => return,
        };
    }
    pub fn content(&self, game: &Game, tab: Tab, page: usize) -> String {
        let simulation = &game.simulation;
        let Some(country) = game.session.player_country else {
            return "Select a country first.".into();
        };
        let country = &simulation.countries[country];
        let mut text = if tab == Tab::Budget {
            format!(
                "TAXES · page {} / {}\nAbsolute amounts (0–5), not percentages. Changes affect all province markets worldwide, including the Moon; city and planetary markets are excluded.\n",
                page + 1,
                self.taxes.len().div_ceil(TAX_PAGE_SIZE)
            )
        } else if self.adding {
            format!(
                "SUBSIDY · selected: {}\nDraft {:.2}%{}. Apply closes the editor even without a selection.\nExisting matching factories in the initial province list are updated, regardless of current ownership; new factories do not inherit the policy.\n",
                self.selected
                    .map_or("None — choose a recipe", |id| simulation.catalog.recipes
                        [id.0]
                        .name
                        .as_str()),
                self.value(Policy::Subsidy, 0) * 100.0,
                if self.full {
                    " (subsidize all losses)"
                } else {
                    ""
                }
            )
        } else if self.view == View::Labor {
            let mut text =
                "LABOR MARKET · initial country province list\n\nGood · Demand · Supply\n"
                    .to_string();
            for &good in &simulation.catalog.labor_goods {
                let (demand, supply) =
                    country
                        .provinces
                        .iter()
                        .fold((0.0, 0.0), |(demand, supply), &province| {
                            let market = &simulation.settlements[province].market;
                            (demand + market.demand[good], supply + market.supply[good])
                        });
                let _ = writeln!(
                    text,
                    "{} · {} · {}",
                    simulation.catalog.goods[good.0].name,
                    float(demand),
                    float(supply)
                );
            }
            text
        } else {
            "SUBSIDIZE INDUSTRY\nRecorded policies appear above. Removing a row does not clear existing factory subsidy values.\n".into()
        };
        if !self.message.is_empty() {
            let _ = writeln!(text, "\n{}", self.message);
        }
        text
    }
}
fn policy_button(
    panel: &mut ChildSpawnerCommands,
    font: &Handle<Font>,
    action: Action,
    name: &str,
) {
    button(panel, font, Inspect::Policy(action), name);
}
fn column() -> Node {
    Node {
        flex_direction: FlexDirection::Column,
        flex_shrink: 0.0,
        row_gap: px(8),
        ..default()
    }
}
fn bar() -> Node {
    Node {
        flex_wrap: FlexWrap::Wrap,
        column_gap: px(8),
        row_gap: px(6),
        flex_shrink: 0.0,
        ..default()
    }
}
fn slider(panel: &mut ChildSpawnerCommands, font: &Handle<Font>, policy: Policy) {
    panel
        .spawn(Node {
            height: px(28),
            min_height: px(28),
            align_items: AlignItems::Center,
            column_gap: px(10),
            flex_shrink: 0.0,
            ..default()
        })
        .with_children(|row| {
            row.spawn((
                ValueLabel(policy),
                Text::default(),
                ui_font(font, 15.0),
                Node {
                    width: px(360),
                    height: px(24),
                    min_height: px(24),
                    flex_shrink: 0.0,
                    ..default()
                },
            ));
            row.spawn((
                Slider(policy),
                RelativeCursorPosition::default(),
                Node {
                    width: px(400),
                    height: px(24),
                    flex_shrink: 0.0,
                    ..default()
                },
                BackgroundColor(Color::srgb(0.10, 0.19, 0.29)),
            ))
            .with_children(|track| {
                track.spawn((
                    Fill(policy),
                    Node {
                        width: percent(0),
                        height: px(24),
                        ..default()
                    },
                    BackgroundColor(Color::srgb(0.72, 0.58, 0.24)),
                ));
            });
        });
}
pub(super) fn spawn(panel: &mut ChildSpawnerCommands, font: &Handle<Font>, game: &Game) {
    panel
        .spawn((Panel::BudgetActions, column()))
        .with_children(|panel| {
            panel.spawn((Section::Tabs, bar())).with_children(|bar| {
                policy_button(
                    bar,
                    font,
                    Action::View(View::Subsidies),
                    "Subsidize Industry",
                );
                policy_button(bar, font, Action::View(View::Labor), "Labor");
            });
            panel
                .spawn((Section::Subsidies, column()))
                .with_children(|panel| {
                    policy_button(panel, font, Action::Add, "Add Subsidy");
                    panel.spawn((
                        Text::new("Good / recipe · Subsidy amount · Remove"),
                        ui_font(font, 15.0),
                    ));
                    for (index, recipe) in game.simulation.catalog.recipes.iter().enumerate() {
                        let id = RecipeId(index);
                        panel
                            .spawn((Section::SubsidyRow(id), bar()))
                            .with_children(|bar| {
                                bar.spawn((
                                    SubsidyLabel(id),
                                    Text::new(&recipe.name),
                                    ui_font(font, 15.0),
                                    Node {
                                        width: px(520),
                                        ..default()
                                    },
                                ));
                                policy_button(bar, font, Action::Remove(id), "x");
                            });
                    }
                });
            panel
                .spawn((
                    Section::Editor,
                    column(),
                    BackgroundColor(Color::srgb(0.06, 0.10, 0.16)),
                ))
                .with_children(|panel| {
                    panel.spawn((
                        Text::new("ADD SUBSIDY · choose a recipe"),
                        ui_font(font, 18.0),
                    ));
                    panel.spawn(bar()).with_children(|bar| {
                        for (index, recipe) in game.simulation.catalog.recipes.iter().enumerate() {
                            policy_button(bar, font, Action::Recipe(RecipeId(index)), &recipe.name);
                        }
                    });
                    slider(panel, font, Policy::Subsidy);
                    panel.spawn(bar()).with_children(|bar| {
                        policy_button(
                            bar,
                            font,
                            Action::Full,
                            "Full subsidy · subsidize all losses",
                        );
                        policy_button(bar, font, Action::Apply, "Apply Subsidy");
                        policy_button(bar, font, Action::Close, "Close subsidy editor");
                    });
                });
            for slot in 0..TAX_PAGE_SIZE {
                panel
                    .spawn((Section::TaxRow(slot), column()))
                    .with_children(|panel| slider(panel, font, Policy::Tax(slot)));
            }
        });
}
#[derive(Component)]
pub(crate) struct SubsidyLabel(RecipeId);
pub(crate) fn controls(
    buttons: Res<ButtonInput<MouseButton>>,
    sliders: Query<(&Slider, &RelativeCursorPosition)>,
    mut inspector: ResMut<Inspector>,
    mut game: ResMut<Game>,
) {
    if !inspector.open
        || !matches!(inspector.tab, Tab::Budget | Tab::Economy)
        || game.session.player_country.is_none()
        || !buttons.pressed(MouseButton::Left)
    {
        if inspector.budget.drag.is_some() {
            inspector.budget.release();
        }
        return;
    }
    for (slider, cursor) in &sliders {
        if !inspector
            .budget
            .available(slider.0, inspector.tab, inspector.page)
        {
            continue;
        }
        if buttons.just_pressed(MouseButton::Left) && cursor.cursor_over() {
            inspector.budget.drag = Some(slider.0);
        }
        if inspector.budget.drag == Some(slider.0)
            && let Some(position) = cursor.normalized
        {
            let page = inspector.page;
            inspector
                .budget
                .set(slider.0, position.x + 0.5, &mut game, page);
        }
    }
}
pub(crate) fn update(
    inspector: Res<Inspector>,
    game: Res<Game>,
    mut sections: Query<(&Section, &mut Node), Without<Fill>>,
    mut labels: Query<(&ValueLabel, &mut Text), Without<SubsidyLabel>>,
    mut subsidy_labels: Query<(&SubsidyLabel, &mut Text), Without<ValueLabel>>,
    mut fills: Query<(&Fill, &mut Node), Without<Section>>,
) {
    let budget = &inspector.budget;
    let economy = inspector.is_economy();
    let country = game
        .session
        .player_country
        .map(|id| &game.simulation.countries[id]);
    for (section, mut node) in &mut sections {
        let shown = match *section {
            Section::Tabs => economy,
            Section::Subsidies => economy && budget.view == View::Subsidies && !budget.adding,
            Section::Editor => economy && budget.adding,
            Section::TaxRow(slot) => {
                inspector.is_budget() && inspector.page * TAX_PAGE_SIZE + slot < budget.taxes.len()
            }
            Section::SubsidyRow(recipe) => {
                economy
                    && !budget.adding
                    && budget.view == View::Subsidies
                    && country.is_some_and(|country| country.subsidies.contains_key(&recipe))
            }
        };
        let display = if shown { Display::Flex } else { Display::None };
        if node.display != display {
            node.display = display;
        }
    }
    if !inspector.open || !matches!(inspector.tab, Tab::Budget | Tab::Economy) {
        return;
    }
    for (label, mut text) in &mut labels {
        let value = budget.value(label.0, inspector.page);
        let next = match label.0 {
            Policy::Tax(slot) => game
                .simulation
                .catalog
                .goods
                .get(inspector.page * TAX_PAGE_SIZE + slot)
                .map_or(String::new(), |good| format!("{}: {value:.3}", good.name)),
            Policy::Subsidy => format!(
                "Subsidy: {:.2}%{}",
                value * 100.0,
                if budget.full { " (locked full)" } else { "" }
            ),
        };
        if text.0 != next {
            text.0 = next;
        }
    }
    for (label, mut text) in &mut subsidy_labels {
        let amount = country
            .and_then(|country| country.subsidies.get(&label.0))
            .copied()
            .unwrap_or(0.0);
        let next = format!(
            "{} · {}%",
            game.simulation.catalog.recipes[label.0.0].name,
            float(amount * 100.0)
        );
        if text.0 != next {
            text.0 = next;
        }
    }
    for (fill, mut node) in &mut fills {
        let width = percent(100.0 * budget.value(fill.0, inspector.page) / fill.0.maximum());
        if node.width != width {
            node.width = width;
        }
    }
}

#[cfg(test)]
mod tests {
    use super::super::tests::{app, apply};
    use super::*;
    use anyhow::Result;
    fn policy(app: &mut App, action: Action) {
        apply(app, Inspect::Policy(action));
    }
    fn pointer(app: &mut App, entity: Entity, position: Option<Vec2>, over: bool) {
        *app.world_mut()
            .get_mut::<RelativeCursorPosition>(entity)
            .unwrap() = RelativeCursorPosition {
            normalized: position,
            cursor_over: over,
        };
    }
    fn release(app: &mut App) {
        app.world_mut()
            .resource_mut::<ButtonInput<MouseButton>>()
            .reset_all();
        app.update();
    }
    #[test]
    fn every_good_tax_is_independent_and_labor_uses_initial_provinces() -> Result<()> {
        let mut app = app()?;
        apply(&mut app, Inspect::Open);
        apply(&mut app, Inspect::Tab(Tab::Budget));
        for good in 0..56 {
            let amount = (good + 1) as f32 / 20.0;
            apply(
                &mut app,
                Inspect::AbsoluteTax {
                    good: GoodId(good),
                    amount,
                },
            );
            assert_eq!(
                app.world().resource::<Inspector>().budget.taxes[good],
                amount
            );
        }
        for site in &app.world().resource::<Game>().simulation.settlements {
            for good in 0..56 {
                assert_eq!(
                    site.market.taxation[GoodId(good)],
                    if site.is_province() {
                        f64::from((good + 1) as f32 / 20.0)
                    } else {
                        0.0
                    }
                );
            }
        }
        apply(
            &mut app,
            Inspect::AbsoluteTax {
                good: GoodId(usize::MAX),
                amount: 1.0,
            },
        );
        let (country, province, labor) = {
            let game = app.world().resource::<Game>();
            let country = game.session.player_country.unwrap();
            (
                country,
                game.simulation.countries[country].provinces[0],
                game.simulation.catalog.labor_goods[0],
            )
        };
        {
            let mut game = app.world_mut().resource_mut::<Game>();
            game.simulation.countries[country].provinces = vec![province];
            let site = &mut game.simulation.settlements[province];
            site.country = None;
            site.market.demand[labor] = 1234.0;
            site.market.supply[labor] = 5678.0;
        }
        apply(&mut app, Inspect::Tab(Tab::Economy));
        policy(&mut app, Action::View(View::Labor));
        assert!(
            app.world()
                .resource::<Inspector>()
                .budget
                .content(app.world().resource::<Game>(), Tab::Economy, 0)
                .contains("1234 · 5678")
        );
        Ok(())
    }
    #[test]
    fn tax_rows_drag_clamp_release_and_page_without_advancing() -> Result<()> {
        let mut app = app()?;
        app.update();
        let history = app.world().resource::<Game>().simulation.history.len();
        apply(&mut app, Inspect::Open);
        apply(&mut app, Inspect::Tab(Tab::Budget));
        let tax = app
            .world_mut()
            .spawn((Slider(Policy::Tax(3)), RelativeCursorPosition::default()))
            .id();
        let label = app
            .world_mut()
            .spawn((ValueLabel(Policy::Tax(3)), Text::default()))
            .id();
        let fill = app
            .world_mut()
            .spawn((Fill(Policy::Tax(3)), Node::default()))
            .id();
        pointer(&mut app, tax, Some(Vec2::new(-0.25, 0.0)), true);
        app.world_mut()
            .resource_mut::<ButtonInput<MouseButton>>()
            .press(MouseButton::Left);
        app.update();
        assert_eq!(app.world().resource::<Inspector>().budget.taxes[3], 1.25);
        assert_eq!(app.world().get::<Node>(fill).unwrap().width, percent(25));
        assert!(app.world().get::<Text>(label).unwrap().contains("1.250"));
        app.world_mut()
            .resource_mut::<ButtonInput<MouseButton>>()
            .clear();
        for (position, expected) in [(1.0, 5.0), (-1.0, 0.0), (f32::NAN, 0.0)] {
            pointer(&mut app, tax, Some(Vec2::new(position, 0.8)), false);
            app.update();
            assert_eq!(
                app.world().resource::<Inspector>().budget.taxes[3],
                expected
            );
        }
        release(&mut app);
        for page in 0..7 {
            assert_eq!(app.world().resource::<Inspector>().page, page);
            pointer(&mut app, tax, Some(Vec2::ZERO), true);
            app.world_mut()
                .resource_mut::<ButtonInput<MouseButton>>()
                .press(MouseButton::Left);
            app.update();
            assert_eq!(
                app.world().resource::<Inspector>().budget.taxes[page * TAX_PAGE_SIZE + 3],
                2.5
            );
            apply(&mut app, Inspect::Next);
            assert!(app.world().resource::<Inspector>().budget.drag.is_none());
            release(&mut app);
        }
        assert_eq!(app.world().resource::<Inspector>().page, 0);
        apply(&mut app, Inspect::Previous);
        assert_eq!(app.world().resource::<Inspector>().page, 6);
        assert_eq!(app.world().resource::<Game>().simulation.date.tick(), 0);
        assert_eq!(
            app.world().resource::<Game>().simulation.history.len(),
            history
        );
        Ok(())
    }
    #[test]
    fn subsidy_selection_is_explicit_draft_persists_and_apply_closes_once() -> Result<()> {
        let mut app = app()?;
        apply(&mut app, Inspect::Open);
        apply(&mut app, Inspect::Tab(Tab::Economy));
        let country = app
            .world()
            .resource::<Game>()
            .session
            .player_country
            .unwrap();
        let province = app.world().resource::<Game>().simulation.countries[country].provinces[0];
        let factory = app
            .world_mut()
            .resource_mut::<Game>()
            .simulation
            .construct_factory(province, RecipeId(0), 1)?;
        policy(&mut app, Action::Add);
        policy(&mut app, Action::Apply);
        assert!(!app.world().resource::<Inspector>().budget.adding);
        assert!(
            app.world().resource::<Game>().simulation.countries[country]
                .subsidies
                .is_empty()
        );
        policy(&mut app, Action::Add);
        for recipe in 0..22 {
            policy(&mut app, Action::Recipe(RecipeId(recipe)));
            assert_eq!(
                app.world().resource::<Inspector>().budget.selected,
                Some(RecipeId(recipe))
            );
        }
        policy(&mut app, Action::Recipe(RecipeId(usize::MAX)));
        assert_eq!(
            app.world().resource::<Inspector>().budget.selected,
            Some(RecipeId(21))
        );
        policy(&mut app, Action::Recipe(RecipeId(0)));
        let slider = app
            .world_mut()
            .spawn((Slider(Policy::Subsidy), RelativeCursorPosition::default()))
            .id();
        pointer(&mut app, slider, Some(Vec2::new(-0.25, 0.0)), true);
        app.world_mut()
            .resource_mut::<ButtonInput<MouseButton>>()
            .press(MouseButton::Left);
        app.update();
        release(&mut app);
        policy(&mut app, Action::Full);
        pointer(&mut app, slider, Some(Vec2::ZERO), true);
        app.world_mut()
            .resource_mut::<ButtonInput<MouseButton>>()
            .press(MouseButton::Left);
        app.update();
        assert_eq!(app.world().resource::<Inspector>().budget.amount, 0.25);
        assert!(app.world().resource::<Inspector>().budget.drag.is_none());
        release(&mut app);
        policy(&mut app, Action::View(View::Labor));
        assert!(!app.world().resource::<Inspector>().budget.adding);
        assert!(
            app.world()
                .resource::<Inspector>()
                .budget
                .content(app.world().resource::<Game>(), Tab::Economy, 0)
                .contains("LABOR MARKET")
        );
        policy(&mut app, Action::Add);
        assert!(!app.world().resource::<Inspector>().budget.adding);
        policy(&mut app, Action::View(View::Subsidies));
        policy(&mut app, Action::Add);
        assert_eq!(
            app.world().resource::<Inspector>().budget.selected,
            Some(RecipeId(0))
        );
        assert_eq!(
            app.world()
                .resource::<Inspector>()
                .budget
                .value(Policy::Subsidy, 0),
            0.25
        );
        app.world_mut()
            .spawn((Inspect::Policy(Action::Apply), Interaction::Pressed));
        app.update();
        assert!(!app.world().resource::<Inspector>().budget.adding);
        assert_eq!(
            app.world().resource::<Game>().simulation.countries[country].subsidies[&RecipeId(0)],
            1.0
        );
        app.world_mut()
            .resource_mut::<Game>()
            .simulation
            .settlements[province]
            .industries[factory]
            .output_subsidy = 0.2;
        app.update();
        assert_eq!(
            app.world().resource::<Game>().simulation.settlements[province].industries[factory]
                .output_subsidy,
            0.2
        );
        policy(&mut app, Action::Remove(RecipeId(0)));
        assert!(
            app.world().resource::<Game>().simulation.countries[country]
                .subsidies
                .is_empty()
        );
        assert_eq!(
            app.world().resource::<Game>().simulation.settlements[province].industries[factory]
                .output_subsidy,
            0.2
        );
        policy(&mut app, Action::Add);
        apply(&mut app, Inspect::Close);
        assert!(!app.world().resource::<Inspector>().budget.adding);
        assert!(app.world().resource::<Inspector>().budget.full);
        apply(&mut app, Inspect::Tab(Tab::Budget));
        policy(&mut app, Action::Add);
        assert!(!app.world().resource::<Inspector>().budget.adding);
        assert_eq!(app.world().resource::<Game>().simulation.date.tick(), 0);
        Ok(())
    }
}
