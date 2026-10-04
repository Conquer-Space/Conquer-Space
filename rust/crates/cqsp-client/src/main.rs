use anyhow::{Context, Result, bail};
use bevy::{
    app::AppExit,
    asset::RenderAssetUsages,
    core_pipeline::tonemapping::Tonemapping,
    input::mouse::{AccumulatedMouseMotion, AccumulatedMouseScroll},
    mesh::Indices,
    prelude::*,
    reflect::TypePath,
    render::{
        render_resource::{
            AsBindGroup, Extent3d, PrimitiveTopology, TextureDimension, TextureFormat,
        },
        view::screenshot::{Screenshot, save_to_disk},
    },
    shader::ShaderRef,
    window::PrimaryWindow,
};
use cqsp_core::{Session, World, default_data_path, intersect_globe, uv_to_direction};
use std::{f32::consts::PI, path::PathBuf};

#[derive(Resource)]
struct Game {
    world: World,
    session: Session,
    hovered: Option<usize>,
    political: bool,
    ordered_countries: Vec<usize>,
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
}

#[derive(Resource)]
struct InitialAssets {
    terrain: Image,
    font: Font,
}

#[derive(Component)]
struct Hud;
#[derive(Component)]
struct BeginButton;
#[derive(Component)]
struct BeginLabel;

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
                    "Conquer Space — Rust port\nUsage: cqsp-client [--data PATH] [--smoke]\n\nRight drag / arrows: orbit; wheel: zoom; left click: select\nTab / Shift+Tab: cycle countries; Enter: start; Esc: selection\nM: terrain/political map; R: reset camera; F12: screenshot"
                );
                return Ok(());
            }
            _ => bail!("Unknown argument: {arg}"),
        }
    }
    let world = World::load(&data)?;
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
    App::new()
        .add_plugins((
            DefaultPlugins
                .set(AssetPlugin {
                    file_path: PathBuf::from(env!("CARGO_MANIFEST_DIR"))
                        .join("assets")
                        .to_string_lossy()
                        .into_owned(),
                    ..default()
                })
                .set(WindowPlugin {
                    primary_window: Some(Window {
                        title: "Conquer Space — Rust / Bevy".into(),
                        resolution: (1280, 800).into(),
                        ..default()
                    }),
                    ..default()
                }),
            MaterialPlugin::<EarthMaterial>::default(),
        ))
        .insert_resource(ClearColor(Color::srgb(0.008, 0.014, 0.03)))
        .insert_resource(Game {
            world,
            session: Session::default(),
            hovered: None,
            political: true,
            ordered_countries,
        })
        .insert_resource(Orbit::default())
        .insert_resource(Smoke {
            enabled: smoke,
            stage: 0,
            elapsed: 0.0,
        })
        .insert_resource(InitialAssets {
            terrain: terrain_image,
            font,
        })
        .add_systems(Startup, setup)
        .add_systems(
            Update,
            (
                camera_controls,
                pick_province,
                selection_controls,
                update_globe,
                update_hud,
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
        Mesh3d(meshes.add(globe_mesh())),
        MeshMaterial3d(material.clone()),
    ));
    commands.insert_resource(GlobeAssets {
        political,
        material,
    });
    commands.spawn((
        Camera3d::default(),
        Tonemapping::Reinhard,
        orbit.transform(),
    ));
    commands.remove_resource::<InitialAssets>();
    commands.spawn((Node {
        position_type: PositionType::Absolute, left: px(24), top: px(24), width: px(292),
        padding: UiRect::all(px(20)), flex_direction: FlexDirection::Column, row_gap: px(18), border_radius: BorderRadius::all(px(12)), ..default()
    }, BackgroundColor(Color::srgba(0.025, 0.05, 0.09, 0.95))))
        .with_children(|panel| {
            panel.spawn((Text::new("CONQUER SPACE"), ui_font(&font, 24.0), TextColor(Color::srgb(0.9, 0.76, 0.42))));
            panel.spawn((Hud, Text::new("Loading Earth…"), ui_font(&font, 17.0), TextColor(Color::srgb(0.82, 0.87, 0.94))));
            panel.spawn((BeginButton, Button, Node { padding: UiRect::all(px(12)), border_radius: BorderRadius::all(px(6)), ..default() },
                BackgroundColor(Color::srgb(0.15, 0.29, 0.43))))
                .with_children(|button| { button.spawn((BeginLabel, Text::new("Select a country"), ui_font(&font, 18.0))); });
            panel.spawn((Text::new("Right drag / arrows: orbit\nWheel: zoom\nLeft click: select province\nTab / Shift+Tab: countries\nEnter: start · Esc: selection\nM: map mode · R: reset\nF12: save screenshot"),
                ui_font(&font, 14.0), TextColor(Color::srgb(0.55, 0.65, 0.77))));
        });
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
        .filter(|position| position.x > 326.0)
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
) {
    if keyboard.just_pressed(KeyCode::Tab) && game.session.player_country.is_none() {
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
    }
    let pressed = button
        .iter()
        .any(|interaction| *interaction == Interaction::Pressed);
    if keyboard.just_pressed(KeyCode::Enter) || pressed {
        if game.session.player_country.is_some() {
            game.session.return_to_selection();
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

fn update_globe(
    game: Res<Game>,
    handles: Res<GlobeAssets>,
    mut images: ResMut<Assets<Image>>,
    mut materials: ResMut<Assets<EarthMaterial>>,
    mut previous: Local<Option<(Option<usize>, bool)>>,
) {
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
    mut hud: Single<&mut Text, With<Hud>>,
    mut label: Single<&mut Text, (With<BeginLabel>, Without<Hud>)>,
) {
    if !game.is_changed() {
        return;
    }
    let selected = game
        .session
        .selected_country
        .map(|id| &game.world.countries[id]);
    let phase = if game.session.player_country.is_some() {
        "Game view · port foundation"
    } else {
        "Choose your country"
    };
    let country = selected.map_or("No country selected".to_owned(), |country| {
        format!("{}\n{}", country.common, country.name)
    });
    let province = game
        .hovered
        .map_or("Hover over land to inspect a province.".to_owned(), |id| {
            let province = &game.world.provinces[id];
            format!(
                "{}\n{}",
                province.name,
                province
                    .country
                    .map_or("Ungoverned", |id| game.world.countries[id].common.as_str())
            )
        });
    let note = if game.session.player_country.is_some() {
        "Economy and orbital simulation\nare not ported yet."
    } else {
        "Click land or use Tab, then start."
    };
    hud.0 = format!(
        "{phase}\n\n{country}\n\nPROVINCE\n{province}\n\n{} countries · {} provinces\n\n{note}",
        game.world.countries.len(),
        game.world.provinces.len()
    );
    label.0 = if game.session.player_country.is_some() {
        "Back to selection"
    } else if selected.is_some() {
        "Start as this country"
    } else {
        "Select a country"
    }
    .into();
}

#[allow(clippy::too_many_arguments)]
fn smoke_test(
    time: Res<Time>,
    mut smoke: ResMut<Smoke>,
    mut game: ResMut<Game>,
    mut commands: Commands,
    window: Single<&Window, With<PrimaryWindow>>,
    camera: Single<(&Camera, &GlobalTransform), With<Camera3d>>,
    mut exit: MessageWriter<AppExit>,
) {
    if !smoke.enabled {
        return;
    }
    smoke.elapsed += time.delta_secs();
    if smoke.elapsed < 5.0 {
        return;
    }
    smoke.elapsed = 0.0;
    match smoke.stage {
        0 => {
            let id = picked_province(
                &game.world,
                camera.0,
                camera.1,
                Vec2::new(window.width() / 2.0, window.height() / 2.0),
            )
            .expect("Smoke check: camera center must hit Delhi");
            let country = game.world.provinces[id]
                .country
                .expect("Smoke check: Delhi has an owner");
            assert_eq!(game.world.countries[country].identifier, "india");
            game.session.select(country);
            println!(
                "SMOKE: center-screen ray selected India ({}).",
                game.world.provinces[id].name
            );
        }
        1 => {
            commands
                .spawn(Screenshot::primary_window())
                .observe(save_to_disk(screenshot_path("selection.png")));
        }
        2 => {
            assert!(game.session.start());
            println!("SMOKE: entered game view as India.");
        }
        3 => {
            commands
                .spawn(Screenshot::primary_window())
                .observe(save_to_disk(screenshot_path("game-view.png")));
        }
        _ => {
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
