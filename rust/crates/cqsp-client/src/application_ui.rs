//! Modal pause menu and silent display options; no C++ settings files are modified.
use super::{
    Game, Navigation, inspection_ui::Inspector, save_ui::Saves, spaceport_ui::SpaceportUi,
    ui_button, ui_font, ui_row,
};
use anyhow::{Context, Result};
use bevy::{
    input::mouse::{AccumulatedMouseScroll, MouseScrollUnit},
    prelude::*,
    window::{MonitorSelection, PrimaryWindow, VideoModeSelection, WindowMode, WindowPosition},
};
use cqsp_core::settings::{GraphicsSettings, RESOLUTIONS};
use std::path::PathBuf;

#[derive(Clone, Copy, Debug, Default, PartialEq, Eq)]
pub(super) enum Screen {
    #[default]
    Closed,
    Pause,
    Graphics,
}
#[derive(Resource)]
pub(super) struct Menu {
    pub screen: Screen,
    pub capture_input: bool,
    pub settings: GraphicsSettings,
    path: PathBuf,
    return_to_pause: bool,
    message: String,
}
impl Menu {
    pub fn new(path: PathBuf, settings: GraphicsSettings) -> Self {
        Self {
            screen: Screen::Closed,
            capture_input: false,
            settings,
            path,
            return_to_pause: false,
            message: String::new(),
        }
    }
    pub fn is_open(&self) -> bool {
        self.screen != Screen::Closed
    }
    fn allows(&self, action: Action) -> bool {
        match self.screen {
            Screen::Closed => matches!(action, Action::OpenGraphics | Action::OpenPause),
            Screen::Pause => matches!(
                action,
                Action::Resume
                    | Action::SaveGame
                    | Action::LoadGame
                    | Action::ExitToMenu
                    | Action::OpenGraphics
                    | Action::ExitGame
            ),
            Screen::Graphics => matches!(
                action,
                Action::Resume
                    | Action::CloseGraphics
                    | Action::Previous
                    | Action::Next
                    | Action::Fullscreen
                    | Action::Persist
            ),
        }
    }
    pub fn apply(&mut self, action: Action) -> Result<()> {
        if !self.allows(action) {
            return Ok(());
        }
        match action {
            Action::OpenPause => {
                self.screen = Screen::Pause;
                self.capture_input = true;
                self.message.clear();
            }
            Action::OpenGraphics => {
                self.return_to_pause = self.screen == Screen::Pause;
                self.screen = Screen::Graphics;
                self.capture_input = true;
                self.message.clear();
            }
            Action::Resume => self.screen = Screen::Closed,
            Action::CloseGraphics => {
                self.screen = if self.return_to_pause {
                    Screen::Pause
                } else {
                    Screen::Closed
                };
            }
            Action::Previous | Action::Next => {
                let index = self.settings.selected_resolution();
                let step = if matches!(action, Action::Previous) {
                    RESOLUTIONS.len() - 1
                } else {
                    1
                };
                self.settings.set_resolution((index + step) % RESOLUTIONS.len())?;
            }
            Action::Fullscreen => self.settings.full_screen = !self.settings.full_screen,
            Action::Persist => {
                self.settings.write(&self.path)?;
                self.message = "Display preferences saved. Audio remains disabled.".into();
            }
            Action::LoadGame => self.message = "Upstream's pause-menu Load Game button is a no-op. The metadata browser is available after resuming.".into(),
            Action::ExitToMenu => self.message = "Upstream's Exit To Menu button is a no-op. This does not change your session.".into(),
            Action::SaveGame | Action::ExitGame => {}
        }
        Ok(())
    }
    fn content(&self, game: &Game) -> String {
        let text = match self.screen {
            Screen::Graphics => format!(
                "Windowed size: {} × {} · preset {}/{}\nFullscreen: {} (primary monitor's current video mode)\nChanges preview immediately; Save preferences persists them.\nStored windowed size is restored when leaving fullscreen.\n\nPreferences: {}\nOriginal C++ settings are not touched.\nDefault: 1280 × 720, decorated window, fullscreen off.",
                self.settings.window.width,
                self.settings.window.height,
                self.settings.selected_resolution() + 1,
                RESOLUTIONS.len(),
                if self.settings.full_screen {
                    "on"
                } else {
                    "off"
                },
                self.path.display()
            ),
            _ => format!(
                "Simulation is halted while this menu is open.\nPlayback remains {}. Resume preserves that choice.\nVersion {} · saves contain metadata only.",
                if game.paused { "paused" } else { "running" },
                cqsp_core::save::ORIGINAL_VERSION
            ),
        };
        if self.message.is_empty() {
            text
        } else {
            format!("{text}\n\n{}", self.message)
        }
    }
}
pub(super) fn default_path() -> Result<PathBuf> {
    Ok(super::save_ui::default_root()?.with_file_name("settings.hjson"))
}
pub(super) fn apply_window(settings: GraphicsSettings, window: &mut Window) {
    window.decorations = settings.window.decorated;
    window.mode = if settings.full_screen {
        WindowMode::Fullscreen(MonitorSelection::Primary, VideoModeSelection::Current)
    } else {
        WindowMode::Windowed
    };
    if !settings.full_screen {
        window
            .resolution
            .set(settings.window.width as f32, settings.window.height as f32);
        window.position = WindowPosition::At(IVec2::new(40, 40));
    }
}
#[derive(Component, Clone, Copy)]
pub(super) enum Action {
    OpenPause,
    OpenGraphics,
    Resume,
    SaveGame,
    LoadGame,
    ExitToMenu,
    ExitGame,
    CloseGraphics,
    Previous,
    Next,
    Fullscreen,
    Persist,
}
#[derive(Component)]
pub(super) enum Panel {
    Launcher,
    Overlay(Screen),
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
                bottom: px(76),
                ..default()
            },
        ))
        .with_children(|bar| {
            ui_button(bar, font, Action::OpenGraphics, "Display settings");
        });
    for screen in [Screen::Pause, Screen::Graphics] {
        commands.spawn((Panel::Overlay(screen), UiTargetCamera(camera), GlobalZIndex(30),
            Node { display: Display::None, position_type: PositionType::Absolute,
                left: if screen == Screen::Pause { percent(25) } else { px(70) },
                right: if screen == Screen::Pause { percent(25) } else { px(70) },
                top: px(70), bottom: px(70), padding: UiRect::all(px(20)), flex_direction: FlexDirection::Column,
                row_gap: px(12), overflow: Overflow::scroll_y(), ..default() },
            BackgroundColor(Color::srgb(0.025, 0.05, 0.09))))
            .with_children(|panel| {
                panel.spawn((Text::new(if screen == Screen::Pause { "PAUSE MENU" } else { "DISPLAY OPTIONS · SILENT" }), ui_font(font, 22.0), TextColor(Color::srgb(0.9, 0.76, 0.42)), Node { flex_shrink: 0.0, ..default() }));
                if screen == Screen::Pause {
                    for (action, label) in [(Action::Resume, "Resume (Esc)"), (Action::SaveGame, "Save Game (metadata only)"), (Action::LoadGame, "Load Game (upstream no-op)"),
                        (Action::OpenGraphics, "Options"), (Action::ExitToMenu, "Exit To Menu (upstream no-op)"), (Action::ExitGame, "Exit Game")] {
                        ui_row(panel, |row| { ui_button(row, font, action, label); });
                    }
                } else {
                    ui_row(panel, |row| {
                        for (action, label) in [(Action::Previous, "Previous resolution"), (Action::Next, "Next resolution"), (Action::Fullscreen, "Toggle fullscreen"),
                            (Action::Persist, "Save preferences"), (Action::CloseGraphics, "Back / close")] {
                            ui_button(row, font, action, label);
                        }
                    });
                    panel.spawn((Text::new("Audio is intentionally unavailable: no music, effects or sound-device backend."), ui_font(font, 16.0), Node { flex_shrink: 0.0, ..default() }));
                }
                panel.spawn((Content, Text::default(), ui_font(font, 16.0), Node { flex_shrink: 0.0, ..default() }));
            });
    }
}
#[allow(clippy::too_many_arguments)]
pub(super) fn controls(
    menu: Option<ResMut<Menu>>,
    keyboard: Res<ButtonInput<KeyCode>>,
    buttons: Query<(&Action, &Interaction), Changed<Interaction>>,
    mut game: ResMut<Game>,
    mut navigation: ResMut<Navigation>,
    mut inspector: ResMut<Inspector>,
    mut ports: ResMut<SpaceportUi>,
    saves: Option<ResMut<Saves>>,
    construction: Option<Res<super::construction_ui::ConstructionTool>>,
    mut windows: Query<&mut Window, With<PrimaryWindow>>,
    mut exit: MessageWriter<AppExit>,
) {
    let Some(mut menu) = menu else {
        return;
    };
    let capture = menu.is_open();
    if menu.capture_input != capture {
        menu.capture_input = capture;
    }
    let other_input = ports.capture_input
        || inspector.capture_input
        || saves.as_ref().is_some_and(|saves| saves.capture_input)
        || construction
            .as_ref()
            .is_some_and(|tool| tool.armed || tool.searching || tool.capture_input);
    if keyboard.just_pressed(KeyCode::Escape) {
        if menu.is_open() {
            menu.apply(Action::Resume).expect("Resume menu");
        } else if !other_input && game.session.player_country.is_some() {
            menu.apply(Action::OpenPause).expect("Open pause menu");
        }
        return;
    }
    let mut saves = saves;
    for (action, interaction) in &buttons {
        if *interaction != Interaction::Pressed || !menu.allows(*action) {
            continue;
        }
        if matches!(action, Action::OpenGraphics)
            && (ports.editing()
                || inspector.open
                || saves.as_ref().is_some_and(|saves| saves.open)
                || construction
                    .as_ref()
                    .is_some_and(|tool| tool.searching || tool.capture_input))
        {
            continue;
        }
        let result = if matches!(action, Action::SaveGame) {
            saves.as_mut().context("Metadata storage unavailable").and_then(|saves| {
                saves.apply(super::save_ui::Action::Save, &mut game, &mut navigation, &mut inspector, &mut ports)
            }).map(|()| menu.message = "Saved date/UUID/version only. Economy, ships and country selection are not saved.".into())
        } else {
            menu.apply(*action)
        };
        if let Err(error) = result {
            menu.message = error.to_string();
            continue;
        }
        if matches!(action, Action::Previous | Action::Next | Action::Fullscreen) {
            for mut window in &mut windows {
                apply_window(menu.settings, &mut window);
            }
        }
        if matches!(action, Action::ExitGame) {
            exit.write(AppExit::Success);
        }
    }
}
pub(super) fn scroll(
    menu: Option<Res<Menu>>,
    mouse: Res<AccumulatedMouseScroll>,
    mut panels: Query<(&Panel, &mut ScrollPosition)>,
) {
    if mouse.delta.y == 0.0 {
        return;
    }
    let Some(menu) = menu.filter(|menu| menu.is_open()) else {
        return;
    };
    let scale = if mouse.unit == MouseScrollUnit::Line {
        32.0
    } else {
        1.0
    };
    for (panel, mut position) in &mut panels {
        if matches!(panel, Panel::Overlay(screen) if *screen == menu.screen) {
            position.0.y = (position.0.y - mouse.delta.y * scale).max(0.0);
        }
    }
}

#[allow(clippy::too_many_arguments)]
pub(super) fn update(
    menu: Option<Res<Menu>>,
    game: Res<Game>,
    navigation: Res<Navigation>,
    ports: Res<SpaceportUi>,
    inspector: Res<Inspector>,
    saves: Option<Res<Saves>>,
    mut panels: Query<(Entity, &Panel, &mut Node)>,
    navigation_menu: Option<Res<super::navigation_ui::Menu>>,
    children: Query<&Children>,
    mut texts: Query<(&mut Text, Option<&Content>)>,
) {
    let Some(menu) = menu else {
        return;
    };
    for (entity, panel, mut node) in &mut panels {
        let shown = match panel {
            Panel::Launcher => {
                !menu.is_open()
                    && !navigation.active
                    && !ports.open
                    && !inspector.open
                    && saves.as_ref().is_none_or(|saves| !saves.open)
            }
            Panel::Overlay(screen) => menu.screen == *screen,
        };
        let shown = shown
            && super::event_ui::pending(&game).is_none()
            && navigation_menu.as_ref().is_none_or(|menu| !menu.open);
        let display = if shown { Display::Flex } else { Display::None };
        if node.display != display {
            node.display = display;
        }
        if shown && matches!(panel, Panel::Overlay(_)) && (menu.is_changed() || game.is_changed()) {
            for child in children.iter_descendants(entity) {
                if let Ok((mut text, content)) = texts.get_mut(child) {
                    if content.is_some() {
                        text.0 = menu.content(&game);
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
    use cqsp_core::{Session, clock::StarDate, default_data_path, save::SaveStore};
    use std::{fs, time::Duration};
    struct TestRoot(PathBuf);
    impl Drop for TestRoot {
        fn drop(&mut self) {
            let _ = fs::remove_dir_all(&self.0);
        }
    }
    fn app() -> Result<(App, TestRoot, Entity)> {
        let root = TestRoot(std::env::temp_dir().join(format!(
            "cqsp-application-ui-{}-{}",
            std::process::id(),
            cqsp_core::save::new_uuid()
        )));
        anyhow::ensure!(!root.0.exists(), "Test root already exists");
        let mut app = super::super::inspection_ui::tests::app()?;
        app.insert_resource(Menu::new(
            root.0.join("settings.hjson"),
            GraphicsSettings::default(),
        ))
        .insert_resource(Saves::new(default_data_path(), root.0.join("saves")));
        let window = app
            .world_mut()
            .spawn((Window::default(), PrimaryWindow))
            .id();
        app.update();
        Ok((app, root, window))
    }
    fn click(app: &mut App, action: Action) {
        let button = app.world_mut().spawn((action, Interaction::Pressed)).id();
        app.update();
        app.world_mut().despawn(button);
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
    fn pause_halts_without_changing_playback_saves_once_and_preserves_upstream_no_ops() -> Result<()>
    {
        let (mut app, root, _) = app()?;
        app.world_mut().resource_mut::<Game>().paused = false;
        key(&mut app, KeyCode::Escape);
        assert_eq!(app.world().resource::<Menu>().screen, Screen::Pause);
        assert!(
            app.world()
                .resource::<Game>()
                .session
                .player_country
                .is_some()
        );
        app.world_mut()
            .resource_mut::<Time>()
            .advance_by(Duration::from_secs(1000));
        for key_code in [
            KeyCode::Space,
            KeyCode::KeyN,
            KeyCode::KeyI,
            KeyCode::KeyP,
            KeyCode::Enter,
        ] {
            key(&mut app, key_code);
        }
        assert!(!app.world().resource::<Game>().paused);
        assert_eq!(app.world().resource::<Game>().simulation.date.tick(), 0);
        let button = app
            .world_mut()
            .spawn((Action::SaveGame, Interaction::Pressed))
            .id();
        app.update();
        let entry = SaveStore(root.0.join("saves")).list()?.remove(0);
        assert_eq!(SaveStore::read(&entry.path)?.date, 0);
        app.world_mut().resource_mut::<Game>().simulation.date = StarDate::from_tick(42);
        app.update();
        assert_eq!(SaveStore::read(&entry.path)?.date, 0);
        app.world_mut().despawn(button);
        for action in [Action::LoadGame, Action::ExitToMenu] {
            click(&mut app, action);
            assert_eq!(app.world().resource::<Menu>().screen, Screen::Pause);
            assert!(app.world().resource::<Menu>().message.contains("no-op"));
            assert!(
                app.world()
                    .resource::<Game>()
                    .session
                    .player_country
                    .is_some()
            );
            assert_eq!(app.world().resource::<Game>().simulation.date.tick(), 42);
        }
        click(&mut app, Action::OpenGraphics);
        click(&mut app, Action::CloseGraphics);
        assert_eq!(app.world().resource::<Menu>().screen, Screen::Pause);
        key(&mut app, KeyCode::Escape);
        assert_eq!(app.world().resource::<Menu>().screen, Screen::Closed);
        assert!(app.world().resource::<Menu>().capture_input);
        assert_eq!(app.world().resource::<Game>().simulation.date.tick(), 42);
        app.update();
        assert_eq!(app.world().resource::<Game>().simulation.date.tick(), 43);
        Ok(())
    }
    #[test]
    fn graphics_cycles_every_preset_keeps_windowed_size_in_fullscreen_and_persists_explicitly()
    -> Result<()> {
        let (mut app, root, window) = app()?;
        app.world_mut().resource_mut::<Game>().session = Session::default();
        click(&mut app, Action::OpenGraphics);
        assert!(!root.0.exists());
        let initial = app
            .world()
            .resource::<Menu>()
            .settings
            .selected_resolution();
        for offset in 1..=15 {
            click(&mut app, Action::Next);
            let (width, height) = RESOLUTIONS[(initial + offset) % 15];
            let native = app.world().get::<Window>(window).unwrap();
            assert_eq!(
                (native.width(), native.height()),
                (width as f32, height as f32)
            );
            assert_eq!(native.position, WindowPosition::At(IVec2::new(40, 40)));
        }
        click(&mut app, Action::Previous);
        let size = {
            let window = app.world().get::<Window>(window).unwrap();
            (window.width(), window.height())
        };
        click(&mut app, Action::Fullscreen);
        assert_eq!(
            app.world().get::<Window>(window).unwrap().mode,
            WindowMode::Fullscreen(MonitorSelection::Primary, VideoModeSelection::Current)
        );
        click(&mut app, Action::Next);
        let native = app.world().get::<Window>(window).unwrap();
        assert_eq!((native.width(), native.height()), size);
        click(&mut app, Action::Fullscreen);
        let settings = app.world().resource::<Menu>().settings;
        let native = app.world().get::<Window>(window).unwrap();
        assert_eq!(native.mode, WindowMode::Windowed);
        assert_eq!(
            (native.width(), native.height()),
            (settings.window.width as f32, settings.window.height as f32)
        );
        assert!(!root.0.exists());
        let save = app
            .world_mut()
            .spawn((Action::Persist, Interaction::Pressed))
            .id();
        app.update();
        assert_eq!(
            GraphicsSettings::load(&root.0.join("settings.hjson"))?,
            settings
        );
        app.world_mut()
            .resource_mut::<Menu>()
            .settings
            .set_resolution(14)?;
        app.update();
        assert_eq!(
            GraphicsSettings::load(&root.0.join("settings.hjson"))?,
            settings
        );
        app.world_mut().despawn(save);
        click(&mut app, Action::CloseGraphics);
        assert_eq!(app.world().resource::<Menu>().screen, Screen::Closed);
        assert_eq!(app.world().resource::<Game>().simulation.date.tick(), 0);
        Ok(())
    }
    #[test]
    fn menu_shortcuts_respect_other_overlays_painting_and_failed_writes_remain_editable()
    -> Result<()> {
        let (mut app, root, _) = app()?;
        key(&mut app, KeyCode::KeyI);
        key(&mut app, KeyCode::Escape);
        assert!(!app.world().resource::<Menu>().is_open());
        assert!(
            app.world()
                .resource::<Game>()
                .session
                .player_country
                .is_some()
        );
        app.update();
        app.init_resource::<super::super::construction_ui::ConstructionTool>();
        key(&mut app, KeyCode::KeyB);
        key(&mut app, KeyCode::Escape);
        assert!(!app.world().resource::<Menu>().is_open());
        app.update();
        click(&mut app, Action::OpenGraphics);
        let path = root.0.join("settings.hjson");
        fs::create_dir_all(&path)?; // Only the agent-owned fixture is made unwritable as a file.
        click(&mut app, Action::Persist);
        assert_eq!(app.world().resource::<Menu>().screen, Screen::Graphics);
        assert!(!app.world().resource::<Menu>().message.contains("saved"));
        assert!(path.is_dir());
        click(&mut app, Action::Next);
        assert_eq!(
            app.world()
                .resource::<Menu>()
                .settings
                .selected_resolution(),
            3
        );
        key(&mut app, KeyCode::Escape);
        app.update();
        click(&mut app, Action::OpenPause);
        click(&mut app, Action::ExitGame);
        assert!(matches!(app.should_exit(), Some(AppExit::Success)));
        Ok(())
    }
}
