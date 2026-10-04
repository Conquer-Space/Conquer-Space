//! Source-mapped star-system tree and VisibleOrbit filter, without ImGui.
use super::{Focus, Game, Navigation, ui_button, ui_font, ui_row};
use anyhow::{Result, ensure};
use bevy::{input::mouse::AccumulatedMouseScroll, prelude::*};
use cqsp_core::{
    flight::FlightStatus,
    solar_system::{BodyId, ShipId, SolarSystem},
};
use std::collections::BTreeSet;

const PAGE_SIZE: usize = 10;
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq)]
pub(super) enum Mode {
    #[default]
    Tree,
    Filter,
}
#[derive(Resource, Default)]
pub(super) struct Menu {
    pub open: bool,
    pub capture_input: bool,
    pub mode: Mode,
    pub hide_all: bool,
    page: usize,
    expanded: BTreeSet<BodyId>,
    last_click: Option<(Focus, f64)>,
    message: String,
}
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub(super) struct Row {
    focus: Focus,
    depth: usize,
    branch: bool,
    expanded: bool,
    visible: bool,
}
impl Row {
    fn name(self, system: &SolarSystem) -> &str {
        match self.focus {
            Focus::Body(id) => &system.bodies[id.0].name,
            Focus::Ship(id) => &system.ships[id.0].name,
        }
    }
}
impl Menu {
    fn rows(&self, system: &SolarSystem) -> Vec<Row> {
        if self.mode == Mode::Filter {
            return system
                .ships
                .iter()
                .enumerate()
                .filter(|(_, ship)| ship.status == FlightStatus::Orbiting)
                .map(|(id, ship)| Row {
                    focus: Focus::Ship(ShipId(id)),
                    depth: 0,
                    branch: false,
                    expanded: false,
                    visible: ship.visible_orbit,
                })
                .collect();
        }
        let mut roots = vec![system.sun];
        roots.extend(&system.bodies[system.sun.0].children);
        roots.sort_by(|a, b| {
            system.bodies[a.0]
                .orbit
                .semi_major_axis
                .total_cmp(&system.bodies[b.0].orbit.semi_major_axis)
        });
        let mut rows = Vec::new();
        for id in roots {
            self.body_rows(id, 0, system, &mut rows);
        }
        rows
    }
    fn body_rows(&self, id: BodyId, depth: usize, system: &SolarSystem, rows: &mut Vec<Row>) {
        let expanded = id != system.sun && self.expanded.contains(&id);
        rows.push(Row {
            focus: Focus::Body(id),
            depth,
            branch: id != system.sun,
            expanded,
            visible: true,
        });
        if !expanded {
            return;
        }
        for &child in &system.bodies[id.0].children {
            self.body_rows(child, depth + 1, system, rows);
        }
        for (index, ship) in system
            .ships
            .iter()
            .enumerate()
            .filter(|(_, ship)| ship.parent == id && ship.status == FlightStatus::Orbiting)
        {
            rows.push(Row {
                focus: Focus::Ship(ShipId(index)),
                depth: depth + 1,
                branch: false,
                expanded: false,
                visible: ship.visible_orbit,
            });
        }
    }
    pub fn apply(
        &mut self,
        action: Action,
        game: &mut Game,
        navigation: &mut Navigation,
        now: f64,
    ) -> Result<()> {
        let system = &mut game.simulation.solar_system;
        match action {
            Action::Open => {
                self.open = true;
                self.capture_input = true;
            }
            Action::Close => {
                self.open = false;
                self.capture_input = true;
                self.last_click = None;
            }
            Action::Tree | Action::Filter => {
                self.mode = if matches!(action, Action::Tree) {
                    Mode::Tree
                } else {
                    Mode::Filter
                };
                self.page = 0;
                self.last_click = None;
            }
            Action::Previous => self.page = self.page.saturating_sub(1),
            Action::Next => {
                self.page =
                    (self.page + 1).min(self.rows(system).len().saturating_sub(1) / PAGE_SIZE)
            }
            Action::HideAll => {
                self.hide_all = !self.hide_all;
                // A checkbox action mutates current tags; it is not a rule for future launches.
                system.set_all_ship_orbits_visible(!self.hide_all);
            }
            Action::ToggleShip(id) => {
                let ship = system
                    .ships
                    .get(id.0)
                    .filter(|ship| ship.status == FlightStatus::Orbiting)
                    .ok_or_else(|| anyhow::anyhow!("Unknown live spacecraft"))?;
                system.set_ship_visible(id, !ship.visible_orbit)?;
            }
            Action::Expand(id) => {
                ensure!(
                    id.0 < system.bodies.len() && id != system.sun,
                    "Invalid tree branch"
                );
                if !self.expanded.remove(&id) {
                    self.expanded.insert(id);
                }
                self.page = self
                    .page
                    .min(self.rows(system).len().saturating_sub(1) / PAGE_SIZE);
            }
            Action::See(focus) => {
                ensure!(
                    self.rows(system).iter().any(|row| row.focus == focus),
                    "Unknown tree entry"
                );
                if self.last_click.is_some_and(|(previous, then)| {
                    previous == focus && (0.0..=0.5).contains(&(now - then))
                }) {
                    navigation.active = true;
                    navigation.select(focus, system);
                    self.last_click = None;
                } else {
                    self.last_click = Some((focus, now));
                }
            }
        }
        self.message.clear();
        Ok(())
    }
}
pub(super) fn input_available(menu: Option<Res<Menu>>) -> bool {
    menu.is_none_or(|menu| !menu.capture_input)
}
#[derive(Component, Clone, Copy)]
pub(super) enum Action {
    Open,
    Close,
    Tree,
    Filter,
    Previous,
    Next,
    HideAll,
    ToggleShip(ShipId),
    Expand(BodyId),
    See(Focus),
}
#[derive(Component)]
pub(super) enum Panel {
    Launcher,
    Overlay,
}
#[derive(Component)]
pub(super) struct Content;
#[derive(Component)]
pub(super) struct Results(Handle<Font>);
pub(super) fn spawn(commands: &mut Commands, camera: Entity, font: &Handle<Font>) {
    commands
        .spawn((
            Panel::Launcher,
            UiTargetCamera(camera),
            Node {
                position_type: PositionType::Absolute,
                left: px(24),
                bottom: px(112),
                ..default()
            },
        ))
        .with_children(|panel| {
            ui_button(panel, font, Action::Open, "Star system / orbits (T)");
        });
    commands
        .spawn((
            Panel::Overlay,
            UiTargetCamera(camera),
            GlobalZIndex(24),
            Interaction::None,
            Node {
                display: Display::None,
                position_type: PositionType::Absolute,
                left: px(24),
                right: px(24),
                top: px(24),
                bottom: px(24),
                padding: UiRect::all(px(24)),
                flex_direction: FlexDirection::Column,
                row_gap: px(10),
                overflow: Overflow::scroll_y(),
                ..default()
            },
            ScrollPosition::default(),
            BackgroundColor(Color::srgb(0.025, 0.05, 0.09)),
        ))
        .with_children(|panel| {
            panel.spawn((
                Text::new("STAR SYSTEM / ORBIT FILTER"),
                ui_font(font, 22.0),
                TextColor(Color::srgb(0.9, 0.76, 0.42)),
            ));
            ui_row(panel, |row| {
                ui_button(row, font, Action::Tree, "Star system tree");
                ui_button(row, font, Action::Filter, "Orbit filter");
                ui_button(row, font, Action::Close, "Close (T / Esc)");
            });
            panel.spawn((Content, Text::default(), ui_font(font, 16.0)));
            ui_row(panel, |row| {
                ui_button(row, font, Action::Previous, "Previous page");
                ui_button(row, font, Action::Next, "Next page");
                ui_button(row, font, Action::HideAll, "Toggle hide all ship orbits");
            });
            panel.spawn((
                Results(font.clone()),
                Node {
                    flex_direction: FlexDirection::Column,
                    row_gap: px(5),
                    flex_shrink: 0.0,
                    ..default()
                },
            ));
        });
}
#[allow(clippy::too_many_arguments)]
pub(super) fn controls(
    menu: Option<ResMut<Menu>>,
    keyboard: Res<ButtonInput<KeyCode>>,
    time: Res<Time>,
    mut game: ResMut<Game>,
    mut navigation: ResMut<Navigation>,
    inspector: Res<super::inspection_ui::Inspector>,
    ports: Res<super::spaceport_ui::SpaceportUi>,
    saves: Option<Res<super::save_ui::Saves>>,
    application: Option<Res<super::application_ui::Menu>>,
    construction: Option<Res<super::construction_ui::ConstructionTool>>,
    buttons: Query<(&Action, &Interaction), Changed<Interaction>>,
) {
    let Some(mut menu) = menu else { return };
    menu.capture_input = menu.open;
    if game.session.player_country.is_none() {
        *menu = Menu::default();
        return;
    }
    if inspector.capture_input
        || ports.capture_input
        || saves.is_some_and(|ui| ui.capture_input)
        || application.is_some_and(|ui| ui.capture_input)
        || construction.is_some_and(|ui| ui.searching)
    {
        return;
    }
    let action = if keyboard.just_pressed(KeyCode::KeyT) {
        Some(if menu.open {
            Action::Close
        } else {
            Action::Open
        })
    } else if menu.open && keyboard.just_pressed(KeyCode::Escape) {
        Some(Action::Close)
    } else {
        None
    };
    if let Some(action) = action {
        menu.apply(action, &mut game, &mut navigation, time.elapsed_secs_f64())
            .unwrap();
        return;
    }
    for (action, interaction) in &buttons {
        if *interaction == Interaction::Pressed && (menu.open || matches!(action, Action::Open)) {
            if let Err(error) =
                menu.apply(*action, &mut game, &mut navigation, time.elapsed_secs_f64())
            {
                menu.message = error.to_string();
            }
            break;
        }
    }
}
pub(super) fn scroll(
    menu: Option<Res<Menu>>,
    mouse: Res<AccumulatedMouseScroll>,
    mut panels: Query<(&Panel, &mut ScrollPosition)>,
) {
    if menu.is_none_or(|menu| !menu.open) || mouse.delta == Vec2::ZERO {
        return;
    }
    let delta = mouse.delta.y
        * if mouse.unit == bevy::input::mouse::MouseScrollUnit::Line {
            32.0
        } else {
            1.0
        };
    for (panel, mut scroll) in &mut panels {
        if matches!(panel, Panel::Overlay) {
            scroll.y = (scroll.y - delta).max(0.0);
        }
    }
}

#[allow(clippy::too_many_arguments, clippy::type_complexity)]
pub(super) fn update(
    mut commands: Commands,
    menu: Option<Res<Menu>>,
    game: Res<Game>,
    navigation: Res<Navigation>,
    inspector: Res<super::inspection_ui::Inspector>,
    ports: Res<super::spaceport_ui::SpaceportUi>,
    saves: Option<Res<super::save_ui::Saves>>,
    application: Option<Res<super::application_ui::Menu>>,
    search: Option<Res<super::search_ui::Search>>,
    construction: Option<Res<super::construction_ui::ConstructionTool>>,
    mut panels: Query<(Entity, &Panel, &mut Node, Option<&mut ScrollPosition>)>,
    mut texts: Query<(&mut Text, Option<&Content>)>,
    children: Query<&Children>,
    lists: Query<(Entity, &Results)>,
    mut buttons: Query<(&Action, &mut BackgroundColor)>,
    mut previous: Local<Option<(Mode, usize, Vec<Row>)>>,
) {
    let Some(menu) = menu else { return };
    let system = &game.simulation.solar_system;
    let rows = menu.rows(system);
    let page = menu.page.min(rows.len().saturating_sub(1) / PAGE_SIZE);
    let visible_rows: Vec<_> = rows
        .iter()
        .skip(page * PAGE_SIZE)
        .take(PAGE_SIZE)
        .copied()
        .collect();
    let key = (menu.mode, page, visible_rows);
    let changed = previous.as_ref() != Some(&key);
    let shown = game.session.player_country.is_some() && super::event_ui::pending(&game).is_none();
    for (entity, panel, mut node, scroll) in &mut panels {
        let shown = shown
            && match panel {
                Panel::Overlay => menu.open,
                Panel::Launcher => {
                    !menu.open
                        && !inspector.open
                        && !ports.open
                        && saves.as_ref().is_none_or(|ui| !ui.open)
                        && application.as_ref().is_none_or(|ui| !ui.is_open())
                        && search.as_ref().is_none_or(|ui| !ui.open)
                        && construction.as_ref().is_none_or(|ui| !ui.searching)
                }
            };
        let display = if shown { Display::Flex } else { Display::None };
        if node.display != display {
            node.display = display;
        }
        if changed && let Some(mut scroll) = scroll {
            scroll.y = 0.0;
        }
        if shown && matches!(panel, Panel::Overlay) {
            for child in children.iter_descendants(entity) {
                if let Ok((mut text, content)) = texts.get_mut(child) {
                    if content.is_some() {
                        text.0 = format!(
                            "{} · page {}/{} · {} entries\nHide all ship orbits: {}\n{}\n{}",
                            if menu.mode == Mode::Tree {
                                "STAR SYSTEM TREE"
                            } else {
                                "ORBIT FILTER"
                            },
                            page + 1,
                            rows.len().div_ceil(PAGE_SIZE).max(1),
                            rows.len(),
                            if menu.hide_all {
                                "checked"
                            } else {
                                "unchecked"
                            },
                            if menu.mode == Mode::Tree {
                                "Expand arrows reveal natural bodies, then ships. Double-click a name to focus; a single click does not move the camera."
                            } else {
                                "[x] visible · [ ] hidden. Imported satellites start hidden; launched ships are visible. Planetary paths are unaffected by this filter."
                            },
                            menu.message
                        );
                    } else {
                        text.set_changed();
                    }
                }
            }
        }
    }
    if changed {
        for (entity, font) in &lists {
            commands.entity(entity).despawn_related::<Children>();
            commands.entity(entity).with_children(|list| {
                for &entry in &key.2 {
                    ui_row(list, |row| {
                        row.spawn(Node {
                            width: px(entry.depth as f32 * 24.0),
                            ..default()
                        });
                        if menu.mode == Mode::Filter {
                            if let Focus::Ship(id) = entry.focus {
                                ui_button(
                                    row,
                                    &font.0,
                                    Action::ToggleShip(id),
                                    &format!(
                                        "[{}] {}",
                                        if entry.visible { "x" } else { " " },
                                        entry.name(system)
                                    ),
                                );
                            }
                        } else {
                            if entry.branch
                                && let Focus::Body(id) = entry.focus
                            {
                                ui_button(
                                    row,
                                    &font.0,
                                    Action::Expand(id),
                                    if entry.expanded { "-" } else { "+" },
                                );
                            }
                            ui_button(row, &font.0, Action::See(entry.focus), entry.name(system));
                        }
                    });
                }
            });
        }
        *previous = Some(key);
    }
    for (action, mut color) in &mut buttons {
        if let Action::See(focus) = action {
            let value = if *focus == navigation.focus {
                Color::srgb(0.25, 0.43, 0.6)
            } else {
                Color::srgb(0.15, 0.29, 0.43)
            };
            if color.0 != value {
                color.0 = value;
            }
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::time::Duration;

    fn app() -> Result<App> {
        let mut app = super::super::inspection_ui::tests::app()?;
        app.init_resource::<Menu>()
            .init_resource::<super::super::search_ui::Search>();
        Ok(app)
    }
    fn apply(app: &mut App, action: Action, now: f64) -> Result<()> {
        app.world_mut()
            .resource_scope(|world, mut menu: Mut<Menu>| {
                world.resource_scope(|world, mut navigation: Mut<Navigation>| {
                    menu.apply(
                        action,
                        &mut world.resource_mut::<Game>(),
                        &mut navigation,
                        now,
                    )
                })
            })
    }
    fn key(app: &mut App, keys: &[KeyCode]) {
        for &key in keys {
            app.world_mut()
                .resource_mut::<ButtonInput<KeyCode>>()
                .press(key);
        }
        app.update();
        app.world_mut()
            .resource_mut::<ButtonInput<KeyCode>>()
            .reset_all();
    }
    #[test]
    fn tree_orders_roots_and_expands_natural_bodies_before_ships() -> Result<()> {
        let mut app = app()?;
        let system = &app.world().resource::<Game>().simulation.solar_system;
        let earth = system.body_id("earth").unwrap();
        let moon = system.body_id("moon").unwrap();
        let sun = system.sun;
        let roots = app.world().resource::<Menu>().rows(system);
        assert_eq!(roots.len(), 10);
        assert_eq!(
            roots.iter().map(|row| row.name(system)).collect::<Vec<_>>(),
            [
                "The Sun", "Mercury", "Venus", "Earth", "Mars", "Jupiter", "Saturn", "Uranus",
                "Neptune", "Pluto"
            ]
        );
        apply(&mut app, Action::Expand(earth), 0.0)?;
        let system = &app.world().resource::<Game>().simulation.solar_system;
        let rows = app.world().resource::<Menu>().rows(system);
        assert_eq!(rows.len(), 75);
        let index = rows
            .iter()
            .position(|row| row.focus == Focus::Body(earth))
            .unwrap();
        assert_eq!(rows[index + 1].focus, Focus::Body(moon));
        assert_eq!(rows[index + 2].focus, Focus::Ship(ShipId(0)));
        assert_eq!(rows[index + 2].depth, 1);
        for _ in 0..20 {
            apply(&mut app, Action::Next, 0.0)?;
        }
        assert_eq!(app.world().resource::<Menu>().page, 7);
        apply(&mut app, Action::Expand(earth), 0.0)?;
        assert_eq!(app.world().resource::<Menu>().page, 0);
        assert!(apply(&mut app, Action::See(Focus::Ship(ShipId(0))), 0.0).is_err());
        assert!(apply(&mut app, Action::Expand(BodyId(usize::MAX)), 0.0).is_err());
        assert!(apply(&mut app, Action::Expand(sun), 0.0).is_err());
        Ok(())
    }
    #[test]
    fn single_click_preserves_camera_double_click_focuses_without_showing_a_ship() -> Result<()> {
        let mut app = app()?;
        let system = &app.world().resource::<Game>().simulation.solar_system;
        let earth = system.body_id("earth").unwrap();
        let mercury = system.body_id("mercury").unwrap();
        let initial = app.world().resource::<Navigation>().focus;
        apply(&mut app, Action::See(Focus::Body(mercury)), 1.0)?;
        assert_eq!(app.world().resource::<Navigation>().focus, initial);
        apply(&mut app, Action::See(Focus::Body(mercury)), 1.6)?;
        assert_eq!(app.world().resource::<Navigation>().focus, initial);
        apply(&mut app, Action::See(Focus::Body(mercury)), 1.7)?;
        assert_eq!(
            app.world().resource::<Navigation>().focus,
            Focus::Body(mercury)
        );
        assert!(app.world().resource::<Navigation>().active);
        apply(&mut app, Action::Expand(earth), 2.0)?;
        apply(&mut app, Action::See(Focus::Ship(ShipId(0))), 2.1)?;
        apply(&mut app, Action::See(Focus::Ship(ShipId(0))), 2.2)?;
        assert_eq!(
            app.world().resource::<Navigation>().focus,
            Focus::Ship(ShipId(0))
        );
        assert!(!app.world().resource::<Game>().simulation.solar_system.ships[0].visible_orbit);
        Ok(())
    }
    #[test]
    fn filter_has_all_live_ships_and_hide_all_is_an_action_not_a_future_rule() -> Result<()> {
        let mut app = app()?;
        apply(&mut app, Action::Filter, 0.0)?;
        let game = app.world().resource::<Game>();
        assert_eq!(
            app.world()
                .resource::<Menu>()
                .rows(&game.simulation.solar_system)
                .len(),
            64
        );
        apply(&mut app, Action::HideAll, 0.0)?;
        assert!(app.world().resource::<Menu>().hide_all);
        apply(&mut app, Action::ToggleShip(ShipId(3)), 0.0)?;
        assert!(app.world().resource::<Menu>().hide_all);
        apply(&mut app, Action::HideAll, 0.0)?;
        assert!(
            app.world()
                .resource::<Game>()
                .simulation
                .solar_system
                .ships
                .iter()
                .all(|ship| ship.visible_orbit)
        );
        apply(&mut app, Action::ToggleShip(ShipId(3)), 0.0)?;
        assert!(!app.world().resource::<Menu>().hide_all);
        apply(&mut app, Action::HideAll, 0.0)?;
        {
            let mut game = app.world_mut().resource_mut::<Game>();
            let city = game
                .simulation
                .settlements
                .iter()
                .position(|site| {
                    site.spaceport.is_some() && site.country == game.session.player_country
                })
                .unwrap();
            let country = game.session.player_country.unwrap();
            let vehicle = game.simulation.design_vehicle(
                country,
                cqsp_core::spaceflight::LaunchVehicle::design(
                    "Filter launch".into(),
                    95.0,
                    10.0,
                    5.0,
                )?,
            )?;
            game.simulation.settlements[city]
                .spaceport
                .as_mut()
                .unwrap()
                .stored_launch_vehicles
                .push(vehicle);
            let id = game.simulation.launch_vehicle(
                city,
                cqsp_core::orbit::Orbit {
                    semi_major_axis: 7000.0,
                    ..default()
                },
                std::f64::consts::FRAC_PI_2,
            )?;
            assert!(game.simulation.solar_system.ships[id.0].visible_orbit);
            game.simulation.solar_system.ships[0].status = FlightStatus::Docked(city);
            game.simulation.solar_system.ships[1].status = FlightStatus::Crashed;
        }
        assert!(app.world().resource::<Menu>().hide_all);
        assert!(apply(&mut app, Action::ToggleShip(ShipId(0)), 0.0).is_err());
        assert!(apply(&mut app, Action::ToggleShip(ShipId(usize::MAX)), 0.0).is_err());
        let game = app.world().resource::<Game>();
        assert_eq!(
            app.world()
                .resource::<Menu>()
                .rows(&game.simulation.solar_system)
                .len(),
            63
        );
        Ok(())
    }
    #[test]
    fn overlay_captures_shortcuts_and_closing_frame_but_running_time_continues() -> Result<()> {
        let mut app = app()?;
        app.update();
        let date = app.world().resource::<Game>().simulation.date;
        key(
            &mut app,
            &[
                KeyCode::KeyT,
                KeyCode::Space,
                KeyCode::KeyN,
                KeyCode::KeyP,
                KeyCode::KeyI,
            ],
        );
        assert!(app.world().resource::<Menu>().open);
        assert!(app.world().resource::<Game>().paused);
        assert_eq!(app.world().resource::<Game>().simulation.date, date);
        assert!(
            !app.world()
                .resource::<super::super::inspection_ui::Inspector>()
                .open
        );
        assert!(
            !app.world()
                .resource::<super::super::spaceport_ui::SpaceportUi>()
                .open
        );
        key(&mut app, &[KeyCode::ControlLeft, KeyCode::KeyP]);
        assert!(
            !app.world()
                .resource::<super::super::search_ui::Search>()
                .open
        );
        app.world_mut().resource_mut::<Game>().paused = false;
        app.world_mut()
            .resource_mut::<Time>()
            .advance_by(Duration::from_secs(2));
        app.update();
        assert!(app.world().resource::<Game>().simulation.date.tick() > date.tick());
        key(&mut app, &[KeyCode::Escape, KeyCode::Space]);
        assert!(!app.world().resource::<Menu>().open);
        assert!(app.world().resource::<Menu>().capture_input);
        assert!(!app.world().resource::<Game>().paused);
        assert!(
            app.world()
                .resource::<Game>()
                .session
                .player_country
                .is_some()
        );
        app.update();
        assert!(!app.world().resource::<Menu>().capture_input);
        key(&mut app, &[KeyCode::Space]);
        assert!(app.world().resource::<Game>().paused);
        Ok(())
    }
    #[test]
    fn overlay_scroll_and_panel_visibility_are_isolated_and_new_selection_resets_state()
    -> Result<()> {
        let mut app = app()?;
        let overlay = app
            .world_mut()
            .spawn((Panel::Overlay, Node::default(), ScrollPosition::default()))
            .id();
        let launcher = app
            .world_mut()
            .spawn((Panel::Launcher, Node::default(), ScrollPosition::default()))
            .id();
        let background = app
            .world_mut()
            .spawn((
                super::super::spaceport_ui::Panel::Background,
                Node::default(),
            ))
            .id();
        let list = app
            .world_mut()
            .spawn((Results(Handle::default()), Node::default()))
            .id();
        app.world_mut().entity_mut(overlay).add_child(list);
        key(&mut app, &[KeyCode::KeyT]);
        assert_eq!(
            app.world().get::<Node>(overlay).unwrap().display,
            Display::Flex
        );
        assert_eq!(
            app.world().get::<Node>(launcher).unwrap().display,
            Display::None
        );
        assert_eq!(
            app.world().get::<Node>(background).unwrap().display,
            Display::None
        );
        assert_eq!(app.world().get::<Children>(list).unwrap().len(), 10);
        app.world_mut()
            .resource_mut::<AccumulatedMouseScroll>()
            .delta = Vec2::new(0.0, -2.0);
        app.update();
        assert_eq!(app.world().get::<ScrollPosition>(overlay).unwrap().y, 64.0);
        assert_eq!(app.world().get::<ScrollPosition>(launcher).unwrap().y, 0.0);
        app.world_mut()
            .resource_mut::<AccumulatedMouseScroll>()
            .delta = Vec2::ZERO;
        apply(&mut app, Action::Filter, 0.0)?;
        app.update();
        assert_eq!(app.world().get::<ScrollPosition>(overlay).unwrap().y, 0.0);
        apply(&mut app, Action::Next, 0.0)?;
        app.world_mut()
            .resource_mut::<Game>()
            .session
            .player_country = None;
        app.update();
        let menu = app.world().resource::<Menu>();
        assert!(!menu.open && !menu.capture_input && !menu.hide_all);
        assert_eq!((menu.mode, menu.page), (Mode::Tree, 0));
        assert!(menu.expanded.is_empty());
        Ok(())
    }
}
