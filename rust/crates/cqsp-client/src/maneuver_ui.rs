//! Functional spacecraft controls from SpaceshipWindow; presentation is native.
use super::{FlightAction, Focus, Game, Navigation, ui_button, ui_font, ui_row};
use anyhow::{Context, Result, ensure};
use bevy::{
    input::{
        ButtonState,
        keyboard::KeyboardInput,
        mouse::{AccumulatedMouseScroll, MouseScrollUnit},
    },
    prelude::*,
};
use cqsp_core::{flight::FlightStatus, solar_system::SolarSystem};
use std::fmt::Write;

const PAGE_SIZE: usize = 4;
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq)]
pub(super) enum Tab {
    #[default]
    Basic,
    Rendezvous,
    Landing,
    Details,
}
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub(super) enum Field {
    Apoapsis,
    Periapsis,
    Orbit,
    Departure,
}
impl Field {
    const ALL: [Self; 4] = [
        Self::Apoapsis,
        Self::Periapsis,
        Self::Orbit,
        Self::Departure,
    ];
    fn index(self) -> usize {
        self as usize
    }
    fn name(self) -> &'static str {
        match self {
            Self::Apoapsis => "Apoapsis radius (km)",
            Self::Periapsis => "Periapsis radius (km)",
            Self::Orbit => "New orbit radius (km)",
            Self::Departure => "Leave SOI: parent-orbit radius (km)",
        }
    }
}
#[derive(Component, Clone, Copy)]
pub(super) enum Action {
    Open,
    Close,
    Tab(Tab),
    Edit(Field),
    Radius(Field),
    Execute(FlightAction),
    Target(Focus),
    Previous,
    Next,
    Rendezvous,
    Intercept,
    MatchPlanes,
    Transfer,
    Land,
}
#[derive(Resource)]
pub(super) struct Menu {
    pub open: bool,
    pub capture_input: bool,
    pub tab: Tab,
    fields: [String; 4],
    focused: Option<Field>,
    replace: bool,
    rendezvous: Option<Focus>,
    landing: Option<Focus>,
    page: usize,
    message: String,
}
impl Default for Menu {
    fn default() -> Self {
        Self {
            open: false,
            capture_input: false,
            tab: Tab::Basic,
            fields: ["0", "0", "0", "8000"].map(String::from),
            focused: None,
            replace: false,
            rendezvous: None,
            landing: None,
            page: 0,
            message: String::new(),
        }
    }
}
impl Menu {
    fn number(&self, field: Field) -> Result<f64> {
        let number: f64 = self.fields[field.index()]
            .trim()
            .parse()
            .with_context(|| format!("{} must be a number", field.name()))?;
        ensure!(number.is_finite(), "{} must be finite", field.name());
        Ok(number)
    }
    fn targets(&self, navigation: &Navigation, system: &SolarSystem) -> Vec<Focus> {
        let Focus::Ship(id) = navigation.focus else {
            return Vec::new();
        };
        let Some(ship) = system.ships.get(id.0) else {
            return Vec::new();
        };
        let Some(body) = system.bodies.get(ship.parent.0) else {
            return Vec::new();
        };
        let mut targets: Vec<_> = body.children.iter().copied().map(Focus::Body).collect();
        if self.tab == Tab::Rendezvous {
            targets.extend(
                system
                    .ships
                    .iter()
                    .enumerate()
                    .filter(|(_, target)| {
                        target.parent == ship.parent && target.status == FlightStatus::Orbiting
                    })
                    .map(|(index, _)| Focus::Ship(cqsp_core::solar_system::ShipId(index))),
            );
        }
        targets
    }
    fn selected(&self) -> Option<Focus> {
        if self.tab == Tab::Landing {
            self.landing
        } else {
            self.rendezvous
        }
    }
    pub fn apply(
        &mut self,
        action: Action,
        navigation: &mut Navigation,
        game: &mut Game,
    ) -> Result<()> {
        if matches!(action, Action::Close) {
            self.open = false;
            self.capture_input = true;
            self.focused = None;
            return Ok(());
        }
        ensure!(
            navigation.active && game.session.player_country.is_some(),
            "Start a country and select a spacecraft in system view"
        );
        let Focus::Ship(id) = navigation.focus else {
            anyhow::bail!("Select a spacecraft first")
        };
        let system = &game.simulation.solar_system;
        let ship = system.ships.get(id.0).context("Unknown spacecraft")?;
        ensure!(
            ship.status == FlightStatus::Orbiting && ship.error.is_none(),
            "Spacecraft is not maneuverable"
        );
        if matches!(action, Action::Open) {
            self.open = true;
            self.capture_input = true;
            self.focused = None;
            self.message.clear();
            if self.rendezvous.is_none() {
                self.rendezvous = system.bodies[ship.parent.0]
                    .children
                    .first()
                    .copied()
                    .map(Focus::Body);
            }
            return Ok(());
        }
        if !self.open {
            return Ok(());
        }
        let flight = match action {
            Action::Tab(tab) => {
                self.tab = tab;
                self.page = 0;
                self.focused = None;
                None
            }
            Action::Edit(field) if self.tab == Tab::Basic => {
                self.focused = Some(field);
                self.replace = true;
                None
            }
            Action::Radius(field) if self.tab == Tab::Basic => {
                navigation.target_radius = self.number(field)?;
                Some(match field {
                    Field::Apoapsis => FlightAction::SetApoapsis,
                    Field::Periapsis => FlightAction::SetPeriapsis,
                    Field::Orbit => FlightAction::Hohmann,
                    Field::Departure => FlightAction::LeaveSoi,
                })
            }
            Action::Execute(action) if self.tab == Tab::Basic => Some(action),
            Action::Target(target) if matches!(self.tab, Tab::Rendezvous | Tab::Landing) => {
                ensure!(
                    self.targets(navigation, system).contains(&target),
                    "Target is not in this spacecraft's current orbital system"
                );
                if self.tab == Tab::Landing {
                    self.landing = Some(target)
                } else {
                    self.rendezvous = Some(target)
                }
                None
            }
            Action::Previous => {
                self.page = self.page.saturating_sub(1);
                None
            }
            Action::Next => {
                self.page = (self.page + 1)
                    .min(self.targets(navigation, system).len().saturating_sub(1) / PAGE_SIZE);
                None
            }
            Action::Rendezvous | Action::Intercept | Action::MatchPlanes
                if self.tab == Tab::Rendezvous =>
            {
                let target = self.rendezvous.context("Select a rendezvous target")?;
                Some(match action {
                    Action::Rendezvous => FlightAction::Rendezvous(target),
                    Action::Intercept => FlightAction::Intercept(target),
                    _ => FlightAction::MatchPlanes(target),
                })
            }
            Action::Transfer | Action::Land if self.tab == Tab::Landing => {
                let Some(Focus::Body(body)) = self.landing else {
                    anyhow::bail!("Select a landing body")
                };
                Some(if matches!(action, Action::Transfer) {
                    FlightAction::TransferBody(body)
                } else {
                    FlightAction::LandBody(body)
                })
            }
            _ => None,
        };
        if let Some(action) = flight {
            navigation.apply(action, game)?;
            self.focused = None;
            self.message.clone_from(&navigation.message);
        }
        Ok(())
    }
    pub fn enter_text(&mut self, text: &str) {
        let Some(field) = self.focused else { return };
        let number: String = text
            .chars()
            .filter(|ch| ch.is_ascii_digit() || ".-+eE".contains(*ch))
            .collect();
        if number.is_empty() {
            return;
        }
        let value = &mut self.fields[field.index()];
        if self.replace {
            value.clear()
        }
        self.replace = false;
        value.extend(number.chars().take(64usize.saturating_sub(value.len())));
    }
    fn content(&self, navigation: &Navigation, game: &Game) -> String {
        let system = &game.simulation.solar_system;
        let Focus::Ship(id) = navigation.focus else {
            return "No spacecraft selected".into();
        };
        let Some(ship) = system.ships.get(id.0) else {
            return "Spacecraft no longer exists".into();
        };
        let orbit = ship.orbit;
        let time = game.simulation.date.seconds();
        let mut text = format!(
            "{} · orbiting {} · {:?}\nPlayback remains {} while inspecting.\n",
            ship.name,
            system.bodies[ship.parent.0].name,
            ship.status,
            if game.paused { "paused" } else { "running" }
        );
        match self.tab {
            Tab::Basic => {
                let _ = write!(
                    text,
                    "\nValues are center-relative radii, as in the original actions.\nCircularize Δv: Pe {} km/s · Ap {} km/s\nClick a value to edit; Ctrl+A replaces; Enter finishes; Tab changes fields.\nFinite signed values are accepted; invalid plans do not change queues.",
                    orbit.circularize_at_periapsis().delta_v.y,
                    orbit.circularize_at_apoapsis().delta_v.y
                );
            }
            Tab::Rendezvous | Tab::Landing => {
                let selected = self.selected();
                let name = selected
                    .and_then(|focus| focus.name(system))
                    .unwrap_or("none");
                let targets = self.targets(navigation, system);
                let _ = write!(
                    text,
                    "\nSelected target: {name}\n{} targets · page {} / {}\n",
                    targets.len(),
                    self.page.min(targets.len().saturating_sub(1) / PAGE_SIZE) + 1,
                    targets.len().saturating_sub(1) / PAGE_SIZE + 1
                );
                if let Some(target) = selected.and_then(|focus| focus.orbit_in(system, ship.parent))
                {
                    let position = match selected.unwrap() {
                        Focus::Body(id) => system.bodies[id.0].kinematics.state.position,
                        Focus::Ship(id) => system.ships[id.0].kinematics.state.position,
                    };
                    let _ = write!(
                        text,
                        "Distance: {} km\nPhase angle: {} rad · transfer angle: {} rad\nTime to ascending node: {} s\n",
                        (position - ship.kinematics.state.position).length(),
                        orbit.phase_angle(target, time),
                        orbit.transfer_angle(target),
                        orbit.time_to_anomaly(orbit.ascending_anomaly(target))
                    );
                } else if self.tab == Tab::Landing && selected == Some(Focus::Body(ship.parent)) {
                    text.push_str("Already orbiting the selected body; landing still uses its first settlement.\n");
                } else if selected.is_some() {
                    text.push_str(
                        "Target is outside the current orbital system; select another target.\n",
                    );
                }
                if self.tab == Tab::Landing {
                    text.push_str("Land uses the body's first settlement, as upstream.\nThe transfer command retains its original 'Moon' algorithm for selected bodies.");
                } else {
                    text.push_str("Rendezvous queues both burns; Maneuver to point queues only the first.\nLambert transfer is an upstream placeholder, not an implemented action.");
                }
            }
            Tab::Details => {
                let _ = write!(
                    text,
                    "\nAxis: {} km · eccentricity: {}\nInclination: {}° · ascending node: {}°\nPeriapsis argument: {}° · mean anomaly at epoch: {}°\nTrue anomaly: {} rad · epoch: {} s\nGM: {} km³/s² · period: {} s\nOrbiting radius: {} km\nPeriapsis: {} km · in {} s\nApoapsis: {} km · in {} s\nPosition (km): {:?}\nVelocity (km/s): {:?}\n\nMANEUVER QUEUE · {} burns / {} trigger commands\n",
                    orbit.semi_major_axis,
                    orbit.eccentricity,
                    orbit.inclination.to_degrees(),
                    orbit.ascending_node.to_degrees(),
                    orbit.periapsis_argument.to_degrees(),
                    orbit.mean_anomaly.to_degrees(),
                    orbit.true_anomaly,
                    orbit.epoch,
                    orbit.mu,
                    orbit.period(),
                    orbit.radius_at(orbit.true_anomaly),
                    orbit.periapsis(),
                    orbit.time_to_anomaly(0.0),
                    orbit.apoapsis(),
                    orbit.time_to_anomaly(std::f64::consts::PI),
                    ship.kinematics.state.position,
                    ship.kinematics.state.velocity,
                    ship.maneuvers.len(),
                    ship.commands.len()
                );
                for burn in &ship.maneuvers {
                    let _ = writeln!(
                        text,
                        "Burn in {} s · Δv {:?} km/s",
                        burn.time - time,
                        burn.delta_v
                    );
                }
                for command in &ship.commands {
                    let _ = writeln!(text, "{:?}: {}", command.trigger, command.command.name());
                }
            }
        }
        if !self.message.is_empty() {
            let _ = write!(text, "\n\n{}", self.message);
        }
        text
    }
}
pub(super) fn input_available(menu: Option<Res<Menu>>) -> bool {
    menu.is_none_or(|menu| !menu.capture_input)
}
#[derive(Component)]
pub(super) struct Panel;
#[derive(Component)]
pub(super) struct Content;
#[derive(Component)]
pub(super) struct Section(Tab);
#[derive(Component)]
pub(super) struct FieldLabel(Field);
#[derive(Component)]
pub(super) struct Targets(Handle<Font>);
pub(super) fn spawn(commands: &mut Commands, camera: Entity, font: &Handle<Font>) {
    commands
        .spawn((
            Panel,
            UiTargetCamera(camera),
            GlobalZIndex(27),
            ScrollPosition::default(),
            Node {
                display: Display::None,
                position_type: PositionType::Absolute,
                left: px(36),
                right: px(36),
                top: px(24),
                bottom: px(24),
                padding: UiRect::all(px(18)),
                flex_direction: FlexDirection::Column,
                row_gap: px(12),
                overflow: Overflow::scroll_y(),
                ..default()
            },
            BackgroundColor(Color::srgb(0.025, 0.05, 0.09)),
        ))
        .with_children(|panel| {
            panel.spawn((
                Text::new("SPACECRAFT CONTROLS"),
                ui_font(font, 22.0),
                TextColor(Color::srgb(0.9, 0.76, 0.42)),
            ));
            ui_row(panel, |row| {
                for (tab, label) in [
                    (Tab::Basic, "Basic maneuvers"),
                    (Tab::Rendezvous, "Rendezvous"),
                    (Tab::Landing, "Body transfer / landing"),
                    (Tab::Details, "Orbit / vectors / queues"),
                ] {
                    ui_button(row, font, Action::Tab(tab), label);
                }
                ui_button(row, font, Action::Close, "Close (Esc)");
            });
            panel.spawn((
                Content,
                Text::default(),
                ui_font(font, 15.0),
                Node {
                    flex_shrink: 0.0,
                    ..default()
                },
            ));
            panel
                .spawn((
                    Section(Tab::Basic),
                    Node {
                        flex_direction: FlexDirection::Column,
                        row_gap: px(8),
                        flex_shrink: 0.0,
                        ..default()
                    },
                ))
                .with_children(|basic| {
                    ui_row(basic, |row| {
                        ui_button(
                            row,
                            font,
                            Action::Execute(FlightAction::CircularizePeriapsis),
                            "Circularize at periapsis",
                        );
                        ui_button(
                            row,
                            font,
                            Action::Execute(FlightAction::CircularizeApoapsis),
                            "Circularize at apoapsis",
                        );
                    });
                    for field in Field::ALL {
                        ui_row(basic, |row| {
                            row.spawn((
                                Action::Edit(field),
                                Button,
                                Node {
                                    padding: UiRect::all(px(8)),
                                    ..default()
                                },
                                BackgroundColor(Color::srgb(0.07, 0.13, 0.2)),
                            ))
                            .with_children(|button| {
                                button.spawn((
                                    FieldLabel(field),
                                    Text::default(),
                                    ui_font(font, 15.0),
                                ));
                            });
                            ui_button(row, font, Action::Radius(field), "Queue maneuver");
                        });
                    }
                });
            for tab in [Tab::Rendezvous, Tab::Landing] {
                panel
                    .spawn((
                        Section(tab),
                        Node {
                            display: Display::None,
                            flex_direction: FlexDirection::Column,
                            row_gap: px(8),
                            flex_shrink: 0.0,
                            ..default()
                        },
                    ))
                    .with_children(|section| {
                        ui_row(section, |row| {
                            let actions: &[(Action, &str)] = if tab == Tab::Rendezvous {
                                &[
                                    (Action::Rendezvous, "Rendezvous"),
                                    (Action::Intercept, "Maneuver to point"),
                                    (Action::MatchPlanes, "Match planes"),
                                ]
                            } else {
                                &[
                                    (Action::Transfer, "Transfer to selected body"),
                                    (Action::Land, "Land on first settlement"),
                                ]
                            };
                            for &(action, label) in actions {
                                ui_button(row, font, action, label);
                            }
                            ui_button(row, font, Action::Previous, "Previous targets");
                            ui_button(row, font, Action::Next, "Next targets");
                        });
                    });
            }
            panel.spawn((
                Targets(font.clone()),
                Node {
                    display: Display::None,
                    flex_direction: FlexDirection::Column,
                    row_gap: px(4),
                    flex_shrink: 0.0,
                    ..default()
                },
            ));
        });
}
pub(super) fn controls(
    menu: Option<ResMut<Menu>>,
    keyboard: Res<ButtonInput<KeyCode>>,
    mut events: MessageReader<KeyboardInput>,
    buttons: Query<(&Action, &Interaction), Changed<Interaction>>,
    mut navigation: ResMut<Navigation>,
    mut game: ResMut<Game>,
) {
    let Some(mut menu) = menu else {
        events.clear();
        return;
    };
    if menu.capture_input != menu.open {
        menu.capture_input = menu.open;
    }
    let live = match navigation.focus {
        Focus::Ship(id) => game
            .simulation
            .solar_system
            .ships
            .get(id.0)
            .is_some_and(|ship| ship.status == FlightStatus::Orbiting && ship.error.is_none()),
        _ => false,
    };
    if !navigation.active || game.session.player_country.is_none() || !live {
        if menu.open {
            menu.apply(Action::Close, &mut navigation, &mut game)
                .unwrap();
        }
        events.clear();
        return;
    }
    if menu.open && keyboard.just_pressed(KeyCode::Escape) {
        if menu.focused.take().is_none() {
            menu.apply(Action::Close, &mut navigation, &mut game)
                .unwrap();
        }
        events.clear();
        return;
    }
    let control = keyboard.pressed(KeyCode::ControlLeft) || keyboard.pressed(KeyCode::ControlRight);
    for event in events
        .read()
        .filter(|event| event.state == ButtonState::Pressed)
    {
        let Some(field) = menu.focused else { continue };
        match event.key_code {
            KeyCode::Enter => menu.focused = None,
            KeyCode::Tab => {
                let step = if keyboard.pressed(KeyCode::ShiftLeft)
                    || keyboard.pressed(KeyCode::ShiftRight)
                {
                    3
                } else {
                    1
                };
                menu.focused = Some(Field::ALL[(field.index() + step) % 4]);
                menu.replace = true;
            }
            KeyCode::KeyA if control => menu.replace = true,
            KeyCode::Backspace => {
                if menu.replace {
                    menu.fields[field.index()].clear();
                } else {
                    menu.fields[field.index()].pop();
                }
                menu.replace = false;
            }
            _ if !control => {
                if let Some(text) = &event.text {
                    menu.enter_text(text);
                }
            }
            _ => {}
        }
    }
    for (action, interaction) in &buttons {
        if *interaction == Interaction::Pressed
            && let Err(error) = menu.apply(*action, &mut navigation, &mut game)
        {
            menu.message = error.to_string();
        }
    }
}
pub(super) fn scroll(
    menu: Option<Res<Menu>>,
    mouse: Res<AccumulatedMouseScroll>,
    mut panels: Query<&mut ScrollPosition, With<Panel>>,
) {
    if menu.is_none_or(|menu| !menu.open) || mouse.delta.y == 0.0 {
        return;
    }
    let scale = if mouse.unit == MouseScrollUnit::Line {
        32.0
    } else {
        1.0
    };
    for mut position in &mut panels {
        position.y = (position.y - mouse.delta.y * scale).max(0.0);
    }
}
#[allow(clippy::too_many_arguments, clippy::type_complexity)]
pub(super) fn update(
    mut commands: Commands,
    menu: Option<Res<Menu>>,
    game: Res<Game>,
    navigation: Res<Navigation>,
    mut panels: Query<(Entity, &mut Node), With<Panel>>,
    mut sections: Query<(&Section, &mut Node), Without<Panel>>,
    mut targets: Query<(Entity, &Targets, &mut Node), (Without<Panel>, Without<Section>)>,
    mut texts: Query<(&mut Text, Option<&Content>, Option<&FieldLabel>)>,
    children: Query<&Children>,
    mut previous: Local<Option<(Vec<Focus>, Option<Focus>, usize, Tab)>>,
) {
    let Some(menu) = menu else { return };
    let shown = menu.open
        && navigation.active
        && game.session.player_country.is_some()
        && super::event_ui::pending(&game).is_none();
    for (entity, mut node) in &mut panels {
        let display = if shown { Display::Flex } else { Display::None };
        if node.display != display {
            node.display = display;
        }
        if shown && (menu.is_changed() || game.is_changed() || navigation.is_changed()) {
            for child in children.iter_descendants(entity) {
                if let Ok((mut text, content, field)) = texts.get_mut(child) {
                    if content.is_some() {
                        text.0 = menu.content(&navigation, &game);
                    } else if let Some(field) = field {
                        text.0 = format!(
                            "{}: {}{}",
                            field.0.name(),
                            menu.fields[field.0.index()],
                            if menu.focused == Some(field.0) {
                                if menu.replace { " [selected]" } else { " |" }
                            } else {
                                ""
                            }
                        );
                    } else {
                        text.set_changed();
                    }
                }
            }
        }
    }
    for (section, mut node) in &mut sections {
        let display = if shown && menu.tab == section.0 {
            Display::Flex
        } else {
            Display::None
        };
        if node.display != display {
            node.display = display;
        }
    }
    let rows = menu.targets(&navigation, &game.simulation.solar_system);
    let page = menu.page.min(rows.len().saturating_sub(1) / PAGE_SIZE);
    let state = (rows, menu.selected(), page, menu.tab);
    for (entity, font, mut node) in &mut targets {
        let display = if shown && matches!(menu.tab, Tab::Rendezvous | Tab::Landing) {
            Display::Flex
        } else {
            Display::None
        };
        if node.display != display {
            node.display = display;
        }
        if previous.as_ref() != Some(&state) {
            commands.entity(entity).despawn_related::<Children>();
            commands.entity(entity).with_children(|list| {
                for &target in state.0.iter().skip(page * PAGE_SIZE).take(PAGE_SIZE) {
                    let name = target
                        .name(&game.simulation.solar_system)
                        .unwrap_or("Unknown target");
                    ui_button(
                        list,
                        &font.0,
                        Action::Target(target),
                        &format!(
                            "{} {name}",
                            if Some(target) == state.1 {
                                "[selected]"
                            } else {
                                "·"
                            }
                        ),
                    );
                }
            });
        }
    }
    *previous = Some(state);
}

#[cfg(test)]
mod tests {
    use super::*;
    use cqsp_core::{
        flight::{Burn, Command, Spacecraft, Trigger},
        orbit::Orbit,
        solar_system::{BodyId, ShipId},
    };
    use std::time::Duration;
    fn app() -> Result<(App, ShipId)> {
        let mut app = super::super::inspection_ui::tests::app()?;
        app.init_resource::<Menu>();
        let id = {
            let mut game = app.world_mut().resource_mut::<Game>();
            let system = &mut game.simulation.solar_system;
            let earth = system.body_id("earth").unwrap();
            system.launch_ship(
                "UI test craft".into(),
                earth,
                Orbit {
                    semi_major_axis: 12000.0,
                    inclination: 0.3,
                    ascending_node: 0.2,
                    periapsis_argument: 0.4,
                    mean_anomaly: 0.5,
                    mu: system.bodies[earth.0].mu,
                    ..default()
                },
            )?
        };
        app.world_mut().resource_scope(|world, game: Mut<Game>| {
            let mut navigation = world.resource_mut::<Navigation>();
            navigation.active = true;
            navigation.select(Focus::Ship(id), &game.simulation.solar_system);
        });
        app.update();
        Ok((app, id))
    }
    fn apply(app: &mut App, action: Action) -> Result<()> {
        app.world_mut()
            .resource_scope(|world, mut menu: Mut<Menu>| {
                world.resource_scope(|world, mut game: Mut<Game>| {
                    menu.apply(action, &mut world.resource_mut::<Navigation>(), &mut game)
                })
            })
    }
    fn click(app: &mut App, action: Action) {
        let button = app.world_mut().spawn((action, Interaction::Pressed)).id();
        app.update();
        app.world_mut().despawn(button);
    }
    fn key(app: &mut App, code: KeyCode, text: Option<&str>) {
        app.world_mut().write_message(KeyboardInput {
            key_code: code,
            logical_key: bevy::input::keyboard::Key::Unidentified(
                bevy::input::keyboard::NativeKey::Unidentified,
            ),
            state: ButtonState::Pressed,
            text: text.map(Into::into),
            repeat: false,
            window: Entity::PLACEHOLDER,
        });
        app.world_mut()
            .resource_mut::<ButtonInput<KeyCode>>()
            .press(code);
        app.update();
        app.world_mut()
            .resource_mut::<ButtonInput<KeyCode>>()
            .reset_all();
    }
    #[test]
    fn solar_targets_are_not_hardcoded_to_the_moon_and_missing_settlements_do_not_change_queues()
    -> Result<()> {
        let (mut app, _) = app()?;
        let id = {
            let mut game = app.world_mut().resource_mut::<Game>();
            let system = &mut game.simulation.solar_system;
            system.launch_ship(
                "Solar craft".into(),
                system.sun,
                Orbit {
                    semi_major_axis: 120000000.0,
                    inclination: 0.2,
                    ascending_node: 0.3,
                    mu: system.bodies[system.sun.0].mu,
                    ..default()
                },
            )?
        };
        app.world_mut().resource_scope(|world, game: Mut<Game>| {
            world
                .resource_mut::<Navigation>()
                .select(Focus::Ship(id), &game.simulation.solar_system);
        });
        apply(&mut app, Action::Open)?;
        apply(&mut app, Action::Tab(Tab::Landing))?;
        let game = app.world().resource::<Game>();
        let system = &game.simulation.solar_system;
        let mars = system.body_id("mars").unwrap();
        let rows = app
            .world()
            .resource::<Menu>()
            .targets(app.world().resource::<Navigation>(), system);
        assert_eq!(rows.len(), 9);
        assert!(rows.contains(&Focus::Body(mars)));
        apply(&mut app, Action::Target(Focus::Body(mars)))?;
        apply(&mut app, Action::Transfer)?;
        let ship = &app.world().resource::<Game>().simulation.solar_system.ships[id.0];
        let before = (ship.maneuvers.len(), ship.commands.len());
        assert_eq!(before, (1, 3));
        assert!(apply(&mut app, Action::Land).is_err());
        let ship = &app.world().resource::<Game>().simulation.solar_system.ships[id.0];
        assert_eq!((ship.maneuvers.len(), ship.commands.len()), before);
        Ok(())
    }
    fn check_burns(ship: &Spacecraft, burns: impl IntoIterator<Item = Burn>, time: f64) {
        let expected: Vec<_> = burns
            .into_iter()
            .map(|burn| burn.at(time).unwrap())
            .collect();
        assert_eq!(ship.maneuvers.len(), expected.len());
        for (actual, expected) in ship.maneuvers.iter().zip(expected) {
            assert_eq!(actual.delta_v, expected.delta_v);
            assert_eq!(actual.time, expected.time);
        }
    }
    #[test]
    fn numeric_drafts_are_independent_precise_persistent_and_invalid_plans_are_atomic() -> Result<()>
    {
        let (mut app, id) = app()?;
        apply(&mut app, Action::Open)?;
        assert_eq!(
            app.world().resource::<Menu>().fields,
            ["0", "0", "0", "8000"].map(String::from)
        );
        let time = app.world().resource::<Game>().simulation.date.seconds();
        let orbit = app.world().resource::<Game>().simulation.solar_system.ships[id.0].orbit;
        for (field, value, burns) in [
            (
                Field::Apoapsis,
                "14567.8901234567",
                vec![orbit.set_apoapsis(14567.8901234567)],
            ),
            (Field::Periapsis, "8e3", vec![orbit.set_periapsis(8000.0)]),
            (
                Field::Orbit,
                "20000.125",
                orbit.hohmann_transfer(20000.125).unwrap().into(),
            ),
        ] {
            app.world_mut().resource_mut::<Menu>().fields[field.index()] = value.into();
            app.world_mut()
                .resource_mut::<Game>()
                .simulation
                .solar_system
                .ships[id.0]
                .maneuvers
                .clear();
            apply(&mut app, Action::Radius(field))?;
            check_burns(
                &app.world().resource::<Game>().simulation.solar_system.ships[id.0],
                burns,
                time,
            );
            assert!(app.world().resource::<Game>().paused);
        }
        let fields = app.world().resource::<Menu>().fields.clone();
        apply(&mut app, Action::Close)?;
        apply(&mut app, Action::Open)?;
        assert_eq!(app.world().resource::<Menu>().fields, fields);
        let before = app.world().resource::<Game>().simulation.solar_system.ships[id.0]
            .maneuvers
            .len();
        for value in ["", "garbage", "nan", "inf", "-inf", "1e999"] {
            app.world_mut().resource_mut::<Menu>().fields[Field::Orbit.index()] = value.into();
            assert!(apply(&mut app, Action::Radius(Field::Orbit)).is_err());
            assert_eq!(
                app.world().resource::<Game>().simulation.solar_system.ships[id.0]
                    .maneuvers
                    .len(),
                before
            );
        }
        // The source InputDouble is unbounded, not an altitude-clamped slider.
        for radius in [0.0, -100.0] {
            app.world_mut().resource_mut::<Menu>().fields[0] = radius.to_string();
            assert_eq!(
                app.world().resource::<Menu>().number(Field::Apoapsis)?,
                radius
            );
            let valid = orbit.set_apoapsis(radius).at(time).is_ok();
            assert_eq!(
                apply(&mut app, Action::Radius(Field::Apoapsis)).is_ok(),
                valid
            );
        }
        Ok(())
    }
    #[test]
    fn rendezvous_point_and_plane_actions_use_the_selected_live_target_and_keep_queue_order()
    -> Result<()> {
        let (mut app, id) = app()?;
        apply(&mut app, Action::Open)?;
        apply(&mut app, Action::Tab(Tab::Rendezvous))?;
        let target = {
            let mut game = app.world_mut().resource_mut::<Game>();
            let system = &mut game.simulation.solar_system;
            let source = system.ships[id.0].orbit;
            system.launch_ship(
                "Target craft".into(),
                system.ships[id.0].parent,
                Orbit {
                    semi_major_axis: 22000.0,
                    inclination: 0.7,
                    ascending_node: 1.2,
                    ..source
                },
            )?
        };
        apply(&mut app, Action::Target(Focus::Ship(target)))?;
        let game = app.world().resource::<Game>();
        let source = game.simulation.solar_system.ships[id.0].orbit;
        let destination = game.simulation.solar_system.ships[target.0].orbit;
        let time = game.simulation.date.seconds();
        let expected: Vec<_> = source.coplanar_intercept(destination, time).into();
        apply(&mut app, Action::Rendezvous)?;
        apply(&mut app, Action::Intercept)?;
        apply(&mut app, Action::MatchPlanes)?;
        let burns = expected.into_iter().chain([
            source.coplanar_intercept(destination, time)[0],
            source.match_planes(destination),
        ]);
        check_burns(
            &app.world().resource::<Game>().simulation.solar_system.ships[id.0],
            burns,
            time,
        );
        app.world_mut()
            .resource_mut::<Game>()
            .simulation
            .solar_system
            .ships[target.0]
            .status = FlightStatus::Docked(0);
        assert!(apply(&mut app, Action::Rendezvous).is_err());
        assert_eq!(
            app.world().resource::<Menu>().rendezvous,
            Some(Focus::Ship(target))
        );
        assert_eq!(
            app.world().resource::<Game>().simulation.solar_system.ships[id.0]
                .maneuvers
                .len(),
            4
        );
        // Identical-period interception yields non-finite wait times upstream; append neither burn.
        apply(&mut app, Action::Target(Focus::Ship(id)))?;
        assert!(apply(&mut app, Action::Rendezvous).is_err());
        assert_eq!(
            app.world().resource::<Game>().simulation.solar_system.ships[id.0]
                .maneuvers
                .len(),
            4
        );
        Ok(())
    }
    #[test]
    fn targets_page_all_siblings_without_changing_selection_and_reject_foreign_or_stale_ids()
    -> Result<()> {
        let (mut app, id) = app()?;
        apply(&mut app, Action::Open)?;
        apply(&mut app, Action::Tab(Tab::Rendezvous))?;
        let rows = app.world().resource::<Menu>().targets(
            app.world().resource::<Navigation>(),
            &app.world().resource::<Game>().simulation.solar_system,
        );
        let selected = app.world().resource::<Menu>().rendezvous;
        assert!(rows.contains(&Focus::Ship(id)));
        let mut seen = Vec::new();
        for _ in 0..rows.len().div_ceil(PAGE_SIZE) {
            let page = app.world().resource::<Menu>().page;
            seen.extend(rows.iter().skip(page * PAGE_SIZE).take(PAGE_SIZE).copied());
            apply(&mut app, Action::Next)?;
        }
        assert_eq!(seen, rows);
        assert_eq!(app.world().resource::<Menu>().rendezvous, selected);
        assert!(apply(&mut app, Action::Target(Focus::Body(BodyId(usize::MAX)))).is_err());
        assert!(apply(&mut app, Action::Target(Focus::Ship(ShipId(usize::MAX)))).is_err());
        let mars = app
            .world()
            .resource::<Game>()
            .simulation
            .solar_system
            .body_id("mars")
            .unwrap();
        assert!(apply(&mut app, Action::Target(Focus::Body(mars))).is_err());
        apply(&mut app, Action::Tab(Tab::Landing))?;
        let game = app.world().resource::<Game>();
        let rows = app.world().resource::<Menu>().targets(
            app.world().resource::<Navigation>(),
            &game.simulation.solar_system,
        );
        assert_eq!(
            rows,
            vec![Focus::Body(
                game.simulation.solar_system.body_id("moon").unwrap()
            )]
        );
        assert!(apply(&mut app, Action::Transfer).is_err());
        assert!(apply(&mut app, Action::Target(Focus::Ship(id))).is_err());
        Ok(())
    }
    #[test]
    fn selected_body_transfer_and_landing_use_original_sequences_and_first_settlement() -> Result<()>
    {
        let (mut app, id) = app()?;
        apply(&mut app, Action::Open)?;
        apply(&mut app, Action::Tab(Tab::Landing))?;
        let game = app.world().resource::<Game>();
        let system = &game.simulation.solar_system;
        let moon = system.body_id("moon").unwrap();
        let time = game.simulation.date.seconds();
        let first = game
            .simulation
            .settlements
            .iter()
            .position(|site| site.body == moon)
            .unwrap();
        let mut expected = Spacecraft::new(
            "expected".into(),
            system.ships[id.0].parent,
            system.ships[id.0].orbit,
        );
        expected.land_on_moon(moon, &system.bodies[moon.0], first, time)?;
        apply(&mut app, Action::Target(Focus::Body(moon)))?;
        apply(&mut app, Action::Land)?;
        let actual = &app.world().resource::<Game>().simulation.solar_system.ships[id.0];
        assert_eq!(
            format!("{:?}", actual.maneuvers),
            format!("{:?}", expected.maneuvers)
        );
        assert_eq!(
            format!("{:?}", actual.commands),
            format!("{:?}", expected.commands)
        );
        assert!(
            matches!(actual.commands.back().unwrap().command, Command::Land(site) if site == first)
        );
        assert_eq!(actual.commands.back().unwrap().trigger, Trigger::Crash);
        apply(&mut app, Action::Close)?;
        apply(&mut app, Action::Open)?;
        assert_eq!(
            app.world().resource::<Menu>().landing,
            Some(Focus::Body(moon))
        );
        Ok(())
    }
    #[test]
    fn departure_uses_the_entered_parent_radius_and_errors_cannot_enqueue_partial_plans()
    -> Result<()> {
        let (mut app, id) = app()?;
        apply(&mut app, Action::Open)?;
        app.world_mut().resource_mut::<Menu>().fields[Field::Departure.index()] = "1.5e8".into();
        let game = app.world().resource::<Game>();
        let ship = &game.simulation.solar_system.ships[id.0];
        let time = game.simulation.date.seconds();
        let burn = ship.orbit.depart_body(
            ship.kinematics.state,
            &game.simulation.solar_system.bodies[ship.parent.0],
            1.5e8,
        );
        apply(&mut app, Action::Radius(Field::Departure))?;
        check_burns(
            &app.world().resource::<Game>().simulation.solar_system.ships[id.0],
            [burn],
            time,
        );
        // Hohmann has no implementation for a non-circular source orbit.
        app.world_mut()
            .resource_mut::<Game>()
            .simulation
            .solar_system
            .ships[id.0]
            .orbit
            .eccentricity = 0.1;
        app.world_mut().resource_mut::<Menu>().fields[Field::Orbit.index()] = "20000".into();
        assert!(apply(&mut app, Action::Radius(Field::Orbit)).is_err());
        assert_eq!(
            app.world().resource::<Game>().simulation.solar_system.ships[id.0]
                .maneuvers
                .len(),
            1
        );
        let sun = app.world().resource::<Game>().simulation.solar_system.sun;
        app.world_mut()
            .resource_mut::<Game>()
            .simulation
            .solar_system
            .ships[id.0]
            .parent = sun;
        assert!(apply(&mut app, Action::Radius(Field::Departure)).is_err());
        Ok(())
    }
    #[test]
    fn editing_captures_shortcuts_clicks_once_and_closing_frame_without_pausing_running_time()
    -> Result<()> {
        let (mut app, id) = app()?;
        click(&mut app, Action::Open);
        click(&mut app, Action::Edit(Field::Apoapsis));
        key(&mut app, KeyCode::Digit1, Some("14567.8901234567"));
        assert_eq!(app.world().resource::<Menu>().fields[0], "14567.8901234567");
        for code in [
            KeyCode::Space,
            KeyCode::KeyN,
            KeyCode::KeyP,
            KeyCode::KeyI,
            KeyCode::KeyT,
        ] {
            key(&mut app, code, None);
        }
        app.world_mut()
            .resource_mut::<ButtonInput<KeyCode>>()
            .press(KeyCode::ControlLeft);
        key(&mut app, KeyCode::KeyP, Some("p"));
        assert!(
            !app.world()
                .resource::<super::super::spaceport_ui::SpaceportUi>()
                .open
        );
        assert!(
            !app.world()
                .resource::<super::super::inspection_ui::Inspector>()
                .open
        );
        assert!(app.world().resource::<Game>().paused);
        key(&mut app, KeyCode::Tab, None);
        assert_eq!(
            app.world().resource::<Menu>().focused,
            Some(Field::Periapsis)
        );
        key(&mut app, KeyCode::Digit8, Some("8e3"));
        key(&mut app, KeyCode::Enter, None);
        assert_eq!(app.world().resource::<Menu>().focused, None);
        let button = app
            .world_mut()
            .spawn((Action::Radius(Field::Apoapsis), Interaction::Pressed))
            .id();
        app.update();
        app.update();
        assert_eq!(
            app.world().resource::<Game>().simulation.solar_system.ships[id.0]
                .maneuvers
                .len(),
            1
        );
        app.world_mut().despawn(button);
        app.world_mut().resource_mut::<Game>().paused = false;
        let tick = app.world().resource::<Game>().simulation.date.tick();
        app.world_mut()
            .resource_mut::<Time>()
            .advance_by(Duration::from_secs(2));
        app.update();
        assert!(app.world().resource::<Game>().simulation.date.tick() > tick);
        assert!(app.world().resource::<Menu>().open);
        key(&mut app, KeyCode::Escape, None);
        assert!(!app.world().resource::<Menu>().open);
        assert!(app.world().resource::<Menu>().capture_input);
        app.update();
        assert!(!app.world().resource::<Menu>().capture_input);
        Ok(())
    }
    #[test]
    fn details_include_original_orbit_vectors_and_full_queue_and_inactive_craft_closes_safely()
    -> Result<()> {
        let (mut app, id) = app()?;
        click(&mut app, Action::Open);
        click(
            &mut app,
            Action::Execute(FlightAction::CircularizePeriapsis),
        );
        click(&mut app, Action::Tab(Tab::Details));
        let menu = app.world().resource::<Menu>();
        let text = menu.content(
            app.world().resource::<Navigation>(),
            app.world().resource::<Game>(),
        );
        for field in [
            "mean anomaly at epoch",
            "True anomaly:",
            "epoch:",
            "GM:",
            "Position (km):",
            "Velocity (km/s):",
            "MANEUVER QUEUE",
            "Burn in",
        ] {
            assert!(text.contains(field), "Missing {field}");
        }
        app.world_mut()
            .resource_mut::<Game>()
            .simulation
            .solar_system
            .ships[id.0]
            .status = FlightStatus::Docked(0);
        app.update();
        assert!(!app.world().resource::<Menu>().open);
        assert!(app.world().resource::<Menu>().capture_input);
        assert_eq!(app.world().resource::<Menu>().focused, None);
        Ok(())
    }
}
