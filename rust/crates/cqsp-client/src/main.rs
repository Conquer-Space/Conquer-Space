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
use cqsp_core::{clock::StarDate, simulation::Simulation};
use std::{f32::consts::PI, path::PathBuf, time::Duration};
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
    tick_accumulator: f64,
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
    let mut args = std::env::args().skip(1);
    while let Some(arg) = args.next() {
        match arg.as_str() {
            "--data" => {
                data = PathBuf::from(args.next().context("--data needs a core data directory")?)
            }
            "--smoke" => smoke = true,
            "--help" | "-h" => {
                println!(
                    "Conquer Space — Rust port (silent)\nUsage: cqsp-client [--data PATH] [--smoke]\n\nRight drag / arrows: orbit; wheel: zoom; left click: select\nTab / Shift+Tab: cycle countries; Enter: start; Esc: selection\nSpace: pause; N: next economy day; -/=: speed\nV: Earth/system; Tab: bodies; Q: spacecraft; Backspace: parent\nP: spaceports, vehicles and lunar missions\nO: orbit paths; [/]: maneuver radius; E: focus Earth\nM: terrain/political map; R: reset camera; F12: screenshot"
                );
                return Ok(());
            }
            _ => bail!("Unknown argument: {arg}"),
        }
    }
    let world = World::load(&data)?;
    let simulation = Simulation::load(&data, &world, 42)?;
    let navigation = Navigation::new(&simulation.solar_system);
    let spaceport_ui = spaceport_ui::SpaceportUi::new(&simulation);
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
            file_path: PathBuf::from(env!("CARGO_MANIFEST_DIR"))
                .join("assets")
                .to_string_lossy()
                .into_owned(),
            ..default()
        })
        .set(WindowPlugin {
            primary_window: (!smoke).then(|| Window {
                title: "Conquer Space — Rust / Bevy".into(),
                resolution: (1280, 800).into(),
                ..default()
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
            speed_index: 0,
            tick_accumulator: 0.0,
        })
        .insert_resource(Orbit::default())
        .insert_resource(navigation)
        .insert_resource(spaceport_ui)
        .insert_resource(Smoke {
            enabled: smoke,
            stage: 0,
            elapsed: 0.0,
            captures: 0,
        })
        .insert_resource(InitialAssets {
            terrain: terrain_image,
            font,
        })
        .add_systems(Startup, (setup, system_view::setup).chain())
        .add_systems(
            Update,
            (
                spaceport_ui::controls.run_if(interactive),
                spaceport_ui::scroll.run_if(interactive),
                camera_controls.run_if(earth_controls),
                pick_province.run_if(earth_controls),
                selection_controls.run_if(world_input),
                simulation_controls.run_if(interactive),
                system_view::controls.run_if(world_input),
                system_view::sync_ships,
                system_view::update,
                update_globe,
                update_hud,
                update_economy_hud,
                spaceport_ui::update,
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
            panel.spawn((Text::new("Right drag / arrows: orbit\nWheel: zoom · Left click: select\nTab / Shift+Tab: countries\nEnter: start · Esc: selection\nV: Earth / system view\nM: map mode · R: reset\nF12: save screenshot"),
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
}

fn ui_font(font: &Handle<Font>, size: f32) -> TextFont {
    TextFont {
        font: FontSource::Handle(font.clone()),
        font_size: FontSize::Px(size),
        ..default()
    }
}

fn screenshot_path(name: &str) -> PathBuf {
    PathBuf::from(env!("CARGO_MANIFEST_DIR"))
        .join("../../target")
        .join(name)
}

fn interactive(smoke: Res<Smoke>) -> bool {
    !smoke.enabled
}

fn world_input(smoke: Res<Smoke>, ui: Res<spaceport_ui::SpaceportUi>) -> bool {
    !smoke.enabled && !ui.capture_input
}

fn earth_controls(
    smoke: Res<Smoke>,
    navigation: Res<Navigation>,
    ui: Res<spaceport_ui::SpaceportUi>,
) -> bool {
    !smoke.enabled && !navigation.active && !ui.capture_input
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
) {
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
    if keyboard.just_pressed(KeyCode::Escape) {
        game.session.return_to_selection();
        game.paused = true;
        game.tick_accumulator = 0.0;
    }
    let pressed = button
        .iter()
        .any(|interaction| *interaction == Interaction::Pressed);
    if keyboard.just_pressed(KeyCode::Enter) || pressed {
        if game.session.player_country.is_some() {
            game.session.return_to_selection();
            game.paused = true;
            game.tick_accumulator = 0.0;
        } else {
            game.session.start();
        }
    }
    if keyboard.just_pressed(KeyCode::KeyM) {
        game.political = !game.political;
    }
    if keyboard.just_pressed(KeyCode::F12) {
        commands
            .spawn(Screenshot::primary_window())
            .observe(save_to_disk(screenshot_path("conquer-space.png")));
    }
}

const SIMULATION_SPEEDS: [f64; 4] = [1.0, 4.0, 16.0, 64.0];

fn simulation_controls(
    time: Res<Time>,
    keyboard: Res<ButtonInput<KeyCode>>,
    mut game: ResMut<Game>,
    ui: Option<Res<spaceport_ui::SpaceportUi>>,
) {
    if game.session.player_country.is_none() {
        return;
    }
    if game.simulation.date == StarDate::default() {
        game.simulation.tick();
    }
    let keyboard_enabled = ui.is_none_or(|ui| !ui.capture_input);
    if keyboard_enabled && keyboard.just_pressed(KeyCode::Space) {
        game.paused = !game.paused;
        game.tick_accumulator = 0.0;
    }
    if keyboard_enabled && keyboard.just_pressed(KeyCode::Minus) {
        game.speed_index = game.speed_index.saturating_sub(1);
    }
    if keyboard_enabled && keyboard.just_pressed(KeyCode::Equal) {
        game.speed_index = (game.speed_index + 1).min(SIMULATION_SPEEDS.len() - 1);
    }
    if keyboard_enabled && keyboard.just_pressed(KeyCode::KeyN) {
        game.paused = true;
        game.tick_accumulator = 0.0;
        game.simulation.advance_day();
    }
    if !game.paused {
        game.tick_accumulator += time.delta_secs_f64() * SIMULATION_SPEEDS[game.speed_index];
        // Cap work per rendered frame without throwing away pending simulation time.
        for _ in 0..32 {
            if game.tick_accumulator < 0.25 {
                break;
            }
            game.tick_accumulator -= 0.25;
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
        "{} {:02}:{:02} · {}\nSpeed: {}x\n\nCOUNTRY\nPopulation: {}\nIndustries: {}\nGDP: {}\n\nGLOBAL\nPopulation: {}\nEmployed: {}\nLiving standard: {:.2}\n\nSpace: pause / resume\nN: next economy day\n- / =: change speed{}",
        date.display(),
        date.hour(0.0),
        date.minute(),
        if game.paused { "Paused" } else { "Running" },
        SIMULATION_SPEEDS[game.speed_index],
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

fn verify_capture(
    capture: On<ScreenshotCaptured>,
    text_nodes: Query<(&Text, &ComputedNode, &UiGlobalTransform)>,
    navigation: Res<Navigation>,
    mut smoke: ResMut<Smoke>,
    ui: Res<spaceport_ui::SpaceportUi>,
) {
    let pixels = capture
        .image
        .clone()
        .try_into_dynamic()
        .expect("Smoke image")
        .to_rgb8();
    let mut checked = 0;
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
    }
    assert!(
        checked >= 8,
        "Smoke check: expected visible text nodes, checked {checked}"
    );
    println!("SMOKE: verified rendered pixels for {checked} text blocks.");
    if navigation.active && !ui.open {
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
    }
    smoke.captures += 1;
}

fn capture_smoke(commands: &mut Commands, target: &SmokeTarget, name: &str) {
    commands
        .spawn(Screenshot::image(target.0.clone()))
        .observe(verify_capture)
        .observe(save_to_disk(screenshot_path(name)));
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
) {
    if !smoke.enabled {
        return;
    }
    smoke.elapsed += time.delta_secs();
    if smoke.elapsed < 5.0 {
        return;
    }
    smoke.elapsed = 0.0;
    if matches!(smoke.stage, 1 | 3 | 5 | 7 | 9 | 11) {
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
        _ => {
            assert_eq!(
                smoke.captures, 6,
                "Smoke check: all six captures must be verified"
            );
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
            tick_accumulator: 0.0,
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
            cqsp_core::clock::DAY + 4
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
            cqsp_core::clock::DAY + 20
        );
        app.world_mut()
            .resource_mut::<Game>()
            .session
            .return_to_selection();
        app.update();
        assert_eq!(
            app.world().resource::<Game>().simulation.date.tick(),
            cqsp_core::clock::DAY + 20
        );
        assert_eq!(
            app.world().get::<Text>(country_text).unwrap().0,
            expected_country
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
