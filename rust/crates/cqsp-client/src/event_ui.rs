//! Source-mapped event modal. Queues are native; the original disabled tutorial stays disabled.
use super::{Game, ui_button, ui_font};
use anyhow::{Context, Result, ensure};
use bevy::{input::mouse::AccumulatedMouseScroll, prelude::*};
use cqsp_core::events::Notification;
use std::path::{Component as PathComponent, Path};

#[derive(Resource, Default)]
pub(super) struct Events {
    pub capture_input: bool,
    displayed: Option<Notification>,
    message: String,
}
pub(super) fn pending(game: &Game) -> Option<&Notification> {
    game.session
        .player_country
        .and_then(|country| game.simulation.countries.get(country))?
        .events
        .front()
}
pub(super) fn input_available(game: Res<Game>, events: Option<Res<Events>>) -> bool {
    pending(&game).is_none() && events.is_none_or(|events| !events.capture_input)
}
#[derive(Component)]
pub(super) struct Action {
    expected: Notification,
    choice: Option<usize>,
}
impl Events {
    pub fn choose(&mut self, game: &mut Game, choice: Option<usize>) -> Result<()> {
        let country = game
            .session
            .player_country
            .context("Start a country first")?;
        game.simulation.resolve_event(country, choice)?;
        self.capture_input = true;
        self.message.clear();
        Ok(())
    }
}
pub(super) fn image(data: &Path) -> Result<Image> {
    let directory = data.join("gfx/event");
    let manifest: serde_json::Value =
        serde_hjson::from_str(&std::fs::read_to_string(directory.join("resource.hjson"))?)?;
    let resource = &manifest["rocket-event"];
    ensure!(
        resource["type"] == "texture",
        "Unknown rocket-event texture"
    );
    let path = resource["path"]
        .as_str()
        .context("Event texture has no path")?;
    ensure!(
        Path::new(path)
            .components()
            .all(|component| matches!(component, PathComponent::Normal(_))),
        "Unsafe event texture path"
    );
    let image = image::open(directory.join(path))
        .context("Loading original rocket event image")?
        .to_rgba8();
    Ok(super::rgba_image(
        image.width(),
        image.height(),
        image.into_raw(),
    ))
}
#[derive(Component)]
pub(super) struct Panel;
#[derive(Component)]
pub(super) struct Choices(Handle<Font>);
#[derive(Component)]
pub(super) struct EventImage;
#[derive(Component)]
pub(super) struct Body;
#[derive(Component)]
pub(super) enum Label {
    Heading,
    Title,
    Content,
    Tooltip,
    Status,
    MissingImage,
}
pub(super) fn spawn(
    commands: &mut Commands,
    camera: Entity,
    font: &Handle<Font>,
    image: &Handle<Image>,
    aspect: f32,
) {
    commands
        .spawn((
            Panel,
            UiTargetCamera(camera),
            Interaction::None,
            GlobalZIndex(40),
            Node {
                display: Display::None,
                position_type: PositionType::Absolute,
                left: percent(50),
                width: px(498),
                margin: UiRect::left(px(-249)),
                top: px(40),
                bottom: px(40),
                padding: UiRect::all(px(24)),
                flex_direction: FlexDirection::Column,
                row_gap: px(8),
                overflow: Overflow::scroll_y(),
                ..default()
            },
            ScrollPosition::default(),
            BackgroundColor(Color::srgb(0.025, 0.05, 0.09)),
        ))
        .with_children(|panel| {
            panel.spawn((
                Label::Heading,
                Text::new("EVENT"),
                ui_font(font, 18.0),
                TextColor(Color::srgb(0.9, 0.76, 0.42)),
                Node {
                    flex_shrink: 0.0,
                    ..default()
                },
            ));
            panel.spawn((
                Label::Title,
                Text::default(),
                ui_font(font, 20.0),
                Node {
                    flex_shrink: 0.0,
                    ..default()
                },
            ));
            panel.spawn((
                EventImage,
                ImageNode::new(image.clone()),
                Node {
                    width: px(450),
                    height: px(450.0 * aspect),
                    flex_shrink: 0.0,
                    ..default()
                },
            ));
            panel.spawn((
                Label::MissingImage,
                Text::default(),
                ui_font(font, 14.0),
                Node {
                    display: Display::None,
                    flex_shrink: 0.0,
                    ..default()
                },
            ));
            panel
                .spawn((
                    Body,
                    Interaction::None,
                    Node {
                        height: px(150),
                        min_height: px(150),
                        overflow: Overflow::scroll_y(),
                        flex_shrink: 0.0,
                        ..default()
                    },
                    ScrollPosition::default(),
                ))
                .with_children(|body| {
                    body.spawn((Label::Content, Text::default(), ui_font(font, 14.0)));
                });
            panel.spawn((
                Choices(font.clone()),
                Node {
                    flex_direction: FlexDirection::Column,
                    row_gap: px(6),
                    flex_shrink: 0.0,
                    ..default()
                },
            ));
            for label in [Label::Tooltip, Label::Status] {
                panel.spawn((
                    label,
                    Text::default(),
                    ui_font(font, 14.0),
                    Node {
                        flex_shrink: 0.0,
                        ..default()
                    },
                ));
            }
        });
}
pub(super) fn controls(
    events: Option<ResMut<Events>>,
    mut game: ResMut<Game>,
    buttons: Query<(&Action, &Interaction), Changed<Interaction>>,
) {
    let Some(mut events) = events else {
        return;
    };
    let event = pending(&game).cloned();
    let capture = event.is_some();
    if events.capture_input != capture {
        events.capture_input = capture;
    }
    let Some(event) = event else {
        return;
    };
    for (action, interaction) in &buttons {
        if *interaction == Interaction::Pressed && action.expected == event {
            if let Err(error) = events.choose(&mut game, action.choice) {
                events.message = error.to_string();
            }
            break; // Clearing the whole queue cannot execute a second stale choice this frame.
        }
    }
}
#[allow(clippy::type_complexity)]
pub(super) fn scroll(
    game: Res<Game>,
    mouse: Res<AccumulatedMouseScroll>,
    mut panels: Query<&mut ScrollPosition, (With<Panel>, Without<Body>)>,
    mut bodies: Query<(&Interaction, &mut ScrollPosition), (With<Body>, Without<Panel>)>,
) {
    if pending(&game).is_none() || mouse.delta == Vec2::ZERO {
        return;
    }
    let delta = mouse.delta.y
        * if mouse.unit == bevy::input::mouse::MouseScrollUnit::Line {
            32.0
        } else {
            1.0
        };
    if bodies.iter().any(|(hover, _)| *hover != Interaction::None) {
        for (hover, mut position) in &mut bodies {
            if *hover != Interaction::None {
                position.y -= delta;
            }
        }
    } else {
        for mut position in &mut panels {
            position.y -= delta;
        }
    }
}
#[allow(clippy::too_many_arguments, clippy::type_complexity)]
pub(super) fn update(
    mut commands: Commands,
    events: Option<ResMut<Events>>,
    game: Res<Game>,
    mut panels: Query<(&mut Node, &mut ScrollPosition), (With<Panel>, Without<Body>)>,
    mut bodies: Query<&mut ScrollPosition, (With<Body>, Without<Panel>)>,
    mut pictures: Query<&mut Node, (With<EventImage>, Without<Panel>)>,
    mut labels: Query<(&Label, &mut Text, &mut Node), (Without<Panel>, Without<EventImage>)>,
    choices: Query<(Entity, &Choices)>,
    buttons: Query<(&Action, &Interaction)>,
) {
    let Some(mut events) = events else {
        return;
    };
    let event = pending(&game);
    let changed = events.displayed.as_ref() != event;
    if changed {
        for mut position in &mut bodies {
            position.y = 0.0;
        }
    }
    for (mut node, mut scroll) in &mut panels {
        let display = if event.is_some() {
            Display::Flex
        } else {
            Display::None
        };
        if node.display != display {
            node.display = display;
        }
        if changed {
            scroll.y = 0.0;
        }
    }
    let image_available = event.is_some_and(|event| event.image == "core:rocket-event");
    for mut node in &mut pictures {
        let display = if image_available {
            Display::Flex
        } else {
            Display::None
        };
        if node.display != display {
            node.display = display;
        }
    }
    let tooltip = buttons
        .iter()
        .find_map(|(action, interaction)| {
            (*interaction == Interaction::Hovered && event == Some(&action.expected))
                .then(|| {
                    action
                        .choice
                        .and_then(|index| action.expected.choices.get(index))
                        .map(|choice| choice.tooltip.as_str())
                })
                .flatten()
        })
        .unwrap_or_default();
    for (label, mut text, mut node) in &mut labels {
        let value = match label {
            Label::Heading => "EVENT",
            Label::Title => event.map_or("", |event| &event.title),
            Label::Content => event.map_or("", |event| &event.content),
            Label::Tooltip => tooltip,
            Label::Status => &events.message,
            Label::MissingImage => {
                if image_available {
                    ""
                } else {
                    "Event image unavailable (safe native fallback)."
                }
            }
        };
        if text.0 != value {
            text.0 = value.into();
        } else if event.is_some() {
            // Changing image/content layout can otherwise leave sibling glyph caches stale.
            text.set_changed();
        }
        if matches!(label, Label::Tooltip | Label::Status | Label::MissingImage) {
            let display = if value.is_empty() {
                Display::None
            } else {
                Display::Flex
            };
            if node.display != display {
                node.display = display;
            }
        }
    }
    if changed {
        events.message.clear();
        for (entity, font) in &choices {
            commands.entity(entity).despawn_related::<Children>();
            if let Some(event) = event {
                commands.entity(entity).with_children(|list| {
                    if event.choices.is_empty() {
                        ui_button(
                            list,
                            &font.0,
                            Action {
                                expected: event.clone(),
                                choice: None,
                            },
                            "Ok",
                        );
                    } else {
                        for (index, choice) in event.choices.iter().enumerate() {
                            ui_button(
                                list,
                                &font.0,
                                Action {
                                    expected: event.clone(),
                                    choice: Some(index),
                                },
                                &choice.name,
                            );
                        }
                    }
                });
            }
        }
        events.displayed = event.cloned();
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use cqsp_core::{clock::StarDate, events::Choice};
    fn app() -> Result<App> {
        let mut app = super::super::inspection_ui::tests::app()?;
        app.init_resource::<Events>()
            .add_systems(Startup, |mut commands: Commands| {
                spawn(
                    &mut commands,
                    Entity::PLACEHOLDER,
                    &Handle::default(),
                    &Handle::default(),
                    420.0 / 640.0,
                );
            });
        app.update();
        Ok(app)
    }
    fn push(app: &mut App, notification: Notification) -> Result<()> {
        let mut game = app.world_mut().resource_mut::<Game>();
        let country = game.session.player_country.unwrap();
        game.simulation.push_event(country, notification)
    }
    fn label(app: &mut App, which: fn(&Label) -> bool) -> String {
        let mut labels = app.world_mut().query::<(&Label, &Text)>();
        labels
            .iter(app.world())
            .find(|(label, _)| which(label))
            .unwrap()
            .1
            .0
            .clone()
    }
    #[test]
    fn events_halt_time_capture_all_shortcuts_and_clear_the_queue_once_without_changing_playback()
    -> Result<()> {
        let mut app = app()?;
        push(&mut app, Notification::tutorial(0).unwrap())?;
        push(&mut app, Notification::tutorial(1).unwrap())?;
        app.world_mut().resource_mut::<Game>().paused = false;
        app.world_mut()
            .resource_mut::<Time>()
            .advance_by(std::time::Duration::from_secs(10));
        for key in [
            KeyCode::Escape,
            KeyCode::Space,
            KeyCode::KeyN,
            KeyCode::KeyI,
            KeyCode::KeyP,
            KeyCode::KeyB,
            KeyCode::Enter,
            KeyCode::Equal,
        ] {
            app.world_mut()
                .resource_mut::<ButtonInput<KeyCode>>()
                .press(key);
        }
        app.update();
        assert!(app.world().resource::<Events>().capture_input);
        assert!(!app.world().resource::<Game>().paused);
        assert_eq!(
            app.world().resource::<Game>().simulation.date,
            StarDate::from_tick(0)
        );
        assert!(
            app.world()
                .resource::<Game>()
                .session
                .player_country
                .is_some()
        );
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
        assert_eq!(
            label(&mut app, |label| matches!(label, Label::Title)),
            "Our Future is in the Stars"
        );
        let button = app
            .world_mut()
            .query_filtered::<Entity, With<Action>>()
            .single(app.world())?;
        *app.world_mut().get_mut::<Interaction>(button).unwrap() = Interaction::Hovered;
        app.update();
        assert!(
            label(&mut app, |label| matches!(label, Label::Tooltip)).contains("Conquer The Stars")
        );
        *app.world_mut().get_mut::<Interaction>(button).unwrap() = Interaction::Pressed;
        app.update();
        assert_eq!(app.world().resource::<Game>().simulation.tutorial_stage, 1);
        assert!(pending(app.world().resource::<Game>()).is_none());
        assert_eq!(app.world().resource::<Game>().simulation.date.tick(), 0);
        assert!(app.world().resource::<Events>().capture_input);
        app.world_mut()
            .resource_mut::<ButtonInput<KeyCode>>()
            .reset_all();
        app.update();
        assert!(!app.world().resource::<Events>().capture_input);
        assert_eq!(app.world().resource::<Game>().simulation.date.tick(), 1);
        Ok(())
    }
    #[test]
    fn notices_without_actions_unknown_images_and_stale_buttons_fail_safely() -> Result<()> {
        let mut app = app()?;
        let notice = Notification {
            title: "Native notice".into(),
            content: "No scripted effects".into(),
            image: "missing".into(),
            choices: vec![],
        };
        push(&mut app, notice.clone())?;
        app.update();
        assert!(
            label(&mut app, |label| matches!(label, Label::MissingImage)).contains("unavailable")
        );
        let picture = app
            .world_mut()
            .query_filtered::<&Node, With<EventImage>>()
            .single(app.world())?;
        assert_eq!(picture.display, Display::None);
        let old = app
            .world_mut()
            .spawn((
                Action {
                    expected: Notification::tutorial(0).unwrap(),
                    choice: Some(0),
                },
                Interaction::Pressed,
            ))
            .id();
        app.update();
        assert_eq!(pending(app.world().resource::<Game>()), Some(&notice));
        app.world_mut().despawn(old);
        let action = app
            .world_mut()
            .query_filtered::<Entity, With<Action>>()
            .single(app.world())?;
        assert!(app.world().get::<Action>(action).unwrap().choice.is_none());
        *app.world_mut().get_mut::<Interaction>(action).unwrap() = Interaction::Pressed;
        app.update();
        assert!(pending(app.world().resource::<Game>()).is_none());
        assert_eq!(app.world().resource::<Game>().simulation.tutorial_stage, 0);
        let mut event = notice;
        event.choices = vec![Choice {
            name: "No effect".into(),
            tooltip: String::new(),
            effect: None,
        }];
        push(&mut app, event.clone())?;
        app.update();
        let invalid = app
            .world_mut()
            .spawn((
                Action {
                    expected: event.clone(),
                    choice: Some(99),
                },
                Interaction::Pressed,
            ))
            .id();
        app.update();
        assert_eq!(pending(app.world().resource::<Game>()), Some(&event));
        assert!(
            app.world()
                .resource::<Events>()
                .message
                .contains("Unknown event action")
        );
        app.world_mut().despawn(invalid);
        app.world_mut()
            .resource_mut::<Game>()
            .session
            .return_to_selection();
        app.update();
        assert!(!app.world().resource::<Events>().capture_input);
        let panel = app
            .world_mut()
            .query_filtered::<&Node, With<Panel>>()
            .single(app.world())?;
        assert_eq!(panel.display, Display::None);
        Ok(())
    }
    #[test]
    fn event_scroll_routes_to_hovered_content_and_loaded_image_keeps_source_dimensions()
    -> Result<()> {
        let texture = image(&cqsp_core::default_data_path())?;
        assert_eq!((texture.width(), texture.height()), (640, 420));
        let mut app = app()?;
        push(&mut app, Notification::tutorial(0).unwrap())?;
        app.update();
        app.world_mut()
            .resource_mut::<AccumulatedMouseScroll>()
            .delta = Vec2::new(0.0, -2.0);
        app.update();
        let panel = app
            .world_mut()
            .query_filtered::<&ScrollPosition, With<Panel>>()
            .single(app.world())?;
        assert_eq!(panel.y, 64.0);
        let body = app
            .world_mut()
            .query_filtered::<Entity, With<Body>>()
            .single(app.world())?;
        *app.world_mut().get_mut::<Interaction>(body).unwrap() = Interaction::Hovered;
        app.update();
        assert_eq!(app.world().get::<ScrollPosition>(body).unwrap().y, 64.0);
        let panel = app
            .world_mut()
            .query_filtered::<&ScrollPosition, With<Panel>>()
            .single(app.world())?;
        assert_eq!(panel.y, 64.0);
        app.world_mut()
            .resource_mut::<AccumulatedMouseScroll>()
            .delta = Vec2::ZERO;
        app.world_mut()
            .resource_scope(|world, mut ui: Mut<Events>| {
                ui.choose(&mut world.resource_mut::<Game>(), Some(0))
                    .unwrap();
            });
        push(&mut app, Notification::tutorial(1).unwrap())?;
        app.update();
        let panel = app
            .world_mut()
            .query_filtered::<&ScrollPosition, With<Panel>>()
            .single(app.world())?;
        assert_eq!(panel.y, 0.0);
        assert_eq!(app.world().get::<ScrollPosition>(body).unwrap().y, 0.0);
        Ok(())
    }
}
