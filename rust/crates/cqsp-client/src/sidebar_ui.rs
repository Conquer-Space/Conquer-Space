//! Native routes for the original RmlUi sidebar and its C++ SelectedMenu context.
use super::{
    Game,
    inspection_ui::{Action as Inspect, Inspector, Tab},
    spaceport_ui, ui_button, ui_font,
};
use bevy::prelude::*;

#[derive(Component, Clone, Copy, Debug, Default, PartialEq, Eq)]
pub(super) enum Selection {
    #[default]
    None,
    Economy,
    Buildings,
    Budget,
    Space,
    Projects,
    Situations,
    Exploration,
}
#[derive(Resource, Default)]
pub(super) struct Sidebar {
    selected: Selection,
}
#[derive(Component)]
pub(super) struct Panel;

impl Sidebar {
    pub fn select(
        &mut self,
        selection: Selection,
        game: &mut Game,
        inspector: &mut Inspector,
        ports: &mut spaceport_ui::SpaceportUi,
    ) {
        // The source has a Situations button but no listener branch for it.
        if selection == Selection::Situations {
            return;
        }
        self.selected = selection;
        inspector.capture_input = true;
        ports.capture_input = true;
        inspector.apply(Inspect::Close, game);
        ports
            .apply(spaceport_ui::Action::Close, game)
            .expect("Close space window");
        let tab = match selection {
            Selection::Economy | Selection::Budget => Some(Tab::Budget),
            Selection::Buildings => Some(Tab::Production),
            Selection::Exploration => Some(Tab::Exploration),
            Selection::Space => {
                if game
                    .session
                    .player_country
                    .is_some_and(|country| game.simulation.countries[country].space_program)
                {
                    ports
                        .apply(spaceport_ui::Action::Open, game)
                        .expect("Open space window");
                }
                None
            }
            _ => None,
        };
        if let Some(tab) = tab {
            inspector.apply(Inspect::Tab(tab), game);
            inspector.apply(Inspect::Open, game);
        }
    }
}

pub(super) fn spawn(commands: &mut Commands, camera: Entity, font: &Handle<Font>) {
    commands
        .spawn((
            Panel,
            Interaction::None,
            UiTargetCamera(camera),
            Node {
                position_type: PositionType::Absolute,
                top: px(164),
                left: px(336),
                right: px(336),
                flex_wrap: FlexWrap::Wrap,
                column_gap: px(6),
                row_gap: px(6),
                padding: UiRect::all(px(8)),
                display: Display::None,
                ..default()
            },
            BackgroundColor(Color::srgb(0.025, 0.05, 0.09)),
        ))
        .with_children(|panel| {
            for (action, name) in [
                (Selection::Economy, "Economy"),
                (Selection::Buildings, "Buildings"),
                (Selection::Budget, "Budget"),
                (Selection::Space, "Space Access"),
                (Selection::Projects, "Projects"),
                (Selection::Situations, "Situations"),
                (Selection::Exploration, "Exploration"),
            ] {
                ui_button(panel, font, action, name);
            }
            panel.spawn((Text::new("Native sidebar"), ui_font(font, 12.0)));
        });
}

pub(super) fn controls(
    sidebar: Option<ResMut<Sidebar>>,
    mut game: ResMut<Game>,
    mut inspector: ResMut<Inspector>,
    mut ports: ResMut<spaceport_ui::SpaceportUi>,
    buttons: Query<(&Selection, &Interaction), Changed<Interaction>>,
) {
    let Some(mut sidebar) = sidebar else {
        return;
    };
    if game.session.player_country.is_none() || inspector.open || ports.open {
        return;
    }
    for (action, interaction) in &buttons {
        if *interaction == Interaction::Pressed {
            sidebar.select(*action, &mut game, &mut inspector, &mut ports);
        }
    }
}

#[allow(clippy::type_complexity)]
pub(super) fn pointer_available(
    hover: Query<&Interaction, Or<(With<Panel>, With<Selection>)>>,
) -> bool {
    hover
        .iter()
        .all(|interaction| *interaction == Interaction::None)
}

#[allow(clippy::too_many_arguments)]
pub(super) fn update(
    sidebar: Option<ResMut<Sidebar>>,
    game: Res<Game>,
    inspector: Res<Inspector>,
    ports: Res<spaceport_ui::SpaceportUi>,
    menu: Option<Res<super::application_ui::Menu>>,
    saves: Option<Res<super::save_ui::Saves>>,
    search: Option<Res<super::search_ui::Search>>,
    navigation: Option<Res<super::navigation_ui::Menu>>,
    construction: Option<Res<super::construction_ui::ConstructionTool>>,
    mut panels: Query<&mut Node, With<Panel>>,
) {
    let Some(mut sidebar) = sidebar else {
        return;
    };
    let closed = match sidebar.selected {
        Selection::Buildings => !inspector.is_production(),
        Selection::Economy | Selection::Budget => !inspector.is_budget(),
        Selection::Space => {
            game.session
                .player_country
                .is_some_and(|country| game.simulation.countries[country].space_program)
                && !ports.open
        }
        // Closing the exploration document does not clear selected_menu in Lua.
        _ => false,
    };
    if (game.session.player_country.is_none() || closed) && sidebar.selected != Selection::None {
        sidebar.selected = Selection::None;
    }
    let shown = game.session.player_country.is_some()
        && !inspector.open
        && !ports.open
        && super::event_ui::pending(&game).is_none()
        && menu.as_ref().is_none_or(|menu| !menu.is_open())
        && saves.as_ref().is_none_or(|saves| !saves.open)
        && search.as_ref().is_none_or(|search| !search.open)
        && navigation.as_ref().is_none_or(|menu| !menu.open);
    let shown = shown && construction.as_ref().is_none_or(|tool| !tool.searching);
    for mut node in &mut panels {
        let display = if shown { Display::Flex } else { Display::None };
        if node.display != display {
            node.display = display;
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn sidebar_clicks_capture_input_once_leave_time_running_and_reset_after_selection_ends() {
        let mut app = super::super::inspection_ui::tests::app().unwrap();
        app.init_resource::<Sidebar>();
        app.init_resource::<super::super::construction_ui::ConstructionTool>();
        let panel = app.world_mut().spawn((Panel, Node::default())).id();
        let button = app
            .world_mut()
            .spawn((Selection::Exploration, Interaction::Pressed))
            .id();
        app.update();
        assert!(app.world().resource::<Inspector>().is_exploration());
        assert_eq!(
            app.world().get::<Node>(panel).unwrap().display,
            Display::None
        );
        for key in [
            KeyCode::Space,
            KeyCode::KeyN,
            KeyCode::Enter,
            KeyCode::Minus,
        ] {
            app.world_mut()
                .resource_mut::<ButtonInput<KeyCode>>()
                .press(key);
        }
        app.update();
        assert!(app.world().resource::<Game>().paused);
        assert_eq!(app.world().resource::<Game>().simulation.date.tick(), 0);
        app.world_mut()
            .resource_mut::<ButtonInput<KeyCode>>()
            .reset_all();
        app.world_mut().resource_mut::<Game>().paused = false;
        app.world_mut()
            .resource_mut::<Time>()
            .advance_by(std::time::Duration::from_secs(2));
        app.update();
        assert!(app.world().resource::<Game>().simulation.date.tick() > 0);
        super::super::inspection_ui::tests::apply(&mut app, Inspect::Close);
        app.update();
        assert!(
            !app.world().resource::<Inspector>().open,
            "Held button must not reopen"
        );
        assert_eq!(
            app.world().get::<Node>(panel).unwrap().display,
            Display::Flex
        );
        app.world_mut()
            .resource_mut::<super::super::construction_ui::ConstructionTool>()
            .searching = true;
        app.update();
        assert_eq!(
            app.world().get::<Node>(panel).unwrap().display,
            Display::None,
            "Recipe search must not obscure laid-out sidebar text"
        );
        app.world_mut()
            .resource_mut::<super::super::construction_ui::ConstructionTool>()
            .searching = false;
        app.update();
        assert_eq!(
            app.world().get::<Node>(panel).unwrap().display,
            Display::Flex
        );
        app.world_mut().despawn(button);
        app.world_mut()
            .resource_mut::<Game>()
            .session
            .player_country = None;
        app.update();
        assert_eq!(app.world().resource::<Sidebar>().selected, Selection::None);
        assert_eq!(
            app.world().get::<Node>(panel).unwrap().display,
            Display::None
        );
    }
    #[test]
    fn routes_match_source_capability_checks_and_inert_buttons_and_close_semantics() {
        let mut app = super::super::inspection_ui::tests::app().unwrap();
        app.init_resource::<Sidebar>();
        fn select(app: &mut App, action: Selection) {
            app.world_mut()
                .resource_scope(|world, mut sidebar: Mut<Sidebar>| {
                    world.resource_scope(|world, mut inspector: Mut<Inspector>| {
                        world.resource_scope(|world, mut ports: Mut<spaceport_ui::SpaceportUi>| {
                            sidebar.select(
                                action,
                                &mut world.resource_mut::<Game>(),
                                &mut inspector,
                                &mut ports,
                            );
                        });
                    });
                });
        }
        select(&mut app, Selection::Buildings);
        assert!(app.world().resource::<Inspector>().is_production());
        assert!(app.world().resource::<Inspector>().capture_input);
        select(&mut app, Selection::Situations);
        assert_eq!(
            app.world().resource::<Sidebar>().selected,
            Selection::Buildings
        );
        assert!(app.world().resource::<Inspector>().is_production());
        select(&mut app, Selection::Projects);
        assert!(!app.world().resource::<Inspector>().open);
        assert!(!app.world().resource::<spaceport_ui::SpaceportUi>().open);
        app.update();
        assert_eq!(
            app.world().resource::<Sidebar>().selected,
            Selection::Projects
        );
        select(&mut app, Selection::Space);
        assert!(app.world().resource::<spaceport_ui::SpaceportUi>().open);
        let foreign = app
            .world()
            .resource::<Game>()
            .simulation
            .countries
            .iter()
            .position(|country| !country.space_program)
            .unwrap();
        app.world_mut()
            .resource_mut::<Game>()
            .session
            .player_country = Some(foreign);
        select(&mut app, Selection::Space);
        assert!(!app.world().resource::<spaceport_ui::SpaceportUi>().open);
        select(&mut app, Selection::Exploration);
        assert!(app.world().resource::<Inspector>().is_exploration());
        super::super::inspection_ui::tests::apply(&mut app, Inspect::Close);
        app.update();
        assert_eq!(
            app.world().resource::<Sidebar>().selected,
            Selection::Exploration
        );
        select(&mut app, Selection::Economy);
        assert!(app.world().resource::<Inspector>().is_budget());
        super::super::inspection_ui::tests::apply(&mut app, Inspect::Close);
        app.update();
        assert_eq!(app.world().resource::<Sidebar>().selected, Selection::None);
    }
}
