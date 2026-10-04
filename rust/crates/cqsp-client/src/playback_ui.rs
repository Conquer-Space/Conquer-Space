use super::{Game, inspection_ui::Inspector, spaceport_ui::SpaceportUi, ui_button, ui_font};
use bevy::prelude::*;
use cqsp_core::clock::PlaybackSpeed;

#[derive(Component, Clone, Copy)]
pub(super) enum Action {
    Toggle,
    Slower,
    Faster,
}
impl Game {
    pub(super) fn playback_action(&mut self, action: Action) {
        match action {
            Action::Toggle => self.paused = !self.paused,
            Action::Slower => self.speed_index = self.speed_index.saturating_sub(1),
            Action::Faster => {
                self.speed_index = (self.speed_index + 1).min(PlaybackSpeed::ALL.len() - 1)
            }
        }
    }
}
#[derive(Component)]
pub(super) struct Panel;
#[derive(Component)]
pub(super) struct Status;
pub(super) fn spawn(commands: &mut Commands, camera: Entity, font: &Handle<Font>) {
    commands
        .spawn((
            Panel,
            UiTargetCamera(camera),
            Node {
                display: Display::None,
                position_type: PositionType::Absolute,
                bottom: px(18),
                left: percent(30),
                padding: UiRect::all(px(8)),
                flex_direction: FlexDirection::Row,
                column_gap: px(10),
                align_items: AlignItems::Center,
                ..default()
            },
            BackgroundColor(Color::srgb(0.025, 0.05, 0.09)),
        ))
        .with_children(|bar| {
            ui_button(bar, font, Action::Slower, "Slower (-)");
            ui_button(bar, font, Action::Toggle, "Pause / play");
            ui_button(bar, font, Action::Faster, "Faster (=)");
            bar.spawn((Status, Text::default(), ui_font(font, 14.0)));
        });
}
pub(super) fn update(
    game: Res<Game>,
    ports: Res<SpaceportUi>,
    inspector: Res<Inspector>,
    mut panels: Query<&mut Node, With<Panel>>,
    mut labels: Query<&mut Text, With<Status>>,
) {
    let display = if game.session.player_country.is_some() && !ports.open && !inspector.open {
        Display::Flex
    } else {
        Display::None
    };
    for mut node in &mut panels {
        if node.display != display {
            node.display = display;
        }
    }
    let label = format!(
        "{} · speed {} · {}",
        if game.paused { "Paused" } else { "Running" },
        game.speed_index,
        PlaybackSpeed::ALL[game.speed_index].label()
    );
    for mut text in &mut labels {
        if text.0 != label {
            text.0.clone_from(&label);
        }
    }
}
