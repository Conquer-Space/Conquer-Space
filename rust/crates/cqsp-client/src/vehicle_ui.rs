//! The source launch-vehicle viewer: country list, retained filter/selection and draft.
use super::{Action, Field, Game, SpaceportUi, button, field_button, row, ui_font};
use bevy::prelude::*;
use cqsp_core::{number_display::float, spaceflight::VehicleId};

pub(super) const PAGE_SIZE: usize = 8;
pub(super) struct Browser {
    pub creating: bool,
    pub query: String,
    pub draft: [String; 4],
    pub page: usize,
    pub selected: Option<VehicleId>,
}
impl Default for Browser {
    fn default() -> Self {
        Self {
            creating: false,
            query: String::new(),
            draft: ["", "0", "0", "0"].map(str::to_owned),
            page: 0,
            selected: None,
        }
    }
}
impl Browser {
    pub fn results(&self, game: &Game) -> Vec<VehicleId> {
        game.session
            .player_country
            .into_iter()
            .flat_map(|country| game.simulation.countries[country].vehicles.iter().copied())
            .filter(|id| game.simulation.vehicles[id.0].matches(&self.query))
            .collect()
    }
    pub fn content(&self, game: &Game) -> String {
        let matches = self.results(game).len();
        let count = game.session.player_country.map_or(0, |country| {
            game.simulation.countries[country].vehicles.len()
        });
        let detail = self
            .selected
            .and_then(|id| game.simulation.vehicles.get(id.0))
            .map_or("No launch vehicle selected.".into(), |vehicle| {
                format!(
                    "{}\nReliability: {}%\nMass to Orbit: {} tons\nFaring Size: {} m3",
                    vehicle.name,
                    float(vehicle.reliability * 100.0),
                    float(vehicle.mass_to_orbit),
                    float(vehicle.fairing_size)
                )
            });
        format!(
            "LAUNCH VEHICLES: {count} · {} matches · page {} / {}\nFiltering does not clear the selected vehicle.\n\n{detail}",
            matches,
            self.page + 1,
            matches.div_ceil(PAGE_SIZE).max(1)
        )
    }
}
#[derive(Component)]
pub(crate) enum Section {
    List,
    Design,
}
#[derive(Component)]
pub(crate) struct ResultRow(pub usize);
#[derive(Component)]
pub(crate) struct ResultLabel(usize);
#[derive(Component)]
pub(crate) struct Content;

pub(super) fn spawn(panel: &mut ChildSpawnerCommands, font: &Handle<Font>) {
    panel
        .spawn((
            Section::List,
            Node {
                flex_direction: FlexDirection::Column,
                flex_shrink: 0.0,
                row_gap: px(8),
                ..default()
            },
        ))
        .with_children(|panel| {
            row(panel, |bar| {
                button(bar, font, Action::AddVehicle, "+ Create launch vehicle");
                field_button(bar, font, Field::Search);
            });
            row(panel, |bar| {
                button(bar, font, Action::VehiclePage(true), "Previous vehicles");
                button(bar, font, Action::VehiclePage(false), "Next vehicles");
            });
            for slot in 0..PAGE_SIZE {
                panel
                    .spawn((
                        ResultRow(slot),
                        Action::SelectVehicle(VehicleId(usize::MAX)),
                        Button,
                        Node {
                            min_height: px(32),
                            flex_shrink: 0.0,
                            padding: UiRect::all(px(6)),
                            ..default()
                        },
                        BackgroundColor(Color::srgb(0.15, 0.29, 0.43)),
                    ))
                    .with_children(|row| {
                        row.spawn((ResultLabel(slot), Text::default(), ui_font(font, 15.0)));
                    });
            }
        });
    panel
        .spawn((
            Section::Design,
            Node {
                flex_direction: FlexDirection::Column,
                flex_shrink: 0.0,
                row_gap: px(8),
                ..default()
            },
        ))
        .with_children(|panel| {
            panel.spawn((Text::new("CREATE LAUNCH VEHICLE"), ui_font(font, 18.0)));
            for field in [Field::Name, Field::Reliability, Field::Fairing, Field::Mass] {
                field_button(panel, font, field);
            }
            row(panel, |bar| {
                button(bar, font, Action::Design, "Design!");
                button(bar, font, Action::CancelDesign, "Close design editor");
            });
        });
    panel.spawn((
        Content,
        Text::default(),
        ui_font(font, 16.0),
        Node {
            flex_shrink: 0.0,
            ..default()
        },
    ));
}
#[allow(clippy::type_complexity)]
pub(crate) fn update(
    ui: Res<SpaceportUi>,
    game: Res<Game>,
    mut sections: Query<(&Section, &mut Node), Without<ResultRow>>,
    mut rows: Query<(&ResultRow, &mut Action, &mut Node, &mut BackgroundColor), Without<Section>>,
    mut labels: Query<(&ResultLabel, &mut Text), Without<Content>>,
    mut content: Query<&mut Text, (With<Content>, Without<ResultLabel>)>,
) {
    let results = ui.browser.results(&game);
    for (section, mut node) in &mut sections {
        let shown = ui.open
            && ui.vehicles
            && match section {
                Section::List => !ui.browser.creating,
                Section::Design => ui.browser.creating,
            };
        let display = if shown { Display::Flex } else { Display::None };
        if node.display != display {
            node.display = display;
        }
    }
    for (row, mut action, mut node, mut color) in &mut rows {
        let id = results.get(ui.browser.page * PAGE_SIZE + row.0).copied();
        let display = if ui.open && ui.vehicles && !ui.browser.creating && id.is_some() {
            Display::Flex
        } else {
            Display::None
        };
        if node.display != display {
            node.display = display;
        }
        let next = id.unwrap_or(VehicleId(usize::MAX));
        if !matches!(*action, Action::SelectVehicle(current) if current == next) {
            *action = Action::SelectVehicle(next);
        }
        let next = BackgroundColor(if id.is_some() && id == ui.browser.selected {
            Color::srgb(0.24, 0.38, 0.24)
        } else {
            Color::srgb(0.15, 0.29, 0.43)
        });
        if *color != next {
            *color = next;
        }
    }
    for (label, mut text) in &mut labels {
        let next = results
            .get(ui.browser.page * PAGE_SIZE + label.0)
            .map_or("", |id| game.simulation.vehicles[id.0].name.as_str());
        if text.0 != next {
            text.0 = next.into();
        }
    }
    for mut text in &mut content {
        let next = if ui.open && ui.vehicles {
            ui.browser.content(&game)
        } else {
            String::new()
        };
        if text.0 != next {
            text.0 = next;
        }
    }
}
