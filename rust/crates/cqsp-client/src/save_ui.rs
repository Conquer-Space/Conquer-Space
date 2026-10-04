//! Native metadata browser. Loading recreates the asset universe, not a complete saved game.
use super::{
    Game, Navigation, inspection_ui::Inspector, spaceport_ui::SpaceportUi, ui_button, ui_font,
};
use anyhow::{Context, Result};
use bevy::input::mouse::{AccumulatedMouseScroll, MouseScrollUnit};
use bevy::prelude::*;
use cqsp_core::{
    Session,
    save::{Entry, Metadata, SaveStore},
    simulation::Simulation,
};
use std::{fmt::Write, path::PathBuf};

#[derive(Resource)]
pub(super) struct Saves {
    pub open: bool,
    pub capture_input: bool,
    data: PathBuf,
    store: SaveStore,
    entries: Vec<Entry>,
    selected: usize,
    message: String,
}
impl Saves {
    pub fn new(data: PathBuf, root: PathBuf) -> Self {
        Self {
            open: false,
            capture_input: false,
            data,
            store: SaveStore(root),
            entries: Vec::new(),
            selected: 0,
            message: String::new(),
        }
    }
    fn refresh(&mut self) -> Result<()> {
        self.entries = self.store.list()?;
        self.selected = self.selected.min(self.entries.len().saturating_sub(1));
        Ok(())
    }
    pub(super) fn apply(
        &mut self,
        action: Action,
        game: &mut Game,
        navigation: &mut Navigation,
        inspector: &mut Inspector,
        ports: &mut SpaceportUi,
    ) -> Result<()> {
        match action {
            Action::Open => {
                if ports.editing() {
                    return Ok(());
                }
                inspector.open = false;
                if ports.open {
                    ports.apply(super::spaceport_ui::Action::Close, game)?;
                }
                self.open = true;
                self.capture_input = true;
                self.refresh()?;
            }
            Action::Close => self.open = false,
            Action::Refresh => self.refresh()?,
            Action::Previous | Action::Next => {
                if !self.entries.is_empty() {
                    self.selected = (self.selected
                        + if matches!(action, Action::Previous) {
                            self.entries.len() - 1
                        } else {
                            1
                        })
                        % self.entries.len();
                }
            }
            Action::Save => {
                let country = game
                    .session
                    .player_country
                    .context("Select and start a country before saving")?;
                let metadata = Metadata::new(game.simulation.date, game.simulation.uuid.clone());
                let path = self
                    .store
                    .write(&game.world.countries[country].identifier, &metadata)?;
                self.message = format!("Saved metadata only: {}", path.display());
                self.refresh()?;
            }
            Action::Load => {
                let entry = self
                    .entries
                    .get(self.selected)
                    .context("Select a metadata save first")?;
                let metadata = SaveStore::read(&entry.path)?;
                let mut simulation = Simulation::load(&self.data, &game.world, 42)?;
                metadata.apply(&mut simulation.date, &mut simulation.uuid);
                simulation.tick(); // UniverseLoadingScene initializes and ticks once after loading metadata.
                *navigation = Navigation::new(&simulation.solar_system);
                *inspector = Inspector::new(&simulation);
                *ports = SpaceportUi::new(&simulation);
                game.simulation = simulation;
                game.session = Session::default();
                game.hovered = None;
                game.paused = true;
                game.last_tick = 0.0;
                self.message = "Loaded date/UUID into fresh assets. Economy, ships, histories, country selection and RNG were not restored.".into();
                self.open = false;
            }
        }
        Ok(())
    }
    fn content(&self, game: &Game) -> String {
        let mut text = format!(
            "METADATA SAVE / LOAD\nUpstream saves only date, UUID and version. This is not a full game-state backup.\nLoading recreates the original assets and applies date/UUID, then ticks once. Country selection, economy, ships, histories and RNG are not saved.\n\nStorage: {}\nOriginal C++ saves are not touched by this default Rust-specific location.\n\n",
            self.store.0.display()
        );
        if let Some(country) = game.session.player_country {
            let _ = writeln!(
                text,
                "Current game: {} · {} · UUID {}",
                game.world.countries[country].name,
                game.simulation.date.display(),
                game.simulation.uuid
            );
        } else {
            text.push_str("Select and start a country to save. Loading is available before country selection.\n");
        }
        let _ = writeln!(
            text,
            "\n{} saved metadata folders · selected {}",
            self.entries.len(),
            if self.entries.is_empty() {
                0
            } else {
                self.selected + 1
            }
        );
        let first = self.selected / 8 * 8;
        for (index, entry) in self.entries.iter().enumerate().skip(first).take(8) {
            let _ = writeln!(
                text,
                "{} {} · tick {}",
                if index == self.selected { ">" } else { " " },
                entry.name,
                entry.date
            );
        }
        if self.entries.is_empty() {
            text.push_str("No metadata saves found.\n");
        }
        if !self.message.is_empty() {
            let _ = writeln!(text, "\n{}", self.message);
        }
        text
    }
}
pub(super) fn default_root() -> Result<PathBuf> {
    Ok(PathBuf::from(
        std::env::var_os("LOCALAPPDATA")
            .context("LOCALAPPDATA is unavailable; pass --saves PATH")?,
    )
    .join("ConquerSpaceRust/saves"))
}
#[derive(Component, Clone, Copy)]
pub(super) enum Action {
    Open,
    Close,
    Save,
    Load,
    Refresh,
    Previous,
    Next,
}
#[derive(Component)]
pub(super) enum Panel {
    Launcher,
    Overlay,
}
#[derive(Component)]
pub(super) struct Content;
pub(super) fn spawn(commands: &mut Commands, camera: Entity, font: &Handle<Font>) {
    commands
        .spawn((
            Panel::Launcher,
            UiTargetCamera(camera),
            Node {
                position_type: PositionType::Absolute,
                right: px(24),
                bottom: px(24),
                ..default()
            },
        ))
        .with_children(|bar| {
            ui_button(bar, font, Action::Open, "Save / load metadata");
        });
    commands
        .spawn((
            Panel::Overlay,
            UiTargetCamera(camera),
            GlobalZIndex(20),
            Node {
                display: Display::None,
                position_type: PositionType::Absolute,
                top: px(70),
                bottom: px(70),
                left: px(70),
                right: px(70),
                padding: UiRect::all(px(20)),
                flex_direction: FlexDirection::Column,
                row_gap: px(16),
                overflow: Overflow::scroll_y(),
                ..default()
            },
            BackgroundColor(Color::srgb(0.025, 0.05, 0.09)),
        ))
        .with_children(|panel| {
            panel.spawn((
                Text::new("SAVE / LOAD · METADATA ONLY"),
                ui_font(font, 22.0),
                TextColor(Color::srgb(0.9, 0.76, 0.42)),
                Node {
                    flex_shrink: 0.0,
                    ..default()
                },
            ));
            super::ui_row(panel, |bar| {
                for (action, label) in [
                    (Action::Save, "Save current metadata"),
                    (Action::Previous, "Previous save"),
                    (Action::Next, "Next save"),
                    (Action::Load, "Load selected (fresh assets)"),
                    (Action::Refresh, "Refresh"),
                    (Action::Close, "Close (Esc)"),
                ] {
                    ui_button(bar, font, action, label);
                }
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
        });
}
pub(super) fn controls(
    keyboard: Res<ButtonInput<KeyCode>>,
    buttons: Query<(&Action, &Interaction), Changed<Interaction>>,
    saves: Option<ResMut<Saves>>,
    mut game: ResMut<Game>,
    mut navigation: ResMut<Navigation>,
    mut inspector: ResMut<Inspector>,
    mut ports: ResMut<SpaceportUi>,
) {
    let Some(mut saves) = saves else {
        return;
    };
    let capture = saves.open;
    if saves.capture_input != capture {
        saves.capture_input = capture;
    }
    if saves.open && keyboard.just_pressed(KeyCode::Escape) {
        saves.open = false;
        return;
    }
    for (action, interaction) in &buttons {
        if *interaction == Interaction::Pressed
            && (saves.open || matches!(action, Action::Open))
            && let Err(error) = saves.apply(
                *action,
                &mut game,
                &mut navigation,
                &mut inspector,
                &mut ports,
            )
        {
            saves.message = error.to_string();
        }
    }
}
pub(super) fn scroll(
    saves: Option<Res<Saves>>,
    mouse: Res<AccumulatedMouseScroll>,
    mut panels: Query<(&Panel, &mut ScrollPosition)>,
) {
    if !saves.is_some_and(|saves| saves.open) || mouse.delta.y == 0.0 {
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
#[allow(clippy::too_many_arguments)]
pub(super) fn update(
    saves: Option<Res<Saves>>,
    game: Res<Game>,
    navigation: Res<Navigation>,
    inspector: Res<Inspector>,
    ports: Res<SpaceportUi>,
    menu: Option<Res<super::application_ui::Menu>>,
    mut panels: Query<(Entity, &Panel, &mut Node)>,
    navigation_menu: Option<Res<super::navigation_ui::Menu>>,
    children: Query<&Children>,
    mut texts: Query<(&mut Text, Option<&Content>)>,
) {
    let Some(saves) = saves else {
        return;
    };
    for (entity, panel, mut node) in &mut panels {
        let shown = match panel {
            Panel::Launcher => {
                !saves.open
                    && !navigation.active
                    && !ports.open
                    && !inspector.open
                    && menu.as_ref().is_none_or(|menu| !menu.is_open())
            }
            Panel::Overlay => saves.open,
        };
        let shown = shown
            && super::event_ui::pending(&game).is_none()
            && navigation_menu.as_ref().is_none_or(|menu| !menu.open);
        let display = if shown { Display::Flex } else { Display::None };
        if node.display != display {
            node.display = display;
        }
        if matches!(panel, Panel::Overlay)
            && saves.open
            && (saves.is_changed() || game.is_changed())
        {
            for child in children.iter_descendants(entity) {
                if let Ok((mut text, content)) = texts.get_mut(child) {
                    if content.is_some() {
                        text.0 = saves.content(&game);
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
    use cqsp_core::{clock::StarDate, default_data_path, economy::GoodId};
    use std::{fs, time::Duration};

    struct TestRoot(PathBuf);
    impl Drop for TestRoot {
        fn drop(&mut self) {
            // Only this test's uniquely named fixture is ever removed.
            let _ = fs::remove_dir_all(&self.0);
        }
    }
    fn app() -> Result<(App, TestRoot)> {
        let root = TestRoot(std::env::temp_dir().join(format!(
            "cqsp-save-ui-{}-{}",
            std::process::id(),
            cqsp_core::save::new_uuid()
        )));
        anyhow::ensure!(!root.0.exists(), "Test fixture already exists");
        let mut app = super::super::inspection_ui::tests::app()?;
        app.insert_resource(Saves::new(default_data_path(), root.0.clone()));
        app.update();
        Ok((app, root))
    }
    fn click(app: &mut App, action: Action) {
        let button = app.world_mut().spawn((action, Interaction::Pressed)).id();
        app.update();
        app.world_mut().despawn(button);
    }

    #[test]
    fn modal_stops_time_captures_shortcuts_and_saves_once_per_click() -> Result<()> {
        let (mut app, root) = app()?;
        app.world_mut().resource_mut::<Game>().paused = false;
        let tick = app.world().resource::<Game>().simulation.date.tick();
        let background = app
            .world_mut()
            .spawn((
                super::super::spaceport_ui::Panel::Background,
                Node::default(),
            ))
            .id();
        let inspection_launcher = app
            .world_mut()
            .spawn((
                super::super::inspection_ui::Panel::Launcher,
                Node::default(),
            ))
            .id();
        click(&mut app, Action::Open);
        assert_eq!(
            app.world().get::<Node>(background).unwrap().display,
            Display::None
        );
        assert_eq!(
            app.world()
                .get::<Node>(inspection_launcher)
                .unwrap()
                .display,
            Display::None
        );
        let overlay = app
            .world_mut()
            .spawn((Panel::Overlay, ScrollPosition::default()))
            .id();
        let launcher = app
            .world_mut()
            .spawn((Panel::Launcher, ScrollPosition::default()))
            .id();
        app.world_mut()
            .resource_mut::<Time>()
            .advance_by(Duration::from_secs(1000));
        for key in [KeyCode::Space, KeyCode::KeyN, KeyCode::KeyI, KeyCode::KeyP] {
            app.world_mut()
                .resource_mut::<ButtonInput<KeyCode>>()
                .press(key);
        }
        app.world_mut()
            .resource_mut::<AccumulatedMouseScroll>()
            .delta
            .y = -2.0;
        let button = app
            .world_mut()
            .spawn((Action::Save, Interaction::Pressed))
            .id();
        app.update();
        assert!(app.world().resource::<Saves>().open);
        assert!(!app.world().resource::<Game>().paused);
        assert_eq!(app.world().resource::<Game>().simulation.date.tick(), tick);
        assert!(!app.world().resource::<Inspector>().open);
        assert!(!app.world().resource::<SpaceportUi>().open);
        assert_eq!(
            app.world().get::<ScrollPosition>(overlay).unwrap().0.y,
            64.0
        );
        assert_eq!(
            app.world().get::<ScrollPosition>(launcher).unwrap().0.y,
            0.0
        );
        let entry = SaveStore(root.0.clone()).list()?.remove(0);
        assert_eq!(SaveStore::read(&entry.path)?.date, tick as i64);
        app.world_mut().resource_mut::<Game>().simulation.date = StarDate::from_tick(42);
        app.update();
        assert_eq!(SaveStore::read(&entry.path)?.date, tick as i64);
        app.world_mut().despawn(button);
        app.world_mut()
            .resource_mut::<ButtonInput<KeyCode>>()
            .reset_all();
        app.world_mut()
            .resource_mut::<ButtonInput<KeyCode>>()
            .press(KeyCode::Escape);
        app.update();
        assert!(!app.world().resource::<Saves>().open);
        assert!(app.world().resource::<Saves>().capture_input);
        assert!(
            app.world()
                .resource::<Game>()
                .session
                .player_country
                .is_some()
        );
        assert_eq!(app.world().resource::<Game>().simulation.date.tick(), 42);
        app.world_mut()
            .resource_mut::<ButtonInput<KeyCode>>()
            .reset_all();
        app.update();
        assert!(!app.world().resource::<Saves>().capture_input);
        assert_eq!(app.world().resource::<Game>().simulation.date.tick(), 43);
        Ok(())
    }

    #[test]
    fn load_recreates_assets_ticks_once_and_resets_all_session_interfaces() -> Result<()> {
        let (mut app, root) = app()?;
        let metadata = Metadata::new(StarDate::from_tick(144), "abcdef".into());
        SaveStore(root.0.clone()).write("usa", &metadata)?;
        {
            let mut game = app.world_mut().resource_mut::<Game>();
            game.simulation.set_good_tax(GoodId(0), 3.0)?;
            let ship = &game.simulation.solar_system.ships[0];
            let (parent, orbit) = (ship.parent, ship.orbit);
            game.simulation
                .solar_system
                .launch_ship("Unsaved rocket".into(), parent, orbit)?;
            game.simulation.date = StarDate::from_tick(900);
            game.hovered = Some(0);
            game.paused = false;
        }
        app.world_mut().resource_mut::<Navigation>().active = true;
        click(&mut app, Action::Open);
        click(&mut app, Action::Load);
        let game = app.world().resource::<Game>();
        assert_eq!(game.simulation.uuid, "abcdef");
        assert_eq!(game.simulation.date.tick(), 145);
        assert_eq!(game.simulation.solar_system.ships.len(), 64);
        assert!(
            game.simulation
                .settlements
                .iter()
                .all(|site| site.market.taxation[GoodId(0)] == 0.0)
        );
        assert!(game.session.player_country.is_none() && game.session.selected_country.is_none());
        assert!(game.paused && game.hovered.is_none());
        assert!(!app.world().resource::<Navigation>().active);
        assert!(!app.world().resource::<Inspector>().open);
        assert!(!app.world().resource::<SpaceportUi>().open);
        assert!(!app.world().resource::<Saves>().open);
        assert!(
            app.world()
                .resource::<Saves>()
                .message
                .contains("not restored")
        );
        assert_eq!(
            SaveStore::read(&SaveStore(root.0.clone()).list()?[0].path)?,
            metadata
        );
        Ok(())
    }

    #[test]
    fn browser_works_before_selection_and_failed_load_preserves_game() -> Result<()> {
        let (mut app, root) = app()?;
        app.world_mut().resource_mut::<Game>().session = Session::default();
        click(&mut app, Action::Open);
        click(&mut app, Action::Save);
        assert!(
            app.world()
                .resource::<Saves>()
                .message
                .contains("Select and start")
        );
        assert!(!root.0.exists());
        click(&mut app, Action::Load);
        assert!(
            app.world()
                .resource::<Saves>()
                .message
                .contains("Select a metadata save")
        );
        let path = SaveStore(root.0.clone()).write(
            "usa",
            &Metadata::new(StarDate::from_tick(144), "abcdef".into()),
        )?;
        fs::write(&path, r#"{"date":"wrong","uuid":"abcdef"}"#)?;
        // Bad numeric types cannot even enter the browser; a corrupt selected file may still occur after refresh.
        click(&mut app, Action::Refresh);
        assert!(app.world().resource::<Saves>().message.contains("numeric"));
        SaveStore(root.0.clone()).write(
            "usa",
            &Metadata::new(StarDate::from_tick(144), "abcdef".into()),
        )?;
        click(&mut app, Action::Refresh);
        fs::write(&path, r#"{"date":"wrong"}"#)?;
        let tick = app.world().resource::<Game>().simulation.date.tick();
        let uuid = app.world().resource::<Game>().simulation.uuid.clone();
        click(&mut app, Action::Load);
        assert!(app.world().resource::<Saves>().open);
        assert_eq!(app.world().resource::<Game>().simulation.date.tick(), tick);
        assert_eq!(app.world().resource::<Game>().simulation.uuid, uuid);
        assert!(app.world().resource::<Saves>().message.contains("numeric"));
        Ok(())
    }

    #[test]
    fn every_metadata_folder_is_reachable_and_refresh_clamps_selection() -> Result<()> {
        let (mut app, root) = app()?;
        let store = SaveStore(root.0.clone());
        for index in 0..10 {
            store.write(
                "usa",
                &Metadata::new(StarDate::from_tick(index), format!("{index:06x}")),
            )?;
        }
        click(&mut app, Action::Open);
        for index in 0..10 {
            let saves = app.world().resource::<Saves>();
            assert_eq!(saves.selected, index);
            assert!(
                saves
                    .content(app.world().resource::<Game>())
                    .contains(&format!("usa_{index:06x}"))
            );
            click(&mut app, Action::Next);
        }
        assert_eq!(app.world().resource::<Saves>().selected, 0);
        click(&mut app, Action::Previous);
        assert_eq!(app.world().resource::<Saves>().selected, 9);
        // Removing only known, test-owned fixture folders exercises stale selection handling.
        for entry in store.list()?.into_iter().skip(1) {
            fs::remove_dir_all(entry.path)?;
        }
        click(&mut app, Action::Refresh);
        assert_eq!(app.world().resource::<Saves>().selected, 0);
        assert_eq!(app.world().resource::<Saves>().entries.len(), 1);
        Ok(())
    }
}
