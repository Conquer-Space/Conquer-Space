//! Silent graphics preferences, stored separately from the C++ application's settings.
use anyhow::{Context, Result, ensure};
use serde::{Deserialize, Serialize};
use std::{fs, path::Path};

pub const RESOLUTIONS: [(u32, u32); 15] = [
    (1024, 768),
    (1280, 1024),
    (1280, 720),
    (1280, 800),
    (1360, 768),
    (1366, 768),
    (1440, 900),
    (1600, 900),
    (1680, 1050),
    (1920, 1200),
    (1920, 1080),
    (2560, 1440),
    (2560, 1080),
    (3440, 1440),
    (3840, 2160),
];
#[derive(Clone, Copy, Debug, PartialEq, Eq, Serialize, Deserialize)]
#[serde(default)]
pub struct WindowSettings {
    pub width: u32,
    pub height: u32,
    pub decorated: bool,
}
impl Default for WindowSettings {
    fn default() -> Self {
        Self {
            width: 1280,
            height: 720,
            decorated: true,
        }
    }
}
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Serialize, Deserialize)]
#[serde(default)]
pub struct GraphicsSettings {
    pub window: WindowSettings,
    pub full_screen: bool,
}
impl GraphicsSettings {
    pub fn validate(&self) -> Result<()> {
        ensure!(
            (1..=16384).contains(&self.window.width) && (1..=16384).contains(&self.window.height),
            "Window dimensions must be 1–16384 pixels"
        );
        Ok(())
    }
    pub fn load(path: &Path) -> Result<Self> {
        if !path.exists() {
            return Ok(Self::default());
        }
        let settings: Self = serde_hjson::from_str(
            &fs::read_to_string(path)
                .with_context(|| format!("Read settings {}", path.display()))?,
        )
        .context("Parse graphics settings")?;
        settings.validate()?;
        Ok(settings)
    }
    pub fn write(&self, path: &Path) -> Result<()> {
        self.validate()?;
        crate::save::write_atomic(path, self)
    }
    pub fn selected_resolution(&self) -> usize {
        RESOLUTIONS
            .iter()
            .position(|size| *size == (self.window.width, self.window.height))
            .unwrap_or(0)
    }
    pub fn set_resolution(&mut self, index: usize) -> Result<()> {
        let (width, height) = *RESOLUTIONS
            .get(index)
            .context("Unknown display resolution")?;
        self.window.width = width;
        self.window.height = height;
        Ok(())
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn resolution_table_matches_the_actual_original_options_source() {
        let source = include_str!("../../../../src/client/scenes/universe/sysoptionswindow.cpp");
        let table = source
            .split_once("common_resolutions[] = {")
            .unwrap()
            .1
            .split_once("};")
            .unwrap()
            .0;
        let sizes: Vec<_> = table
            .split("ImVec2(")
            .skip(1)
            .map(|size| {
                let (width, height) = size.split_once(')').unwrap().0.split_once(',').unwrap();
                (
                    width.trim().parse::<u32>().unwrap(),
                    height.trim().parse::<u32>().unwrap(),
                )
            })
            .collect();
        assert_eq!(sizes, RESOLUTIONS);
    }
    #[test]
    fn graphics_defaults_merge_validate_and_round_trip_without_audio_or_partial_writes()
    -> Result<()> {
        let root = std::env::temp_dir().join(format!(
            "cqsp-settings-{}-{}",
            std::process::id(),
            crate::save::new_uuid()
        ));
        ensure!(!root.exists(), "Fixture already exists");
        let path = root.join("settings.hjson");
        assert_eq!(GraphicsSettings::load(&path)?, GraphicsSettings::default());
        assert!(!root.exists());
        let mut settings = GraphicsSettings::default();
        for index in 0..15 {
            settings.set_resolution(index)?;
            assert_eq!(settings.selected_resolution(), index);
        }
        let previous = settings;
        assert!(settings.set_resolution(15).is_err());
        assert_eq!(settings, previous);
        settings.full_screen = true;
        settings.write(&path)?;
        assert_eq!(GraphicsSettings::load(&path)?, settings);
        let written = settings;
        assert!(!fs::read_to_string(&path)?.contains("audio"));
        settings.window.width = 0;
        assert!(settings.write(&path).is_err());
        assert_eq!(GraphicsSettings::load(&path)?, written);
        fs::write(&path, r#"{"window":{"width":1600},"audio":{"music":1}}"#)?;
        assert_eq!(
            GraphicsSettings::load(&path)?.window,
            WindowSettings {
                width: 1600,
                ..WindowSettings::default()
            }
        );
        fs::write(&path, r#"{"window":{"height":0}}"#)?;
        assert!(GraphicsSettings::load(&path).is_err());
        // Only this unique test fixture is removed.
        fs::remove_dir_all(root)?;
        Ok(())
    }
}
