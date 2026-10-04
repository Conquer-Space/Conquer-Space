//! Native replacements for the original economy subsidy and absolute-tax windows.
use super::{Action, Game, Inspector, Panel, Tab, button, row, ui_font};
use bevy::{prelude::*, ui::RelativeCursorPosition};
use cqsp_core::{catalog::RecipeId, economy::GoodId};
use std::fmt::Write;

#[derive(Clone, Copy, PartialEq, Eq)]
enum Policy {
    Tax,
    Subsidy,
}
impl Policy {
    fn maximum(self) -> f32 {
        if self == Self::Tax { 5.0 } else { 1.0 }
    }
}
#[derive(Component)]
pub(crate) struct Slider(Policy);
#[derive(Component)]
pub(crate) struct Fill(Policy);
#[derive(Component)]
pub(crate) struct ValueLabel(Policy);

pub(super) struct Budget {
    taxes: Vec<f32>,
    amount: f32,
    full: bool,
    drag: Option<Policy>,
    message: String,
}
impl Budget {
    pub fn new(goods: usize) -> Self {
        Self {
            taxes: vec![0.0; goods],
            amount: 0.0,
            full: false,
            drag: None,
            message: String::new(),
        }
    }
    pub fn release(&mut self) {
        self.drag = None;
    }
    fn value(&self, policy: Policy, good: usize) -> f32 {
        match policy {
            Policy::Tax => self.taxes[good],
            Policy::Subsidy => {
                if self.full {
                    1.0
                } else {
                    self.amount
                }
            }
        }
    }
    fn set(&mut self, policy: Policy, position: f32, game: &mut Game, good: usize) {
        if !position.is_finite() {
            return;
        }
        let amount = position.clamp(0.0, 1.0) * policy.maximum();
        match policy {
            Policy::Tax => self.set_tax(amount, game, good),
            Policy::Subsidy => {
                if !self.full {
                    self.amount = amount;
                }
            }
        }
    }
    pub fn set_tax(&mut self, amount: f32, game: &mut Game, good: usize) {
        if !amount.is_finite() {
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
    pub fn apply(&mut self, action: Action, game: &mut Game, recipe: usize) {
        if matches!(action, Action::FullSubsidy) {
            self.full = !self.full;
            self.release();
            return;
        }
        let Some(country) = game.session.player_country else {
            return;
        };
        self.message = match action {
            Action::ApplySubsidy => match game.simulation.apply_subsidy(
                country,
                RecipeId(recipe),
                self.value(Policy::Subsidy, 0),
            ) {
                Ok(count) => {
                    format!("Applied to {count} existing factories in the initial province list.")
                }
                Err(error) => error.to_string(),
            },
            Action::RemoveSubsidy => match game.simulation.remove_subsidy(country, RecipeId(recipe))
            {
                Ok(true) => {
                    "Removed policy row. Existing factory subsidy values are retained, as upstream."
                        .into()
                }
                Ok(false) => "No recorded subsidy for this recipe.".into(),
                Err(error) => error.to_string(),
            },
            _ => return,
        };
    }
    pub fn content(&self, game: &Game, good: usize, recipe: usize) -> String {
        let simulation = &game.simulation;
        let Some(country) = game.session.player_country else {
            return "Select a country first.".into();
        };
        let country = &simulation.countries[country];
        let mut text = format!(
            "BUDGET / NATIONAL ECONOMY\nTax good {}/{}: {} · absolute amount {:.3}, not a percentage\nTaxes apply globally to all province markets, including other countries and the Moon; city and planetary markets are excluded.\n\nSubsidy recipe {}/{}: {} · chosen {:.2}%{}\nPrevious/next item selects a recipe. Apply targets existing matching factories in the initial country province list, independent of current ownership. New factories do not inherit it.\n\n",
            good + 1,
            simulation.catalog.goods.len(),
            simulation.catalog.goods[good].name,
            self.taxes[good],
            recipe + 1,
            simulation.catalog.recipes.len(),
            simulation.catalog.recipes[recipe].name,
            self.value(Policy::Subsidy, good) * 100.0,
            if self.full { " (full subsidy)" } else { "" }
        );
        text.push_str("Recorded subsidies (removing a row does not clear factory values):\n");
        for (&recipe, &amount) in &country.subsidies {
            let _ = writeln!(
                text,
                "{}: {:.2}%",
                simulation.catalog.recipes[recipe.0].name,
                amount * 100.0
            );
        }
        if country.subsidies.is_empty() {
            text.push_str("None\n");
        }
        text.push_str("\nNATIONAL LABOR · initial province list\n");
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
                "{} · demand {:.2} · supply {:.2}",
                simulation.catalog.goods[good.0].name, demand, supply
            );
        }
        if !self.message.is_empty() {
            let _ = writeln!(text, "\n{}", self.message);
        }
        text
    }
}

pub(super) fn spawn(panel: &mut ChildSpawnerCommands, font: &Handle<Font>) {
    panel
        .spawn((
            Panel::BudgetActions,
            Node {
                flex_direction: FlexDirection::Column,
                flex_shrink: 0.0,
                row_gap: px(8),
                ..default()
            },
        ))
        .with_children(|panel| {
            row(panel, |bar| {
                button(bar, font, Action::TaxGood, "Next tax good");
                button(bar, font, Action::FullSubsidy, "Toggle full subsidy");
                button(bar, font, Action::ApplySubsidy, "Apply national subsidy");
                button(bar, font, Action::RemoveSubsidy, "Remove selected policy");
            });
            for policy in [Policy::Tax, Policy::Subsidy] {
                panel
                    .spawn(Node {
                        height: px(24),
                        min_height: px(24),
                        flex_direction: FlexDirection::Row,
                        align_items: AlignItems::Center,
                        column_gap: px(10),
                        flex_shrink: 0.0,
                        ..default()
                    })
                    .with_children(|bar| {
                        bar.spawn((
                            ValueLabel(policy),
                            Text::default(),
                            ui_font(font, 15.0),
                            Node {
                                width: px(260),
                                ..default()
                            },
                        ));
                        bar.spawn((
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
        });
}
pub(crate) fn controls(
    buttons: Res<ButtonInput<MouseButton>>,
    sliders: Query<(&Slider, &RelativeCursorPosition)>,
    mut inspector: ResMut<Inspector>,
    mut game: ResMut<Game>,
) {
    if !inspector.open || inspector.tab != Tab::Budget || game.session.player_country.is_none() {
        if inspector.budget.drag.is_some() {
            inspector.budget.release();
        }
        return;
    }
    if !buttons.pressed(MouseButton::Left) {
        if inspector.budget.drag.is_some() {
            inspector.budget.release();
        }
        return;
    }
    for (slider, cursor) in &sliders {
        if slider.0 == Policy::Subsidy && inspector.budget.full {
            continue;
        }
        if buttons.just_pressed(MouseButton::Left) && cursor.cursor_over() {
            inspector.budget.drag = Some(slider.0);
        }
        if inspector.budget.drag == Some(slider.0)
            && let Some(position) = cursor.normalized
        {
            let good = inspector.good;
            inspector
                .budget
                .set(slider.0, position.x + 0.5, &mut game, good);
        }
    }
}
pub(crate) fn update(
    inspector: Res<Inspector>,
    mut labels: Query<(&ValueLabel, &mut Text)>,
    mut fills: Query<(&Fill, &mut Node)>,
) {
    if !inspector.open || inspector.tab != Tab::Budget {
        return;
    }
    for (label, mut text) in &mut labels {
        let value = inspector.budget.value(label.0, inspector.good);
        let next = match label.0 {
            Policy::Tax => format!("Absolute tax: {value:.3} (0–5)"),
            Policy::Subsidy => format!(
                "Subsidy: {:.2}%{}",
                value * 100.0,
                if inspector.budget.full {
                    " (locked full)"
                } else {
                    ""
                }
            ),
        };
        if text.0 != next {
            text.0 = next;
        }
    }
    for (fill, mut node) in &mut fills {
        let width =
            percent(100.0 * inspector.budget.value(fill.0, inspector.good) / fill.0.maximum());
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
    fn sliders_capture_outside_drag_clamp_lock_and_release_without_advancing() -> Result<()> {
        let mut app = app()?;
        app.update(); // The client takes its initial tick-zero sample even while paused.
        let history = app.world().resource::<Game>().simulation.history.len();
        apply(&mut app, Action::Open);
        apply(&mut app, Action::Tab(Tab::Budget));
        let tax = app
            .world_mut()
            .spawn((Slider(Policy::Tax), RelativeCursorPosition::default()))
            .id();
        let subsidy = app
            .world_mut()
            .spawn((Slider(Policy::Subsidy), RelativeCursorPosition::default()))
            .id();
        let label = app
            .world_mut()
            .spawn((ValueLabel(Policy::Tax), Text::default()))
            .id();
        let fill = app
            .world_mut()
            .spawn((Fill(Policy::Tax), Node::default()))
            .id();
        pointer(&mut app, tax, Some(Vec2::new(-0.25, 0.0)), true);
        app.world_mut()
            .resource_mut::<ButtonInput<MouseButton>>()
            .press(MouseButton::Left);
        app.update();
        assert_eq!(app.world().resource::<Inspector>().budget.taxes[0], 1.25);
        assert_eq!(app.world().get::<Node>(fill).unwrap().width, percent(25));
        assert!(app.world().get::<Text>(label).unwrap().contains("1.250"));
        app.world_mut()
            .resource_mut::<ButtonInput<MouseButton>>()
            .clear();
        pointer(&mut app, tax, Some(Vec2::new(1.0, 0.8)), false);
        app.update();
        assert_eq!(app.world().resource::<Inspector>().budget.taxes[0], 5.0);
        pointer(&mut app, tax, Some(Vec2::new(-1.0, 0.8)), false);
        app.update();
        assert_eq!(app.world().resource::<Inspector>().budget.taxes[0], 0.0);
        pointer(&mut app, tax, Some(Vec2::new(f32::NAN, 0.0)), true);
        app.update();
        assert_eq!(app.world().resource::<Inspector>().budget.taxes[0], 0.0);
        release(&mut app);
        assert!(app.world().resource::<Inspector>().budget.drag.is_none());
        pointer(&mut app, tax, None, false);
        pointer(&mut app, subsidy, Some(Vec2::new(-0.25, 0.0)), true);
        app.world_mut()
            .resource_mut::<ButtonInput<MouseButton>>()
            .press(MouseButton::Left);
        app.update();
        assert_eq!(app.world().resource::<Inspector>().budget.amount, 0.25);
        release(&mut app);
        apply(&mut app, Action::FullSubsidy);
        assert_eq!(
            app.world()
                .resource::<Inspector>()
                .budget
                .value(Policy::Subsidy, 0),
            1.0
        );
        pointer(&mut app, subsidy, Some(Vec2::ZERO), true);
        app.world_mut()
            .resource_mut::<ButtonInput<MouseButton>>()
            .press(MouseButton::Left);
        app.update();
        assert!(app.world().resource::<Inspector>().budget.drag.is_none());
        assert_eq!(app.world().resource::<Inspector>().budget.amount, 0.25);
        release(&mut app);
        apply(&mut app, Action::FullSubsidy);
        assert_eq!(
            app.world()
                .resource::<Inspector>()
                .budget
                .value(Policy::Subsidy, 0),
            0.25
        );
        pointer(&mut app, tax, Some(Vec2::ZERO), true);
        pointer(&mut app, subsidy, None, false);
        app.world_mut()
            .resource_mut::<ButtonInput<MouseButton>>()
            .press(MouseButton::Left);
        app.update();
        apply(&mut app, Action::Close);
        pointer(&mut app, tax, Some(Vec2::new(0.25, 0.0)), true);
        app.update();
        assert!(app.world().resource::<Inspector>().budget.drag.is_none());
        assert_eq!(app.world().resource::<Inspector>().budget.taxes[0], 2.5);
        assert_eq!(app.world().resource::<Game>().simulation.date.tick(), 0);
        assert_eq!(
            app.world().resource::<Game>().simulation.history.len(),
            history
        );
        Ok(())
    }
    #[test]
    fn policy_buttons_are_one_shot_and_every_good_and_recipe_is_reachable() -> Result<()> {
        let mut app = app()?;
        apply(&mut app, Action::Open);
        apply(&mut app, Action::Tab(Tab::Budget));
        for good in 0..56 {
            assert_eq!(app.world().resource::<Inspector>().good, good);
            apply(&mut app, Action::TaxGood);
        }
        assert_eq!(app.world().resource::<Inspector>().good, 0);
        for recipe in 0..22 {
            assert_eq!(app.world().resource::<Inspector>().recipe, recipe);
            apply(&mut app, Action::Next);
        }
        assert_eq!(app.world().resource::<Inspector>().recipe, 0);
        apply(&mut app, Action::Previous);
        assert_eq!(app.world().resource::<Inspector>().recipe, 21);
        apply(&mut app, Action::Next);
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
        apply(&mut app, Action::FullSubsidy);
        app.world_mut()
            .spawn((Action::ApplySubsidy, Interaction::Pressed));
        app.update();
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
        apply(&mut app, Action::RemoveSubsidy);
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
        assert!(
            app.world()
                .resource::<Inspector>()
                .budget
                .content(app.world().resource::<Game>(), 0, 0)
                .contains("NATIONAL LABOR")
        );
        apply(&mut app, Action::Tab(Tab::Markets));
        apply(&mut app, Action::ApplySubsidy);
        assert!(
            app.world().resource::<Game>().simulation.countries[country]
                .subsidies
                .is_empty()
        );
        assert_eq!(app.world().resource::<Game>().simulation.date.tick(), 0);
        Ok(())
    }
}
