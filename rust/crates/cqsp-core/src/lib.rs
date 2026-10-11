//! Renderer-independent domain data and simulation rules for the Rust port.
pub mod catalog;
pub mod clock;
pub mod economy;
pub mod events;
pub mod flight;
pub mod infrastructure;
pub mod modifiers;
pub mod number_display;
pub mod orbit;
pub mod paths;
pub mod random;
pub mod save;
pub mod science;
pub mod search;
pub mod settings;
pub mod simulation;
pub mod solar_system;
pub mod spaceflight;
pub mod surface;
pub mod timezones;
pub mod tooltips;
pub mod units;
use anyhow::{Context, Result, bail, ensure};
use serde::Deserialize;
use serde_json::Value;
use std::{
    collections::HashMap,
    f32::consts::{PI, TAU},
    fs,
    path::{Path, PathBuf},
};

pub fn default_data_path() -> PathBuf {
    paths::asset_directory(
        "assets/core",
        Path::new(env!("CARGO_MANIFEST_DIR")).join("../../../binaries/data/core"),
    )
}

#[derive(Debug)]
pub struct Country {
    pub identifier: String,
    pub name: String,
    pub common: String,
    pub color: [u8; 3],
}

#[derive(Debug)]
pub struct Province {
    pub identifier: String,
    pub name: String,
    pub country: Option<usize>,
}

pub struct World {
    pub countries: Vec<Country>,
    pub provinces: Vec<Province>,
    pub width: u32,
    pub height: u32,
    pub unmapped_land_pixels: usize,
    pub provinces_without_color: Vec<String>,
    pub unresolved_countries: Vec<String>,
    pub color_conflicts: Vec<String>,
    pixels: Vec<Option<usize>>,
}

impl World {
    pub fn load(data: &Path) -> Result<Self> {
        let mut countries = Vec::new();
        let mut country_ids = HashMap::new();
        for entry in read_entries::<CountryEntry>(&data.join("data/countries"))? {
            ensure!(
                !country_ids.contains_key(&entry.identifier),
                "Duplicate country {}",
                entry.identifier
            );
            let color = entry
                .color
                .as_ref()
                .map(parse_color)
                .transpose()?
                .unwrap_or_else(|| identifier_color(&entry.identifier));
            country_ids.insert(entry.identifier.clone(), countries.len());
            countries.push(Country {
                common: entry.common.unwrap_or_else(|| entry.name.clone()),
                name: entry.name,
                identifier: entry.identifier,
                color,
            });
        }
        let mut provinces = Vec::new();
        let mut colors = HashMap::new();
        let mut provinces_without_color = Vec::new();
        let mut unresolved_countries = Vec::new();
        let mut color_conflicts = Vec::new();
        for entry in read_entries::<ProvinceEntry>(&data.join("data/planet_data/provinces/earth"))?
        {
            // Skip the literal template row; preserve real ungoverned provinces.
            if entry.identifier == "name" && entry.color == Some(Value::String("color".into())) {
                continue;
            }
            let country = entry
                .country
                .as_ref()
                .and_then(|owner| country_ids.get(owner))
                .copied();
            if let Some(owner) = &entry.country
                && country.is_none()
                && !unresolved_countries.contains(owner)
            {
                unresolved_countries.push(owner.clone());
            }
            // Province names are country-scoped in the legacy data (e.g. Cordoba).
            // Stable vector indices and unique map colors identify domain entities.
            if let Some(value) = &entry.color {
                let color = parse_color(value)?;
                // Match the C++ loader's last-definition-wins rule, deterministically
                // ordered by filename. Keep the conflict visible for asset cleanup.
                if let Some(previous) = colors.insert(color, provinces.len()) {
                    let previous: &Province = &provinces[previous];
                    color_conflicts.push(format!(
                        "{color:?}: {} -> {} ({})",
                        previous.identifier,
                        entry.identifier,
                        entry.country.as_deref().unwrap_or("ungoverned")
                    ));
                }
            } else {
                // Preserve the domain entity but do not invent a map color.
                provinces_without_color.push(entry.identifier.clone());
            }
            provinces.push(Province {
                name: entry
                    .name
                    .unwrap_or_else(|| display_identifier(&entry.identifier)),
                identifier: entry.identifier,
                country,
            });
        }
        let map = image::open(data.join("data/planet_data/earth/map/countryprovinces2k.png"))
            .context("Loading the Earth province map")?
            .to_rgb8();
        ensure!(
            map.width() >= 2 && map.height() >= 2,
            "Province map is too small"
        );
        let mut unmapped_land_pixels = 0;
        let pixels = map
            .pixels()
            .map(|pixel| {
                let id = colors.get(&pixel.0).copied();
                if id.is_none() && pixel.0 != [255, 255, 255] && pixel.0 != [0, 0, 0] {
                    unmapped_land_pixels += 1;
                }
                id
            })
            .collect();
        Ok(Self {
            countries,
            provinces,
            width: map.width(),
            height: map.height(),
            pixels,
            unmapped_land_pixels,
            provinces_without_color,
            unresolved_countries,
            color_conflicts,
        })
    }

    pub fn province_at_uv(&self, u: f32, v: f32) -> Option<usize> {
        if !u.is_finite() || !v.is_finite() {
            return None;
        }
        // rem_euclid can round a tiny negative coordinate up to exactly 1.0.
        let x = ((u.rem_euclid(1.0) * self.width as f32) as u32).min(self.width - 1);
        let y = ((v.clamp(0.0, 1.0) * self.height as f32) as u32).min(self.height - 1);
        self.pixels[(y * self.width + x) as usize]
    }

    pub fn province_at_direction(&self, point: [f32; 3]) -> Option<usize> {
        let [u, v] = direction_to_uv(point);
        self.province_at_uv(u, v)
    }

    /// RGBA political overlay: transparent ocean, country tint, province/country borders.
    pub fn political_texture(&self, selected: Option<usize>) -> Vec<u8> {
        let mut rgba = Vec::with_capacity(self.pixels.len() * 4);
        for (index, province) in self.pixels.iter().enumerate() {
            let Some(province) = province else {
                rgba.extend([0, 0, 0, 0]);
                continue;
            };
            let country = self.provinces[*province].country;
            let x = index as u32 % self.width;
            let y = index as u32 / self.width;
            let neighbors = [
                self.pixels[(y * self.width + (x + 1) % self.width) as usize],
                self.pixels[((y + 1).min(self.height - 1) * self.width + x) as usize],
            ];
            let border = neighbors.iter().any(|other| *other != Some(*province));
            let country_border = neighbors
                .iter()
                .any(|other| other.is_none_or(|id| self.provinces[id].country != country));
            let highlighted = selected.is_some() && selected == country;
            let color = if highlighted {
                [246, 203, 91]
            } else {
                country.map_or([150, 150, 150], |id| self.countries[id].color)
            };
            let shade = if country_border {
                0.25
            } else if border {
                0.65
            } else {
                1.0
            };
            rgba.extend(color.map(|channel| (channel as f32 * shade) as u8));
            rgba.push(if highlighted { 220 } else { 150 });
        }
        rgba
    }
}

#[derive(Default, Debug, PartialEq, Eq)]
pub struct Session {
    pub selected_country: Option<usize>,
    pub player_country: Option<usize>,
}

impl Session {
    pub fn select(&mut self, country: usize) {
        if self.player_country.is_none() {
            self.selected_country = Some(country);
        }
    }
    pub fn start(&mut self) -> bool {
        self.player_country = self.selected_country;
        self.player_country.is_some()
    }
    pub fn return_to_selection(&mut self) {
        self.player_country = None;
    }
}

/// Equirectangular map: Greenwich at u=0.5, north pole at v=0.
pub fn uv_to_direction([u, v]: [f32; 2]) -> [f32; 3] {
    let longitude = (u - 0.5) * TAU;
    let latitude = (0.5 - v) * PI;
    [
        latitude.cos() * longitude.cos(),
        latitude.sin(),
        latitude.cos() * longitude.sin(),
    ]
}

pub fn direction_to_uv([x, y, z]: [f32; 3]) -> [f32; 2] {
    let length = (x * x + y * y + z * z).sqrt();
    if length == 0.0 {
        return [f32::NAN; 2];
    }
    [
        ((z.atan2(x) / TAU) + 0.5).rem_euclid(1.0),
        0.5 - (y / length).clamp(-1.0, 1.0).asin() / PI,
    ]
}

/// Closest forward intersection with a unit sphere. Handles rays originating inside it.
pub fn intersect_globe(origin: [f32; 3], direction: [f32; 3]) -> Option<[f32; 3]> {
    let dot = |a: [f32; 3], b: [f32; 3]| a.into_iter().zip(b).map(|(a, b)| a * b).sum::<f32>();
    let a = dot(direction, direction);
    if a <= 0.0 || !a.is_finite() {
        return None;
    }
    let b = dot(origin, direction);
    let discriminant = b * b - a * (dot(origin, origin) - 1.0);
    if discriminant < 0.0 || !discriminant.is_finite() {
        return None;
    }
    let near = (-b - discriminant.sqrt()) / a;
    let far = (-b + discriminant.sqrt()) / a;
    let distance = if near >= 0.0 {
        near
    } else if far >= 0.0 {
        far
    } else {
        return None;
    };
    Some(std::array::from_fn(|i| origin[i] + direction[i] * distance))
}

#[derive(Deserialize)]
struct CountryEntry {
    identifier: String,
    name: String,
    common: Option<String>,
    color: Option<Value>,
}

#[derive(Deserialize)]
struct ProvinceEntry {
    identifier: String,
    name: Option<String>,
    country: Option<String>,
    color: Option<Value>,
}

/// Read original HJSON arrays recursively in stable filename order.
pub fn read_entries<T: serde::de::DeserializeOwned>(directory: &Path) -> Result<Vec<T>> {
    let mut files = fs::read_dir(directory)
        .with_context(|| format!("Reading {}", directory.display()))?
        .map(|entry| entry.map(|entry| entry.path()))
        .collect::<std::io::Result<Vec<_>>>()?;
    files.sort();
    let mut entries = Vec::new();
    for path in files {
        if path.is_dir() {
            entries.extend(read_entries::<T>(&path)?);
            continue;
        }
        if !path.extension().is_some_and(|ext| ext == "hjson") {
            continue;
        }
        let content = fs::read_to_string(&path)?;
        // Deserialize through Value: serde-hjson's Option visitor mistakes unquoted
        // strings beginning with 'n' (such as n__cyprus) for null.
        let values: Vec<Value> = serde_hjson::from_str(&content)
            .with_context(|| format!("Parsing {}", path.display()))?;
        for value in values {
            let identifier = value["identifier"]
                .as_str()
                .unwrap_or("<unnamed>")
                .to_owned();
            entries.push(
                serde_json::from_value(value)
                    .with_context(|| format!("Reading {identifier} in {}", path.display()))?,
            );
        }
    }
    Ok(entries)
}

fn parse_color(value: &Value) -> Result<[u8; 3]> {
    if let Some(channels) = value.as_array() {
        ensure!(channels.len() == 3, "Expected three RGB channels: {value}");
        let mut color = [0; 3];
        for (target, channel) in color.iter_mut().zip(channels) {
            *target = channel
                .as_u64()
                .filter(|channel| *channel <= 255)
                .with_context(|| format!("Invalid RGB channel {channel}"))?
                as u8;
        }
        return Ok(color);
    }
    let hex = match value {
        Value::String(value) => value.trim_start_matches('#').to_owned(),
        Value::Number(value) => format!("{value:0>6}"),
        _ => bail!("Invalid color {value}"),
    };
    ensure!(hex.len() == 6, "Expected six-digit hex color: {hex}");
    let packed =
        u32::from_str_radix(&hex, 16).with_context(|| format!("Invalid hex color {hex}"))?;
    Ok([(packed >> 16) as u8, (packed >> 8) as u8, packed as u8])
}

fn identifier_color(identifier: &str) -> [u8; 3] {
    let (mut hash, mut power) = (0_i64, 1_i64);
    for byte in identifier.bytes() {
        hash = (hash + (byte as i64 - b'a' as i64 + 1) * power) % 0xffffff;
        power = (power * 31) % 0xffffff;
    }
    let hash = hash as u32;
    [hash as u8, (hash >> 8) as u8, (hash >> 16) as u8]
}

fn display_identifier(identifier: &str) -> String {
    identifier
        .replace('_', " ")
        .split_whitespace()
        .map(|word| {
            let mut chars = word.chars();
            chars
                .next()
                .map(|first| first.to_uppercase().to_string() + chars.as_str())
                .unwrap_or_default()
        })
        .collect::<Vec<_>>()
        .join(" ")
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn diagnostic_json_round_trips_preserve_numeric_bits() {
        let mut random = crate::random::LegacyRandom::new(0);
        for _ in 0..1000 {
            let sample = random.normal(0.0, 1.0);
            let decoded: f64 =
                serde_json::from_str(&serde_json::to_string(&sample).unwrap()).unwrap();
            assert_eq!(sample.to_bits(), decoded.to_bits(), "JSON changed {sample}");
        }
    }

    #[test]
    fn globe_coordinates_round_trip() {
        for uv in [[0.5, 0.5], [0.1, 0.3], [0.9, 0.8]] {
            let result = direction_to_uv(uv_to_direction(uv));
            assert!((result[0] - uv[0]).abs() < 0.00001);
            assert!((result[1] - uv[1]).abs() < 0.00001);
        }
        assert_eq!(uv_to_direction([0.5, 0.5]), [1.0, 0.0, 0.0]);
    }

    #[test]
    fn ray_intersections_reject_misses_and_backwards_hits() {
        assert_eq!(
            intersect_globe([3.0, 0.0, 0.0], [-1.0, 0.0, 0.0]),
            Some([1.0, 0.0, 0.0])
        );
        assert_eq!(
            intersect_globe([0.0; 3], [0.0, 2.0, 0.0]),
            Some([0.0, 1.0, 0.0])
        );
        assert!(intersect_globe([3.0, 0.0, 0.0], [1.0, 0.0, 0.0]).is_none());
        assert!(intersect_globe([3.0, 0.0, 0.0], [0.0, 1.0, 0.0]).is_none());
        assert!(intersect_globe([0.0; 3], [0.0; 3]).is_none());
    }

    #[test]
    fn selection_requires_a_country_and_locks_after_start() {
        let mut session = Session::default();
        assert!(!session.start());
        session.select(4);
        assert!(session.start());
        session.select(7);
        assert_eq!(session.player_country, Some(4));
        session.return_to_selection();
        session.select(7);
        assert_eq!(session.selected_country, Some(7));
    }

    #[test]
    fn legacy_colors_support_hex_and_rgb() -> Result<()> {
        assert_eq!(
            parse_color(&Value::String("#1DA03E".into()))?,
            [29, 160, 62]
        );
        assert_eq!(
            parse_color(&serde_json::json!([255, 153, 51]))?,
            [255, 153, 51]
        );
        assert_eq!(parse_color(&serde_json::json!(456109))?, [69, 97, 9]);
        assert!(parse_color(&serde_json::json!([256, 0, 0])).is_err());
        assert!(parse_color(&Value::String("color".into())).is_err());
        Ok(())
    }

    #[test]
    fn loads_legacy_assets_and_picks_india() -> Result<()> {
        let world = World::load(&default_data_path())?;
        assert!(world.countries.len() > 190);
        assert!(world.provinces.len() > 1000);
        assert!(
            world
                .provinces_without_color
                .iter()
                .any(|id| id == "saratov")
        );
        assert!(
            world
                .provinces
                .iter()
                .any(|province| province.identifier == "antarctica" && province.country.is_none())
        );
        assert!(
            world
                .countries
                .iter()
                .any(|country| country.identifier == "usa" && country.color == [66, 94, 193])
        );
        // Delhi, safely inland. Country ownership and rendered UV share one convention.
        let province = world
            .province_at_uv(0.5 + 77.2 / 360.0, 0.5 - 28.6 / 180.0)
            .unwrap();
        assert_eq!(
            world.countries[world.provinces[province].country.unwrap()].identifier,
            "india"
        );
        assert!(world.province_at_uv(0.25, 0.5).is_none());
        assert_eq!(
            world.province_at_uv(0.0, 0.5),
            world.province_at_uv(1.0, 0.5)
        );
        assert!(world.province_at_uv(f32::NAN, 0.0).is_none());
        assert_eq!(
            world.province_at_uv(-f32::EPSILON / 4.0, 1.0),
            world.province_at_uv(1.0 - f32::EPSILON, 1.0)
        );
        let rgba = world.political_texture(None);
        assert_eq!(rgba.len(), (world.width * world.height * 4) as usize);
        let ocean = (world.height / 2 * world.width + world.width / 4) as usize * 4;
        assert_eq!(&rgba[ocean..ocean + 4], &[0, 0, 0, 0]);
        Ok(())
    }
}
