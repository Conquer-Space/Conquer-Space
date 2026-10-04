//! The original ConstructionMenu's owned-province painting tool, without Lua/ImGui.
use super::{Game, Navigation, ui_button, ui_font, ui_row};
use anyhow::Result;
use bevy::{
    input::{ButtonState, keyboard::KeyboardInput, mouse::AccumulatedMouseScroll},
    prelude::*,
};
use cqsp_core::catalog::RecipeId;

#[derive(Resource, Default)]
pub(super) struct ConstructionTool {
    pub armed: bool,
    pub capture_input: bool,
    pub searching: bool,
    recipe: usize,
    query: String,
    highlighted: Option<usize>,
    replace: bool,
    message: String,
}
impl ConstructionTool {
    pub fn apply(&mut self, action: Action, game: &Game) {
        let count = game.simulation.catalog.recipes.len();
        if self.searching {
            match action {
                Action::Search | Action::CloseSearch => self.searching = false,
                Action::SelectRecipe(index) if self.matches(game, index) => {
                    self.recipe = index;
                    self.searching = false;
                }
                _ => {}
            }
            return;
        }
        match action {
            Action::Search => {
                self.searching = true;
                self.capture_input = true;
                self.query.clear();
                self.replace = false;
                self.highlighted = count.checked_sub(1).map(|last| self.recipe.min(last));
            }
            Action::Toggle => self.armed = !self.armed,
            Action::Previous | Action::Next if count > 0 => {
                self.recipe = (self.recipe
                    + if matches!(action, Action::Previous) {
                        count - 1
                    } else {
                        1
                    })
                    % count;
            }
            _ => {}
        }
    }
    fn matches(&self, game: &Game, index: usize) -> bool {
        game.simulation
            .catalog
            .recipes
            .get(index)
            .is_some_and(|recipe| {
                recipe
                    .name
                    .to_ascii_lowercase()
                    .contains(&self.query.to_ascii_lowercase())
            })
    }
    fn results(&self, game: &Game) -> Vec<usize> {
        (0..game.simulation.catalog.recipes.len())
            .filter(|&index| self.matches(game, index))
            .collect()
    }
    pub fn set_query(&mut self, query: &str, game: &Game) {
        self.query.clear();
        self.replace = false;
        self.append_query(query);
        self.refresh_highlight(game);
    }
    fn append_query(&mut self, text: &str) {
        for ch in text.chars().filter(|ch| !ch.is_control()) {
            if self.query.len() + ch.len_utf8() > 127 {
                break;
            }
            self.query.push(ch);
        }
    }
    fn refresh_highlight(&mut self, game: &Game) {
        if !self
            .highlighted
            .is_some_and(|index| self.matches(game, index))
        {
            self.highlighted = self.results(game).first().copied();
        }
    }
    fn move_highlight(&mut self, game: &Game, previous: bool) {
        let results = self.results(game);
        self.highlighted = if results.is_empty() {
            None
        } else {
            let index = self
                .highlighted
                .and_then(|index| results.iter().position(|&id| id == index));
            Some(
                results[match index {
                    None => 0,
                    Some(index) => {
                        (index + if previous { results.len() - 1 } else { 1 }) % results.len()
                    }
                }],
            )
        };
    }
    fn edit(&mut self, event: &KeyboardInput, control: bool, game: &Game) {
        match event.key_code {
            KeyCode::Escape => self.searching = false,
            KeyCode::Enter => {
                if let Some(index) = self.highlighted {
                    self.apply(Action::SelectRecipe(index), game);
                }
            }
            KeyCode::ArrowUp | KeyCode::ArrowDown => {
                self.move_highlight(game, event.key_code == KeyCode::ArrowUp)
            }
            KeyCode::KeyA if control => self.replace = true,
            KeyCode::Backspace | KeyCode::Delete => {
                if self.replace {
                    self.query.clear();
                } else {
                    self.query.pop();
                }
                self.replace = false;
            }
            _ if !control => {
                if let Some(text) = &event.text {
                    let mut chars = text.chars().filter(|ch| !ch.is_control()).peekable();
                    if chars.peek().is_some() && self.replace {
                        self.query.clear();
                        self.replace = false;
                    }
                    self.append_query(text);
                }
            }
            _ => {}
        }
        self.refresh_highlight(game);
    }
    fn search_content(&self, game: &Game) -> String {
        format!(
            "Search: {}{} · {} matches\nType to filter · Ctrl+A replaces · Up/Down chooses · Enter selects · Esc closes",
            self.query,
            if self.replace { " [selected]" } else { "|" },
            self.results(game).len()
        )
    }
    pub fn paint(&mut self, game: &mut Game, count: i32) -> Result<()> {
        let Some(country) = game.session.player_country else {
            return Ok(());
        };
        let Some(province) = game.hovered else {
            return Ok(());
        };
        if let Some(factory) =
            game.simulation
                .paint_factory(country, province, RecipeId(self.recipe), count)?
        {
            let site = &game.simulation.settlements[province];
            let industry = &site.industries[factory];
            self.message = format!(
                "{} · {} · size {:.0}, utilization {:.0}",
                site.name,
                game.simulation.catalog.recipes[industry.recipe.0].name,
                industry.size,
                industry.utilization
            );
        } else {
            self.message = "Painting requires a province currently owned by your country.".into();
        }
        Ok(())
    }
    fn content(&self, game: &Game) -> String {
        let Some(recipe) = game.simulation.catalog.recipes.get(self.recipe) else {
            return "No recipes loaded.".into();
        };
        format!(
            "{} · recipe {}/{}: {}\nOwned provinces only. Left Shift: 5 · Left Ctrl: 10 · both: 5\nEmpty province: new factory. Otherwise only the last factory's size grows.\n{}",
            if self.armed {
                "PAINTING · Esc cancels"
            } else {
                "Painting off · B toggles"
            },
            self.recipe + 1,
            game.simulation.catalog.recipes.len(),
            recipe.name,
            self.message
        )
    }
}
#[derive(Component, Clone, Copy)]
pub(super) enum Action {
    Toggle,
    Previous,
    Next,
    Search,
    CloseSearch,
    SelectRecipe(usize),
}
#[derive(Component)]
pub(super) enum Panel {
    Toolbar,
    Search,
}
#[derive(Component)]
pub(super) struct Content;
#[derive(Component)]
pub(super) struct SearchContent;
#[derive(Component)]
pub(super) struct SearchResults;
pub(super) fn spawn(
    commands: &mut Commands,
    camera: Entity,
    font: &Handle<Font>,
    recipes: &[cqsp_core::catalog::Recipe],
) {
    commands
        .spawn((
            Panel::Toolbar,
            UiTargetCamera(camera),
            Interaction::None,
            Node {
                display: Display::None,
                position_type: PositionType::Absolute,
                left: px(336),
                right: px(336),
                top: px(24),
                padding: UiRect::all(px(12)),
                flex_direction: FlexDirection::Column,
                row_gap: px(6),
                ..default()
            },
            BackgroundColor(Color::srgba(0.025, 0.05, 0.09, 0.95)),
        ))
        .with_children(|panel| {
            ui_row(panel, |row| {
                ui_button(row, font, Action::Toggle, "Construct / cancel (B)");
                ui_button(row, font, Action::Previous, "Previous recipe");
                ui_button(row, font, Action::Next, "Next recipe");
                ui_button(row, font, Action::Search, "Search recipes");
            });
            panel.spawn((
                Content,
                Text::default(),
                ui_font(font, 14.0),
                Node {
                    flex_shrink: 0.0,
                    ..default()
                },
            ));
        });
    commands
        .spawn((
            Panel::Search,
            UiTargetCamera(camera),
            Interaction::None,
            GlobalZIndex(25),
            Node {
                display: Display::None,
                position_type: PositionType::Absolute,
                left: px(336),
                right: px(336),
                top: px(180),
                bottom: px(80),
                padding: UiRect::all(px(12)),
                flex_direction: FlexDirection::Column,
                row_gap: px(8),
                ..default()
            },
            BackgroundColor(Color::srgba(0.025, 0.05, 0.09, 0.99)),
        ))
        .with_children(|panel| {
            panel.spawn((
                Text::new("SEARCH CONSTRUCTION RECIPES"),
                ui_font(font, 18.0),
                Node {
                    flex_shrink: 0.0,
                    ..default()
                },
            ));
            ui_button(panel, font, Action::CloseSearch, "Close search (Esc)");
            panel.spawn((
                SearchContent,
                Text::default(),
                ui_font(font, 14.0),
                Node {
                    flex_shrink: 0.0,
                    ..default()
                },
            ));
            panel
                .spawn((
                    SearchResults,
                    Node {
                        flex_direction: FlexDirection::Column,
                        row_gap: px(4),
                        overflow: Overflow::scroll_y(),
                        min_height: px(0),
                        flex_grow: 1.0,
                        ..default()
                    },
                    ScrollPosition::default(),
                ))
                .with_children(|list| {
                    for (index, recipe) in recipes.iter().enumerate() {
                        ui_button(list, font, Action::SelectRecipe(index), &recipe.name);
                    }
                });
        });
}
#[allow(clippy::too_many_arguments)]
pub(super) fn controls(
    tool: Option<ResMut<ConstructionTool>>,
    keyboard: Res<ButtonInput<KeyCode>>,
    mouse: Res<ButtonInput<MouseButton>>,
    actions: Query<(&Action, &Interaction), Changed<Interaction>>,
    ui_hover: Query<&Interaction>,
    navigation: Res<Navigation>,
    mut game: ResMut<Game>,
    mut events: MessageReader<KeyboardInput>,
) {
    let Some(mut tool) = tool else {
        return;
    };
    if tool.capture_input != tool.searching {
        tool.capture_input = tool.searching;
    }
    if game.session.player_country.is_none() || navigation.active {
        tool.searching = false;
        events.clear();
        return;
    }
    if tool.searching {
        if keyboard.just_pressed(KeyCode::Escape) {
            tool.searching = false;
            events.clear();
            return;
        }
        let control =
            keyboard.pressed(KeyCode::ControlLeft) || keyboard.pressed(KeyCode::ControlRight);
        for event in events
            .read()
            .filter(|event| event.state == ButtonState::Pressed)
        {
            if tool.searching {
                tool.edit(event, control, &game);
            }
        }
        for (action, interaction) in &actions {
            if *interaction == Interaction::Pressed && tool.searching {
                tool.apply(*action, &game);
            }
        }
        return;
    }
    events.clear();
    if tool.armed && keyboard.just_pressed(KeyCode::Escape) {
        tool.armed = false;
        tool.capture_input = true; // Cancel cannot also leave the game in this frame.
        return;
    }
    if keyboard.just_pressed(KeyCode::KeyB) {
        tool.armed = !tool.armed;
    }
    for (action, interaction) in &actions {
        if *interaction == Interaction::Pressed {
            tool.apply(*action, &game);
        }
    }
    if tool.capture_input {
        return;
    }
    if tool.armed
        && mouse.just_pressed(MouseButton::Left)
        && !ui_hover
            .iter()
            .any(|interaction| *interaction != Interaction::None)
    {
        let count = if keyboard.pressed(KeyCode::ShiftLeft) {
            5
        } else if keyboard.pressed(KeyCode::ControlLeft) {
            10
        } else {
            1
        };
        if let Err(error) = tool.paint(&mut game, count) {
            tool.message = error.to_string();
        }
    }
}
pub(super) fn scroll(
    tool: Option<Res<ConstructionTool>>,
    mouse: Res<AccumulatedMouseScroll>,
    mut lists: Query<&mut ScrollPosition, With<SearchResults>>,
) {
    if tool.is_none_or(|tool| !tool.searching) || mouse.delta == Vec2::ZERO {
        return;
    }
    let delta = mouse.delta.y
        * if mouse.unit == bevy::input::mouse::MouseScrollUnit::Line {
            32.0
        } else {
            1.0
        };
    for mut position in &mut lists {
        position.y -= delta;
    }
}
#[allow(clippy::too_many_arguments)]
pub(super) fn update(
    tool: Option<ResMut<ConstructionTool>>,
    game: Res<Game>,
    navigation: Res<Navigation>,
    ports: Res<super::spaceport_ui::SpaceportUi>,
    inspector: Res<super::inspection_ui::Inspector>,
    saves: Option<Res<super::save_ui::Saves>>,
    menu: Option<Res<super::application_ui::Menu>>,
    mut panels: Query<(Entity, &Panel, &mut Node)>,
    mut choices: Query<(&Action, &mut Node, &mut BackgroundColor), Without<Panel>>,
    mut results: Query<&mut ScrollPosition, With<SearchResults>>,
    mut previous_search: Local<Option<(bool, String)>>,
    children: Query<&Children>,
    mut texts: Query<(&mut Text, Option<&Content>, Option<&SearchContent>)>,
) {
    let Some(mut tool) = tool else {
        return;
    };
    if game.session.player_country.is_none()
        && (tool.armed || tool.searching || !tool.message.is_empty())
    {
        tool.armed = false;
        tool.searching = false;
        tool.capture_input = false;
        tool.message.clear();
    }
    let shown = game.session.player_country.is_some()
        && !navigation.active
        && !ports.open
        && !inspector.open
        && saves.is_none_or(|saves| !saves.open);
    let shown = shown
        && menu.is_none_or(|menu| !menu.is_open())
        && super::event_ui::pending(&game).is_none();
    for (entity, panel, mut node) in &mut panels {
        let visible = shown && (matches!(panel, Panel::Toolbar) || tool.searching);
        let display = if visible {
            Display::Flex
        } else {
            Display::None
        };
        if node.display != display {
            node.display = display;
        }
        if visible && (tool.is_changed() || game.is_changed()) {
            for child in children.iter_descendants(entity) {
                if let Ok((mut text, content, search)) = texts.get_mut(child) {
                    if content.is_some() {
                        text.0 = tool.content(&game);
                    } else if search.is_some() {
                        text.0 = tool.search_content(&game);
                    } else {
                        text.set_changed();
                    }
                }
            }
        }
    }
    for (action, mut node, mut background) in &mut choices {
        if let Action::SelectRecipe(index) = *action {
            let display = if tool.matches(&game, index) {
                Display::Flex
            } else {
                Display::None
            };
            if node.display != display {
                node.display = display;
            }
            let color = if tool.highlighted == Some(index) {
                Color::srgb(0.25, 0.43, 0.6)
            } else {
                Color::srgb(0.14, 0.27, 0.4)
            };
            if background.0 != color {
                background.0 = color;
            }
        }
    }
    let state = (tool.searching, tool.query.clone());
    if previous_search.as_ref() != Some(&state) {
        for mut position in &mut results {
            position.y = 0.0;
        }
        *previous_search = Some(state);
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use bevy::input::keyboard::Key;
    use cqsp_core::Session;

    fn app() -> Result<App> {
        let mut app = super::super::inspection_ui::tests::app()?;
        app.init_resource::<ConstructionTool>();
        app.update();
        let province = {
            let game = app.world().resource::<Game>();
            game.simulation.countries[game.session.player_country.unwrap()].provinces[0]
        };
        let mut game = app.world_mut().resource_mut::<Game>();
        game.hovered = Some(province);
        game.simulation.settlements[province].industries.clear();
        Ok(app)
    }
    fn action(app: &mut App, action: Action) {
        let button = app.world_mut().spawn((action, Interaction::Pressed)).id();
        app.update();
        app.world_mut().despawn(button);
    }
    fn paint(app: &mut App, keys: &[KeyCode]) {
        for key in keys {
            app.world_mut()
                .resource_mut::<ButtonInput<KeyCode>>()
                .press(*key);
        }
        app.world_mut()
            .resource_mut::<ButtonInput<MouseButton>>()
            .press(MouseButton::Left);
        app.update();
        app.world_mut()
            .resource_mut::<ButtonInput<KeyCode>>()
            .reset_all();
        app.world_mut()
            .resource_mut::<ButtonInput<MouseButton>>()
            .reset_all();
    }
    fn key(app: &mut App, code: KeyCode, text: Option<&str>) {
        app.world_mut()
            .resource_mut::<ButtonInput<KeyCode>>()
            .press(code);
        app.world_mut().write_message(KeyboardInput {
            key_code: code,
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
    #[test]
    fn search_filters_every_recipe_case_insensitively_selects_once_and_resets_on_open() -> Result<()>
    {
        let mut app = app()?;
        for index in 0..22 {
            action(&mut app, Action::Search);
            let name = app.world().resource::<Game>().simulation.catalog.recipes[index]
                .name
                .to_ascii_uppercase();
            key(&mut app, KeyCode::KeyC, Some(&name));
            let tool = app.world().resource::<ConstructionTool>();
            let game = app.world().resource::<Game>();
            assert!(tool.results(game).contains(&index));
            assert!(tool.results(game).iter().all(|&id| {
                game.simulation.catalog.recipes[id]
                    .name
                    .to_ascii_uppercase()
                    .contains(&name)
            }));
            action(&mut app, Action::SelectRecipe(usize::MAX));
            assert!(app.world().resource::<ConstructionTool>().searching);
            action(&mut app, Action::SelectRecipe(index));
            assert_eq!(app.world().resource::<ConstructionTool>().recipe, index);
            assert!(!app.world().resource::<ConstructionTool>().searching);
            assert!(app.world().resource::<ConstructionTool>().capture_input);
            app.update();
            assert!(!app.world().resource::<ConstructionTool>().capture_input);
            assert_eq!(app.world().resource::<Game>().simulation.date.tick(), 0);
        }
        action(&mut app, Action::Search);
        assert!(app.world().resource::<ConstructionTool>().query.is_empty());
        key(&mut app, KeyCode::KeyZ, Some("no recipe like this"));
        assert!(
            app.world()
                .resource::<ConstructionTool>()
                .highlighted
                .is_none()
        );
        key(&mut app, KeyCode::Enter, None);
        assert!(app.world().resource::<ConstructionTool>().searching);
        assert_eq!(app.world().resource::<ConstructionTool>().recipe, 21);
        action(&mut app, Action::CloseSearch);
        action(&mut app, Action::Search);
        key(&mut app, KeyCode::ArrowDown, None);
        assert_eq!(
            app.world().resource::<ConstructionTool>().highlighted,
            Some(0)
        );
        key(&mut app, KeyCode::ArrowUp, None);
        assert_eq!(
            app.world().resource::<ConstructionTool>().highlighted,
            Some(21)
        );
        key(&mut app, KeyCode::Enter, None);
        assert!(!app.world().resource::<ConstructionTool>().searching);
        Ok(())
    }
    #[test]
    fn search_edits_unicode_safely_and_captures_shortcuts_painting_and_close_frame() -> Result<()> {
        let mut app = app()?;
        action(&mut app, Action::Toggle);
        action(&mut app, Action::Search);
        for code in [
            KeyCode::KeyB,
            KeyCode::Space,
            KeyCode::KeyN,
            KeyCode::KeyI,
            KeyCode::KeyP,
        ] {
            key(&mut app, code, Some("é"));
            assert!(app.world().resource::<ConstructionTool>().armed);
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
            assert!(app.world().resource::<Game>().paused);
            assert_eq!(app.world().resource::<Game>().simulation.date.tick(), 0);
        }
        key(&mut app, KeyCode::Backspace, None);
        assert_eq!(app.world().resource::<ConstructionTool>().query, "éééé");
        app.world_mut()
            .resource_mut::<ButtonInput<KeyCode>>()
            .press(KeyCode::ControlLeft);
        key(&mut app, KeyCode::KeyA, Some("a"));
        key(&mut app, KeyCode::KeyC, Some("CoAl\n"));
        assert_eq!(app.world().resource::<ConstructionTool>().query, "CoAl");
        paint(&mut app, &[]);
        let province = app.world().resource::<Game>().hovered.unwrap();
        assert!(
            app.world().resource::<Game>().simulation.settlements[province]
                .industries
                .is_empty()
        );
        key(&mut app, KeyCode::Escape, None);
        assert!(!app.world().resource::<ConstructionTool>().searching);
        assert!(app.world().resource::<ConstructionTool>().armed);
        assert!(app.world().resource::<ConstructionTool>().capture_input);
        assert!(
            app.world()
                .resource::<Game>()
                .session
                .player_country
                .is_some()
        );
        app.update();
        assert!(!app.world().resource::<ConstructionTool>().capture_input);
        key(&mut app, KeyCode::Escape, None);
        assert!(!app.world().resource::<ConstructionTool>().armed);
        action(&mut app, Action::Search);
        key(&mut app, KeyCode::KeyC, Some(&"é".repeat(100)));
        assert_eq!(app.world().resource::<ConstructionTool>().query.len(), 126);
        key(&mut app, KeyCode::KeyC, Some("ab"));
        assert_eq!(app.world().resource::<ConstructionTool>().query.len(), 127);
        key(&mut app, KeyCode::Delete, None);
        assert_eq!(app.world().resource::<ConstructionTool>().query.len(), 126);
        Ok(())
    }
    #[test]
    fn search_visibility_filter_and_scrolling_reset_without_mutating_simulation() -> Result<()> {
        let mut app = app()?;
        let panel = app.world_mut().spawn((Panel::Search, Node::default())).id();
        let list = app
            .world_mut()
            .spawn((SearchResults, ScrollPosition::default()))
            .id();
        let choices: Vec<_> = (0..22)
            .map(|id| {
                app.world_mut()
                    .spawn((
                        Action::SelectRecipe(id),
                        Node::default(),
                        BackgroundColor::default(),
                    ))
                    .id()
            })
            .collect();
        action(&mut app, Action::Search);
        assert_eq!(
            app.world().get::<Node>(panel).unwrap().display,
            Display::Flex
        );
        app.world_mut()
            .resource_mut::<AccumulatedMouseScroll>()
            .delta = Vec2::new(0.0, -3.0);
        app.update();
        assert_eq!(app.world().get::<ScrollPosition>(list).unwrap().y, 96.0);
        app.world_mut()
            .resource_mut::<AccumulatedMouseScroll>()
            .delta = Vec2::ZERO;
        key(&mut app, KeyCode::KeyC, Some("coal"));
        assert_eq!(app.world().get::<ScrollPosition>(list).unwrap().y, 0.0);
        for (id, entity) in choices.iter().enumerate() {
            let matches = app
                .world()
                .resource::<ConstructionTool>()
                .matches(app.world().resource::<Game>(), id);
            assert_eq!(
                app.world().get::<Node>(*entity).unwrap().display,
                if matches {
                    Display::Flex
                } else {
                    Display::None
                }
            );
        }
        app.world_mut().resource_mut::<Game>().session = Session::default();
        app.update();
        assert!(!app.world().resource::<ConstructionTool>().searching);
        assert_eq!(
            app.world().get::<Node>(panel).unwrap().display,
            Display::None
        );
        assert_eq!(app.world().resource::<Game>().simulation.date.tick(), 0);
        Ok(())
    }
    #[test]
    fn painting_modifiers_recipe_cycles_one_shot_and_cancel_preserve_the_source_rules() -> Result<()>
    {
        let mut app = app()?;
        for recipe in 0..22 {
            let tool = app.world().resource::<ConstructionTool>();
            assert_eq!(tool.recipe, recipe);
            assert!(
                tool.content(app.world().resource::<Game>())
                    .contains(&format!("recipe {}/22", recipe + 1))
            );
            action(&mut app, Action::Next);
        }
        action(&mut app, Action::Previous);
        assert_eq!(app.world().resource::<ConstructionTool>().recipe, 21);
        action(&mut app, Action::Next);
        let toggle = app
            .world_mut()
            .spawn((Action::Toggle, Interaction::Pressed))
            .id();
        app.update();
        app.update();
        assert!(app.world().resource::<ConstructionTool>().armed);
        app.world_mut().despawn(toggle);
        let province = app.world().resource::<Game>().hovered.unwrap();
        for (keys, size) in [
            (vec![], 1.0),
            (vec![KeyCode::ShiftLeft], 6.0),
            (vec![KeyCode::ControlLeft], 16.0),
            (vec![KeyCode::ShiftLeft, KeyCode::ControlLeft], 21.0),
            (vec![KeyCode::ShiftRight], 22.0),
            (vec![KeyCode::ControlRight], 23.0),
        ] {
            paint(&mut app, &keys);
            let game = app.world().resource::<Game>();
            let industries = &game.simulation.settlements[province].industries;
            assert_eq!(industries.len(), 1);
            assert_eq!((industries[0].size, industries[0].utilization), (size, 1.0));
            assert!(industries[0].construction.is_none());
            assert_eq!(game.simulation.date.tick(), 0);
        }
        action(&mut app, Action::Next);
        paint(&mut app, &[]);
        assert_eq!(
            app.world().resource::<Game>().simulation.settlements[province].industries[0].recipe,
            RecipeId(0)
        );
        app.world_mut()
            .resource_mut::<ButtonInput<KeyCode>>()
            .press(KeyCode::Escape);
        app.update();
        assert!(!app.world().resource::<ConstructionTool>().armed);
        assert!(app.world().resource::<ConstructionTool>().capture_input);
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
        app.update();
        assert!(!app.world().resource::<ConstructionTool>().capture_input);
        paint(&mut app, &[]);
        assert_eq!(
            app.world().resource::<Game>().simulation.settlements[province].industries[0].size,
            24.0
        );
        Ok(())
    }
    #[test]
    fn ui_hover_overlays_foreign_provinces_and_system_view_cannot_paint() -> Result<()> {
        let mut app = app()?;
        let province = app.world().resource::<Game>().hovered.unwrap();
        action(&mut app, Action::Toggle);
        let hovered_button = app.world_mut().spawn(Interaction::Hovered).id();
        paint(&mut app, &[]);
        assert!(
            app.world().resource::<Game>().simulation.settlements[province]
                .industries
                .is_empty()
        );
        app.world_mut().despawn(hovered_button);
        app.world_mut()
            .resource_mut::<super::super::inspection_ui::Inspector>()
            .capture_input = true;
        app.world_mut()
            .resource_mut::<super::super::inspection_ui::Inspector>()
            .open = true;
        paint(&mut app, &[]);
        assert!(
            app.world().resource::<Game>().simulation.settlements[province]
                .industries
                .is_empty()
        );
        app.world_mut()
            .resource_mut::<super::super::inspection_ui::Inspector>()
            .open = false;
        app.world_mut()
            .resource_mut::<super::super::inspection_ui::Inspector>()
            .capture_input = false;
        app.world_mut().resource_mut::<Navigation>().active = true;
        paint(&mut app, &[]);
        assert!(
            app.world().resource::<Game>().simulation.settlements[province]
                .industries
                .is_empty()
        );
        app.world_mut().resource_mut::<Navigation>().active = false;
        app.world_mut()
            .resource_mut::<Game>()
            .simulation
            .settlements[province]
            .country = None;
        paint(&mut app, &[]);
        assert!(
            app.world().resource::<Game>().simulation.settlements[province]
                .industries
                .is_empty()
        );
        assert!(
            app.world()
                .resource::<ConstructionTool>()
                .message
                .contains("owned")
        );
        app.world_mut().resource_mut::<Game>().session = Session::default();
        app.update();
        assert!(!app.world().resource::<ConstructionTool>().armed);
        Ok(())
    }
}
