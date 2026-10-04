use super::{Game, Orbit as GlobeCamera, ui_font};
use anyhow::{Context, Result, ensure};
use bevy::{
    input::mouse::{AccumulatedMouseMotion, AccumulatedMouseScroll},
    prelude::*,
};
use cqsp_core::{
    clock::StarDate,
    flight::{Burn, FlightStatus},
    orbit::Orbit,
    solar_system::{BodyId, CelestialBody, ShipId, SolarSystem},
};
use std::{
    collections::BTreeMap,
    f64::consts::PI,
    path::{Component as PathComponent, Path},
};

#[derive(Resource, Default)]
pub(super) struct BodyTextures(BTreeMap<BodyId, Image>);
impl BodyTextures {
    pub fn load(data: &Path, system: &SolarSystem) -> Result<Self> {
        let directory = data.join("gfx/img");
        let manifest: serde_json::Value =
            serde_hjson::from_str(&std::fs::read_to_string(directory.join("resource.hjson"))?)?;
        let mut images = BTreeMap::new();
        for (index, body) in system.bodies.iter().enumerate() {
            let Some(name) = &body.terrain.terrain else {
                continue;
            };
            let resource = &manifest[name];
            ensure!(
                resource["type"] == "texture",
                "{}: unknown terrain texture {name}",
                body.identifier
            );
            let path = resource["path"]
                .as_str()
                .context("Terrain texture has no path")?;
            ensure!(
                Path::new(path)
                    .components()
                    .all(|component| matches!(component, PathComponent::Normal(_))),
                "Unsafe terrain texture path {path}"
            );
            let image = image::open(directory.join(path))
                .with_context(|| format!("Loading {} terrain", body.name))?
                .to_rgba8();
            images.insert(
                BodyId(index),
                super::rgba_image(image.width(), image.height(), image.into_raw()),
            );
        }
        Ok(Self(images))
    }
}

fn render_rotation(body: &CelestialBody, time: f64) -> Quat {
    let rotation = body.orientation(time);
    // Reflection from legacy Z-up to Bevy Y-up: positions (x,z,y), axes (-x,-z,-y).
    Quat::from_xyzw(-rotation.x, -rotation.z, -rotation.y, rotation.w)
}

#[derive(Component, Clone, Copy, Debug, PartialEq, Eq)]
pub(super) enum Focus {
    Body(BodyId),
    Ship(ShipId),
}
impl Focus {
    fn visible(self, system: &SolarSystem) -> bool {
        match self {
            Self::Body(_) => true,
            Self::Ship(id) => system.ships[id.0].visible_orbit,
        }
    }
}

#[derive(Resource)]
pub(super) struct Navigation {
    pub active: bool,
    pub focus: Focus,
    pub show_orbits: bool,
    pub show_ground_tracks: bool,
    pub target_radius: f64,
    pub message: String,
    surface: bool,
    yaw: f32,
    pitch: f32,
    distance: f32,
}

impl Navigation {
    pub fn new(system: &SolarSystem) -> Self {
        Self {
            active: false,
            focus: Focus::Body(system.body_id("earth").expect("Loaded Earth")),
            show_orbits: true,
            show_ground_tracks: true,
            target_radius: 14000.0,
            message: String::new(),
            surface: false,
            yaw: 0.4,
            pitch: 0.9,
            distance: 3.5,
        }
    }
    fn frame(&self, system: &SolarSystem) -> SceneFrame {
        if self.surface
            && let Focus::Body(id) = self.focus
        {
            return SceneFrame {
                parent: id,
                center: system.bodies[id.0].kinematics.absolute_position(),
                span: system.bodies[id.0].radius,
            };
        }
        let (parent, span) = match self.focus {
            Focus::Body(id) => {
                let body = &system.bodies[id.0];
                let span = body
                    .children
                    .iter()
                    .map(|id| system.bodies[id.0].orbit.apoapsis())
                    .fold(body.radius * 8.0, f64::max);
                (id, span * 1.15)
            }
            Focus::Ship(id) => {
                let ship = &system.ships[id.0];
                let body = &system.bodies[ship.parent.0];
                (
                    ship.parent,
                    ship.orbit.apoapsis().abs().max(body.radius * 2.0) * 1.5,
                )
            }
        };
        SceneFrame {
            parent,
            center: system.bodies[parent.0].kinematics.absolute_position(),
            span,
        }
    }
    pub fn select(&mut self, focus: Focus, system: &SolarSystem) {
        self.focus = focus;
        self.surface = false;
        self.distance = 3.5;
        self.message.clear();
        if let Focus::Ship(id) = focus {
            self.target_radius = system.ships[id.0].orbit.apoapsis().abs() * 1.25;
        }
    }
    pub fn is_surface_focus(&self) -> bool {
        self.surface
    }
    /// Original FocusOnProvince does nothing if no SurfaceCoordinate was generated.
    pub fn focus_province(&mut self, province: usize, game: &Game) -> bool {
        let Some(site) = game
            .simulation
            .settlements
            .get(province)
            .filter(|site| site.is_province())
        else {
            return false;
        };
        let Some([latitude, longitude]) = site.province_center else {
            return false;
        };
        let system = &game.simulation.solar_system;
        self.select(Focus::Body(site.body), system);
        self.active = true;
        self.surface = true;
        self.pitch = latitude as f32;
        self.yaw = longitude as f32;
        self.distance = ((system.bodies[site.body.0].radius + 800.0)
            / system.bodies[site.body.0].radius) as f32;
        self.message = format!(
            "{} · {:.2}°, {:.2}° · 800 km above the surface",
            site.name,
            latitude.to_degrees(),
            longitude.to_degrees()
        );
        true
    }
    pub fn cycle_ship(&mut self, system: &SolarSystem, previous: bool) {
        let parent = match self.focus {
            Focus::Body(id) => id,
            Focus::Ship(id) => system.ships[id.0].parent,
        };
        let ships: Vec<_> = system
            .ships
            .iter()
            .enumerate()
            .filter(|(_, ship)| ship.parent == parent && ship.status == FlightStatus::Orbiting)
            .map(|(id, _)| Focus::Ship(ShipId(id)))
            .collect();
        if ships.is_empty() {
            self.message = "No orbiting spacecraft at this body.".into();
            return;
        }
        let current = ships.iter().position(|focus| *focus == self.focus);
        let next = cycle_index(current, ships.len(), previous);
        self.select(ships[next], system);
    }
    fn transform(&self, system: &SolarSystem, time: f64) -> Transform {
        let position = Vec3::new(
            self.yaw.cos() * self.pitch.cos(),
            self.pitch.sin(),
            self.yaw.sin() * self.pitch.cos(),
        ) * self.distance;
        let rotation = if self.surface
            && let Focus::Body(id) = self.focus
        {
            render_rotation(&system.bodies[id.0], time)
        } else {
            Quat::IDENTITY
        };
        Transform::from_translation(rotation * position).looking_at(Vec3::ZERO, rotation * Vec3::Y)
    }
    pub fn focus_label(&self, system: &SolarSystem) -> String {
        match self.focus {
            Focus::Body(id) => format!(
                "FOCUSED BODY\n{}\n{} orbiting bodies",
                system.bodies[id.0].name,
                system.bodies[id.0].children.len()
            ),
            Focus::Ship(id) => format!(
                "SPACECRAFT\n{}\nOrbiting {}",
                system.ships[id.0].name, system.bodies[system.ships[id.0].parent.0].name
            ),
        }
    }
    pub fn inspector(&self, system: &SolarSystem, date: StarDate, paused: bool) -> String {
        let details = match self.focus {
            Focus::Body(id) => {
                let body = &system.bodies[id.0];
                let satellites = system
                    .ships
                    .iter()
                    .filter(|ship| ship.parent == id && ship.status == FlightStatus::Orbiting)
                    .count();
                let children = body
                    .children
                    .iter()
                    .map(|id| system.bodies[id.0].name.as_str())
                    .collect::<Vec<_>>()
                    .join(", ");
                format!(
                    "{}\nRadius: {:.1} km\nGM: {:.2} km³/s²\nSOI: {:.1} km\nSpacecraft: {}\n\nChildren: {}",
                    body.name,
                    body.radius,
                    body.mu,
                    body.sphere_of_influence,
                    satellites,
                    if children.is_empty() {
                        "none"
                    } else {
                        &children
                    }
                )
            }
            Focus::Ship(id) => {
                let ship = &system.ships[id.0];
                let orbit = ship.orbit;
                let next = ship.maneuvers.front().map_or("none".into(), |burn| {
                    format!("{:.0} s", burn.time - date.seconds())
                });
                let ground = system.bodies[ship.parent.0]
                    .ground_track(ship.kinematics.state.position, date.seconds())
                    .map_or("unavailable".into(), |[latitude, longitude]| {
                        format!(
                            "{:.2}°, {:.2}°",
                            latitude.to_degrees(),
                            longitude.to_degrees()
                        )
                    });
                format!(
                    "{}\nStatus: {:?}\nAxis: {:.1} km · e: {:.4}\nInclination: {:.2}°\nAscending node: {:.2}°\nPeriapsis angle: {:.2}°\nPeriapsis: {:.1} km\nApoapsis: {:.1} km\nSpeed: {:.3} km/s\nPeriod: {:.0} s\nSurface: {}\n\nBurns: {} · commands: {}\nNext burn: {}\nTarget radius: {:.0} km",
                    ship.name,
                    ship.status,
                    orbit.semi_major_axis,
                    orbit.eccentricity,
                    orbit.inclination.to_degrees(),
                    orbit.ascending_node.to_degrees(),
                    orbit.periapsis_argument.to_degrees(),
                    orbit.periapsis(),
                    orbit.apoapsis(),
                    ship.kinematics.state.velocity.length(),
                    orbit.period(),
                    ground,
                    ship.maneuvers.len(),
                    ship.commands.len(),
                    next,
                    self.target_radius
                )
            }
        };
        format!(
            "SYSTEM VIEW · {}\n{} {:02}:{:02}\n\n{}\n\n{}\n\nV: Earth / system · E: Earth\nTab: bodies · Q: spacecraft\nShift: previous · Backspace: parent\nO: orbits · G: ground track\n[ / ]: target radius",
            if paused { "Paused" } else { "Running" },
            date.display(),
            date.hour(0.0),
            date.minute(),
            details,
            self.message
        )
    }
    pub fn apply(&mut self, action: FlightAction, game: &mut Game) -> Result<()> {
        let Focus::Ship(id) = self.focus else {
            anyhow::bail!("Select a spacecraft first");
        };
        let time = game.simulation.date.seconds();
        let system = &mut game.simulation.solar_system;
        let moon = system.body_id("moon").expect("Loaded Moon");
        let target = system.bodies[moon.0].orbit;
        let ship = &mut system.ships[id.0];
        ensure!(
            ship.status == FlightStatus::Orbiting && ship.error.is_none(),
            "Spacecraft is not maneuverable"
        );
        let burns: Vec<Burn> = match action {
            FlightAction::CircularizePeriapsis => vec![ship.orbit.circularize_at_periapsis()],
            FlightAction::CircularizeApoapsis => vec![ship.orbit.circularize_at_apoapsis()],
            FlightAction::SetApoapsis => vec![ship.orbit.set_apoapsis(self.target_radius)],
            FlightAction::SetPeriapsis => vec![ship.orbit.set_periapsis(self.target_radius)],
            FlightAction::Hohmann => ship
                .orbit
                .hohmann_transfer(self.target_radius)
                .ok_or_else(|| anyhow::anyhow!("Hohmann transfer requires a circular orbit"))?
                .into(),
            FlightAction::InclinationDown => vec![
                ship.orbit
                    .set_inclination((ship.orbit.inclination - 5.0_f64.to_radians()).max(0.0)),
            ],
            FlightAction::InclinationUp => vec![
                ship.orbit
                    .set_inclination((ship.orbit.inclination + 5.0_f64.to_radians()).min(PI)),
            ],
            FlightAction::MatchMoon => {
                ensure!(
                    system.bodies[moon.0].parent == Some(ship.parent),
                    "Moon must orbit the same parent"
                );
                vec![ship.orbit.match_planes(target)]
            }
            FlightAction::TransferMoon => {
                ensure!(
                    system.bodies[moon.0].parent == Some(ship.parent),
                    "Moon must orbit the same parent"
                );
                ship.transfer_to_moon(target, time)?;
                self.message = "Moon transfer sequence queued.".into();
                return Ok(());
            }
        };
        let maneuvers = burns
            .into_iter()
            .map(|burn| burn.at(time))
            .collect::<Result<Vec<_>>>()?;
        let count = maneuvers.len();
        ship.maneuvers.extend(maneuvers);
        self.message = format!("{count} maneuver(s) queued. Space resumes.");
        Ok(())
    }
}

fn cycle_index(current: Option<usize>, count: usize, previous: bool) -> usize {
    current.map_or(if previous { count - 1 } else { 0 }, |index| {
        (index + if previous { count - 1 } else { 1 }) % count
    })
}

struct SceneFrame {
    parent: BodyId,
    center: bevy::math::DVec3,
    span: f64,
}
impl SceneFrame {
    fn point(&self, position: bevy::math::DVec3) -> Vec3 {
        let p = (position - self.center) / self.span;
        Vec3::new(p.x as f32, p.z as f32, p.y as f32)
    }
    fn includes(&self, focus: Focus, system: &SolarSystem) -> bool {
        match focus {
            Focus::Body(id) => id == self.parent || system.bodies[id.0].parent == Some(self.parent),
            Focus::Ship(id) => {
                system.ships[id.0].parent == self.parent
                    && system.ships[id.0].status == FlightStatus::Orbiting
            }
        }
    }
}

#[derive(Component, Clone, Copy)]
pub(super) enum FlightAction {
    CircularizePeriapsis,
    CircularizeApoapsis,
    SetApoapsis,
    SetPeriapsis,
    Hohmann,
    InclinationDown,
    InclinationUp,
    MatchMoon,
    TransferMoon,
}
#[derive(Component)]
pub(super) struct FlightControls;

pub(super) fn spawn_controls(panel: &mut ChildSpawnerCommands, font: &Handle<Font>) {
    panel
        .spawn((
            FlightControls,
            Node {
                display: Display::None,
                flex_direction: FlexDirection::Row,
                flex_wrap: FlexWrap::Wrap,
                column_gap: px(6),
                row_gap: px(6),
                ..default()
            },
        ))
        .with_children(|controls| {
            for (action, label) in [
                (FlightAction::CircularizePeriapsis, "Circularize Pe"),
                (FlightAction::CircularizeApoapsis, "Circularize Ap"),
                (FlightAction::SetApoapsis, "Set apoapsis"),
                (FlightAction::SetPeriapsis, "Set periapsis"),
                (FlightAction::Hohmann, "New orbit"),
                (FlightAction::MatchMoon, "Match Moon"),
                (FlightAction::InclinationDown, "Inclination −5°"),
                (FlightAction::InclinationUp, "Inclination +5°"),
                (FlightAction::TransferMoon, "Transfer to Moon"),
            ] {
                controls
                    .spawn((
                        action,
                        Button,
                        Node {
                            padding: UiRect::all(px(7)),
                            ..default()
                        },
                        BackgroundColor(Color::srgb(0.15, 0.29, 0.43)),
                    ))
                    .with_children(|button| {
                        button.spawn((Text::new(label), ui_font(font, 13.0)));
                    });
            }
        });
}

pub(super) fn setup(
    mut commands: Commands,
    game: Res<Game>,
    mut meshes: ResMut<Assets<Mesh>>,
    mut materials: ResMut<Assets<StandardMaterial>>,
    mut images: ResMut<Assets<Image>>,
    textures: Res<BodyTextures>,
) {
    let mesh = meshes.add(Sphere::new(1.0).mesh().ico(3).expect("Marker sphere"));
    let body_mesh = meshes.add(super::globe_mesh());
    for (index, body) in game.simulation.solar_system.bodies.iter().enumerate() {
        let texture = textures
            .0
            .get(&BodyId(index))
            .map(|image| images.add(image.clone()));
        let color = if texture.is_some() {
            Color::WHITE
        } else if body.is_star {
            Color::srgb(1.0, 0.8, 0.3)
        } else {
            Color::srgb(0.3, 0.65, 0.9)
        };
        commands.spawn((
            Focus::Body(BodyId(index)),
            Mesh3d(body_mesh.clone()),
            MeshMaterial3d(materials.add(StandardMaterial {
                base_color: color,
                base_color_texture: texture,
                unlit: true,
                ..default()
            })),
            Visibility::Hidden,
        ));
    }
    let ship_material = materials.add(StandardMaterial {
        base_color: Color::srgb(0.95, 0.7, 0.3),
        unlit: true,
        ..default()
    });
    for index in 0..game.simulation.solar_system.ships.len() {
        commands.spawn((
            Focus::Ship(ShipId(index)),
            Mesh3d(mesh.clone()),
            MeshMaterial3d(ship_material.clone()),
            Visibility::Hidden,
        ));
    }
    commands.insert_resource(MarkerAssets {
        mesh,
        ship_material,
        ships: game.simulation.solar_system.ships.len(),
    });
    commands.remove_resource::<BodyTextures>();
}

#[derive(Resource)]
pub(super) struct MarkerAssets {
    mesh: Handle<Mesh>,
    ship_material: Handle<StandardMaterial>,
    ships: usize,
}

pub(super) fn sync_ships(
    mut commands: Commands,
    game: Res<Game>,
    mut assets: ResMut<MarkerAssets>,
    markers: Query<(Entity, &Focus)>,
) {
    for (entity, focus) in &markers {
        if matches!(focus, Focus::Ship(id) if id.0 >= game.simulation.solar_system.ships.len()) {
            commands.entity(entity).despawn();
        }
    }
    for index in assets.ships..game.simulation.solar_system.ships.len() {
        commands.spawn((
            Focus::Ship(ShipId(index)),
            Mesh3d(assets.mesh.clone()),
            MeshMaterial3d(assets.ship_material.clone()),
            Visibility::Hidden,
        ));
    }
    assets.ships = game.simulation.solar_system.ships.len();
}

#[allow(clippy::too_many_arguments)]
pub(super) fn controls(
    keyboard: Res<ButtonInput<KeyCode>>,
    mouse: Res<ButtonInput<MouseButton>>,
    motion: Res<AccumulatedMouseMotion>,
    scroll: Res<AccumulatedMouseScroll>,
    time: Res<Time>,
    window: Single<&Window>,
    camera: Single<(&Camera, &GlobalTransform), With<Camera3d>>,
    markers: Query<(&Focus, &GlobalTransform), Without<Camera3d>>,
    buttons: Query<(&FlightAction, &Interaction), Changed<Interaction>>,
    mut navigation: ResMut<Navigation>,
    mut game: ResMut<Game>,
) {
    if game.session.player_country.is_none() {
        if navigation.active {
            navigation.active = false;
        }
        return;
    }
    if keyboard.just_pressed(KeyCode::KeyV) {
        navigation.active = !navigation.active;
    }
    if !navigation.active {
        return;
    }
    let system = &game.simulation.solar_system;
    let previous = keyboard.pressed(KeyCode::ShiftLeft) || keyboard.pressed(KeyCode::ShiftRight);
    if keyboard.just_pressed(KeyCode::Tab) {
        let current = if let Focus::Body(id) = navigation.focus {
            Some(id.0)
        } else {
            None
        };
        let index = cycle_index(current, system.bodies.len(), previous);
        navigation.select(Focus::Body(BodyId(index)), system);
    }
    if keyboard.just_pressed(KeyCode::KeyQ) {
        navigation.cycle_ship(system, previous);
    }
    if keyboard.just_pressed(KeyCode::KeyE) {
        navigation.select(Focus::Body(system.body_id("earth").unwrap()), system);
    }
    if keyboard.just_pressed(KeyCode::Backspace) {
        let parent = match navigation.focus {
            Focus::Body(id) => system.bodies[id.0].parent,
            Focus::Ship(id) => Some(system.ships[id.0].parent),
        };
        if let Some(parent) = parent {
            navigation.select(Focus::Body(parent), system);
        }
    }
    if keyboard.just_pressed(KeyCode::KeyO) {
        navigation.show_orbits = !navigation.show_orbits;
    }
    if keyboard.just_pressed(KeyCode::KeyG) {
        navigation.show_ground_tracks = !navigation.show_ground_tracks;
    }
    if keyboard.just_pressed(KeyCode::BracketLeft) {
        navigation.target_radius /= 1.1;
    }
    if keyboard.just_pressed(KeyCode::BracketRight) {
        navigation.target_radius *= 1.1;
    }
    if mouse.pressed(MouseButton::Right) {
        navigation.yaw -= motion.delta.x * 0.005;
        navigation.pitch += motion.delta.y * 0.005;
    }
    let axis = |a, b| (keyboard.pressed(a) as i8 - keyboard.pressed(b) as i8) as f32;
    let horizontal = axis(KeyCode::ArrowRight, KeyCode::ArrowLeft);
    let vertical = axis(KeyCode::ArrowUp, KeyCode::ArrowDown);
    if horizontal != 0.0 {
        navigation.yaw += horizontal * time.delta_secs();
    }
    if vertical != 0.0 || motion.delta.y != 0.0 {
        navigation.pitch = (navigation.pitch + vertical * time.delta_secs()).clamp(-1.5, 1.5);
    }
    if scroll.delta.y != 0.0 {
        navigation.distance = (navigation.distance * (-scroll.delta.y * 0.08).exp())
            .clamp(if navigation.surface { 1.01 } else { 0.3 }, 20.0);
    }
    if mouse.just_pressed(MouseButton::Left)
        && let Some(cursor) = window
            .cursor_position()
            .filter(|p| p.x > 326.0 && p.x < window.width() - 326.0)
    {
        let frame = navigation.frame(system);
        let picked = markers
            .iter()
            .filter(|(focus, _)| frame.includes(**focus, system) && focus.visible(system))
            .filter_map(|(focus, transform)| {
                camera
                    .0
                    .world_to_viewport(camera.1, transform.translation())
                    .ok()
                    .map(|p| (*focus, p.distance_squared(cursor)))
            })
            .filter(|(_, distance)| *distance < 18.0 * 18.0)
            .min_by(|a, b| a.1.total_cmp(&b.1));
        if let Some((focus, _)) = picked {
            navigation.select(focus, system);
        }
    }
    for (action, interaction) in &buttons {
        if *interaction == Interaction::Pressed
            && let Err(error) = navigation.apply(*action, &mut game)
        {
            navigation.message = error.to_string();
        }
    }
}

pub(super) fn update(
    game: Res<Game>,
    navigation: Res<Navigation>,
    globe: Res<GlobeCamera>,
    mut camera: Single<&mut Transform, (With<Camera3d>, Without<Focus>)>,
    mut markers: Query<(&Focus, &mut Transform, &mut Visibility), Without<Camera3d>>,
    mut controls: Query<&mut Node, With<FlightControls>>,
    mut gizmos: Gizmos,
) {
    let system = &game.simulation.solar_system;
    let frame = navigation.frame(system);
    **camera = if navigation.active {
        navigation.transform(system, game.simulation.date.seconds())
    } else {
        globe.transform()
    };
    for mut node in &mut controls {
        let display = if navigation.active && matches!(navigation.focus, Focus::Ship(_)) {
            Display::Flex
        } else {
            Display::None
        };
        if node.display != display {
            node.display = display;
        }
    }
    for (&focus, mut transform, mut visibility) in &mut markers {
        let visible = navigation.active && frame.includes(focus, system) && focus.visible(system);
        *visibility = if visible {
            Visibility::Visible
        } else {
            Visibility::Hidden
        };
        if !visible {
            continue;
        }
        let (position, radius) = match focus {
            Focus::Body(id) => (
                system.bodies[id.0].kinematics.absolute_position(),
                if navigation.surface && id == frame.parent {
                    1.0
                } else {
                    (system.bodies[id.0].radius / frame.span).clamp(0.018, 0.6) as f32
                },
            ),
            Focus::Ship(id) => (
                system.ships[id.0].kinematics.absolute_position(),
                if focus == navigation.focus {
                    0.016
                } else {
                    0.006
                },
            ),
        };
        *transform =
            Transform::from_translation(frame.point(position)).with_scale(Vec3::splat(radius));
        if let Focus::Body(id) = focus {
            transform.rotation =
                render_rotation(&system.bodies[id.0], game.simulation.date.seconds());
        }
        if focus == navigation.focus {
            gizmos.sphere(
                Isometry3d::from_translation(transform.translation),
                radius * 1.5,
                Color::srgb(1.0, 0.9, 0.5),
            );
        }
        if !navigation.show_orbits {
            continue;
        }
        let orbit = match focus {
            Focus::Body(id) if id != frame.parent => Some(system.bodies[id.0].orbit),
            Focus::Ship(id) => Some(system.ships[id.0].orbit),
            _ => None,
        };
        if let Some(orbit) = orbit {
            let points = orbit_path(orbit).map(|p| frame.point(p + frame.center));
            gizmos.linestrip(
                points,
                if focus == navigation.focus {
                    Color::srgb(1.0, 0.8, 0.35)
                } else {
                    Color::srgba(0.25, 0.55, 0.65, 0.45)
                },
            );
        }
    }
    if navigation.active
        && navigation.show_ground_tracks
        && let Focus::Ship(id) = navigation.focus
    {
        let ship = &system.ships[id.0];
        if ship.status == FlightStatus::Orbiting && ship.visible_orbit {
            let body = &system.bodies[ship.parent.0];
            let radius = (body.radius / frame.span).clamp(0.018, 0.6) as f32;
            let center = frame.point(body.kinematics.absolute_position());
            let points = ground_path(body, ship.orbit, game.simulation.date.seconds())
                .into_iter()
                .map(|point| center + point * radius * 1.015);
            gizmos.linestrip(points, Color::srgb(0.95, 0.25, 0.7));
        }
    }
}

fn ground_path(body: &CelestialBody, orbit: Orbit, time: f64) -> Vec<Vec3> {
    let period = orbit.period();
    if !period.is_finite() || period <= 0.0 || orbit.eccentricity >= 1.0 {
        return Vec::new();
    }
    let rotation = render_rotation(body, time);
    (0..=160)
        .filter_map(|index| {
            let sample = time + period * index as f64 / 160.0;
            let [latitude, longitude] =
                body.ground_track(orbit.state_at(sample).position, sample)?;
            let (latitude, longitude) = (latitude as f32, longitude as f32);
            Some(
                rotation
                    * Vec3::new(
                        latitude.cos() * longitude.cos(),
                        latitude.sin(),
                        latitude.cos() * longitude.sin(),
                    ),
            )
        })
        .collect()
}

fn orbit_path(orbit: Orbit) -> impl Iterator<Item = bevy::math::DVec3> {
    let limit = if orbit.eccentricity > 1.0 {
        (-1.0 / orbit.eccentricity).acos() - 0.01
    } else {
        PI
    };
    (0..=160)
        .map(move |index| {
            orbit
                .state_at_anomaly(-limit + 2.0 * limit * index as f64 / 160.0)
                .position
        })
        .filter(|point| point.is_finite())
}

#[cfg(test)]
mod tests {
    use super::*;
    use cqsp_core::{Session, World, default_data_path, simulation::Simulation};

    #[test]
    fn original_manifest_resolves_all_nine_planet_textures() -> Result<()> {
        let data = default_data_path();
        let system = SolarSystem::load(&data)?;
        let textures = BodyTextures::load(&data, &system)?;
        assert_eq!(textures.0.len(), 9);
        for (id, image) in &textures.0 {
            let size = image.texture_descriptor.size;
            let width = match system.bodies[id.0].identifier.as_str() {
                "earth" => 8192,
                "moon" => 1024, // The source's 2k_moon.jpg actually contains a 1K image.
                _ => 2048,
            };
            assert_eq!(size.width, width);
            assert_eq!(size.width, size.height * 2);
            assert_eq!(
                image.data.as_ref().unwrap().len(),
                (size.width * size.height * 4) as usize
            );
        }
        assert!(!textures.0.contains_key(&system.sun));
        Ok(())
    }

    #[test]
    fn textured_body_rotation_and_ground_path_share_the_source_coordinate_frame() -> Result<()> {
        let system = SolarSystem::load(&default_data_path())?;
        let earth = system.body_id("earth").unwrap();
        let body = &system.bodies[earth.0];
        let ship = &system.ships[0];
        let source = ship.orbit.state_at(86400.0).position.as_vec3().normalize();
        let rotated = body.orientation(86400.0) * source;
        let expected = Vec3::new(rotated.x, rotated.z, rotated.y);
        let actual = render_rotation(body, 86400.0) * Vec3::new(source.x, source.z, source.y);
        assert!(actual.distance(expected) < 1e-6);
        let points = ground_path(body, ship.orbit, 86400.0);
        assert_eq!(points.len(), 161);
        assert!(
            points
                .iter()
                .all(|point| point.is_finite() && (point.length() - 1.0).abs() < 1e-6)
        );
        assert!(points[0].distance(points[160]) > 0.001); // Body rotates during one spacecraft orbit.
        assert!(
            ground_path(
                body,
                Orbit {
                    semi_major_axis: -7000.0,
                    eccentricity: 1.5,
                    ..ship.orbit
                },
                0.0
            )
            .is_empty()
        );
        Ok(())
    }

    #[test]
    fn newly_launched_spacecraft_get_exactly_one_render_marker() -> Result<()> {
        let data = default_data_path();
        let world = World::load(&data)?;
        let simulation = Simulation::load(&data, &world, 42)?;
        let initial = simulation.solar_system.ships.len() + simulation.solar_system.bodies.len();
        let mut app = App::new();
        app.insert_resource(Game {
            world,
            simulation,
            session: Session::default(),
            hovered: None,
            political: true,
            ordered_countries: Vec::new(),
            paused: true,
            speed_index: 0,
            last_tick: 0.0,
        })
        .init_resource::<Assets<Mesh>>()
        .init_resource::<Assets<StandardMaterial>>()
        .init_resource::<Assets<Image>>()
        .init_resource::<BodyTextures>()
        .add_systems(Startup, setup)
        .add_systems(Update, sync_ships);
        app.update();
        let mut markers = app.world_mut().query::<&Focus>();
        assert_eq!(markers.iter(app.world()).count(), initial);
        let id = {
            let mut game = app.world_mut().resource_mut::<Game>();
            let earth = game.simulation.solar_system.body_id("earth").unwrap();
            game.simulation.solar_system.launch_ship(
                "New rocket".into(),
                earth,
                Orbit {
                    semi_major_axis: 7000.0,
                    ..Orbit::default()
                },
            )?
        };
        app.update();
        app.update();
        assert_eq!(markers.iter(app.world()).count(), initial + 1);
        assert_eq!(
            markers
                .iter(app.world())
                .filter(|focus| **focus == Focus::Ship(id))
                .count(),
            1
        );
        app.world_mut()
            .resource_mut::<Game>()
            .simulation
            .solar_system
            .ships
            .truncate(id.0);
        app.update();
        assert_eq!(markers.iter(app.world()).count(), initial);
        let replacement = {
            let mut game = app.world_mut().resource_mut::<Game>();
            let earth = game.simulation.solar_system.body_id("earth").unwrap();
            game.simulation.solar_system.launch_ship(
                "Replacement".into(),
                earth,
                Orbit {
                    semi_major_axis: 7000.0,
                    ..Orbit::default()
                },
            )?
        };
        assert_eq!(replacement, id);
        app.update();
        assert_eq!(markers.iter(app.world()).count(), initial + 1);
        assert_eq!(
            markers
                .iter(app.world())
                .filter(|focus| **focus == Focus::Ship(id))
                .count(),
            1
        );
        Ok(())
    }

    #[test]
    fn province_focus_keeps_exact_map_center_and_800_km_altitude_in_the_rotating_body_frame()
    -> Result<()> {
        let app = super::super::inspection_ui::tests::app()?;
        let game = app.world().resource::<Game>();
        let system = &game.simulation.solar_system;
        let mut navigation = Navigation::new(system);
        for name in ["earth", "moon"] {
            let body = system.body_id(name).unwrap();
            let province = game
                .simulation
                .settlements
                .iter()
                .position(|site| site.body == body && site.province_center.is_some())
                .unwrap();
            assert!(navigation.focus_province(province, game));
            assert!(navigation.active && navigation.surface);
            assert_eq!(navigation.frame(system).span, system.bodies[body.0].radius);
            let [latitude, longitude] = game.simulation.settlements[province]
                .province_center
                .unwrap();
            for time in [0.0, 86400.0, 86400.0 * 365.0] {
                let camera = navigation.transform(system, time);
                let expected_distance = 1.0 + 800.0 / system.bodies[body.0].radius;
                assert!((camera.translation.length() as f64 - expected_distance).abs() < 2e-6);
                let position = bevy::math::DVec3::new(
                    camera.translation.x as f64,
                    camera.translation.z as f64,
                    camera.translation.y as f64,
                );
                let ground = system.bodies[body.0].ground_track(position, time).unwrap();
                assert!((ground[0] - latitude).abs() < 2e-6);
                assert!((ground[1] - longitude).abs() < 2e-6);
                assert!(camera.rotation.is_finite());
            }
        }
        let previous = navigation.focus;
        let distance = navigation.distance;
        let missing = game
            .simulation
            .settlements
            .iter()
            .position(|site| site.is_province() && site.province_center.is_none())
            .unwrap();
        assert!(!navigation.focus_province(missing, game));
        assert!(!navigation.focus_province(usize::MAX, game));
        assert_eq!(navigation.focus, previous);
        assert_eq!(navigation.distance, distance);
        navigation.select(Focus::Body(system.sun), system);
        assert!(!navigation.surface);
        Ok(())
    }

    #[test]
    fn system_navigation_frames_keep_kilometer_precision_and_cycle_ships() -> Result<()> {
        let system = SolarSystem::load(&default_data_path())?;
        let mut navigation = Navigation::new(&system);
        let earth = system.body_id("earth").unwrap();
        let frame = navigation.frame(&system);
        assert_eq!(frame.parent, earth);
        assert_eq!(
            frame.point(system.bodies[earth.0].kinematics.absolute_position()),
            Vec3::ZERO
        );
        navigation.cycle_ship(&system, false);
        let Focus::Ship(first) = navigation.focus else {
            panic!("Earth has spacecraft");
        };
        assert_eq!(system.ships[first.0].parent, earth);
        navigation.cycle_ship(&system, true);
        let last = navigation.focus;
        assert_ne!(last, Focus::Ship(first));
        navigation.cycle_ship(&system, false);
        assert_eq!(navigation.focus, Focus::Ship(first));
        navigation.select(Focus::Body(system.sun), &system);
        assert!(navigation.frame(&system).span > frame.span);
        Ok(())
    }

    #[test]
    fn native_maneuver_action_queues_and_executes_on_simulation_ticks() -> Result<()> {
        let data = default_data_path();
        let world = World::load(&data)?;
        let mut simulation = Simulation::load(&data, &world, 42)?;
        simulation.tick();
        let earth = simulation.solar_system.body_id("earth").unwrap();
        let id = simulation.solar_system.launch_ship(
            "Flight test".into(),
            earth,
            Orbit {
                semi_major_axis: 7000.0,
                ..Orbit::default()
            },
        )?;
        let mut navigation = Navigation::new(&simulation.solar_system);
        navigation.select(Focus::Ship(id), &simulation.solar_system);
        navigation.target_radius = 14000.0;
        let mut game = Game {
            world,
            simulation,
            session: Session::default(),
            hovered: None,
            political: true,
            ordered_countries: Vec::new(),
            paused: true,
            speed_index: 0,
            last_tick: 0.0,
        };
        navigation.apply(FlightAction::Hohmann, &mut game)?;
        assert_eq!(game.simulation.solar_system.ships[id.0].maneuvers.len(), 2);
        for _ in 0..20 {
            game.simulation.tick();
        }
        let ship = &game.simulation.solar_system.ships[id.0];
        assert!(ship.maneuvers.is_empty());
        assert!(ship.orbit.eccentricity < 1e-10);
        assert!((ship.orbit.semi_major_axis - 14000.0).abs() < 1e-6);
        game.simulation.validate()?;
        Ok(())
    }

    #[test]
    fn overlay_paths_close_for_ellipses_and_remain_finite_for_escape_orbits() {
        let orbit = Orbit {
            semi_major_axis: 7000.0,
            mu: 398600.0,
            eccentricity: 0.1,
            ..Orbit::default()
        };
        let points: Vec<_> = orbit_path(orbit).collect();
        assert!((points[0] - points[160]).length() < 1e-8);
        assert_eq!(
            orbit_path(Orbit {
                semi_major_axis: -7000.0,
                eccentricity: 1.5,
                ..orbit
            })
            .count(),
            161
        );
        assert_eq!(cycle_index(None, 11, true), 10);
        assert_eq!(cycle_index(Some(0), 11, true), 10);
    }

    #[test]
    fn native_buttons_fire_once_and_keyboard_navigates_without_a_window_runner() -> Result<()> {
        let data = default_data_path();
        let world = World::load(&data)?;
        let mut simulation = Simulation::load(&data, &world, 42)?;
        simulation.tick();
        let mut navigation = Navigation::new(&simulation.solar_system);
        navigation.active = true;
        navigation.cycle_ship(&simulation.solar_system, false);
        let Focus::Ship(id) = navigation.focus else {
            panic!("Earth spacecraft");
        };
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
        .init_resource::<ButtonInput<MouseButton>>()
        .init_resource::<AccumulatedMouseMotion>()
        .init_resource::<AccumulatedMouseScroll>()
        .add_systems(Update, controls);
        app.world_mut().spawn(Window::default()); // ECS data only: no OS window/plugin.
        app.world_mut()
            .spawn((Camera3d::default(), GlobalTransform::default()));
        app.world_mut()
            .spawn((FlightAction::CircularizePeriapsis, Interaction::Pressed));
        app.update();
        assert_eq!(
            app.world().resource::<Game>().simulation.solar_system.ships[id.0]
                .maneuvers
                .len(),
            1
        );
        app.update();
        assert_eq!(
            app.world().resource::<Game>().simulation.solar_system.ships[id.0]
                .maneuvers
                .len(),
            1
        );
        app.world_mut()
            .resource_mut::<ButtonInput<KeyCode>>()
            .press(KeyCode::Backspace);
        app.update();
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
        app.world_mut()
            .resource_mut::<ButtonInput<KeyCode>>()
            .reset_all();
        app.world_mut()
            .resource_mut::<ButtonInput<KeyCode>>()
            .press(KeyCode::KeyG);
        app.update();
        assert!(!app.world().resource::<Navigation>().show_ground_tracks);
        app.world_mut()
            .resource_mut::<ButtonInput<KeyCode>>()
            .reset_all();
        app.world_mut()
            .resource_mut::<ButtonInput<KeyCode>>()
            .press(KeyCode::KeyV);
        app.update();
        assert!(!app.world().resource::<Navigation>().active);
        Ok(())
    }
}
