//! The original ConstructionMenu's owned-province painting tool, without Lua/ImGui.
use super::{Game, Navigation, ui_button, ui_font, ui_row};
use anyhow::Result;
use bevy::prelude::*;
use cqsp_core::catalog::RecipeId;

#[derive(Resource, Default)]
pub(super) struct ConstructionTool {
    pub armed: bool,
    pub capture_input: bool,
    recipe: usize,
    message: String,
}
impl ConstructionTool {
    pub fn apply(&mut self, action: Action, game: &Game) {
        let count = game.simulation.catalog.recipes.len();
        match action {
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
}
#[derive(Component)]
pub(super) struct Panel;
#[derive(Component)]
pub(super) struct Content;
pub(super) fn spawn(commands: &mut Commands, camera: Entity, font: &Handle<Font>) {
    commands
        .spawn((
            Panel,
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
}
pub(super) fn controls(
    tool: Option<ResMut<ConstructionTool>>,
    keyboard: Res<ButtonInput<KeyCode>>,
    mouse: Res<ButtonInput<MouseButton>>,
    actions: Query<(&Action, &Interaction), Changed<Interaction>>,
    ui_hover: Query<&Interaction>,
    navigation: Res<Navigation>,
    mut game: ResMut<Game>,
) {
    let Some(mut tool) = tool else {
        return;
    };
    if tool.capture_input {
        tool.capture_input = false;
    }
    if game.session.player_country.is_none() || navigation.active {
        return;
    }
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
#[allow(clippy::too_many_arguments)]
pub(super) fn update(
    tool: Option<ResMut<ConstructionTool>>,
    game: Res<Game>,
    navigation: Res<Navigation>,
    ports: Res<super::spaceport_ui::SpaceportUi>,
    inspector: Res<super::inspection_ui::Inspector>,
    saves: Option<Res<super::save_ui::Saves>>,
    mut panels: Query<(Entity, &mut Node), With<Panel>>,
    children: Query<&Children>,
    mut texts: Query<(&mut Text, Option<&Content>)>,
) {
    let Some(mut tool) = tool else {
        return;
    };
    if game.session.player_country.is_none() && (tool.armed || !tool.message.is_empty()) {
        tool.armed = false;
        tool.message.clear();
    }
    let shown = game.session.player_country.is_some()
        && !navigation.active
        && !ports.open
        && !inspector.open
        && saves.is_none_or(|saves| !saves.open);
    for (entity, mut node) in &mut panels {
        let display = if shown { Display::Flex } else { Display::None };
        if node.display != display {
            node.display = display;
        }
        if shown && (tool.is_changed() || game.is_changed()) {
            for child in children.iter_descendants(entity) {
                if let Ok((mut text, content)) = texts.get_mut(child) {
                    if content.is_some() {
                        text.0 = tool.content(&game);
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
