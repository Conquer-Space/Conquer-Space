//! Native vehicle, spaceport and lunar mission controls; no scripting runtime.
use super::{Game, ui_button as button, ui_font, ui_row as row};
use anyhow::{Context, Result, ensure};
use bevy::{
    input::{ButtonState, keyboard::KeyboardInput},
    prelude::*,
};
use cqsp_core::{
    clock::DAY,
    orbit::Orbit,
    simulation::Simulation,
    spaceflight::{LaunchVehicle, MissionId, VehicleId},
};
use std::fmt::Write;

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub(super) enum Field {
    Name,
    Reliability,
    Fairing,
    Mass,
    Axis,
    Eccentricity,
    Azimuth,
    Node,
    Periapsis,
    Amount,
}
impl Field {
    const ALL: [Self; 10] = [
        Self::Name,
        Self::Reliability,
        Self::Fairing,
        Self::Mass,
        Self::Axis,
        Self::Eccentricity,
        Self::Azimuth,
        Self::Node,
        Self::Periapsis,
        Self::Amount,
    ];
    fn label(self) -> &'static str {
        match self {
            Self::Name => "Vehicle name",
            Self::Reliability => "Reliability (0–100)",
            Self::Fairing => "Fairing input",
            Self::Mass => "Mass to orbit (tons)",
            Self::Axis => "Orbit axis (km)",
            Self::Eccentricity => "Eccentricity",
            Self::Azimuth => "Azimuth (degrees)",
            Self::Node => "Ascending node (degrees)",
            Self::Periapsis => "Periapsis angle (degrees)",
            Self::Amount => "Cargo amount",
        }
    }
}

#[derive(Component, Clone, Copy)]
pub(super) enum Action {
    Edit(Field),
    Open,
    Close,
    NextPort,
    NextVehicle,
    Design,
    Manufacture,
    Launch,
    PreviousTarget,
    NextTarget,
    Colonize,
    QueueStage,
    QueueCore,
    NextMission,
    Dispatch,
    NextGood,
    QueueGoods,
    NextDay,
}
#[derive(Component)]
pub(super) enum Panel {
    Overlay,
    Launcher,
    Background,
}
#[derive(Component)]
pub(super) enum Label {
    Field(Field),
    Port,
    Mission,
    Feedback,
}

#[derive(Resource)]
pub(super) struct SpaceportUi {
    pub open: bool,
    pub capture_input: bool,
    focused: Option<Field>,
    replace: bool,
    values: [String; 10],
    ports: Vec<usize>,
    targets: Vec<usize>,
    port: usize,
    target: usize,
    vehicle: Option<VehicleId>,
    mission: Option<MissionId>,
    good: usize,
    message: String,
}
impl SpaceportUi {
    pub fn editing(&self) -> bool {
        self.focused.is_some()
    }
    pub fn new(simulation: &Simulation) -> Self {
        let moon = simulation.solar_system.body_id("moon");
        Self {
            open: false,
            capture_input: false,
            focused: None,
            replace: false,
            values: [
                "New Launch Vehicle",
                "90",
                "500",
                "10",
                "7000",
                "0.001",
                "90",
                "0",
                "0",
                "100",
            ]
            .map(str::to_owned),
            ports: simulation
                .settlements
                .iter()
                .enumerate()
                .filter(|(_, site)| site.spaceport.is_some())
                .map(|(id, _)| id)
                .collect(),
            targets: simulation
                .settlements
                .iter()
                .enumerate()
                .filter(|(_, site)| site.is_province() && Some(site.body) == moon)
                .map(|(id, _)| id)
                .collect(),
            port: 0,
            target: 0,
            vehicle: None,
            mission: None,
            good: 0,
            message:
                "Click a field to edit. Ctrl+A replaces; Tab moves; Enter finishes. P / Esc closes."
                    .into(),
        }
    }
    fn number(&self, field: Field) -> Result<f64> {
        let number: f64 = self.values[field as usize]
            .trim()
            .parse()
            .with_context(|| format!("{} needs a number", field.label()))?;
        ensure!(number.is_finite(), "{} must be finite", field.label());
        Ok(number)
    }
    pub fn apply(&mut self, action: Action, game: &mut Game) -> Result<()> {
        let country = game
            .session
            .player_country
            .context("Start a country first")?;
        if let Action::Edit(field) = action {
            self.focused = Some(field);
            self.replace = true;
            return Ok(());
        }
        self.focused = None;
        let simulation = &mut game.simulation;
        match action {
            Action::Edit(_) => unreachable!(),
            Action::Open => self.open = true,
            Action::Close => self.open = false,
            Action::NextPort => {
                ensure!(!self.ports.is_empty(), "No spaceports in loaded data");
                self.port = (self.port + 1) % self.ports.len();
            }
            Action::NextVehicle => {
                let vehicles = &simulation.countries[country].vehicles;
                ensure!(!vehicles.is_empty(), "Design a vehicle first");
                let next = self
                    .vehicle
                    .and_then(|id| vehicles.iter().position(|value| *value == id))
                    .map_or(0, |index| (index + 1) % vehicles.len());
                self.vehicle = Some(vehicles[next]);
            }
            Action::Design => {
                let vehicle = LaunchVehicle::design(
                    self.values[Field::Name as usize].clone(),
                    self.number(Field::Reliability)?,
                    self.number(Field::Fairing)?,
                    self.number(Field::Mass)?,
                )?;
                self.vehicle = Some(simulation.design_vehicle(country, vehicle)?);
                self.message = "Vehicle design added to your country.".into();
            }
            Action::Manufacture => {
                simulation.manufacture_vehicle(
                    self.city()?,
                    self.vehicle.context("Design or select a vehicle first")?,
                )?;
                self.message = "Manufacturing queued: 90 daily production steps. No costs are charged by the original system.".into();
            }
            Action::Launch => {
                let ship = simulation.launch_vehicle(
                    self.city()?,
                    Orbit {
                        semi_major_axis: self.number(Field::Axis)?,
                        eccentricity: self.number(Field::Eccentricity)?,
                        ascending_node: self.number(Field::Node)?.to_radians(),
                        periapsis_argument: self.number(Field::Periapsis)?.to_radians(),
                        ..Orbit::default()
                    },
                    self.number(Field::Azimuth)?.to_radians(),
                )?;
                self.message = format!(
                    "Launched {}. V opens system view; Q selects spacecraft.",
                    simulation.solar_system.ships[ship.0].name
                );
            }
            Action::PreviousTarget | Action::NextTarget => {
                ensure!(!self.targets.is_empty(), "No lunar provinces loaded");
                self.target = (self.target
                    + if matches!(action, Action::PreviousTarget) {
                        self.targets.len() - 1
                    } else {
                        1
                    })
                    % self.targets.len();
            }
            Action::Colonize => {
                simulation.begin_colonization(self.province()?, country)?;
                self.message =
                    "Colonization target set. Queue a stage or colony core mission.".into();
            }
            Action::QueueStage | Action::QueueCore => {
                self.mission = Some(simulation.queue_colonization_mission(
                    self.province()?,
                    country,
                    matches!(action, Action::QueueCore),
                )?);
                self.message = "Mission queued. Your first owned port dispatches automatically each day; Dispatch uses the selected port.".into();
            }
            Action::NextMission => {
                let missions = &simulation.countries[country].missions;
                ensure!(!missions.is_empty(), "Queue a mission first");
                let next = self
                    .mission
                    .and_then(|id| missions.iter().position(|value| *value == id))
                    .map_or(0, |index| (index + 1) % missions.len());
                self.mission = Some(missions[next]);
            }
            Action::Dispatch => {
                simulation.dispatch_mission(
                    self.city()?,
                    self.mission.context("Select a mission first")?,
                )?;
                self.message = "Mission assigned to this port's delivery queue. It launches on the next daily step.".into();
            }
            Action::NextGood => self.good = (self.good + 1) % simulation.catalog.goods.len(),
            Action::QueueGoods => {
                let province = self.province()?;
                simulation.queue_goods(
                    self.city()?,
                    simulation.settlements[province].body,
                    cqsp_core::economy::GoodId(self.good),
                    self.number(Field::Amount)?,
                    Some(province),
                )?;
                self.message = "Cargo queued for the selected lunar province. The original province docking does not unload goods without a port.".into();
            }
            Action::NextDay => {
                game.paused = true;
                game.last_tick = 0.0;
                simulation.advance_day();
            }
        }
        Ok(())
    }
    fn city(&self) -> Result<usize> {
        self.ports
            .get(self.port)
            .copied()
            .context("No spaceport selected")
    }
    fn province(&self) -> Result<usize> {
        self.targets
            .get(self.target)
            .copied()
            .context("No lunar province selected")
    }
    fn port_status(&self, game: &Game) -> String {
        let Ok(city) = self.city() else {
            return "No spaceports loaded.".into();
        };
        let simulation = &game.simulation;
        let site = &simulation.settlements[city];
        let port = site.spaceport.as_ref().unwrap();
        let owner = site
            .country
            .map_or("Unowned", |id| game.world.countries[id].common.as_str());
        let vehicle = self.vehicle.map_or("No design selected".into(), |id| {
            let vehicle = &simulation.vehicles[id.0];
            format!(
                "{} · reliability {:.0}% · fairing {:.2} · mass {:.1} t",
                vehicle.name,
                vehicle.reliability * 100.0,
                vehicle.fairing_size,
                vehicle.mass_to_orbit
            )
        });
        let mut text = format!(
            "{} · {}\n{} · {}\n\n{}\nStored vehicles: {} · projects: {} · shipments: {}",
            site.name,
            simulation.solar_system.bodies[site.body.0].name,
            owner,
            simulation.date.display(),
            vehicle,
            port.stored_launch_vehicles.len(),
            port.projects.len(),
            port.deliveries.values().map(Vec::len).sum::<usize>()
        );
        if let Some(zone) = site
            .time_zone
            .map(|id| &simulation.time_zones.entries[id.0])
        {
            let (hour, minute) = zone.local_time(simulation.date);
            let _ = write!(
                text,
                "\n{} · legacy local time {hour:02}:{minute:02} · fixed offset {:+}h",
                zone.identifier, zone.offset
            );
        }
        for project in port.projects.iter().take(3) {
            let _ = write!(
                text,
                "\n{}: {:.0}/{:.0} days",
                simulation.vehicles[project.result.0].name,
                project.progress as f64 / DAY as f64,
                project.max_progress as f64 / DAY as f64
            );
        }
        text
    }
    fn mission_status(&self, game: &Game) -> String {
        let Ok(province) = self.province() else {
            return "No lunar provinces loaded.".into();
        };
        let simulation = &game.simulation;
        let site = &simulation.settlements[province];
        let step = site
            .colonization
            .as_ref()
            .map_or("Not started", |target| target.step.name());
        let owner = site
            .country
            .map_or("Unowned", |id| game.world.countries[id].common.as_str());
        let cores = site
            .colony
            .as_ref()
            .map_or(0, |colony| colony.components.len());
        let mission = self.mission.map_or("No mission selected".into(), |id| {
            let mission = &simulation.missions[id.0];
            format!(
                "{} -> {}\n{}",
                mission.name,
                simulation.settlements[mission.province].name,
                if mission.in_progress {
                    "In progress (marker retained after delivery)"
                } else {
                    "Awaiting dispatch"
                }
            )
        });
        format!(
            "Moon province {}/{}: {}\nOwner: {} · stage: {}\nColony components: {}\n\n{}\n\nCargo good: {}",
            self.target + 1,
            self.targets.len(),
            site.name,
            owner,
            step,
            cores,
            mission,
            simulation.catalog.goods[self.good].name
        )
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
                bottom: px(24),
                padding: UiRect::all(px(12)),
                ..default()
            },
            BackgroundColor(Color::srgb(0.15, 0.29, 0.43)),
        ))
        .with_children(|button| {
            button.spawn((Text::new("Spaceports & missions (P)"), ui_font(font, 16.0)));
        });
    commands.spawn((Panel::Overlay, UiTargetCamera(camera), GlobalZIndex(10),
        Node { display: Display::None, position_type: PositionType::Absolute,
            left: px(40), right: px(40), top: px(24), bottom: px(24), padding: UiRect::all(px(20)),
            flex_direction: FlexDirection::Column, row_gap: px(10), overflow: Overflow::scroll_y(), ..default() },
        BackgroundColor(Color::srgb(0.025, 0.05, 0.09))))
        .with_children(|panel| {
            panel.spawn((Text::new("SPACEPORTS · VEHICLES · LUNAR MISSIONS"), ui_font(font, 22.0), TextColor(Color::srgb(0.9, 0.76, 0.42))));
            row(panel, |bar| { button(bar, font, Action::NextPort, "Next port"); button(bar, font, Action::NextDay, "Advance day"); button(bar, font, Action::Close, "Close (P / Esc)"); });
            row(panel, |columns| {
                columns.spawn(Node { width: percent(49), flex_direction: FlexDirection::Column, row_gap: px(7), ..default() }).with_children(|column| {
                    column.spawn((Label::Port, Text::default(), ui_font(font, 15.0)));
                    row(column, |bar| { button(bar, font, Action::NextVehicle, "Next design"); button(bar, font, Action::Manufacture, "Manufacture"); });
                    field_button(column, font, Field::Name);
                    row(column, |bar| { for field in [Field::Reliability, Field::Fairing, Field::Mass] { field_button(bar, font, field); } });
                    button(column, font, Action::Design, "Create design");
                    column.spawn((Text::new("LAUNCH · last manufactured vehicle"), ui_font(font, 16.0)));
                    for field in [Field::Axis, Field::Eccentricity, Field::Azimuth, Field::Node, Field::Periapsis] { field_button(column, font, field); }
                    button(column, font, Action::Launch, "Launch vehicle");
                });
                columns.spawn(Node { width: percent(49), flex_direction: FlexDirection::Column, row_gap: px(10), ..default() }).with_children(|column| {
                    column.spawn((Label::Mission, Text::default(), ui_font(font, 16.0)));
                    row(column, |bar| { button(bar, font, Action::PreviousTarget, "Previous province"); button(bar, font, Action::NextTarget, "Next province"); });
                    button(column, font, Action::Colonize, "Begin colonization");
                    row(column, |bar| { button(bar, font, Action::QueueStage, "Queue stage mission"); button(bar, font, Action::QueueCore, "Queue colony core"); });
                    row(column, |bar| { button(bar, font, Action::NextMission, "Next mission"); button(bar, font, Action::Dispatch, "Dispatch at this port"); });
                    button(column, font, Action::NextGood, "Next cargo good");
                    field_button(column, font, Field::Amount);
                    button(column, font, Action::QueueGoods, "Queue cargo to this province");
                    column.spawn((Text::new("Source rules retained:\nDelivery queues are LIFO; cargo missions do not consume manufactured vehicles.\nManual launches consume one vehicle.\nPort ownership is not checked by original manual actions.\nFive province deliveries complete colonization.\nNo audio output is built in."), ui_font(font, 14.0), TextColor(Color::srgb(0.6, 0.7, 0.82))));
                });
            });
            panel.spawn((Label::Feedback, Text::default(), ui_font(font, 15.0), TextColor(Color::srgb(0.9, 0.76, 0.42))));
        });
}
fn field_button(parent: &mut ChildSpawnerCommands, font: &Handle<Font>, field: Field) {
    parent
        .spawn((
            Action::Edit(field),
            Button,
            Node {
                padding: UiRect::axes(px(8), px(5)),
                ..default()
            },
            BackgroundColor(Color::srgb(0.07, 0.13, 0.2)),
        ))
        .with_children(|parent| {
            parent.spawn((Label::Field(field), Text::default(), ui_font(font, 14.0)));
        });
}

pub(super) fn controls(
    keyboard: Res<ButtonInput<KeyCode>>,
    mut events: MessageReader<KeyboardInput>,
    buttons: Query<(&Action, &Interaction), Changed<Interaction>>,
    mut ui: ResMut<SpaceportUi>,
    mut game: ResMut<Game>,
) {
    if ui.capture_input != ui.open {
        ui.capture_input = ui.open;
    }
    if game.session.player_country.is_none() {
        ui.open = false;
        ui.focused = None;
        ui.capture_input = false;
        events.clear();
        return;
    }
    if ui.focused.is_none()
        && keyboard.just_pressed(KeyCode::KeyP)
        && !keyboard.pressed(KeyCode::ControlLeft)
    {
        ui.open = !ui.open;
    }
    for (action, interaction) in &buttons {
        if matches!(action, Action::Open) && *interaction == Interaction::Pressed {
            ui.open = true;
        }
    }
    if ui.open && !ui.capture_input {
        ui.capture_input = true;
    }
    if !ui.open {
        events.clear();
        return;
    }
    if keyboard.just_pressed(KeyCode::Escape) {
        if ui.focused.take().is_none() {
            ui.open = false;
        }
        events.clear();
        return;
    }
    for event in events
        .read()
        .filter(|event| event.state == ButtonState::Pressed)
    {
        let Some(field) = ui.focused else {
            continue;
        };
        match event.key_code {
            KeyCode::Enter => ui.focused = None,
            KeyCode::Tab => {
                ui.focused = Some(
                    Field::ALL[(field as usize
                        + if keyboard.pressed(KeyCode::ShiftLeft)
                            || keyboard.pressed(KeyCode::ShiftRight)
                        {
                            Field::ALL.len() - 1
                        } else {
                            1
                        })
                        % Field::ALL.len()],
                );
                ui.replace = true;
            }
            KeyCode::KeyA
                if keyboard.pressed(KeyCode::ControlLeft)
                    || keyboard.pressed(KeyCode::ControlRight) =>
            {
                ui.replace = true
            }
            KeyCode::Backspace => {
                if ui.replace {
                    ui.values[field as usize].clear();
                } else {
                    ui.values[field as usize].pop();
                }
                ui.replace = false;
            }
            _ if !keyboard.pressed(KeyCode::ControlLeft)
                && !keyboard.pressed(KeyCode::ControlRight) =>
            {
                if let Some(text) = &event.text {
                    let text: String = text.chars().filter(|ch| !ch.is_control()).collect();
                    if !text.is_empty() {
                        if ui.replace {
                            ui.values[field as usize].clear();
                        }
                        ui.replace = false;
                        let remaining =
                            255usize.saturating_sub(ui.values[field as usize].chars().count());
                        ui.values[field as usize].extend(text.chars().take(remaining));
                    }
                }
            }
            _ => {}
        }
    }
    for (action, interaction) in &buttons {
        if *interaction == Interaction::Pressed
            && let Err(error) = ui.apply(*action, &mut game)
        {
            ui.message = error.to_string();
        }
    }
}

pub(super) fn scroll(
    ui: Res<SpaceportUi>,
    mouse: Res<bevy::input::mouse::AccumulatedMouseScroll>,
    mut panels: Query<(&Panel, &mut ScrollPosition)>,
) {
    if !ui.open || mouse.delta.y == 0.0 {
        return;
    }
    let scale = if mouse.unit == bevy::input::mouse::MouseScrollUnit::Line {
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
    ui: Res<SpaceportUi>,
    game: Res<Game>,
    inspector: Option<Res<super::inspection_ui::Inspector>>,
    saves: Option<Res<super::save_ui::Saves>>,
    menu: Option<Res<super::application_ui::Menu>>,
    mut panels: Query<(&Panel, &mut Node)>,
    mut labels: Query<(&Label, &mut Text)>,
) {
    let inspecting = inspector.is_some_and(|ui| ui.open)
        || saves.is_some_and(|saves| saves.open)
        || menu.is_some_and(|menu| menu.is_open());
    for (panel, mut node) in &mut panels {
        let shown = match panel {
            Panel::Overlay => ui.open,
            Panel::Launcher => !ui.open && !inspecting && game.session.player_country.is_some(),
            Panel::Background => !ui.open && !inspecting,
        };
        let shown = shown && super::event_ui::pending(&game).is_none();
        let display = if shown { Display::Flex } else { Display::None };
        if node.display != display {
            node.display = display;
        }
    }
    if !ui.open || (!ui.is_changed() && !game.is_changed()) {
        return;
    }
    // Refresh sibling labels together, matching the HUD's glyph-cache workaround.
    for (label, mut text) in &mut labels {
        text.0 = match label {
            Label::Field(field) => {
                let field = *field;
                let focused = ui.focused == Some(field);
                format!(
                    "{}: {}{}{}",
                    field.label(),
                    if focused && ui.replace { "[" } else { "" },
                    ui.values[field as usize],
                    if focused {
                        if ui.replace { "]" } else { " |" }
                    } else {
                        ""
                    }
                )
            }
            Label::Port => ui.port_status(&game),
            Label::Mission => ui.mission_status(&game),
            Label::Feedback => ui.message.clone(),
        };
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use bevy::input::keyboard::Key;
    use cqsp_core::{Session, World, default_data_path, spaceflight::ColonizationStep};

    fn app() -> Result<App> {
        let data = default_data_path();
        let world = World::load(&data)?;
        let simulation = Simulation::load(&data, &world, 42)?;
        let country = world
            .countries
            .iter()
            .position(|country| country.identifier == "usa")
            .unwrap();
        let mut session = Session::default();
        session.select(country);
        session.start();
        let mut ui = SpaceportUi::new(&simulation);
        ui.open = true;
        let mut app = App::new();
        app.insert_resource(ui)
            .insert_resource(Game {
                world,
                simulation,
                session,
                hovered: None,
                political: true,
                ordered_countries: Vec::new(),
                paused: true,
                speed_index: 0,
                last_tick: 0.0,
            })
            .init_resource::<Time>()
            .init_resource::<ButtonInput<KeyCode>>()
            .add_message::<KeyboardInput>()
            .add_systems(
                Update,
                (controls, super::super::simulation_controls, update).chain(),
            );
        Ok(app)
    }
    fn apply(app: &mut App, action: Action) -> Result<()> {
        app.world_mut()
            .resource_scope(|world, mut ui: Mut<SpaceportUi>| {
                ui.apply(action, &mut world.resource_mut::<Game>())
            })
    }
    fn key(app: &mut App, code: KeyCode, text: Option<&str>) {
        app.world_mut().write_message(KeyboardInput {
            key_code: code,
            logical_key: Key::Character(text.unwrap_or("").into()),
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
    fn native_button_fires_once_and_bad_design_is_non_mutating() -> Result<()> {
        let mut app = app()?;
        let button = app
            .world_mut()
            .spawn((Action::Design, Interaction::Pressed))
            .id();
        app.update();
        app.update();
        assert_eq!(app.world().resource::<Game>().simulation.vehicles.len(), 1);
        *app.world_mut().get_mut::<Interaction>(button).unwrap() = Interaction::None;
        app.world_mut().resource_mut::<SpaceportUi>().values[Field::Reliability as usize] =
            "NaN".into();
        app.update();
        *app.world_mut().get_mut::<Interaction>(button).unwrap() = Interaction::Pressed;
        app.update();
        assert_eq!(app.world().resource::<Game>().simulation.vehicles.len(), 1);
        assert!(
            app.world()
                .resource::<SpaceportUi>()
                .message
                .contains("finite")
        );
        Ok(())
    }

    #[test]
    fn text_editing_is_unicode_safe_and_captures_game_shortcuts() -> Result<()> {
        let mut app = app()?;
        apply(&mut app, Action::Edit(Field::Name))?;
        key(&mut app, KeyCode::KeyN, Some("星"));
        key(&mut app, KeyCode::Space, Some(" "));
        assert_eq!(
            app.world().resource::<SpaceportUi>().values[Field::Name as usize],
            "星 "
        );
        assert!(app.world().resource::<Game>().paused);
        assert_eq!(app.world().resource::<Game>().simulation.date.tick(), 0);
        key(&mut app, KeyCode::Backspace, None);
        assert_eq!(
            app.world().resource::<SpaceportUi>().values[Field::Name as usize],
            "星"
        );
        key(&mut app, KeyCode::Tab, None);
        assert_eq!(
            app.world().resource::<SpaceportUi>().focused,
            Some(Field::Reliability)
        );
        key(&mut app, KeyCode::Digit9, Some("99"));
        assert_eq!(
            app.world().resource::<SpaceportUi>().values[Field::Reliability as usize],
            "99"
        );
        app.world_mut()
            .resource_mut::<ButtonInput<KeyCode>>()
            .press(KeyCode::ControlLeft);
        key(&mut app, KeyCode::KeyA, Some("a"));
        key(&mut app, KeyCode::Digit8, Some("80"));
        assert_eq!(
            app.world().resource::<SpaceportUi>().values[Field::Reliability as usize],
            "80"
        );
        key(&mut app, KeyCode::Enter, None);
        assert!(app.world().resource::<SpaceportUi>().focused.is_none());
        key(&mut app, KeyCode::Escape, None);
        assert!(!app.world().resource::<SpaceportUi>().open);
        assert!(app.world().resource::<SpaceportUi>().capture_input);
        assert!(
            app.world()
                .resource::<Game>()
                .session
                .player_country
                .is_some()
        );
        app.update();
        assert!(!app.world().resource::<SpaceportUi>().capture_input);
        Ok(())
    }

    #[test]
    fn scrolling_moves_only_the_open_overlay() -> Result<()> {
        use bevy::input::mouse::AccumulatedMouseScroll;
        let mut app = app()?;
        app.insert_resource(AccumulatedMouseScroll {
            delta: Vec2::new(0.0, -2.0),
            ..default()
        })
        .add_systems(Update, scroll);
        let panel = app
            .world_mut()
            .spawn((Panel::Overlay, ScrollPosition::default()))
            .id();
        app.update();
        assert_eq!(app.world().get::<ScrollPosition>(panel).unwrap().0.y, 64.0);
        app.world_mut().resource_mut::<SpaceportUi>().open = false;
        app.update();
        assert_eq!(app.world().get::<ScrollPosition>(panel).unwrap().0.y, 64.0);
        Ok(())
    }

    #[test]
    fn native_vehicle_and_colony_actions_complete_the_real_simulation_flow() -> Result<()> {
        let mut app = app()?;
        apply(&mut app, Action::Design)?;
        let city = app.world().resource::<SpaceportUi>().city()?;
        assert!(apply(&mut app, Action::Launch).is_err());
        apply(&mut app, Action::Manufacture)?;
        for _ in 0..90 {
            apply(&mut app, Action::NextDay)?;
        }
        assert_eq!(
            app.world().resource::<Game>().simulation.settlements[city]
                .spaceport
                .as_ref()
                .unwrap()
                .stored_launch_vehicles
                .len(),
            1
        );
        let ships = app
            .world()
            .resource::<Game>()
            .simulation
            .solar_system
            .ships
            .len();
        apply(&mut app, Action::Launch)?;
        assert_eq!(
            app.world()
                .resource::<Game>()
                .simulation
                .solar_system
                .ships
                .len(),
            ships + 1
        );
        assert!(
            app.world().resource::<Game>().simulation.settlements[city]
                .spaceport
                .as_ref()
                .unwrap()
                .stored_launch_vehicles
                .is_empty()
        );
        apply(&mut app, Action::Colonize)?;
        apply(&mut app, Action::QueueCore)?;
        apply(&mut app, Action::Dispatch)?;
        assert!(apply(&mut app, Action::Dispatch).is_err());
        for _ in 0..20 {
            apply(&mut app, Action::NextDay)?;
        }
        let province = app.world().resource::<SpaceportUi>().province()?;
        let simulation = &app.world().resource::<Game>().simulation;
        assert_eq!(
            simulation.settlements[province]
                .colonization
                .as_ref()
                .unwrap()
                .step,
            ColonizationStep::Preparation
        );
        assert_eq!(
            simulation.settlements[province]
                .colony
                .as_ref()
                .unwrap()
                .components
                .len(),
            1
        );
        simulation.validate()?;
        Ok(())
    }
}
