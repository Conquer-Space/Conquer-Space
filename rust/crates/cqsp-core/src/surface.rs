//! Row-major float centroids used by the original PlanetProvinceLoader.
use anyhow::{Context, Result, ensure};
use image::RgbImage;
use std::{
    collections::HashMap,
    path::{Component, Path},
};

pub fn load_province_map(data: &Path, identifier: &str) -> Result<RgbImage> {
    let directory = data.join(match identifier {
        "province_map" => "data/planet_data/earth/map",
        "moon_province_map" => "map/moon",
        _ => anyhow::bail!("Unsupported province map {identifier}"),
    });
    let manifest: serde_json::Value =
        serde_hjson::from_str(&std::fs::read_to_string(directory.join("resource.hjson"))?)?;
    let resource = &manifest[identifier];
    ensure!(
        resource["type"] == "binary",
        "Unknown province map {identifier}"
    );
    let path = resource["path"]
        .as_str()
        .context("Province map has no path")?;
    ensure!(
        Path::new(path)
            .components()
            .all(|part| matches!(part, Component::Normal(_))),
        "Unsafe province map path"
    );
    Ok(image::open(directory.join(path))?.to_rgb8())
}

/// Latitude/longitude in radians. No pixel-center offset or spherical/dateline correction.
pub fn province_centers(
    map: &RgbImage,
    colors: &HashMap<[u8; 3], usize>,
) -> HashMap<usize, [f64; 2]> {
    let mut weights = HashMap::<usize, (u32, [f32; 2])>::new();
    for (index, pixel) in map.pixels().enumerate() {
        let Some(&province) = colors.get(&pixel.0) else {
            continue;
        };
        let (count, sum) = weights.entry(province).or_default();
        *count += 1;
        sum[0] += (index as u32 % map.width()) as f32;
        sum[1] += (index as u32 / map.width()) as f32;
    }
    weights
        .into_iter()
        .map(|(province, (count, [x, y]))| {
            // Keep each division in float before the source's double degree arithmetic.
            let u = (x / count as f32 / map.width() as f32) as f64;
            let v = (y / count as f32 / map.height() as f32) as f64;
            (
                province,
                [
                    ((0.5 - v) * 180.0).to_radians(),
                    ((u - 0.5) * 360.0).to_radians(),
                ],
            )
        })
        .collect()
}

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn centers_use_flat_pixel_weights_and_missing_colors_have_no_coordinate() {
        let map = RgbImage::from_fn(8, 4, |x, y| {
            image::Rgb(if y == 1 && (x == 0 || x == 7) {
                [1, 2, 3]
            } else {
                [0, 0, 0]
            })
        });
        let colors = HashMap::from([([1, 2, 3], 5), ([4, 5, 6], 8)]);
        let centers = province_centers(&map, &colors);
        assert_eq!(centers.len(), 1);
        assert_eq!(centers[&5], [45_f64.to_radians(), (-22.5_f64).to_radians()]);
    }
}
