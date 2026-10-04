#![cfg_attr(all(windows, not(debug_assertions)), windows_subsystem = "windows")]

use anyhow::{Context, Result, bail};
use bevy::{
    app::{AppExit, ScheduleRunnerPlugin},
    asset::RenderAssetUsages,
    camera::RenderTarget,
    core_pipeline::tonemapping::Tonemapping,
    input::mouse::{AccumulatedMouseMotion, AccumulatedMouseScroll},
    mesh::Indices,
    prelude::*,
    reflect::TypePath,
    render::{
        RenderPlugin,
        render_resource::{
            AsBindGroup, Extent3d, PrimitiveTopology, TextureDimension, TextureFormat,
        },
        view::screenshot::{Screenshot, ScreenshotCaptured, save_to_disk},
    },
    shader::ShaderRef,
    window::{ExitCondition, PrimaryWindow},
    winit::WinitPlugin,
};
use cqsp_core::{Session, World, default_data_path, intersect_globe, uv_to_direction};
use cqsp_core::{
    clock::{PlaybackSpeed, StarDate},
    simulation::Simulation,
};
use std::{f32::consts::PI, path::PathBuf, time::Duration};
mod application_ui;
mod chart;
mod construction_ui;
mod event_ui;
mod inspection_ui;
mod pie;
mod playback_ui;
mod save_ui;
mod spaceport_ui;
mod system_view;
use system_view::{FlightAction, Focus, Navigation};

#[derive(Resource)]
struct Game {
    world: World,
    session: Session,
    hovered: Option<usize>,
    political: bool,
    ordered_countries: Vec<usize>,
    simulation: Simulation,
    paused: bool,
    speed_index: usize,
    last_tick: f64,
}

#[derive(Resource)]
struct Orbit {
    longitude: f32,
    latitude: f32,
    distance: f32,
}

impl Default for Orbit {
    fn default() -> Self {
        Self {
            longitude: 77.2_f32.to_radians(),
            latitude: 28.6_f32.to_radians(),
            distance: 3.1,
        }
    }
}

impl Orbit {
    fn position(&self) -> Vec3 {
        Vec3::from_array(uv_to_direction([
            0.5 + self.longitude / (2.0 * PI),
            0.5 - self.latitude / PI,
        ])) * self.distance
    }
    fn transform(&self) -> Transform {
        Transform::from_translation(self.position()).looking_at(Vec3::ZERO, Vec3::Y)
    }
}

#[derive(Asset, TypePath, AsBindGroup, Debug, Clone)]
struct EarthMaterial {
    #[uniform(0)]
    options: Vec4,
    #[texture(1)]
    #[sampler(2)]
    terrain: Handle<Image>,
    #[texture(3)]
    #[sampler(4)]
    political: Handle<Image>,
}

impl Material for EarthMaterial {
    fn fragment_shader() -> ShaderRef {
        "earth.wgsl".into()
    }
}

#[derive(Resource)]
struct GlobeAssets {
    political: Handle<Image>,
    material: Handle<EarthMaterial>,
}

#[derive(Resource)]
struct Smoke {
    enabled: bool,
    stage: u8,
    elapsed: f32,
    captures: u8,
}

#[derive(Resource)]
struct SmokeTarget(Handle<Image>);

#[derive(Resource)]
struct InitialAssets {
    terrain: Image,
    event: Image,
    font: Font,
}

#[derive(Component)]
enum Hud {
    Phase,
    Country,
    Province,
    Note,
}
#[derive(Component)]
struct EconomyHud;
#[derive(Component)]
struct BeginButton;
#[derive(Component)]
struct BeginLabel;
#[derive(Component)]
struct EarthGlobe;

fn main() -> Result<()> {
    let mut data = default_data_path();
    let mut smoke = false;
    let mut print_paths = false;
    let mut saves_root = None;
    let mut settings_path = None;
    let mut args = std::env::args().skip(1);
    while let Some(arg) = args.next() {
        match arg.as_str() {
            "--data" => {
                data = PathBuf::from(args.next().context("--data needs a core data directory")?)
            }
            "--smoke" => smoke = true,
            "--paths" => print_paths = true,
            "--settings" => {
                settings_path = Some(PathBuf::from(
                    args.next().context("--settings needs a preferences file")?,
                ))
            }
            "--saves" => {
                saves_root = Some(PathBuf::from(
                    args.next().context("--saves needs a directory")?,
                ))
            }
            "--help" | "-h" => {
                println!(
                    "Conquer Space — Rust port (silent)\nUsage: cqsp-client [--data PATH] [--saves DIRECTORY] [--settings FILE] [--smoke] [--paths]\n--paths: print resolved asset/save/settings locations without launching\n--saves: override the Rust-specific metadata save location\n--settings: override the Rust-specific display preferences file\n--smoke: offscreen verification with isolated metadata/preferences fixtures\n\nRight drag / arrows: orbit; wheel: zoom; left click: select\nTab / Shift+Tab: cycle countries; Enter: start/back; Esc: pause menu\nSpace: pause; N: next economy day; -/=: speed\nV: Earth/system; Tab: bodies; Q: spacecraft; Backspace: parent\nP: spaceports, vehicles and lunar missions\nI: markets, industries and science\nO: orbit paths; G: ground track; [/]: maneuver radius; E: focus Earth\nM: terrain/political map; R: reset camera; F12: screenshot"
                );
                println!("B: owned-province construction painting; Esc cancels painting");
                return Ok(());
            }
            _ => bail!("Unknown argument: {arg}"),
        }
    }
    let assets = cqsp_core::paths::asset_directory(
        "assets/client",
        PathBuf::from(env!("CARGO_MANIFEST_DIR")).join("assets"),
    );
    let saves_root = if smoke {
        std::env::current_dir()?
            .join("target")
            .join(format!("metadata-smoke-{}", cqsp_core::save::new_uuid()))
    } else {
        saves_root.map_or_else(save_ui::default_root, Ok)?
    };
    let settings_path = if smoke {
        std::env::current_dir()?.join("target").join(format!(
            "settings-smoke-{}.hjson",
            cqsp_core::save::new_uuid()
        ))
    } else {
        settings_path.map_or_else(application_ui::default_path, Ok)?
    };
    if print_paths {
        println!(
            "{}",
            serde_json::json!({"data":data,"client_assets":assets,"saves":saves_root,"settings":settings_path})
        );
        return Ok(());
    }
    let world = World::load(&data)?;
    let settings = cqsp_core::settings::GraphicsSettings::load(&settings_path)?;
    let menu = application_ui::Menu::new(settings_path, settings);
    let saves = save_ui::Saves::new(data.clone(), saves_root);
    let simulation = Simulation::load(&data, &world, 42)?;
    let navigation = Navigation::new(&simulation.solar_system);
    let body_textures = system_view::BodyTextures::load(&data, &simulation.solar_system)?;
    let spaceport_ui = spaceport_ui::SpaceportUi::new(&simulation);
    let inspector = inspection_ui::Inspector::new(&simulation);
    println!(
        "Economy slice: {} goods, {} jobs, {} recipes, {} settlements.",
        simulation.catalog.goods.len(),
        simulation.catalog.jobs.len(),
        simulation.catalog.recipes.len(),
        simulation.settlements.len()
    );
    for diagnostic in &simulation.diagnostics {
        eprintln!("Legacy economy data: {diagnostic}");
    }
    println!(
        "Loaded {} countries, {} provinces, {}x{} map; {} unmapped land pixels.",
        world.countries.len(),
        world.provinces.len(),
        world.width,
        world.height,
        world.unmapped_land_pixels
    );
    let mut ordered_countries: Vec<_> = (0..world.countries.len())
        .filter(|id| {
            world
                .provinces
                .iter()
                .any(|province| province.country == Some(*id))
        })
        .collect();
    if !world.provinces_without_color.is_empty() {
        eprintln!(
            "Legacy provinces without map colors: {}",
            world.provinces_without_color.join(", ")
        );
    }
    anyhow::ensure!(
        !ordered_countries.is_empty(),
        "No playable countries loaded"
    );
    if !world.unresolved_countries.is_empty() {
        eprintln!(
            "Legacy country references without definitions (displayed ungoverned): {}",
            world.unresolved_countries.join(", ")
        );
    }
    for conflict in &world.color_conflicts {
        eprintln!("Legacy province map color conflict (last definition wins): {conflict}");
    }
    ordered_countries.sort_by_key(|id| world.countries[*id].common.to_lowercase());
    let terrain = image::open(data.join("gfx/img/earth2k.jpg"))
        .context("Loading Earth terrain")?
        .to_rgba8();
    let terrain_image = rgba_image(terrain.width(), terrain.height(), terrain.into_raw());
    let event_image = event_ui::image(&data)?;
    let font = Font::from_bytes(
        std::fs::read(data.join("gfx/fonts/Roboto-Regular.ttf"))
            .context("Loading the bundled UI font")?,
    );
    let mut plugins = DefaultPlugins
        // Async pipeline preparation intermittently omitted UI text in captures.
        .set(RenderPlugin {
            synchronous_pipeline_compilation: true,
            ..default()
        })
        .set(AssetPlugin {
            file_path: assets.to_string_lossy().into_owned(),
            ..default()
        })
        .set(WindowPlugin {
            primary_window: (!smoke).then(|| {
                let mut window = Window {
                    title: "Conquer Space — Rust / Bevy".into(),
                    ..default()
                };
                application_ui::apply_window(settings, &mut window);
                window
            }),
            exit_condition: if smoke {
                ExitCondition::DontExit
            } else {
                ExitCondition::OnAllClosed
            },
            ..default()
        });
    if smoke {
        plugins = plugins.disable::<WinitPlugin>();
    }
    let mut app = App::new();
    app.add_plugins((plugins, MaterialPlugin::<EarthMaterial>::default()));
    if smoke {
        app.add_plugins(ScheduleRunnerPlugin::run_loop(Duration::from_millis(16)));
    }
    app.insert_resource(ClearColor(Color::srgb(0.008, 0.014, 0.03)))
        .insert_resource(Game {
            world,
            session: Session::default(),
            hovered: None,
            political: true,
            ordered_countries,
            simulation,
            paused: true,
            speed_index: PlaybackSpeed::DEFAULT_INDEX,
            last_tick: 0.0,
        })
        .insert_resource(Orbit::default())
        .insert_resource(navigation)
        .insert_resource(body_textures)
        .insert_resource(spaceport_ui)
        .insert_resource(inspector)
        .insert_resource(saves)
        .insert_resource(menu)
        .init_resource::<construction_ui::ConstructionTool>()
        .init_resource::<event_ui::Events>()
        .insert_resource(Smoke {
            enabled: smoke,
            stage: 0,
            elapsed: 0.0,
            captures: 0,
        })
        .insert_resource(InitialAssets {
            terrain: terrain_image,
            event: event_image,
            font,
        })
        .add_systems(Startup, (setup, system_view::setup).chain())
        .add_systems(
            Update,
            (
                (
                    event_ui::controls.run_if(interactive),
                    event_ui::scroll.run_if(interactive),
                    (
                        application_ui::controls.run_if(interactive),
                        application_ui::scroll.run_if(interactive),
                        save_ui::controls.run_if(menu_input),
                        save_ui::scroll.run_if(menu_input),
                        construction_ui::controls.run_if(construction_input),
                        construction_ui::scroll.run_if(construction_input),
                        inspection_ui::controls.run_if(application_input),
                        inspection_ui::chart_controls.run_if(application_input),
                        inspection_ui::pie_controls.run_if(application_input),
                        inspection_ui::budget::controls.run_if(application_input),
                    )
                        .chain()
                        .run_if(event_ui::input_available),
                )
                    .chain(),
                (
                    inspection_ui::scroll.run_if(application_input),
                    spaceport_ui::controls.run_if(application_input),
                    spaceport_ui::scroll.run_if(application_input),
                    camera_controls.run_if(earth_controls),
                    pick_province.run_if(earth_controls),
                    selection_controls.run_if(world_input),
                )
                    .chain()
                    .run_if(event_ui::input_available),
                simulation_controls.run_if(interactive),
                system_view::controls
                    .run_if(world_input)
                    .run_if(event_ui::input_available),
                system_view::sync_ships,
                system_view::update,
                update_globe,
                update_hud,
                (
                    update_economy_hud,
                    playback_ui::update,
                    save_ui::update,
                    construction_ui::update,
                    application_ui::update,
                    event_ui::update,
                )
                    .chain(),
                spaceport_ui::update,
                inspection_ui::update,
                inspection_ui::update_plot,
                inspection_ui::update_pies,
                inspection_ui::budget::update,
                smoke_test,
            )
                .chain(),
        )
        .run();
    Ok(())
}

fn rgba_image(width: u32, height: u32, pixels: Vec<u8>) -> Image {
    Image::new(
        Extent3d {
            width,
            height,
            depth_or_array_layers: 1,
        },
        TextureDimension::D2,
        pixels,
        TextureFormat::Rgba8UnormSrgb,
        RenderAssetUsages::default(),
    )
}

#[allow(clippy::too_many_arguments)]
fn setup(
    mut commands: Commands,
    mut meshes: ResMut<Assets<Mesh>>,
    mut images: ResMut<Assets<Image>>,
    mut materials: ResMut<Assets<EarthMaterial>>,
    mut fonts: ResMut<Assets<Font>>,
    game: Res<Game>,
    orbit: Res<Orbit>,
    initial: Res<InitialAssets>,
    smoke: Res<Smoke>,
) {
    let font = fonts.add(initial.font.clone());
    let political = images.add(rgba_image(
        game.world.width,
        game.world.height,
        game.world.political_texture(None),
    ));
    let material = materials.add(EarthMaterial {
        options: Vec4::X,
        terrain: images.add(initial.terrain.clone()),
        political: political.clone(),
    });
    commands.spawn((
        EarthGlobe,
        Mesh3d(meshes.add(globe_mesh())),
        MeshMaterial3d(material.clone()),
    ));
    commands.insert_resource(GlobeAssets {
        political,
        material,
    });
    let mut camera = commands.spawn((
        Camera3d::default(),
        Tonemapping::Reinhard,
        orbit.transform(),
    ));
    if smoke.enabled {
        let image = images.add(Image::new_target_texture(
            1280,
            800,
            TextureFormat::Rgba8UnormSrgb,
            None,
        ));
        camera.insert(RenderTarget::Image(image.clone().into()));
        camera.commands().insert_resource(SmokeTarget(image));
    }
    let camera = camera.id();
    commands.remove_resource::<InitialAssets>();
    commands.spawn((spaceport_ui::Panel::Background, Node {
        position_type: PositionType::Absolute, left: px(24), top: px(24), width: px(292),
        padding: UiRect::all(px(20)), flex_direction: FlexDirection::Column, row_gap: px(18), border_radius: BorderRadius::all(px(12)), ..default()
    }, UiTargetCamera(camera), BackgroundColor(Color::srgba(0.025, 0.05, 0.09, 0.95))))
        .with_children(|panel| {
            panel.spawn((Text::new("CONQUER SPACE"), ui_font(&font, 24.0), TextColor(Color::srgb(0.9, 0.76, 0.42))));
            for section in [Hud::Phase, Hud::Country, Hud::Province] {
                panel.spawn((section, Text::new("Loading Earth…"), ui_font(&font, 17.0), TextColor(Color::srgb(0.82, 0.87, 0.94))));
            }
            panel.spawn((Text::new(format!("{} countries · {} provinces", game.world.countries.len(), game.world.provinces.len())), ui_font(&font, 14.0), TextColor(Color::srgb(0.82, 0.87, 0.94))));
            panel.spawn((Hud::Note, Text::new(""), ui_font(&font, 16.0), TextColor(Color::srgb(0.82, 0.87, 0.94))));
            panel.spawn((BeginButton, Button, Node { padding: UiRect::all(px(12)), border_radius: BorderRadius::all(px(6)), ..default() },
                BackgroundColor(Color::srgb(0.15, 0.29, 0.43))))
                .with_children(|button| { button.spawn((BeginLabel, Text::new("Select a country"), ui_font(&font, 18.0))); });
            panel.spawn((Text::new("Right drag / arrows: orbit\nWheel: zoom · Left click: select\nTab / Shift+Tab: countries\nEnter: start/back · Esc: pause\nV: Earth / system view\nB: construction painting\nM: map mode · R: reset\nF12: save screenshot"),
                ui_font(&font, 14.0), TextColor(Color::srgb(0.55, 0.65, 0.77))));
        });
    commands
        .spawn((
            spaceport_ui::Panel::Background,
            Node {
                position_type: PositionType::Absolute,
                right: px(24),
                top: px(24),
                width: px(288),
                padding: UiRect::all(px(18)),
                border_radius: BorderRadius::all(px(12)),
                flex_direction: FlexDirection::Column,
                row_gap: px(12),
                ..default()
            },
            BackgroundColor(Color::srgba(0.025, 0.05, 0.09, 0.95)),
            UiTargetCamera(camera),
        ))
        .with_children(|panel| {
            panel.spawn((
                EconomyHud,
                Text::new("Loading economy…"),
                ui_font(&font, 16.0),
                TextColor(Color::srgb(0.82, 0.87, 0.94)),
            ));
            system_view::spawn_controls(panel, &font);
        });
    spaceport_ui::spawn(&mut commands, camera, &font);
    playback_ui::spawn(&mut commands, camera, &font);
    save_ui::spawn(&mut commands, camera, &font);
    construction_ui::spawn(
        &mut commands,
        camera,
        &font,
        &game.simulation.catalog.recipes,
    );
    application_ui::spawn(&mut commands, camera, &font);
    let event_image = images.add(initial.event.clone());
    event_ui::spawn(
        &mut commands,
        camera,
        &font,
        &event_image,
        initial.event.height() as f32 / initial.event.width() as f32,
    );
    inspection_ui::spawn(
        &mut commands,
        camera,
        &font,
        game.simulation.catalog.goods.len(),
    );
}

fn ui_font(font: &Handle<Font>, size: f32) -> TextFont {
    TextFont {
        font: FontSource::Handle(font.clone()),
        font_size: FontSize::Px(size),
        ..default()
    }
}

fn screenshot_path(name: &str) -> Result<PathBuf> {
    let directory = PathBuf::from("target");
    std::fs::create_dir_all(&directory).context("Create screenshot directory")?;
    Ok(directory.join(name))
}

fn interactive(smoke: Res<Smoke>) -> bool {
    !smoke.enabled
}
fn menu_input(
    smoke: Res<Smoke>,
    menu: Option<Res<application_ui::Menu>>,
    construction: Option<Res<construction_ui::ConstructionTool>>,
) -> bool {
    !smoke.enabled
        && menu.is_none_or(|menu| !menu.capture_input)
        && construction.is_none_or(|tool| !tool.capture_input)
}
fn application_input(
    smoke: Res<Smoke>,
    saves: Option<Res<save_ui::Saves>>,
    menu: Option<Res<application_ui::Menu>>,
    construction: Option<Res<construction_ui::ConstructionTool>>,
) -> bool {
    !smoke.enabled
        && saves.is_none_or(|saves| !saves.capture_input)
        && menu.is_none_or(|menu| !menu.capture_input)
        && construction.is_none_or(|tool| !tool.capture_input)
}

fn construction_input(
    smoke: Res<Smoke>,
    ui: Res<spaceport_ui::SpaceportUi>,
    inspector: Res<inspection_ui::Inspector>,
    saves: Option<Res<save_ui::Saves>>,
    menu: Option<Res<application_ui::Menu>>,
) -> bool {
    !smoke.enabled
        && !ui.capture_input
        && !inspector.capture_input
        && saves.is_none_or(|saves| !saves.capture_input)
        && menu.is_none_or(|menu| !menu.capture_input)
}
fn world_input(
    smoke: Res<Smoke>,
    ui: Res<spaceport_ui::SpaceportUi>,
    inspector: Res<inspection_ui::Inspector>,
    saves: Option<Res<save_ui::Saves>>,
    menu: Option<Res<application_ui::Menu>>,
    construction: Option<Res<construction_ui::ConstructionTool>>,
) -> bool {
    construction.is_none_or(|tool| !tool.capture_input)
        && construction_input(smoke, ui, inspector, saves, menu)
}

fn earth_controls(
    smoke: Res<Smoke>,
    navigation: Res<Navigation>,
    ui: Res<spaceport_ui::SpaceportUi>,
    inspector: Res<inspection_ui::Inspector>,
    saves: Option<Res<save_ui::Saves>>,
    menu: Option<Res<application_ui::Menu>>,
    construction: Option<Res<construction_ui::ConstructionTool>>,
) -> bool {
    !smoke.enabled
        && !navigation.active
        && !ui.capture_input
        && !inspector.capture_input
        && saves.is_none_or(|saves| !saves.capture_input)
        && menu.is_none_or(|menu| !menu.capture_input)
        && construction.is_none_or(|tool| !tool.capture_input)
}

fn globe_mesh() -> Mesh {
    let (columns, rows) = (192, 96);
    let mut positions = Vec::new();
    let mut uv = Vec::new();
    let mut indices = Vec::new();
    for row in 0..=rows {
        for column in 0..=columns {
            let point = [column as f32 / columns as f32, row as f32 / rows as f32];
            positions.push(uv_to_direction(point));
            uv.push(point);
        }
    }
    for row in 0..rows {
        for column in 0..columns {
            let a = row * (columns + 1) + column;
            let b = a + columns + 1;
            indices.extend([a, a + 1, b, a + 1, b + 1, b]);
        }
    }
    Mesh::new(
        PrimitiveTopology::TriangleList,
        RenderAssetUsages::default(),
    )
    .with_inserted_attribute(Mesh::ATTRIBUTE_NORMAL, positions.clone())
    .with_inserted_attribute(Mesh::ATTRIBUTE_POSITION, positions)
    .with_inserted_attribute(Mesh::ATTRIBUTE_UV_0, uv)
    .with_inserted_indices(Indices::U32(indices))
}

fn camera_controls(
    mouse: Res<ButtonInput<MouseButton>>,
    keyboard: Res<ButtonInput<KeyCode>>,
    motion: Res<AccumulatedMouseMotion>,
    scroll: Res<AccumulatedMouseScroll>,
    time: Res<Time>,
    mut orbit: ResMut<Orbit>,
    mut camera: Single<&mut Transform, With<Camera3d>>,
) {
    if keyboard.just_pressed(KeyCode::KeyR) {
        *orbit = Orbit::default();
    }
    if mouse.pressed(MouseButton::Right) {
        orbit.longitude -= motion.delta.x * 0.005;
        orbit.latitude += motion.delta.y * 0.005;
    }
    let axis = |positive, negative| {
        (keyboard.pressed(positive) as i8 - keyboard.pressed(negative) as i8) as f32
    };
    orbit.longitude += axis(KeyCode::ArrowRight, KeyCode::ArrowLeft) * time.delta_secs();
    orbit.latitude += axis(KeyCode::ArrowUp, KeyCode::ArrowDown) * time.delta_secs();
    orbit.latitude = orbit.latitude.clamp(-1.5, 1.5);
    orbit.longitude = orbit.longitude.rem_euclid(2.0 * PI);
    orbit.distance = (orbit.distance * (-scroll.delta.y * 0.08).exp()).clamp(1.15, 8.0);
    **camera = orbit.transform();
}

fn picked_province(
    world: &World,
    camera: &Camera,
    transform: &GlobalTransform,
    cursor: Vec2,
) -> Option<usize> {
    let ray = camera.viewport_to_world(transform, cursor).ok()?;
    let hit = intersect_globe(ray.origin.to_array(), ray.direction.to_array())?;
    world.province_at_direction(hit)
}

fn pick_province(
    window: Single<&Window, With<PrimaryWindow>>,
    camera: Single<(&Camera, &GlobalTransform), With<Camera3d>>,
    mouse: Res<ButtonInput<MouseButton>>,
    mut game: ResMut<Game>,
) {
    let hovered = window
        .cursor_position()
        .filter(|position| position.x > 326.0 && position.x < window.width() - 326.0)
        .and_then(|cursor| picked_province(&game.world, camera.0, camera.1, cursor));
    if game.hovered != hovered {
        game.hovered = hovered;
    }
    if mouse.just_pressed(MouseButton::Left)
        && let Some(province) = hovered
        && let Some(country) = game.world.provinces[province].country
    {
        game.session.select(country);
    }
}

fn selection_controls(
    keyboard: Res<ButtonInput<KeyCode>>,
    button: Query<&Interaction, (Changed<Interaction>, With<BeginButton>)>,
    mut commands: Commands,
    mut game: ResMut<Game>,
    navigation: Res<Navigation>,
    construction: Option<Res<construction_ui::ConstructionTool>>,
    menu: Option<Res<application_ui::Menu>>,
) {
    if construction.is_some_and(|tool| tool.capture_input) {
        return;
    }
    if keyboard.just_pressed(KeyCode::Tab)
        && !navigation.active
        && game.session.player_country.is_none()
    {
        let count = game.ordered_countries.len();
        let current = game
            .session
            .selected_country
            .and_then(|id| game.ordered_countries.iter().position(|value| *value == id));
        let previous =
            keyboard.pressed(KeyCode::ShiftLeft) || keyboard.pressed(KeyCode::ShiftRight);
        let index = current.map_or(if previous { count - 1 } else { 0 }, |index| {
            if previous {
                (index + count - 1) % count
            } else {
                (index + 1) % count
            }
        });
        let country = game.ordered_countries[index];
        game.session.select(country);
    }
    if keyboard.just_pressed(KeyCode::Escape)
        && (menu.is_none() || game.session.player_country.is_none())
    {
        game.session.return_to_selection();
        game.paused = true;
        game.last_tick = 0.0;
    }
    let pressed = button
        .iter()
        .any(|interaction| *interaction == Interaction::Pressed);
    if keyboard.just_pressed(KeyCode::Enter) || pressed {
        if game.session.player_country.is_some() {
            game.session.return_to_selection();
            game.paused = true;
            game.last_tick = 0.0;
        } else {
            game.session.start();
        }
    }
    if keyboard.just_pressed(KeyCode::KeyM) {
        game.political = !game.political;
    }
    if keyboard.just_pressed(KeyCode::F12) {
        match screenshot_path("conquer-space.png") {
            Ok(path) => {
                commands
                    .spawn(Screenshot::primary_window())
                    .observe(save_to_disk(path));
            }
            Err(error) => eprintln!("Cannot save screenshot: {error:#}"),
        }
    }
}

#[allow(clippy::too_many_arguments)]
fn simulation_controls(
    time: Res<Time>,
    keyboard: Res<ButtonInput<KeyCode>>,
    buttons: Query<(&playback_ui::Action, &Interaction), Changed<Interaction>>,
    mut game: ResMut<Game>,
    ui: Option<Res<spaceport_ui::SpaceportUi>>,
    inspector: Option<Res<inspection_ui::Inspector>>,
    saves: Option<Res<save_ui::Saves>>,
    menu: Option<Res<application_ui::Menu>>,
    construction: Option<Res<construction_ui::ConstructionTool>>,
    events: Option<Res<event_ui::Events>>,
) {
    if game.session.player_country.is_none()
        || event_ui::pending(&game).is_some()
        || events.is_some_and(|events| events.capture_input)
    {
        return;
    }
    if game.simulation.date == StarDate::default() {
        game.simulation.tick();
    }
    if saves.is_some_and(|saves| saves.open || saves.capture_input)
        || menu.is_some_and(|menu| menu.is_open() || menu.capture_input)
    {
        return;
    }
    let keyboard_enabled = ui.is_none_or(|ui| !ui.capture_input)
        && inspector.is_none_or(|ui| !ui.capture_input)
        && construction.is_none_or(|tool| !tool.capture_input);
    if keyboard_enabled && keyboard.just_pressed(KeyCode::Space) {
        game.playback_action(playback_ui::Action::Toggle);
    }
    if keyboard_enabled && keyboard.just_pressed(KeyCode::Minus) {
        game.playback_action(playback_ui::Action::Slower);
    }
    if keyboard_enabled && keyboard.just_pressed(KeyCode::Equal) {
        game.playback_action(playback_ui::Action::Faster);
    }
    if keyboard_enabled && keyboard.just_pressed(KeyCode::KeyN) {
        game.paused = true;
        game.last_tick = time.elapsed_secs_f64();
        game.simulation.advance_day();
    }
    if keyboard_enabled {
        for (action, interaction) in &buttons {
            if *interaction == Interaction::Pressed {
                game.playback_action(*action);
            }
        }
    }
    let now = time.elapsed_secs_f64();
    let ticks = PlaybackSpeed::ALL[game.speed_index].ticks_after(now - game.last_tick);
    if !game.paused && ticks != 0 {
        // Source scheduling drops excess elapsed time rather than catching up a backlog.
        game.last_tick = now;
        for _ in 0..ticks {
            game.simulation.tick();
        }
    }
}

fn update_globe(
    game: Res<Game>,
    handles: Res<GlobeAssets>,
    mut images: ResMut<Assets<Image>>,
    mut materials: ResMut<Assets<EarthMaterial>>,
    mut previous: Local<Option<(Option<usize>, bool)>>,
    navigation: Res<Navigation>,
    mut globe: Single<&mut Visibility, With<EarthGlobe>>,
) {
    **globe = if navigation.active {
        Visibility::Hidden
    } else {
        Visibility::Visible
    };
    let state = (game.session.selected_country, game.political);
    if *previous == Some(state) {
        return;
    }
    if let Some(mut image) = images.get_mut(&handles.political) {
        image.data = Some(game.world.political_texture(state.0));
    }
    if let Some(mut material) = materials.get_mut(&handles.material) {
        material.options.x = game.political as u8 as f32;
    }
    *previous = Some(state);
}

fn update_hud(
    game: Res<Game>,
    navigation: Res<Navigation>,
    mut hud: Query<(&Hud, &mut Text), Without<BeginLabel>>,
    mut label: Single<&mut Text, (With<BeginLabel>, Without<Hud>)>,
) {
    if !game.is_changed() && !navigation.is_changed() {
        return;
    }
    let selected = game
        .session
        .selected_country
        .map(|id| &game.world.countries[id]);
    let phase = if navigation.active {
        "Game view · solar system"
    } else if game.session.player_country.is_some() {
        "Game view · economy slice"
    } else {
        "Choose your country"
    };
    let country = selected.map_or("No country selected".to_owned(), |country| {
        format!("{}\n{}", country.common, country.name)
    });
    let province = if navigation.active {
        navigation.focus_label(&game.simulation.solar_system)
    } else {
        game.hovered.map_or(
            "PROVINCE\nHover over land to inspect a province.".to_owned(),
            |id| {
                let province = &game.world.provinces[id];
                format!(
                    "PROVINCE\n{}\n{}",
                    province.name,
                    province
                        .country
                        .map_or("Ungoverned", |id| game.world.countries[id].common.as_str())
                )
            },
        )
    };
    let note = if game.session.player_country.is_some() {
        "Earth economy slice is loaded.\nFull parity is still in progress."
    } else {
        "Click land or use Tab, then start."
    };
    for (section, mut text) in &mut hud {
        let content = match section {
            Hud::Phase => phase,
            Hud::Country => &country,
            Hud::Province => &province,
            Hud::Note => note,
        };
        // Refresh sibling layouts together: Bevy 0.19 can drop unchanged glyphs
        // when a neighboring text block changes the panel's layout.
        text.0 = content.into();
    }
    label.0 = if game.session.player_country.is_some() {
        "Back to selection"
    } else if selected.is_some() {
        "Start as this country"
    } else {
        "Select a country"
    }
    .into();
}

fn update_economy_hud(
    game: Res<Game>,
    navigation: Res<Navigation>,
    mut hud: Single<&mut Text, With<EconomyHud>>,
) {
    if !game.is_changed() && !navigation.is_changed() {
        return;
    }
    let simulation = &game.simulation;
    if navigation.active {
        hud.0 = navigation.inspector(&simulation.solar_system, simulation.date, game.paused);
        return;
    }
    if game.session.player_country.is_none() {
        hud.0 = format!(
            "ECONOMY PORT\n\n{} goods · {} jobs\n{} recipes · {} settlements\n\nStart to inspect the simulation.\n\nPartial Earth economy.\nSpaceflight and remaining UI\nare still being ported.",
            simulation.catalog.goods.len(),
            simulation.catalog.jobs.len(),
            simulation.catalog.recipes.len(),
            simulation.settlements.len()
        );
        return;
    }
    let global = simulation.summary(None);
    let owned = simulation.summary(game.session.player_country);
    let date = simulation.date;
    let local = game.hovered.map_or(String::new(), |index| {
        let settlement = &simulation.settlements[index];
        let population: u64 = settlement
            .populations
            .iter()
            .map(|population| population.size)
            .sum();
        format!(
            "\n\nLOCAL MARKET\n{}\nPopulation: {}\nGDP: {}\nTrade balance: {}",
            settlement.name,
            human_number(population as f64),
            human_number(settlement.market.gdp),
            human_number(settlement.market.last_trade_deficit)
        )
    });
    hud.0 = format!(
        "{} {:02}:{:02} · {}\nSpeed {}: {}\n\nCOUNTRY\nPopulation: {}\nIndustries: {}\nGDP: {}\n\nGLOBAL\nPopulation: {}\nEmployed: {}\nLiving standard: {:.2}\n\nSpace: pause / resume\nN: next economy day\n- / =: change speed{}",
        date.display(),
        date.hour(0.0),
        date.minute(),
        if game.paused { "Paused" } else { "Running" },
        game.speed_index,
        PlaybackSpeed::ALL[game.speed_index].label(),
        human_number(owned.population as f64),
        owned.industries,
        human_number(owned.gdp),
        human_number(global.population as f64),
        human_number(global.employed as f64),
        global.standard_of_living,
        local
    );
}

fn human_number(value: f64) -> String {
    for (threshold, suffix) in [(1e12, "T"), (1e9, "B"), (1e6, "M"), (1e3, "K")] {
        if value.abs() >= threshold {
            return format!("{:.2}{suffix}", value / threshold);
        }
    }
    format!("{value:.2}")
}

#[allow(clippy::too_many_arguments)]
fn verify_capture(
    capture: On<ScreenshotCaptured>,
    text_nodes: Query<(&Text, &ComputedNode, &UiGlobalTransform)>,
    navigation: Res<Navigation>,
    mut smoke: ResMut<Smoke>,
    ui: Res<spaceport_ui::SpaceportUi>,
    inspector: Res<inspection_ui::Inspector>,
    camera: Single<(&Camera, &GlobalTransform), With<Camera3d>>,
    markers: Query<(&Focus, &Transform)>,
    plots: Query<(&ComputedNode, &UiGlobalTransform), With<inspection_ui::PlotImage>>,
    pies: Query<(&ComputedNode, &UiGlobalTransform), With<inspection_ui::PieImage>>,
    policy_fills: Query<(&ComputedNode, &UiGlobalTransform), With<inspection_ui::budget::Fill>>,
    game: Res<Game>,
    saves: Res<save_ui::Saves>,
    construction: Res<construction_ui::ConstructionTool>,
    menu: Res<application_ui::Menu>,
    event_images: Query<(&ComputedNode, &UiGlobalTransform), With<event_ui::EventImage>>,
) {
    let pixels = capture
        .image
        .clone()
        .try_into_dynamic()
        .expect("Smoke image")
        .to_rgb8();
    let mut checked = 0;
    let mut inspection_title = false;
    let mut save_title = false;
    let mut painting_label = false;
    let mut pause_title = false;
    let mut graphics_title = false;
    let mut recipe_title = false;
    let mut event_heading = false;
    for (text, node, transform) in &text_nodes {
        if text.0.trim().is_empty() || node.is_empty() {
            continue;
        }
        let center = transform.affine().translation;
        let min = (center - node.size() / 2.0).max(Vec2::ZERO).as_uvec2();
        let max = (center + node.size() / 2.0)
            .min(Vec2::new(pixels.width() as f32, pixels.height() as f32))
            .as_uvec2();
        let mut bright = 0;
        for y in min.y..max.y {
            for x in min.x..max.x {
                if pixels
                    .get_pixel(x, y)
                    .0
                    .iter()
                    .all(|channel| *channel > 100)
                {
                    bright += 1;
                }
            }
        }
        assert!(
            bright >= 3,
            "Smoke check: text is laid out but not rendered: {:?}",
            text.0
        );
        checked += 1;
        if text.0 == "EVENT" {
            let mut gold = 0;
            for y in min.y..max.y {
                for x in min.x..max.x {
                    let [r, g, b] = pixels.get_pixel(x, y).0;
                    gold += usize::from(r > 160 && g > 100 && (b as f32) < r as f32 * 0.8);
                }
            }
            assert!(gold > 20, "Smoke check: event heading glyphs missing");
        }
        inspection_title |= text.0.starts_with("NATIVE INSPECTION");
        save_title |= text.0.starts_with("SAVE / LOAD · METADATA ONLY");
        painting_label |= text.0.starts_with("PAINTING · Esc cancels");
        pause_title |= text.0 == "PAUSE MENU";
        graphics_title |= text.0 == "DISPLAY OPTIONS · SILENT";
        recipe_title |= text.0 == "SEARCH CONSTRUCTION RECIPES";
        event_heading |= text.0 == "EVENT";
    }
    assert!(
        checked
            >= if event_ui::pending(&game).is_some() {
                4
            } else {
                8
            },
        "Smoke check: expected visible text nodes, checked {checked}"
    );
    println!("SMOKE: verified rendered pixels for {checked} text blocks.");
    if let Some(event) = event_ui::pending(&game) {
        assert!(event_heading, "Smoke check: event header missing");
        if event.image == "core:rocket-event" {
            let (node, transform) = event_images.single().expect("Native event image");
            assert!(
                (node.size().x - 450.0).abs() < 1.0,
                "Original event image width must be 450 pixels"
            );
            let center = transform.affine().translation;
            let min = (center - node.size() / 2.0).max(Vec2::ZERO).as_uvec2();
            let max = (center + node.size() / 2.0)
                .min(Vec2::new(pixels.width() as f32, pixels.height() as f32))
                .as_uvec2();
            let mut colors = std::collections::HashSet::new();
            for y in min.y..max.y {
                for x in min.x..max.x {
                    colors.insert(pixels.get_pixel(x, y).0);
                }
            }
            assert!(
                colors.len() > 500,
                "Smoke check: original event texture not rendered"
            );
            println!(
                "SMOKE: verified original event image ({} colors).",
                colors.len()
            );
        }
        smoke.captures += 1;
        return;
    }
    assert!(
        !saves.open || save_title,
        "Smoke check: metadata browser header clipped away"
    );
    assert!(
        menu.screen != application_ui::Screen::Pause || pause_title,
        "Smoke check: pause header missing"
    );
    assert!(
        menu.screen != application_ui::Screen::Graphics || graphics_title,
        "Smoke check: display options header missing"
    );
    assert!(
        !construction.searching || recipe_title,
        "Smoke check: recipe search header missing"
    );
    if construction.armed
        && !navigation.active
        && !saves.open
        && !inspector.open
        && !ui.open
        && !menu.is_open()
    {
        assert!(
            painting_label,
            "Smoke check: construction painting status missing"
        );
    }
    assert!(
        !inspector.open || inspection_title,
        "Smoke check: inspector header clipped away"
    );
    if inspector.open {
        let mut gold = 0;
        for y in 40..72 {
            for x in 60..600 {
                let [r, g, b] = pixels.get_pixel(x, y).0;
                gold += usize::from(r > 160 && g > 100 && (b as f32) < r as f32 * 0.8);
            }
        }
        assert!(
            gold > 20,
            "Smoke check: missing inspector header pixels ({gold})"
        );
    }
    if inspector.is_demographics() {
        let mut verified = 0;
        for (node, transform) in &pies {
            if node.is_empty() {
                continue;
            }
            let center = transform.affine().translation;
            let min = (center - node.size() / 2.0).max(Vec2::ZERO).as_uvec2();
            let max = (center + node.size() / 2.0)
                .min(Vec2::new(pixels.width() as f32, pixels.height() as f32))
                .as_uvec2();
            let mut colored = 0;
            for y in min.y..max.y {
                for x in min.x..max.x {
                    let pixel = pixels.get_pixel(x, y).0;
                    colored += usize::from((0..10).any(|index| pixel == pie::color(index)[..3]));
                }
            }
            assert!(
                colored > 1000,
                "Smoke check: consumption pie missing ({colored} colored pixels)"
            );
            verified += 1;
        }
        assert_eq!(
            verified, 2,
            "Smoke check: both paged population pies must be visible"
        );
        println!("SMOKE: verified rendered slices in both consumption pies.");
    }
    if inspector.is_history() {
        let (node, transform) = plots.single().expect("History plot node");
        assert!(!node.is_empty(), "Smoke check: history plot not laid out");
        let center = transform.affine().translation;
        let min = (center - node.size() / 2.0).max(Vec2::ZERO).as_uvec2();
        let max = (center + node.size() / 2.0)
            .min(Vec2::new(pixels.width() as f32, pixels.height() as f32))
            .as_uvec2();
        let mut plotted = 0;
        for y in min.y..max.y {
            for x in min.x..max.x {
                let [r, g, b] = pixels.get_pixel(x, y).0;
                plotted += usize::from(r < 100 && g > 150 && b > 100 && b < g);
            }
        }
        assert!(
            plotted > 30,
            "Smoke check: history series not rendered ({plotted} pixels)"
        );
        println!("SMOKE: verified history plot ({plotted} series pixels).");
    }
    if inspector.is_budget() {
        let mut verified = 0;
        for (node, transform) in &policy_fills {
            assert!(
                !node.is_empty(),
                "Smoke check: policy slider fill not laid out"
            );
            let center = transform.affine().translation;
            let min = (center - node.size() / 2.0).max(Vec2::ZERO).as_uvec2();
            let max = (center + node.size() / 2.0)
                .min(Vec2::new(pixels.width() as f32, pixels.height() as f32))
                .as_uvec2();
            let mut gold = 0;
            for y in min.y..max.y {
                for x in min.x..max.x {
                    let [r, g, b] = pixels.get_pixel(x, y).0;
                    gold += usize::from(r > 160 && g > 100 && (b as f32) < r as f32 * 0.8);
                }
            }
            assert!(
                gold > 1500,
                "Smoke check: policy slider obscured ({gold} gold pixels, bounds {min:?}..{max:?})"
            );
            verified += 1;
        }
        assert_eq!(verified, 2, "Smoke check: both policy sliders must render");
        println!("SMOKE: verified both policy slider fill bars.");
    }
    if navigation.active && !ui.open && !inspector.open && !saves.open && !menu.is_open() {
        let mut orbit_pixels = 0;
        for y in 0..pixels.height() {
            for x in 330..pixels.width() - 330 {
                if Vec2::new(x as f32, y as f32).distance(Vec2::new(640.0, 400.0)) < 80.0 {
                    continue;
                }
                let [r, g, b] = pixels.get_pixel(x, y).0;
                if r < 90 && g > 70 && b > g && g as f32 > r as f32 * 1.4 {
                    orbit_pixels += 1;
                }
            }
        }
        assert!(
            orbit_pixels > 200,
            "Smoke check: missing rendered orbital overlays ({orbit_pixels} pixels)"
        );
        if let Focus::Ship(id) = navigation.focus {
            let parent = game.simulation.solar_system.ships[id.0].parent;
            let (_, transform) = markers
                .iter()
                .find(|(focus, _)| **focus == Focus::Body(parent))
                .expect("Parent marker");
            let center = camera
                .0
                .world_to_viewport(camera.1, transform.translation)
                .unwrap();
            let right = camera.1.compute_transform().rotation * Vec3::X;
            let edge = camera
                .0
                .world_to_viewport(camera.1, transform.translation + right * transform.scale.x)
                .unwrap();
            let radius = center.distance(edge);
            let mut colors = std::collections::HashSet::new();
            let mut ground_pixels = 0;
            for y in 0..pixels.height() {
                for x in 330..pixels.width() - 330 {
                    if Vec2::new(x as f32, y as f32).distance(center) > radius * 0.85 {
                        continue;
                    }
                    let color = pixels.get_pixel(x, y).0;
                    colors.insert(color);
                    let [r, g, b] = color;
                    if r > 120 && (g as f32) < r as f32 * 0.7 && b as f32 > r as f32 * 0.6 {
                        ground_pixels += 1;
                    }
                }
            }
            assert!(
                colors.len() > 100,
                "Smoke check: planet texture missing ({} colors)",
                colors.len()
            );
            assert!(
                ground_pixels > 10,
                "Smoke check: ground track missing ({ground_pixels} pixels)"
            );
            println!(
                "SMOKE: verified planet texture ({} colors) and ground track ({ground_pixels} pixels).",
                colors.len()
            );
        }
    }
    smoke.captures += 1;
}

fn ui_row(parent: &mut ChildSpawnerCommands, children: impl FnOnce(&mut ChildSpawnerCommands)) {
    parent
        .spawn(Node {
            flex_direction: FlexDirection::Row,
            flex_wrap: FlexWrap::Wrap,
            flex_shrink: 0.0,
            column_gap: px(10),
            row_gap: px(6),
            ..default()
        })
        .with_children(children);
}

fn ui_button<A: Component>(
    parent: &mut ChildSpawnerCommands,
    font: &Handle<Font>,
    action: A,
    label: &str,
) {
    parent
        .spawn((
            action,
            Button,
            Node {
                padding: UiRect::all(px(7)),
                ..default()
            },
            BackgroundColor(Color::srgb(0.15, 0.29, 0.43)),
        ))
        .with_children(|parent| {
            parent.spawn((Text::new(label), ui_font(font, 14.0)));
        });
}

fn capture_smoke(commands: &mut Commands, target: &SmokeTarget, name: &str) {
    commands
        .spawn(Screenshot::image(target.0.clone()))
        .observe(verify_capture)
        .observe(save_to_disk(
            screenshot_path(name).expect("Smoke screenshot directory"),
        ));
}

#[allow(clippy::too_many_arguments)]
fn smoke_test(
    time: Res<Time>,
    mut smoke: ResMut<Smoke>,
    mut game: ResMut<Game>,
    mut commands: Commands,
    camera: Single<(&Camera, &GlobalTransform), With<Camera3d>>,
    mut exit: MessageWriter<AppExit>,
    text_layouts: Query<(&Text, &bevy::text::TextLayoutInfo)>,
    target: Option<Res<SmokeTarget>>,
    mut navigation: ResMut<Navigation>,
    mut ui: ResMut<spaceport_ui::SpaceportUi>,
    mut inspector: ResMut<inspection_ui::Inspector>,
    mut saves: ResMut<save_ui::Saves>,
    mut construction: ResMut<construction_ui::ConstructionTool>,
    mut menu: ResMut<application_ui::Menu>,
    mut events: ResMut<event_ui::Events>,
) {
    if !smoke.enabled {
        return;
    }
    smoke.elapsed += time.delta_secs();
    if smoke.elapsed < 5.0 {
        return;
    }
    smoke.elapsed = 0.0;
    if !smoke.stage.is_multiple_of(2) {
        for (text, layout) in &text_layouts {
            assert!(
                text.0.trim().is_empty() || !layout.glyphs.is_empty(),
                "Smoke check: missing text layout for {:?}",
                text.0
            );
        }
    }
    match smoke.stage {
        0 => {
            let id = picked_province(
                &game.world,
                camera.0,
                camera.1,
                camera
                    .0
                    .logical_viewport_rect()
                    .expect("Smoke check: viewport initialized")
                    .center(),
            )
            .expect("Smoke check: camera center must hit Delhi");
            let country = game.world.provinces[id]
                .country
                .expect("Smoke check: Delhi has an owner");
            assert_eq!(game.world.countries[country].identifier, "india");
            game.session.select(country);
            game.hovered = Some(id);
            println!(
                "SMOKE: center-screen ray selected India ({}).",
                game.world.provinces[id].name
            );
        }
        1 => {
            capture_smoke(
                &mut commands,
                target.as_ref().expect("Smoke render target"),
                "selection.png",
            );
        }
        2 => {
            assert!(game.session.start());
            for _ in 0..7 {
                game.simulation.advance_day();
            }
            game.simulation
                .validate()
                .expect("Smoke check: valid economy after a week");
            assert_eq!(game.simulation.history.len(), 7);
            println!(
                "SMOKE: entered game as India; seven daily economy steps passed ({}).",
                game.simulation.date.display()
            );
        }
        3 => {
            capture_smoke(
                &mut commands,
                target.as_ref().expect("Smoke render target"),
                "game-view.png",
            );
        }
        4 => {
            navigation.active = true;
            let earth = game.simulation.solar_system.body_id("earth").unwrap();
            navigation.select(Focus::Body(earth), &game.simulation.solar_system);
        }
        5 => {
            capture_smoke(
                &mut commands,
                target.as_ref().expect("Smoke render target"),
                "system-view.png",
            );
        }
        6 => {
            navigation.cycle_ship(&game.simulation.solar_system, false);
            assert!(matches!(navigation.focus, Focus::Ship(_)));
            navigation
                .apply(FlightAction::CircularizePeriapsis, &mut game)
                .expect("Smoke maneuver action");
            game.simulation.advance_day();
            game.simulation.validate().expect("Smoke flight validation");
            println!("SMOKE: system navigation and native spacecraft action passed.");
        }
        7 => {
            capture_smoke(
                &mut commands,
                target.as_ref().expect("Smoke render target"),
                "spacecraft-view.png",
            );
        }
        8 => {
            use spaceport_ui::Action;
            ui.open = true;
            ui.apply(Action::Design, &mut game)
                .expect("Smoke native vehicle design");
            ui.apply(Action::Manufacture, &mut game)
                .expect("Smoke native manufacturing");
            for _ in 0..90 {
                ui.apply(Action::NextDay, &mut game).unwrap();
            }
            let ship = cqsp_core::solar_system::ShipId(game.simulation.solar_system.ships.len());
            ui.apply(Action::Launch, &mut game)
                .expect("Smoke manufactured vehicle launch");
            navigation.select(Focus::Ship(ship), &game.simulation.solar_system);
            ui.apply(Action::Colonize, &mut game)
                .expect("Smoke lunar target");
            ui.apply(Action::QueueCore, &mut game)
                .expect("Smoke colony core mission");
            ui.apply(Action::Dispatch, &mut game)
                .expect("Smoke manual mission dispatch");
            for _ in 0..20 {
                ui.apply(Action::NextDay, &mut game).unwrap();
            }
            assert!(
                game.simulation
                    .settlements
                    .iter()
                    .any(|site| site.colony.is_some())
            );
            game.simulation
                .validate()
                .expect("Smoke spaceport / mission validation");
            println!(
                "SMOKE: native design, 90-day manufacturing, launch and lunar colony delivery passed."
            );
        }
        9 => capture_smoke(
            &mut commands,
            target.as_ref().expect("Smoke render target"),
            "spaceport-view.png",
        ),
        10 => {
            ui.open = false;
        }
        11 => capture_smoke(
            &mut commands,
            target.as_ref().expect("Smoke render target"),
            "launched-vehicle.png",
        ),
        12 => {
            inspector.apply(inspection_ui::Action::Open, &mut game);
        }
        13 => capture_smoke(
            &mut commands,
            target.as_ref().expect("Smoke render target"),
            "market-inspector.png",
        ),
        14 => {
            inspector.apply(
                inspection_ui::Action::Tab(inspection_ui::Tab::Industries),
                &mut game,
            );
            inspector.apply(inspection_ui::Action::Ownership, &mut game);
            inspector.apply(inspection_ui::Action::Subsidy(0.1), &mut game);
            inspector.apply(inspection_ui::Action::BoostExpertise, &mut game);
        }
        15 => capture_smoke(
            &mut commands,
            target.as_ref().expect("Smoke render target"),
            "industry-inspector.png",
        ),
        16 => {
            inspector.apply(
                inspection_ui::Action::Tab(inspection_ui::Tab::Science),
                &mut game,
            );
        }
        17 => capture_smoke(
            &mut commands,
            target.as_ref().expect("Smoke render target"),
            "science-inspector.png",
        ),
        18 => inspector.apply(
            inspection_ui::Action::Tab(inspection_ui::Tab::Demographics),
            &mut game,
        ),
        19 => capture_smoke(
            &mut commands,
            target.as_ref().expect("Smoke render target"),
            "demographics-inspector.png",
        ),
        20 => {
            inspector.apply(inspection_ui::Action::PerCapita, &mut game);
            inspector.apply(inspection_ui::Action::ConsumptionCost, &mut game);
            let good = game
                .simulation
                .settlements
                .iter()
                .flat_map(|site| &site.populations)
                .next()
                .unwrap()
                .consumption[0]
                .0;
            inspector.apply(inspection_ui::Action::ConsumptionSlice(good), &mut game);
        }
        21 => capture_smoke(
            &mut commands,
            target.as_ref().expect("Smoke render target"),
            "demographics-count-inspector.png",
        ),
        22 => {
            for segment in 0..2 {
                inspector.zoom_pie(segment, (0.5, 0.5), 0.8);
                inspector.pan_pie(segment, (0.05, 0.025));
            }
        }
        23 => capture_smoke(
            &mut commands,
            target.as_ref().expect("Smoke render target"),
            "demographics-zoomed-inspector.png",
        ),
        24 => {
            let province = game
                .simulation
                .settlements
                .iter()
                .position(|site| !site.populations.is_empty())
                .unwrap();
            let before = game.simulation.settlements[province].industries.len();
            game.hovered = Some(province);
            inspector.apply(inspection_ui::Action::Hovered, &mut game);
            inspector.apply(
                inspection_ui::Action::Tab(inspection_ui::Tab::Construction),
                &mut game,
            );
            inspector.apply(inspection_ui::Action::FactoryCount(26), &mut game);
            inspector.apply(inspection_ui::Action::ConstructFactory, &mut game);
            assert_eq!(
                game.simulation.settlements[province].industries.len(),
                before + 1,
                "Smoke check: manual factory construction"
            );
            game.simulation
                .validate()
                .expect("Smoke check: valid constructed factory");
        }
        25 => capture_smoke(
            &mut commands,
            target.as_ref().expect("Smoke render target"),
            "construction-inspector.png",
        ),
        26 => inspector.apply(
            inspection_ui::Action::Tab(inspection_ui::Tab::Civilization),
            &mut game,
        ),
        27 => capture_smoke(
            &mut commands,
            target.as_ref().expect("Smoke render target"),
            "civilization-inspector.png",
        ),
        28 => {
            inspector.apply(
                inspection_ui::Action::Tab(inspection_ui::Tab::History),
                &mut game,
            );
            inspector.apply(inspection_ui::Action::Next, &mut game);
            inspector.apply(inspection_ui::Action::ChartWindow, &mut game);
        }
        29 => capture_smoke(
            &mut commands,
            target.as_ref().expect("Smoke render target"),
            "history-inspector.png",
        ),
        30 => {
            // The late price series sits at its minimum; keep that band in view.
            inspector.zoom_plot(&game, (0.5, 0.0), 0.5);
            inspector.pan_plot(&game, (0.1, 0.1));
        }
        31 => capture_smoke(
            &mut commands,
            target.as_ref().expect("Smoke render target"),
            "history-zoomed-inspector.png",
        ),
        32 => {
            game.hovered = game
                .simulation
                .settlements
                .iter()
                .position(|site| site.identifier == "illinois");
            inspector.apply(inspection_ui::Action::Hovered, &mut game);
            inspector.apply(
                inspection_ui::Action::Tab(inspection_ui::Tab::Infrastructure),
                &mut game,
            );
        }
        33 => capture_smoke(
            &mut commands,
            target.as_ref().expect("Smoke render target"),
            "infrastructure-inspector.png",
        ),
        34 => {
            let country = game.session.player_country.expect("Started smoke player");
            let province = game.simulation.countries[country].provinces[0];
            game.hovered = Some(province);
            inspector.apply(inspection_ui::Action::Hovered, &mut game);
            inspector.apply(
                inspection_ui::Action::Tab(inspection_ui::Tab::Construction),
                &mut game,
            );
            inspector.apply(inspection_ui::Action::ConstructFactory, &mut game);
            inspector.apply(
                inspection_ui::Action::Tab(inspection_ui::Tab::Budget),
                &mut game,
            );
            inspector.apply(inspection_ui::Action::FullSubsidy, &mut game);
            inspector.apply(inspection_ui::Action::ApplySubsidy, &mut game);
            inspector.apply(inspection_ui::Action::AbsoluteTax(2.5), &mut game);
            let site = &game.simulation.settlements[province];
            assert_eq!(site.industries.last().unwrap().output_subsidy, 1.0);
            assert_eq!(game.simulation.countries[country].subsidies.len(), 1);
            assert!(
                game.simulation
                    .settlements
                    .iter()
                    .filter(|site| site.is_province())
                    .all(|site| site.market.taxation[cqsp_core::economy::GoodId(0)] == 2.5)
            );
        }
        35 => capture_smoke(
            &mut commands,
            target.as_ref().expect("Smoke render target"),
            "budget-inspector.png",
        ),
        36 => {
            inspector.apply(inspection_ui::Action::Close, &mut game);
            navigation.active = false;
            let country = game.session.player_country.expect("Smoke player");
            game.hovered = game.simulation.countries[country]
                .provinces
                .iter()
                .copied()
                .find(|&id| game.simulation.settlements[id].industries.is_empty());
            assert!(
                game.hovered.is_some(),
                "Smoke player needs an empty province"
            );
            construction.apply(construction_ui::Action::Toggle, &game);
            construction
                .paint(&mut game, 1)
                .expect("Paint new smoke factory");
            construction.apply(construction_ui::Action::Next, &game);
            construction
                .paint(&mut game, 5)
                .expect("Enlarge smoke factory despite different selection");
            let industry = &game.simulation.settlements[game.hovered.unwrap()].industries[0];
            assert_eq!((industry.size, industry.utilization), (6.0, 1.0));
            assert_eq!(industry.recipe, cqsp_core::catalog::RecipeId(0));
            assert!(industry.construction.is_none());
        }
        37 => capture_smoke(
            &mut commands,
            target.as_ref().expect("Smoke render target"),
            "construction-painting.png",
        ),
        38 => {
            construction.apply(construction_ui::Action::Search, &game);
            construction.set_query("CoAl", &game);
            assert!(construction.searching && construction.capture_input);
        }
        39 => capture_smoke(
            &mut commands,
            target.as_ref().expect("Smoke render target"),
            "construction-search.png",
        ),
        40 => {
            let recipe = game
                .simulation
                .catalog
                .recipes
                .iter()
                .position(|recipe| recipe.name.to_ascii_lowercase().contains("coal"))
                .expect("Loaded coal recipe");
            construction.apply(construction_ui::Action::SelectRecipe(recipe), &game);
            assert!(!construction.searching);
            construction.apply(construction_ui::Action::Toggle, &game);
            let country = game.session.player_country.expect("Smoke event player");
            game.simulation
                .push_event(
                    country,
                    cqsp_core::events::Notification::tutorial(0).unwrap(),
                )
                .expect("Queue explicit event fixture");
            game.simulation
                .push_event(
                    country,
                    cqsp_core::events::Notification::tutorial(1).unwrap(),
                )
                .expect("Queue discarded second fixture");
        }
        41 => capture_smoke(
            &mut commands,
            target.as_ref().expect("Smoke render target"),
            "event-choice.png",
        ),
        42 => {
            let tick = game.simulation.date.tick();
            events
                .choose(&mut game, Some(0))
                .expect("Resolve native event effect");
            assert_eq!(game.simulation.tutorial_stage, 1);
            assert!(event_ui::pending(&game).is_none());
            assert_eq!(game.simulation.date.tick(), tick);
            let country = game.session.player_country.expect("Smoke event player");
            game.simulation.push_event(country, cqsp_core::events::Notification {
                title: "Native queue notice".into(), content: "This explicit smoke fixture has no actions. Ok clears the entire player queue, matching upstream. The disabled tutorial is not registered during normal play.".into(),
                image: "missing-fixture-image".into(), choices: vec![],
            }).expect("Queue actionless fixture");
        }
        43 => capture_smoke(
            &mut commands,
            target.as_ref().expect("Smoke render target"),
            "event-notice.png",
        ),
        44 => {
            events
                .choose(&mut game, None)
                .expect("Acknowledge actionless native event");
            assert!(event_ui::pending(&game).is_none());
            let tick = game.simulation.date.tick();
            menu.apply(application_ui::Action::OpenPause)
                .expect("Open smoke pause menu");
            menu.apply(application_ui::Action::LoadGame)
                .expect("Original no-op load");
            assert_eq!(game.simulation.date.tick(), tick);
            assert!(game.session.player_country.is_some());
        }
        45 => capture_smoke(
            &mut commands,
            target.as_ref().expect("Smoke render target"),
            "pause-menu.png",
        ),
        46 => {
            menu.apply(application_ui::Action::OpenGraphics)
                .expect("Open smoke display options");
            menu.apply(application_ui::Action::Next)
                .expect("Next original resolution");
            menu.apply(application_ui::Action::Fullscreen)
                .expect("Select fullscreen without an OS window");
            menu.apply(application_ui::Action::Persist)
                .expect("Write isolated smoke preferences");
            assert_eq!(
                (menu.settings.window.width, menu.settings.window.height),
                (1280, 800)
            );
            assert!(menu.settings.full_screen);
        }
        47 => capture_smoke(
            &mut commands,
            target.as_ref().expect("Smoke render target"),
            "display-options.png",
        ),
        48 => {
            menu.apply(application_ui::Action::CloseGraphics)
                .expect("Return to pause");
            assert_eq!(menu.screen, application_ui::Screen::Pause);
            menu.apply(application_ui::Action::Resume)
                .expect("Resume without changing playback");
            saves
                .apply(
                    save_ui::Action::Open,
                    &mut game,
                    &mut navigation,
                    &mut inspector,
                    &mut ui,
                )
                .expect("Open smoke metadata browser");
            saves
                .apply(
                    save_ui::Action::Save,
                    &mut game,
                    &mut navigation,
                    &mut inspector,
                    &mut ui,
                )
                .expect("Save isolated smoke metadata");
        }
        49 => capture_smoke(
            &mut commands,
            target.as_ref().expect("Smoke render target"),
            "metadata-browser.png",
        ),
        50 => {
            let uuid = game.simulation.uuid.clone();
            let tick = game.simulation.date.tick();
            saves
                .apply(
                    save_ui::Action::Load,
                    &mut game,
                    &mut navigation,
                    &mut inspector,
                    &mut ui,
                )
                .expect("Load isolated smoke metadata");
            assert_eq!(game.simulation.uuid, uuid);
            assert_eq!(game.simulation.date.tick(), tick + 1);
            assert_eq!(game.simulation.solar_system.ships.len(), 64);
            assert!(game.session.player_country.is_none());
            assert!(game.paused && !navigation.active && !inspector.open && !ui.open);
        }
        _ => {
            assert_eq!(
                smoke.captures, 25,
                "Smoke check: all twenty-five captures must be verified"
            );
            assert!(!construction.armed, "Load must cancel stale painting");
            println!("SMOKE: complete.");
            exit.write(AppExit::Success);
        }
    }
    smoke.stage += 1;
}

#[cfg(test)]
mod tests {
    use super::*;
    use bevy::mesh::VertexAttributeValues;

    #[test]
    fn simulation_keys_step_pause_speed_and_stop_in_selection() -> Result<()> {
        let data = default_data_path();
        let world = World::load(&data)?;
        let expected_country =
            format!("{}\n{}", world.countries[0].common, world.countries[0].name);
        let simulation = Simulation::load(&data, &world, 42)?;
        let navigation = Navigation::new(&simulation.solar_system);
        let mut session = Session::default();
        session.select(0);
        session.start();
        let mut app = App::new();
        app.insert_resource(Game {
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
        .insert_resource(navigation)
        .init_resource::<Time>()
        .init_resource::<ButtonInput<KeyCode>>()
        .add_systems(Update, (simulation_controls, update_hud).chain());
        let country_text = app.world_mut().spawn((Hud::Country, Text::default())).id();
        app.world_mut().spawn((BeginLabel, Text::default()));
        app.update();
        assert_eq!(
            app.world().get::<Text>(country_text).unwrap().0,
            expected_country
        );
        assert_eq!(app.world().resource::<Game>().simulation.date.tick(), 0);
        app.world_mut()
            .resource_mut::<ButtonInput<KeyCode>>()
            .press(KeyCode::KeyN);
        app.update();
        assert_eq!(
            app.world().resource::<Game>().simulation.date.tick(),
            cqsp_core::clock::DAY
        );
        assert!(app.world().resource::<Game>().paused);
        app.world_mut()
            .resource_mut::<ButtonInput<KeyCode>>()
            .reset_all();
        app.world_mut()
            .resource_mut::<ButtonInput<KeyCode>>()
            .press(KeyCode::Space);
        app.world_mut()
            .resource_mut::<Time>()
            .advance_by(std::time::Duration::from_secs(1));
        app.update();
        assert!(!app.world().resource::<Game>().paused);
        assert_eq!(
            app.world().resource::<Game>().simulation.date.tick(),
            cqsp_core::clock::DAY // Exactly one second is not greater than the speed-zero interval.
        );
        app.world_mut()
            .resource_mut::<ButtonInput<KeyCode>>()
            .reset_all();
        app.world_mut()
            .resource_mut::<ButtonInput<KeyCode>>()
            .press(KeyCode::Equal);
        app.update();
        assert_eq!(app.world().resource::<Game>().speed_index, 1);
        assert_eq!(
            app.world().resource::<Game>().simulation.date.tick(),
            cqsp_core::clock::DAY + 1
        );
        app.world_mut()
            .resource_mut::<Game>()
            .session
            .return_to_selection();
        app.update();
        assert_eq!(
            app.world().resource::<Game>().simulation.date.tick(),
            cqsp_core::clock::DAY + 1
        );
        assert_eq!(
            app.world().get::<Text>(country_text).unwrap().0,
            expected_country
        );
        app.world_mut()
            .resource_mut::<ButtonInput<KeyCode>>()
            .reset_all();
        app.world_mut().resource_mut::<Game>().session.start();
        app.world_mut().resource_mut::<Game>().paused = true;
        let faster = app
            .world_mut()
            .spawn((playback_ui::Action::Faster, Interaction::Pressed))
            .id();
        app.update();
        app.update();
        assert_eq!(
            app.world().resource::<Game>().speed_index,
            2,
            "Button must fire once"
        );
        for index in 3..=10 {
            *app.world_mut().get_mut::<Interaction>(faster).unwrap() = Interaction::None;
            app.update();
            *app.world_mut().get_mut::<Interaction>(faster).unwrap() = Interaction::Pressed;
            app.update();
            assert_eq!(app.world().resource::<Game>().speed_index, index);
        }
        let game = app.world().resource::<Game>();
        let inspector = inspection_ui::Inspector::new(&game.simulation);
        let ports = spaceport_ui::SpaceportUi::new(&game.simulation);
        app.insert_resource(inspector)
            .insert_resource(ports)
            .add_systems(Update, playback_ui::update.after(simulation_controls));
        let panel = app
            .world_mut()
            .spawn((playback_ui::Panel, Node::default()))
            .id();
        let status = app
            .world_mut()
            .spawn((playback_ui::Status, Text::default()))
            .id();
        app.update();
        assert_eq!(
            app.world().get::<Node>(panel).unwrap().display,
            Display::Flex
        );
        assert!(
            app.world()
                .get::<Text>(status)
                .unwrap()
                .contains("speed 10")
        );
        app.world_mut()
            .resource_mut::<inspection_ui::Inspector>()
            .open = true;
        app.world_mut()
            .resource_mut::<inspection_ui::Inspector>()
            .capture_input = true;
        let toggle = app
            .world_mut()
            .spawn((playback_ui::Action::Toggle, Interaction::Pressed))
            .id();
        app.update();
        assert!(
            app.world().resource::<Game>().paused,
            "Hidden playback controls cannot fire"
        );
        assert_eq!(
            app.world().get::<Node>(panel).unwrap().display,
            Display::None
        );
        app.world_mut()
            .resource_mut::<inspection_ui::Inspector>()
            .open = false;
        app.world_mut()
            .resource_mut::<inspection_ui::Inspector>()
            .capture_input = false;
        *app.world_mut().get_mut::<Interaction>(toggle).unwrap() = Interaction::None;
        app.update();
        *app.world_mut().get_mut::<Interaction>(toggle).unwrap() = Interaction::Pressed;
        app.world_mut()
            .resource_mut::<Time>()
            .advance_by(Duration::from_secs(1000));
        let date = app.world().resource::<Game>().simulation.date.tick();
        app.update();
        assert_eq!(
            app.world().resource::<Game>().simulation.date.tick(),
            date + 10,
            "Long frame executes one batch, not a time backlog"
        );
        app.update();
        assert_eq!(
            app.world().resource::<Game>().simulation.date.tick(),
            date + 10
        );
        Ok(())
    }

    #[test]
    fn economic_numbers_keep_sign_and_units() {
        assert_eq!(human_number(549_700_354.0), "549.70M");
        assert_eq!(human_number(-16000.0), "-16.00K");
        assert_eq!(human_number(1.07), "1.07");
    }

    #[test]
    fn globe_triangles_face_outward() {
        let mesh = globe_mesh();
        let Some(VertexAttributeValues::Float32x3(positions)) =
            mesh.attribute(Mesh::ATTRIBUTE_POSITION)
        else {
            panic!("Globe requires 3D positions");
        };
        let Some(Indices::U32(indices)) = mesh.indices() else {
            panic!("Globe requires indices");
        };
        for triangle in indices.as_chunks::<3>().0 {
            let [a, b, c] =
                std::array::from_fn(|i| Vec3::from_array(positions[triangle[i] as usize]));
            assert!((b - a).cross(c - a).dot(a) >= -0.000001);
        }
    }

    #[test]
    fn initial_camera_faces_delhi_at_the_requested_distance() {
        let orbit = Orbit::default();
        assert!((orbit.position().length() - orbit.distance).abs() < 0.00001);
        let uv = cqsp_core::direction_to_uv(orbit.position().to_array());
        assert!((uv[0] - (0.5 + 77.2 / 360.0)).abs() < 0.00001);
        assert!((uv[1] - (0.5 - 28.6 / 180.0)).abs() < 0.00001);
    }
}
