//! Global production table; scope filters intentionally do not apply upstream.
use super::{Game, Inspector, Panel, ui_font};
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
