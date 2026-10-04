//! Native replacement for SearchMenu's functional body/province navigation.
use super::{Focus, Game, Navigation, ui_font};
use bevy::{
    input::{ButtonState, keyboard::KeyboardInput, mouse::AccumulatedMouseScroll},
    prelude::*,
};
use cqsp_core::{search::SearchTarget, simulation::Simulation};

#[derive(Resource, Default)]
pub(super) struct Search {
    pub open: bool,
    pub capture_input: bool,
    query: String,
    results: Vec<SearchTarget>,
    selected: Option<usize>,
    replace: bool,
    revision: u64,
}
impl Search {
    pub fn set_query(&mut self, query: &str, simulation: &Simulation) {
        self.query.clear();
        self.append(query);
        self.changed(simulation);
    }
    fn append(&mut self, text: &str) {
        for ch in text.chars().filter(|ch| !ch.is_control()) {
            if self.query.len() + ch.len_utf8() > 4096 {
                break;
            }
            self.query.push(ch);
        }
    }
    fn changed(&mut self, simulation: &Simulation) {
        self.results = simulation.search(&self.query);
        self.selected = None;
        self.replace = false;
        self.revision += 1;
    }
    fn move_selection(&mut self, delta: i32) {
        if self.results.is_empty() {
            return;
        }
        self.selected = Some(
            (self.selected.map_or(-1, |index| index as i32) + delta)
                .clamp(0, self.results.len() as i32 - 1) as usize,
        );
    }
    fn close(&mut self) {
        self.open = false;
        self.capture_input = true; // Do not pass the dismissal to the world beneath it.
    }
    pub fn activate(
        &mut self,
        target: SearchTarget,
        zoom: bool,
        game: &Game,
        navigation: &mut Navigation,
    ) {
        if !self.open || !self.results.contains(&target) {
            return;
        }
        if zoom && let SearchTarget::Province(province) = target {
            navigation.focus_province(province, game);
        } else {
            navigation.active = true;
            navigation.select(
                Focus::Body(target.body(&game.simulation)),
                &game.simulation.solar_system,
            );
        }
        self.set_query("", &game.simulation);
        self.close();
    }
    fn edit(
        &mut self,
        event: &KeyboardInput,
        control: bool,
        game: &Game,
        navigation: &mut Navigation,
    ) {
        match event.key_code {
            KeyCode::ArrowDown => self.move_selection(1),
            KeyCode::ArrowUp => self.move_selection(-1),
            KeyCode::PageDown => self.move_selection(10),
            KeyCode::PageUp => self.move_selection(-10),
            KeyCode::Enter => {
                if let Some(target) = self
                    .selected
                    .and_then(|index| self.results.get(index))
                    .copied()
                {
                    self.activate(target, true, game, navigation);
                }
            }
            KeyCode::Escape => self.close(),
            KeyCode::KeyA if control => self.replace = true,
            KeyCode::Backspace | KeyCode::Delete => {
                if self.replace {
                    self.query.clear();
                } else {
                    self.query.pop();
                }
                self.changed(&game.simulation);
            }
            _ if !control => {
                if let Some(text) = event
                    .text
                    .as_deref()
                    .filter(|text| text.chars().any(|ch| !ch.is_control()))
                {
                    if self.replace {
                        self.query.clear();
                    }
                    self.append(text);
                    self.changed(&game.simulation);
                }
            }
            _ => {}
        }
    }
}
pub(super) fn input_available(search: Option<Res<Search>>) -> bool {
    search.is_none_or(|search| !search.capture_input)
}
#[derive(Component)]
pub(super) struct Panel;
#[derive(Component)]
pub(super) struct Content;
#[derive(Component)]
pub(super) struct Results(Handle<Font>);
#[derive(Component)]
pub(super) struct Action(SearchTarget);
#[derive(Component)]
pub(super) struct Row(usize);
pub(super) fn spawn(commands: &mut Commands, camera: Entity, font: &Handle<Font>) {
    commands.spawn((
        Panel, UiTargetCamera(camera), Interaction::None, GlobalZIndex(26),
        Node {
            display: Display::None,
            position_type: PositionType::Absolute,
            left: percent(25), width: percent(50), top: percent(10),
            padding: UiRect::all(px(12)), flex_direction: FlexDirection::Column, row_gap: px(8), ..default()
        },
        BackgroundColor(Color::srgb(0.025, 0.05, 0.09)),
    )).with_children(|panel| {
        panel.spawn((
            Text::new("SEARCH BODIES / PROVINCES"), ui_font(font, 18.0),
            TextColor(Color::srgb(0.9, 0.76, 0.42)),
        ));
        panel.spawn((Content, Text::default(), ui_font(font, 14.0)));
        panel.spawn((Text::new("Type to search · Ctrl+A replaces · Up/Down / PageUp/PageDown select\nEnter: zoom to province · Click: focus planet · Esc / outside click: close"), ui_font(font, 14.0)));
        panel.spawn((
            Results(font.clone()), Interaction::None,
            Node {
                max_height: px(300), overflow: Overflow::scroll_y(),
                flex_direction: FlexDirection::Column, flex_shrink: 0.0, ..default()
            },
            ScrollPosition::default(),
        ));
    });
}
#[allow(clippy::too_many_arguments, clippy::type_complexity)]
pub(super) fn controls(
    search: Option<ResMut<Search>>,
    keyboard: Res<ButtonInput<KeyCode>>,
    mouse: Res<ButtonInput<MouseButton>>,
    mut events: MessageReader<KeyboardInput>,
    game: Res<Game>,
    mut navigation: ResMut<Navigation>,
    inspector: Res<super::inspection_ui::Inspector>,
    ports: Res<super::spaceport_ui::SpaceportUi>,
    saves: Option<Res<super::save_ui::Saves>>,
    menu: Option<Res<super::application_ui::Menu>>,
    construction: Option<Res<super::construction_ui::ConstructionTool>>,
    navigation_menu: Option<Res<super::navigation_ui::Menu>>,
    buttons: Query<(&Action, &Interaction), Changed<Interaction>>,
    hover: Query<&Interaction, Or<(With<Panel>, With<Results>, With<Action>)>>,
) {
    let Some(mut search) = search else {
        events.clear();
        return;
    };
    search.capture_input = search.open;
    if game.session.player_country.is_none() {
        *search = Search::default();
        events.clear();
        return;
    }
    if inspector.capture_input
        || ports.capture_input
        || saves.is_some_and(|ui| ui.capture_input)
        || menu.is_some_and(|ui| ui.capture_input)
        || construction.is_some_and(|ui| ui.searching)
        || navigation_menu.is_some_and(|ui| ui.capture_input)
    {
        events.clear();
        return;
    }
    if keyboard.pressed(KeyCode::ControlLeft) && keyboard.just_pressed(KeyCode::KeyP) {
        search.open = !search.open;
        search.capture_input = true;
        events.clear();
        return;
    }
    if !search.open {
        events.clear();
        return;
    }
    if keyboard.just_pressed(KeyCode::Escape) {
        search.close();
        events.clear();
        return;
    }
    for (action, interaction) in &buttons {
        if *interaction == Interaction::Pressed {
            search.activate(action.0, false, &game, &mut navigation);
            if !search.open {
                events.clear();
                return;
            }
        }
    }
    if [MouseButton::Left, MouseButton::Right, MouseButton::Middle]
        .into_iter()
        .any(|button| mouse.just_pressed(button))
        && !hover
            .iter()
            .any(|interaction| *interaction != Interaction::None)
    {
        search.close();
        events.clear();
        return;
    }
    let control = keyboard.pressed(KeyCode::ControlLeft) || keyboard.pressed(KeyCode::ControlRight);
    for event in events
        .read()
        .filter(|event| event.state == ButtonState::Pressed)
    {
        if search.open {
            search.edit(event, control, &game, &mut navigation);
        }
    }
}
pub(super) fn scroll(
    search: Option<Res<Search>>,
    mouse: Res<AccumulatedMouseScroll>,
    mut lists: Query<&mut ScrollPosition, With<Results>>,
) {
    if search.is_none_or(|search| !search.open) || mouse.delta == Vec2::ZERO {
        return;
    }
    let delta = mouse.delta.y
        * if mouse.unit == bevy::input::mouse::MouseScrollUnit::Line {
            32.0
        } else {
            1.0
        };
    for mut scroll in &mut lists {
        scroll.y -= delta;
    }
}
#[allow(clippy::type_complexity, clippy::too_many_arguments)]
pub(super) fn update(
    mut commands: Commands,
    search: Option<Res<Search>>,
    game: Res<Game>,
    mut panels: Query<(&mut Node, &Children), With<Panel>>,
    navigation_menu: Option<Res<super::navigation_ui::Menu>>,
    mut texts: Query<(&mut Text, Option<&Content>)>,
    children: Query<&Children>,
    mut lists: Query<(Entity, &Results, &ComputedNode, &mut ScrollPosition)>,
    mut rows: Query<(&Row, &mut BackgroundColor), Without<Results>>,
    mut previous: Local<Option<(u64, Option<usize>)>>,
) {
    let Some(search) = search else { return };
    let shown = search.open
        && navigation_menu.is_none_or(|menu| !menu.open)
        && game.session.player_country.is_some()
        && super::event_ui::pending(&game).is_none();
    for (mut node, descendants) in &mut panels {
        let display = if shown { Display::Flex } else { Display::None };
        if node.display != display {
            node.display = display;
        }
        if shown {
            for entity in descendants
                .iter()
                .flat_map(|child| std::iter::once(child).chain(children.iter_descendants(child)))
            {
                if let Ok((mut text, content)) = texts.get_mut(entity) {
                    if content.is_some() {
                        text.0 = format!(
                            "Search: {}{} · {} matches",
                            search.query,
                            if search.replace { " [selected]" } else { "|" },
                            search.results.len()
                        );
                    } else {
                        text.set_changed();
                    }
                }
            }
        }
    }
    let state = (search.revision, search.selected);
    let changed = previous.as_ref() != Some(&state);
    let rebuild = previous
        .as_ref()
        .is_none_or(|(revision, _)| *revision != search.revision);
    for (entity, font, layout, mut scroll) in &mut lists {
        if rebuild {
            scroll.y = 0.0;
            commands.entity(entity).despawn_related::<Children>();
            commands.entity(entity).with_children(|list| {
                for (index, &target) in search.results.iter().enumerate() {
                    list.spawn((
                        Action(target),
                        Row(index),
                        Button,
                        Node {
                            height: px(30),
                            min_height: px(30),
                            flex_shrink: 0.0,
                            padding: UiRect::horizontal(px(8)),
                            align_items: AlignItems::Center,
                            ..default()
                        },
                        BackgroundColor(Color::srgb(0.14, 0.27, 0.4)),
                    ))
                    .with_children(|row| {
                        row.spawn((
                            Text::new(target.name(&game.simulation)),
                            ui_font(&font.0, 14.0),
                        ));
                    });
                }
            });
        } else if changed && let Some(index) = search.selected {
            let top = index as f32 * 30.0;
            let height = layout.size().y.max(30.0);
            if top < scroll.y {
                scroll.y = top;
            } else if top + 30.0 > scroll.y + height {
                scroll.y = top + 30.0 - height;
            }
        }
    }
    for (row, mut color) in &mut rows {
        let value = if Some(row.0) == search.selected {
            Color::srgb(0.25, 0.43, 0.6)
        } else {
            Color::srgb(0.14, 0.27, 0.4)
        };
        if color.0 != value {
            color.0 = value;
        }
    }
    *previous = Some(state);
}

#[cfg(test)]
mod tests {
    use super::*;
    use bevy::input::keyboard::Key;
    fn app() -> anyhow::Result<App> {
        let mut app = super::super::inspection_ui::tests::app()?;
        app.init_resource::<Search>()
            .add_systems(Startup, |mut commands: Commands| {
                spawn(&mut commands, Entity::PLACEHOLDER, &Handle::default());
            });
        app.update();
        Ok(app)
    }
    fn key(app: &mut App, key_code: KeyCode, text: Option<&str>) {
        app.world_mut()
            .resource_mut::<ButtonInput<KeyCode>>()
            .press(key_code);
        app.world_mut().write_message(KeyboardInput {
            key_code,
            logical_key: Key::Character(text.unwrap_or("").into()),
            state: ButtonState::Pressed,
            text: text.map(Into::into),
            repeat: false,
            window: Entity::PLACEHOLDER,
        });
        app.update();
        app.world_mut()
            .resource_mut::<ButtonInput<KeyCode>>()
            .reset_all();
    }
    fn query(app: &mut App, value: &str) {
        app.world_mut()
            .resource_scope(|world, mut search: Mut<Search>| {
                search.open = true;
                search.set_query(value, &world.resource::<Game>().simulation);
            });
        app.update();
    }
    #[test]
    fn search_toggles_with_left_control_captures_shortcuts_and_keeps_running_time()
    -> anyhow::Result<()> {
        let mut app = app()?;
        app.world_mut()
            .resource_mut::<ButtonInput<KeyCode>>()
            .press(KeyCode::ControlLeft);
        key(&mut app, KeyCode::KeyP, Some("p"));
        assert!(app.world().resource::<Search>().open);
        assert!(
            !app.world()
                .resource::<super::super::spaceport_ui::SpaceportUi>()
                .open
        );
        app.world_mut().resource_mut::<Game>().paused = false;
        app.world_mut()
            .resource_mut::<Time>()
            .advance_by(std::time::Duration::from_secs(10));
        for key_code in [
            KeyCode::KeyN,
            KeyCode::Space,
            KeyCode::KeyI,
            KeyCode::KeyB,
            KeyCode::Enter,
            KeyCode::Equal,
        ] {
            app.world_mut()
                .resource_mut::<ButtonInput<KeyCode>>()
                .press(key_code);
        }
        app.update();
        assert_eq!(app.world().resource::<Game>().simulation.date.tick(), 1);
        assert!(!app.world().resource::<Game>().paused);
        assert!(
            !app.world()
                .resource::<super::super::inspection_ui::Inspector>()
                .open
        );
        assert!(
            app.world()
                .resource::<Game>()
                .session
                .player_country
                .is_some()
        );
        app.world_mut()
            .resource_mut::<ButtonInput<KeyCode>>()
            .reset_all();
        key(&mut app, KeyCode::Escape, None);
        assert!(!app.world().resource::<Search>().open);
        assert!(app.world().resource::<Search>().capture_input);
        assert!(
            app.world()
                .resource::<Game>()
                .session
                .player_country
                .is_some()
        );
        app.update();
        assert!(!app.world().resource::<Search>().capture_input);
        app.world_mut()
            .resource_mut::<ButtonInput<KeyCode>>()
            .press(KeyCode::ControlRight);
        key(&mut app, KeyCode::KeyP, None);
        assert!(!app.world().resource::<Search>().open);
        assert!(
            app.world()
                .resource::<super::super::spaceport_ui::SpaceportUi>()
                .open
        );
        Ok(())
    }
    #[test]
    fn keyboard_clamps_pages_click_focuses_planet_enter_zooms_and_missing_centers_do_not_focus()
    -> anyhow::Result<()> {
        let mut app = app()?;
        query(&mut app, "a");
        assert_eq!(app.world().resource::<Search>().selected, None);
        key(&mut app, KeyCode::PageDown, None);
        assert_eq!(app.world().resource::<Search>().selected, Some(9));
        let list = app
            .world_mut()
            .query_filtered::<Entity, With<Results>>()
            .single(app.world())?;
        assert!(app.world().get::<ScrollPosition>(list).unwrap().y > 0.0);
        key(&mut app, KeyCode::PageUp, None);
        assert_eq!(app.world().resource::<Search>().selected, Some(0));
        key(&mut app, KeyCode::ArrowUp, None);
        assert_eq!(app.world().resource::<Search>().selected, Some(0));
        query(&mut app, "ILLINOIS");
        let target = app.world().resource::<Search>().results[0];
        assert!(matches!(target, SearchTarget::Province(_)));
        let button = app
            .world_mut()
            .spawn((Action(target), Interaction::Pressed))
            .id();
        app.update();
        app.world_mut().despawn(button);
        assert!(!app.world().resource::<Search>().open);
        assert!(app.world().resource::<Search>().query.is_empty());
        let earth = app
            .world()
            .resource::<Game>()
            .simulation
            .solar_system
            .body_id("earth")
            .unwrap();
        assert_eq!(
            app.world().resource::<Navigation>().focus,
            Focus::Body(earth)
        );
        query(&mut app, "ILLINOIS");
        key(&mut app, KeyCode::Enter, None);
        assert!(app.world().resource::<Search>().open); // Query changes never preselect a row.
        key(&mut app, KeyCode::ArrowDown, None);
        key(&mut app, KeyCode::Enter, None);
        assert!(!app.world().resource::<Search>().open);
        assert!(
            app.world()
                .resource::<Navigation>()
                .message
                .contains("800 km")
        );
        let missing = app
            .world()
            .resource::<Game>()
            .simulation
            .settlements
            .iter()
            .find(|site| site.searchable() && site.province_center.is_none())
            .unwrap()
            .name
            .clone();
        query(&mut app, &missing);
        let selected = app.world().resource::<Search>().results.iter().position(|target| matches!(target, SearchTarget::Province(id) if app.world().resource::<Game>().simulation.settlements[*id].province_center.is_none())).unwrap();
        app.world_mut().resource_mut::<Search>().selected = Some(selected);
        let message = app.world().resource::<Navigation>().message.clone();
        key(&mut app, KeyCode::Enter, None);
        assert!(!app.world().resource::<Search>().open);
        assert_eq!(app.world().resource::<Navigation>().message, message);
        Ok(())
    }
    #[test]
    fn editing_is_utf8_safe_clears_selection_and_scroll_and_outside_clicks_close_once()
    -> anyhow::Result<()> {
        let mut app = app()?;
        query(&mut app, "a");
        key(&mut app, KeyCode::ArrowDown, None);
        app.world_mut()
            .resource_mut::<ButtonInput<KeyCode>>()
            .press(KeyCode::ControlLeft);
        key(&mut app, KeyCode::KeyA, Some("a"));
        key(&mut app, KeyCode::KeyM, Some("mOoN"));
        assert_eq!(app.world().resource::<Search>().query, "mOoN");
        assert_eq!(app.world().resource::<Search>().selected, None);
        assert!(
            app.world()
                .resource::<Search>()
                .results
                .iter()
                .any(|target| matches!(target, SearchTarget::Body(_)))
        );
        key(&mut app, KeyCode::KeyA, Some("é\n"));
        assert_eq!(app.world().resource::<Search>().query, "mOoNé");
        key(&mut app, KeyCode::Backspace, None);
        assert_eq!(app.world().resource::<Search>().query, "mOoN");
        query(&mut app, &"é".repeat(5000));
        assert_eq!(app.world().resource::<Search>().query.len(), 4096);
        assert!(app.world().resource::<Search>().results.is_empty());
        key(&mut app, KeyCode::ArrowDown, None);
        assert_eq!(app.world().resource::<Search>().selected, None);
        for button in [MouseButton::Left, MouseButton::Right, MouseButton::Middle] {
            query(&mut app, "Mars");
            app.world_mut()
                .resource_mut::<ButtonInput<MouseButton>>()
                .press(button);
            app.update();
            assert!(!app.world().resource::<Search>().open);
            assert!(app.world().resource::<Search>().capture_input);
            app.world_mut()
                .resource_mut::<ButtonInput<MouseButton>>()
                .reset_all();
            app.update();
            assert!(!app.world().resource::<Search>().capture_input);
        }
        query(&mut app, "Earth");
        let stale = app
            .world_mut()
            .spawn((Action(SearchTarget::Province(0)), Interaction::Pressed))
            .id();
        app.update();
        assert!(app.world().resource::<Search>().open);
        app.world_mut().despawn(stale);
        app.world_mut()
            .resource_mut::<Game>()
            .session
            .return_to_selection();
        app.update();
        assert!(!app.world().resource::<Search>().open);
        assert!(!app.world().resource::<Search>().capture_input);
        Ok(())
    }
}
