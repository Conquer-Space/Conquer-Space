//! Global production table; scope filters intentionally do not apply upstream.
use super::{Game, Inspector, Panel, Tab, ui_font};
use bevy::prelude::*;
use cqsp_core::economy::{industry::Industry, production_summary::COLUMNS};

pub(super) const PAGE_SIZE: usize = 8;
const WIDTHS: [f32; 9] = [24.0, 6.0, 10.0, 10.0, 11.0, 10.0, 8.0, 11.0, 10.0];
#[derive(Component)]
pub(crate) struct Cell {
    row: usize,
    column: usize,
}
#[derive(Component)]
pub(crate) struct Progress(usize);
#[derive(Component)]
pub(crate) struct TableRow(usize);
#[derive(Component)]
pub(crate) struct Tooltip;
#[derive(Component)]
pub(crate) struct Expertise;
#[derive(Component)]
pub(crate) struct TooltipGlyph;
#[derive(Component)]
pub(crate) struct TooltipCard {
    font: Handle<Font>,
    pub lines: Vec<cqsp_core::tooltips::Line>,
}
#[derive(Clone, Copy)]
pub(crate) enum Hover {
    Factory(usize),
    MaxExpertise,
}

pub(super) fn spawn_tooltip(commands: &mut Commands, camera: Entity, font: &Handle<Font>) {
    commands.spawn((
        TooltipCard {
            font: font.clone(),
            lines: Vec::new(),
        },
        UiTargetCamera(camera),
        GlobalZIndex(30),
        bevy::ui::FocusPolicy::Pass,
        Node {
            display: Display::None,
            position_type: PositionType::Absolute,
            width: px(430),
            padding: UiRect::all(px(12)),
            flex_direction: FlexDirection::Column,
            ..default()
        },
        BackgroundColor(Color::srgb(0.055, 0.075, 0.105)),
    ));
}

#[allow(clippy::too_many_arguments)]
pub(crate) fn update_tooltip(
    mut commands: Commands,
    inspector: Res<Inspector>,
    game: Res<Game>,
    smoke: Option<Res<super::super::Smoke>>,
    overlays: (
        Option<Res<super::super::navigation_ui::Menu>>,
        Option<Res<super::super::search_ui::Search>>,
    ),
    cells: Query<(&Cell, &Interaction)>,
    mut expertise: Query<(&Interaction, &mut Text), With<Expertise>>,
    mut cards: Query<(Entity, &mut TooltipCard, &mut Node)>,
    windows: Query<&Window, With<bevy::window::PrimaryWindow>>,
    cameras: Query<&Camera>,
) {
    let industry = (inspector.open && inspector.tab == Tab::Industries)
        .then(|| inspector.factories(&game).nth(inspector.factory))
        .flatten()
        .map(|(site, factory)| &game.simulation.settlements[site].industries[factory]);
    let mut hover = None;
    if inspector.is_production() {
        hover = cells
            .iter()
            .find(|(cell, interaction)| cell.column == 0 && **interaction != Interaction::None)
            .map(|(cell, _)| Hover::Factory(inspector.page * PAGE_SIZE + cell.row));
    }
    for (interaction, mut text) in &mut expertise {
        let value = industry.map_or("No factory selected.".into(), |industry| {
            format!(
                "Max Expertise: {} · hover for modifier contributions",
                cqsp_core::number_display::float(industry.max_expertise.value())
            )
        });
        if text.0 != value {
            text.0 = value;
        }
        if inspector.open && inspector.tab == Tab::Industries && *interaction != Interaction::None {
            hover = Some(Hover::MaxExpertise);
        }
    }
    if let Some(smoke) = smoke.filter(|smoke| smoke.enabled) {
        hover = smoke.inspection_hover.or(hover);
    }
    let lines = if !inspector.open
        || super::super::event_ui::pending(&game).is_some()
        || overlays.0.is_some_and(|menu| menu.open)
        || overlays.1.is_some_and(|search| search.open)
    {
        Vec::new()
    } else {
        match hover {
            Some(Hover::Factory(index)) if inspector.is_production() => industries(&game)
                .nth(index)
                .map_or_else(Vec::new, |industry| {
                    industry.tooltip(&game.simulation.catalog)
                }),
            Some(Hover::MaxExpertise) if inspector.tab == Tab::Industries => industry
                .map_or_else(Vec::new, |industry| {
                    industry.max_expertise.tooltip(&game.simulation.modifiers)
                }),
            _ => Vec::new(),
        }
    };
    let viewport = cameras
        .iter()
        .find_map(Camera::logical_viewport_size)
        .unwrap_or(Vec2::new(1280.0, 800.0));
    let cursor = windows
        .iter()
        .find_map(Window::cursor_position)
        .unwrap_or(Vec2::new(740.0, 270.0));
    for (entity, mut card, mut node) in &mut cards {
        let display = if lines.is_empty() {
            Display::None
        } else {
            Display::Flex
        };
        let left = px((cursor.x + 16.0).clamp(8.0, (viewport.x - 438.0).max(8.0)));
        let top = px((cursor.y + 16.0).clamp(
            8.0,
            (viewport.y - (lines.len() as f32 * 16.0 + 32.0)).max(8.0),
        ));
        if (node.display, node.left, node.top) != (display, left, top) {
            node.display = display;
            node.left = left;
            node.top = top;
        }
        if card.lines != lines {
            card.lines.clone_from(&lines);
            let font = card.font.clone();
            commands.entity(entity).despawn_children();
            commands.entity(entity).with_children(|panel| {
                for line in &lines {
                    let [r, g, b] = line.color;
                    panel.spawn((
                        TooltipGlyph,
                        Text::new(&line.text),
                        ui_font(&font, 13.0),
                        TextColor(Color::srgb(r, g, b)),
                    ));
                }
            });
        }
    }
}

pub(super) fn industries(game: &Game) -> impl Iterator<Item = &Industry> {
    game.simulation
        .settlements
        .iter()
        .flat_map(|site| &site.industries)
}

pub(super) fn spawn(parent: &mut ChildSpawnerCommands, font: &Handle<Font>) {
    parent
        .spawn((
            Panel::Production,
            Node {
                flex_direction: FlexDirection::Column,
                flex_shrink: 0.0,
                ..default()
            },
        ))
        .with_children(|table| {
            table
                .spawn(Node {
                    min_height: px(40),
                    flex_shrink: 0.0,
                    ..default()
                })
                .with_children(|row| {
                    for (column, heading) in COLUMNS.iter().enumerate() {
                        row.spawn((
                            Text::new(*heading),
                            ui_font(font, 13.0),
                            column_node(column),
                        ));
                    }
                });
            for index in 0..PAGE_SIZE {
                table
                    .spawn((
                        TableRow(index),
                        Node {
                            min_height: px(36),
                            flex_shrink: 0.0,
                            ..default()
                        },
                        BackgroundColor(Color::srgb(0.045 + index as f32 % 2.0 * 0.02, 0.08, 0.12)),
                    ))
                    .with_children(|row| {
                        for column in 0..9 {
                            row.spawn((
                                Cell { row: index, column },
                                Text::default(),
                                ui_font(font, 13.0),
                                TextColor(Color::WHITE),
                                Interaction::default(),
                                column_node(column),
                            ));
                        }
                    });
                // Separate progress row keeps the construction fill independently measurable.
                table.spawn((
                    Progress(index),
                    Node {
                        height: px(4),
                        flex_shrink: 0.0,
                        margin: UiRect::left(percent(30)),
                        ..default()
                    },
                    BackgroundColor(Color::srgb(0.18, 0.7, 0.42)),
                ));
            }
            table.spawn((
                Tooltip,
                Text::default(),
                ui_font(font, 14.0),
                Node {
                    min_height: px(24),
                    ..default()
                },
            ));
        });
}

fn column_node(column: usize) -> Node {
    Node {
        width: percent(WIDTHS[column]),
        padding: UiRect::all(px(4)),
        flex_shrink: 0.0,
        ..default()
    }
}

pub(crate) fn update(
    inspector: Res<Inspector>,
    game: Res<Game>,
    mut cells: Query<(&Cell, &Interaction, &mut Text, &mut TextColor), Without<Tooltip>>,
    mut rows: Query<(&TableRow, &mut Node), Without<Progress>>,
    mut progress: Query<(&Progress, &mut Node), Without<TableRow>>,
    mut tooltip: Query<&mut Text, With<Tooltip>>,
) {
    if !inspector.is_production() {
        return;
    }
    let summaries: Vec<_> = industries(&game)
        .skip(inspector.page * PAGE_SIZE)
        .take(PAGE_SIZE)
        .map(|industry| industry.summary(&game.simulation.catalog))
        .collect();
    let mut hover = String::new();
    for (cell, interaction, mut text, mut color) in &mut cells {
        let Some(summary) = summaries.get(cell.row) else {
            if !text.is_empty() {
                text.clear();
            }
            continue;
        };
        let next = &summary.cells[cell.column];
        if &text.0 != next {
            text.0.clone_from(next);
        }
        let [r, g, b] = summary.colors[cell.column];
        let next_color = Color::srgb(r, g, b);
        if color.0 != next_color {
            color.0 = next_color;
        }
        if *interaction != Interaction::None {
            hover = match cell.column {
                0 => format!(
                    "{} · {:?}",
                    summary.cells[0],
                    industries(&game)
                        .nth(inspector.page * PAGE_SIZE + cell.row)
                        .unwrap()
                        .state
                ),
                1 if summary.shortage => "Resource Shortage!".into(),
                7 => summary.items_sold.clone(),
                _ => String::new(),
            };
        }
    }
    for (row, mut node) in &mut rows {
        node.display = if row.0 < summaries.len() {
            Display::Flex
        } else {
            Display::None
        };
    }
    for (bar, mut node) in &mut progress {
        let value = summaries
            .get(bar.0)
            .and_then(|summary| summary.construction);
        node.display = if value.is_some() {
            Display::Flex
        } else {
            Display::None
        };
        node.width = percent(
            10.0 * value
                .filter(|value| value.is_finite())
                .unwrap_or(0.0)
                .clamp(0.0, 1.0),
        );
    }
    for mut text in &mut tooltip {
        if text.0 != hover {
            text.0.clone_from(&hover);
        }
    }
}

#[cfg(test)]
mod tests {
    use super::super::{
        Action, Tab,
        tests::{app, apply},
    };
    use super::*;
    fn card(app: &mut App) -> Entity {
        app.world_mut()
            .spawn((
                TooltipCard {
                    font: Handle::default(),
                    lines: Vec::new(),
                },
                Node::default(),
            ))
            .id()
    }
    #[test]
    fn factory_hover_refreshes_details_reuses_unchanged_children_and_hides_stale_rows() {
        let mut app = app().unwrap();
        apply(&mut app, Action::Open);
        apply(&mut app, Action::Tab(Tab::Production));
        let card = card(&mut app);
        let cell = app
            .world_mut()
            .spawn((
                Cell { row: 0, column: 0 },
                Interaction::Hovered,
                Text::default(),
                TextColor::default(),
            ))
            .id();
        app.update();
        assert_eq!(
            app.world().get::<Node>(card).unwrap().display,
            Display::Flex
        );
        assert_eq!(
            app.world().get::<TooltipCard>(card).unwrap().lines.len(),
            21
        );
        let children = app.world().get::<Children>(card).unwrap().to_vec();
        app.update();
        assert_eq!(
            app.world().get::<Children>(card).unwrap().to_vec(),
            children
        );
        let mut menu = super::super::super::navigation_ui::Menu::default();
        menu.open = true;
        app.insert_resource(menu);
        app.update();
        assert_eq!(
            app.world().get::<Node>(card).unwrap().display,
            Display::None
        );
        app.world_mut()
            .resource_mut::<super::super::super::navigation_ui::Menu>()
            .open = false;
        app.update();
        assert_eq!(
            app.world().get::<Node>(card).unwrap().display,
            Display::Flex
        );
        let mut search = super::super::super::search_ui::Search::default();
        search.open = true;
        app.insert_resource(search);
        app.update();
        assert_eq!(
            app.world().get::<Node>(card).unwrap().display,
            Display::None
        );
        app.world_mut()
            .resource_mut::<super::super::super::search_ui::Search>()
            .open = false;
        app.update();
        assert_eq!(
            app.world().get::<Node>(card).unwrap().display,
            Display::Flex
        );
        apply(&mut app, Action::Next);
        app.update();
        let expected = {
            let game = app.world().resource::<Game>();
            industries(game)
                .nth(PAGE_SIZE)
                .unwrap()
                .tooltip(&game.simulation.catalog)
        };
        assert_eq!(
            app.world().get::<TooltipCard>(card).unwrap().lines,
            expected
        );
        app.world_mut().get_mut::<Cell>(cell).unwrap().row = usize::MAX / PAGE_SIZE;
        app.update();
        assert_eq!(
            app.world().get::<Node>(card).unwrap().display,
            Display::None
        );
        assert!(
            app.world()
                .get::<TooltipCard>(card)
                .unwrap()
                .lines
                .is_empty()
        );
        assert_eq!(app.world().resource::<Game>().simulation.date.tick(), 0);
    }
    #[test]
    fn expertise_hover_keeps_duplicate_contributions_and_clears_on_close_or_events() {
        let mut app = app().unwrap();
        apply(&mut app, Action::Open);
        apply(&mut app, Action::Tab(Tab::Industries));
        apply(&mut app, Action::BoostExpertise);
        apply(&mut app, Action::BoostExpertise);
        let card = card(&mut app);
        let label = app
            .world_mut()
            .spawn((Expertise, Interaction::Hovered, Text::default()))
            .id();
        app.update();
        let lines = &app.world().get::<TooltipCard>(card).unwrap().lines;
        assert_eq!(
            lines
                .iter()
                .map(|line| line.text.as_str())
                .collect::<Vec<_>>(),
            [
                "Value: 3.5",
                "Modifier: 2",
                "Higher Expertise: +1",
                "Higher Expertise: +1"
            ]
        );
        assert!(
            app.world()
                .get::<Text>(label)
                .unwrap()
                .starts_with("Max Expertise: 3.5")
        );
        apply(&mut app, Action::Close);
        app.update();
        assert_eq!(
            app.world().get::<Node>(card).unwrap().display,
            Display::None
        );
        apply(&mut app, Action::Open);
        app.update();
        assert_eq!(
            app.world().get::<Node>(card).unwrap().display,
            Display::Flex
        );
        {
            let mut game = app.world_mut().resource_mut::<Game>();
            let country = game.session.player_country.unwrap();
            game.simulation
                .push_event(
                    country,
                    cqsp_core::events::Notification::tutorial(0).unwrap(),
                )
                .unwrap();
        }
        app.update();
        assert_eq!(
            app.world().get::<Node>(card).unwrap().display,
            Display::None
        );
        assert_eq!(app.world().resource::<Game>().simulation.date.tick(), 0);
    }
    #[test]
    fn worldwide_rows_ignore_ownership_planet_and_province_filters_and_page_without_mutation() {
        let mut app = app().unwrap();
        let total = industries(app.world().resource::<Game>()).count();
        assert!(total > PAGE_SIZE);
        apply(&mut app, Action::Open);
        apply(&mut app, Action::Tab(Tab::Production));
        apply(&mut app, Action::Planet);
        apply(&mut app, Action::Ownership);
        assert_eq!(industries(app.world().resource::<Game>()).count(), total);
        assert!(
            app.world()
                .resource::<Inspector>()
                .scope(app.world().resource::<Game>())
                .contains("Global scope")
        );
        let cell = app
            .world_mut()
            .spawn((
                Cell { row: 0, column: 5 },
                Interaction::None,
                Text::default(),
                TextColor::default(),
            ))
            .id();
        let revenue = app
            .world_mut()
            .spawn((
                Cell { row: 0, column: 7 },
                Interaction::Hovered,
                Text::default(),
                TextColor::default(),
            ))
            .id();
        let tooltip = app.world_mut().spawn((Tooltip, Text::default())).id();
        app.update();
        assert_eq!(app.world().get::<Text>(cell).unwrap().0, "0/0");
        assert!(
            app.world()
                .get::<Text>(tooltip)
                .unwrap()
                .starts_with("Items sold: ")
        );
        app.world_mut()
            .get_mut::<Interaction>(revenue)
            .unwrap()
            .set_if_neq(Interaction::None);
        apply(&mut app, Action::Next);
        assert_eq!(app.world().resource::<Inspector>().page, 1);
        for _ in 1..total.div_ceil(PAGE_SIZE) {
            apply(&mut app, Action::Next);
        }
        assert_eq!(app.world().resource::<Inspector>().page, 0);
        assert_eq!(app.world().resource::<Game>().simulation.date.tick(), 0);
    }
}
