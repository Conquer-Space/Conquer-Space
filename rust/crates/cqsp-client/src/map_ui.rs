//! Native Earth overlay selection; informational modes are separate from styling.
use super::{Game, ui_button, ui_font, ui_row};
use anyhow::{Result, ensure};
use bevy::prelude::*;
use cqsp_core::{
    economy::GoodId,
    map::{Mode, price_scale},
};
const PAGE_SIZE: usize = 8;
#[derive(Resource, Default)]
pub(super) struct Menu {
    pub open: bool,
    pub capture_input: bool,
    page: usize,
}
#[derive(Component, Clone, Copy)]
pub(super) enum Action {
    Open,
    Mode(Mode),
    Good(GoodId),
    Page(bool),
    Close,
}
impl Menu {
    pub fn apply(&mut self, action: Action, game: &mut Game) -> Result<()> {
        if matches!(action, Action::Open) {
            ensure!(
                game.session.player_country.is_some(),
                "Start a country first"
            );
            self.open = true;
            self.capture_input = true;
            self.page = game.map_good.0 / PAGE_SIZE;
            return Ok(());
        }
        if matches!(action, Action::Close) {
            self.open = false;
            self.capture_input = true;
            return Ok(());
        }
        if !self.open {
            return Ok(());
        }
        match action {
            Action::Mode(mode) => game.map_mode = mode,
            Action::Good(good) => {
                ensure!(
                    good.0 < game.simulation.catalog.goods.len(),
                    "Unknown map good"
                );
                game.map_good = good;
            }
            Action::Page(previous) => {
                let pages = game
                    .simulation
                    .catalog
                    .goods
                    .len()
                    .div_ceil(PAGE_SIZE)
                    .max(1);
                self.page = if previous {
                    self.page.saturating_sub(1)
                } else {
                    (self.page + 1).min(pages - 1)
                };
            }
            Action::Open | Action::Close => unreachable!(),
        }
        Ok(())
    }
    pub fn visible_goods(&self, game: &Game) -> usize {
        game.simulation
            .catalog
            .goods
            .len()
            .saturating_sub(self.page * PAGE_SIZE)
            .min(PAGE_SIZE)
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
pub(super) struct Goods(Handle<Font>);
pub(super) fn spawn(commands: &mut Commands, camera: Entity, font: &Handle<Font>) {
    commands
        .spawn((
            Panel,
            ScrollPosition::default(),
            UiTargetCamera(camera),
            GlobalZIndex(28),
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
                Text::new("MAP OVERLAYS"),
                ui_font(font, 22.0),
                TextColor(Color::srgb(0.9, 0.76, 0.42)),
            ));
            ui_row(panel, |row| {
                for mode in Mode::ALL {
                    ui_button(row, font, Action::Mode(mode), mode.name());
                }
                ui_button(row, font, Action::Close, "Close (M / Esc)");
            });
            panel.spawn((Content, Text::default(), ui_font(font, 16.0)));
            ui_row(panel, |row| {
                ui_button(row, font, Action::Page(true), "Previous goods");
                ui_button(row, font, Action::Page(false), "Next goods");
            });
            panel.spawn((
                Goods(font.clone()),
                Node {
                    flex_direction: FlexDirection::Column,
                    row_gap: px(6),
                    flex_shrink: 0.0,
                    ..default()
                },
            ));
        });
}
pub(super) fn controls(
    menu: Option<ResMut<Menu>>,
    keyboard: Res<ButtonInput<KeyCode>>,
    buttons: Query<(&Action, &Interaction), Changed<Interaction>>,
    mut game: ResMut<Game>,
) {
    let Some(mut menu) = menu else { return };
    if menu.capture_input != menu.open {
        menu.capture_input = menu.open
    }
    if game.session.player_country.is_none() {
        menu.open = false;
        return;
    }
    if menu.open && (keyboard.just_pressed(KeyCode::Escape) || keyboard.just_pressed(KeyCode::KeyM))
    {
        menu.apply(Action::Close, &mut game).unwrap();
        return;
    }
    if keyboard.just_pressed(KeyCode::KeyM) {
        menu.apply(Action::Open, &mut game).unwrap();
    }
    for (action, interaction) in &buttons {
        if *interaction == Interaction::Pressed {
            let _ = menu.apply(*action, &mut game);
        }
    }
}
pub(super) fn earth_view(navigation: Res<super::Navigation>) -> bool {
    !navigation.active
}
pub(super) fn scroll(
    menu: Option<Res<Menu>>,
    mouse: Res<super::AccumulatedMouseScroll>,
    mut panels: Query<&mut ScrollPosition, With<Panel>>,
) {
    if menu.is_none_or(|menu| !menu.open) || mouse.delta.y == 0.0 {
        return;
    }
    let scale = if mouse.unit == bevy::input::mouse::MouseScrollUnit::Line {
        32.0
    } else {
        1.0
    };
    for mut scroll in &mut panels {
        scroll.y = (scroll.y - mouse.delta.y * scale).max(0.0);
    }
}
pub(super) fn description(game: &Game) -> String {
    if game.map_mode != Mode::Price {
        return format!("MAP · {}", game.map_mode.name());
    }
    let Some(good) = game.simulation.catalog.goods.get(game.map_good.0) else {
        return "MAP · invalid good".into();
    };
    let local = game.hovered.map_or(String::new(), |id| {
        price_scale(&game.simulation, id, game.map_good).map_or(
            "\nPrice scale unavailable (non-finite ratio).".into(),
            |scale| {
                format!(
                    "\n{} · Price: {:.2}\nPrice / configured maximum: {} / {} = {}",
                    game.simulation.settlements[id].name,
                    scale.price,
                    scale.price,
                    scale.maximum,
                    scale.ratio
                )
            },
        )
    });
    format!(
        "MAP · {}\n{} · red: low price / green: high price{}",
        game.map_mode.name(),
        good.name,
        local
    )
}
#[allow(clippy::type_complexity, clippy::too_many_arguments)]
pub(super) fn update(
    mut commands: Commands,
    menu: Option<Res<Menu>>,
    game: Res<Game>,
    mut panels: Query<&mut Node, With<Panel>>,
    mut content: Query<&mut Text, With<Content>>,
    goods: Query<(Entity, &Goods)>,
    children: Query<&Children>,
    mut previous: Local<Option<(usize, GoodId)>>,
) {
    let Some(menu) = menu else { return };
    let shown = menu.open && super::event_ui::pending(&game).is_none();
    for mut panel in &mut panels {
        panel.display = if shown { Display::Flex } else { Display::None };
    }
    for mut text in &mut content {
        text.0 = format!(
            "{}\nChoose a mode, then close to inspect the Earth map.\nSelected good is retained across modes; prices refresh each tick.\nResource/science overlays and other-body maps are not implemented yet.\n{} goods · page {} / {}",
            description(&game),
            game.simulation.catalog.goods.len(),
            menu.page + 1,
            game.simulation
                .catalog
                .goods
                .len()
                .div_ceil(PAGE_SIZE)
                .max(1)
        );
    }
    let state = (menu.page, game.map_good);
    if *previous != Some(state) {
        for (entity, font) in &goods {
            if children.get(entity).is_ok() {
                commands.entity(entity).despawn_related::<Children>();
            }
            commands.entity(entity).with_children(|list| {
                for (id, good) in game
                    .simulation
                    .catalog
                    .goods
                    .iter()
                    .enumerate()
                    .skip(menu.page * PAGE_SIZE)
                    .take(PAGE_SIZE)
                {
                    ui_button(
                        list,
                        &font.0,
                        Action::Good(GoodId(id)),
                        &format!(
                            "{} {}",
                            if id == game.map_good.0 {
                                "[selected]"
                            } else {
                                "·"
                            },
                            good.name
                        ),
                    );
                }
            });
        }
        *previous = Some(state);
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn all_goods_and_modes_are_reachable_without_economy_mutation_and_bad_ids_fail() -> Result<()> {
        let mut app = super::super::inspection_ui::tests::app()?;
        let mut menu = Menu {
            open: true,
            ..default()
        };
        let mut game = app.world_mut().resource_mut::<Game>();
        let date = game.simulation.date;
        let population = game.simulation.summary(None).population;
        for mode in Mode::ALL {
            menu.apply(Action::Mode(mode), &mut game)?;
            assert_eq!(game.map_mode, mode);
        }
        for index in 0..game.simulation.catalog.goods.len() {
            menu.apply(Action::Good(GoodId(index)), &mut game)?;
            assert_eq!(game.map_good, GoodId(index));
        }
        let selected = game.map_good;
        assert!(
            menu.apply(Action::Good(GoodId(usize::MAX)), &mut game)
                .is_err()
        );
        assert_eq!(game.map_good, selected);
        for _ in 0..100 {
            menu.apply(Action::Page(false), &mut game)?;
        }
        assert_eq!(
            menu.page,
            game.simulation.catalog.goods.len().div_ceil(PAGE_SIZE) - 1
        );
        menu.apply(Action::Mode(Mode::Province), &mut game)?;
        menu.apply(Action::Close, &mut game)?;
        assert!(menu.capture_input);
        assert_eq!(game.map_good, selected);
        assert_eq!(game.simulation.date, date);
        assert_eq!(game.simulation.summary(None).population, population);
        Ok(())
    }
    #[test]
    fn map_close_frame_captures_shortcuts_and_hover_shows_actual_selected_price() -> Result<()> {
        let mut app = super::super::inspection_ui::tests::app()?;
        app.init_resource::<Menu>();
        app.world_mut()
            .resource_mut::<ButtonInput<KeyCode>>()
            .press(KeyCode::KeyM);
        app.update();
        assert!(app.world().resource::<Menu>().open);
        assert!(app.world().resource::<Menu>().capture_input);
        app.world_mut()
            .resource_mut::<ButtonInput<KeyCode>>()
            .reset_all();
        {
            let mut game = app.world_mut().resource_mut::<Game>();
            game.map_mode = Mode::Price;
            game.map_good = game.simulation.catalog.good("steel").unwrap();
            game.hovered = Some(0);
            let good = game.map_good;
            game.simulation.settlements[0].market.price[good] = 12.345;
            assert!(description(&game).contains("Price: 12.35"));
        }
        app.world_mut()
            .resource_mut::<ButtonInput<KeyCode>>()
            .press(KeyCode::Escape);
        app.update();
        assert!(!app.world().resource::<Menu>().open);
        assert!(app.world().resource::<Menu>().capture_input);
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
        assert!(!app.world().resource::<Menu>().capture_input);
        Ok(())
    }
    #[test]
    fn map_overlay_blocks_game_shortcuts_but_not_running_time_and_events_take_priority()
    -> Result<()> {
        let mut app = super::super::inspection_ui::tests::app()?;
        app.init_resource::<Menu>();
        app.world_mut()
            .resource_scope(|world, mut menu: Mut<Menu>| {
                menu.apply(Action::Open, &mut world.resource_mut::<Game>())
            })?;
        for key in [
            KeyCode::Space,
            KeyCode::KeyN,
            KeyCode::KeyP,
            KeyCode::KeyI,
            KeyCode::KeyT,
            KeyCode::Tab,
        ] {
            app.world_mut()
                .resource_mut::<ButtonInput<KeyCode>>()
                .press(key);
            app.update();
            app.world_mut()
                .resource_mut::<ButtonInput<KeyCode>>()
                .reset_all();
        }
        assert!(app.world().resource::<Game>().paused);
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
        assert!(!app.world().resource::<super::super::Navigation>().active);
        app.world_mut().resource_mut::<Game>().paused = false;
        let tick = app.world().resource::<Game>().simulation.date.tick();
        app.world_mut()
            .resource_mut::<Time>()
            .advance_by(std::time::Duration::from_secs(2));
        app.update();
        assert!(app.world().resource::<Game>().simulation.date.tick() > tick);
        assert!(app.world().resource::<Menu>().open);
        let action = app
            .world_mut()
            .spawn((Action::Page(false), Interaction::Pressed))
            .id();
        app.update();
        app.update();
        assert_eq!(app.world().resource::<Menu>().page, 1);
        app.world_mut().despawn(action);
        {
            let mut game = app.world_mut().resource_mut::<Game>();
            let country = game.session.player_country.unwrap();
            game.simulation.countries[country]
                .events
                .push(cqsp_core::events::Notification::tutorial(0).unwrap());
        }
        let action = app
            .world_mut()
            .spawn((Action::Mode(Mode::Province), Interaction::Pressed))
            .id();
        app.update();
        assert_eq!(app.world().resource::<Game>().map_mode, Mode::Country);
        app.world_mut().despawn(action);
        Ok(())
    }
    #[test]
    fn price_image_refreshes_on_ticks_mode_changes_and_system_view_hides_the_globe() -> Result<()> {
        use super::super::{
            EarthGlobe, EarthMaterial, GlobeAssets, Navigation, rgba_image, update_globe,
        };
        let mut app = super::super::inspection_ui::tests::app()?;
        let (width, height, offset, good) = {
            let mut game = app.world_mut().resource_mut::<Game>();
            let (u, v) = (0.5 + 77.2 / 360.0, 0.5 - 28.6 / 180.0);
            let id = game.world.province_at_uv(u, v).unwrap();
            let good = game.simulation.catalog.good("steel").unwrap();
            game.simulation.settlements[id].market.price[good] = 0.0;
            game.map_mode = Mode::Price;
            game.map_good = good;
            game.simulation.date = cqsp_core::clock::StarDate::from_tick(0);
            let offset = (((v * game.world.height as f32) as u32 * game.world.width
                + (u * game.world.width as f32) as u32)
                * 4) as usize;
            (game.world.width, game.world.height, offset, good)
        };
        let overlay = app
            .world_mut()
            .resource_mut::<Assets<Image>>()
            .add(rgba_image(
                width,
                height,
                vec![0; (width * height * 4) as usize],
            ));
        app.init_resource::<Assets<EarthMaterial>>();
        let material = app
            .world_mut()
            .resource_mut::<Assets<EarthMaterial>>()
            .add(EarthMaterial {
                options: Vec4::ONE,
                terrain: default(),
                political: overlay.clone(),
            });
        app.insert_resource(GlobeAssets {
            political: overlay.clone(),
            material,
        });
        let globe = app
            .world_mut()
            .spawn((EarthGlobe, Visibility::Visible))
            .id();
        app.add_systems(
            Update,
            update_globe.after(super::super::simulation_controls),
        );
        app.update();
        let before = app
            .world()
            .resource::<Assets<Image>>()
            .get(&overlay)
            .unwrap()
            .data
            .as_ref()
            .unwrap()[offset..offset + 4]
            .to_vec();
        {
            let mut game = app.world_mut().resource_mut::<Game>();
            let id = game
                .world
                .province_at_uv(0.5 + 77.2 / 360.0, 0.5 - 28.6 / 180.0)
                .unwrap();
            game.simulation.settlements[id].market.price[good] =
                game.simulation.catalog.goods[good.0].price * 1.75;
            game.simulation.date = cqsp_core::clock::StarDate::from_tick(1);
        }
        app.update();
        let after = app
            .world()
            .resource::<Assets<Image>>()
            .get(&overlay)
            .unwrap()
            .data
            .as_ref()
            .unwrap()[offset..offset + 4]
            .to_vec();
        assert_ne!(before, after);
        app.world_mut().resource_mut::<Game>().map_mode = Mode::Province;
        app.update();
        let province = app
            .world()
            .resource::<Assets<Image>>()
            .get(&overlay)
            .unwrap()
            .data
            .as_ref()
            .unwrap()[offset..offset + 4]
            .to_vec();
        assert_ne!(after, province);
        app.world_mut().resource_mut::<Navigation>().active = true;
        app.update();
        assert_eq!(
            app.world().get::<Visibility>(globe),
            Some(&Visibility::Hidden)
        );
        Ok(())
    }
}
