//! Source-mapped map information; display layout and border styling are native.
use crate::{World, economy::GoodId, simulation::Simulation};

#[derive(Clone, Copy, Debug, Default, PartialEq, Eq)]
pub enum Mode {
    Terrain,
    #[default]
    Country,
    Province,
    Domestic,
    Price,
}
impl Mode {
    pub const ALL: [Self; 5] = [
        Self::Terrain,
        Self::Country,
        Self::Province,
        Self::Domestic,
        Self::Price,
    ];
    pub fn name(self) -> &'static str {
        match self {
            Self::Terrain => "Terrain",
            Self::Country => "Countries",
            Self::Province => "Provinces",
            Self::Domestic => "Domestic provinces",
            Self::Price => "Selected-good prices",
        }
    }
    pub fn texture(
        self,
        world: &World,
        simulation: &Simulation,
        selected: Option<usize>,
        player: Option<usize>,
        good: GoodId,
    ) -> Vec<u8> {
        if self == Self::Country {
            return world.political_texture(selected);
        }
        let colors: Vec<_> = world
            .provinces
            .iter()
            .enumerate()
            .map(|(index, province)| {
                let settlement = simulation.settlements.get(index);
                let country = settlement.map_or(province.country, |site| site.country);
                let color = match self {
                    Self::Terrain => return [0; 4],
                    Self::Province => province.color.unwrap_or([0; 3]),
                    Self::Domestic if player.is_some() && country == player => {
                        province.color.unwrap_or([0; 3])
                    }
                    Self::Domestic => country
                        .and_then(|id| world.countries.get(id))
                        .map_or([150; 3], |owner| owner.color),
                    Self::Price => {
                        return price_scale(simulation, index, good)
                            .map_or([0; 4], |scale| scale.rgba());
                    }
                    Self::Country => unreachable!(),
                };
                [color[0], color[1], color[2], 166]
            })
            .collect();
        world.overlay_texture(&colors)
    }
}

#[derive(Clone, Copy, Debug)]
pub struct PriceScale {
    pub price: f64,
    pub maximum: f64,
    pub ratio: f64,
}
impl PriceScale {
    /// Source uses configurable maximum price, then casts the ratio to float for MixScale.
    pub fn new(price: f64, base: f64, deviation: f64) -> Option<Self> {
        let maximum = base * (1.0 + deviation);
        let ratio = price / maximum;
        (price.is_finite()
            && maximum.is_finite()
            && ratio.is_finite()
            && (ratio as f32).is_finite())
        .then_some(Self {
            price,
            maximum,
            ratio,
        })
    }
    pub fn color(self) -> [f32; 3] {
        let amount = self.ratio as f32;
        std::array::from_fn(|index| {
            let red = [194.0, 4.0, 4.0][index] / 255.0;
            let green = [37.0, 243.0, 7.0][index] / 255.0;
            red * (1.0 - amount) + green * amount
        })
    }
    pub fn rgba(self) -> [u8; 4] {
        let [r, g, b] = self
            .color()
            .map(|channel| (channel.clamp(0.0, 1.0) * 255.0) as u8);
        [r, g, b, 166]
    }
}
pub fn price_scale(simulation: &Simulation, province: usize, good: GoodId) -> Option<PriceScale> {
    let site = simulation.settlements.get(province)?;
    let base = simulation.catalog.goods.get(good.0)?.price;
    PriceScale::new(
        *site.market.price.values().get(good.0)?,
        base,
        simulation.catalog.config.market_config.base_price_deviation,
    )
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::{default_data_path, simulation::Simulation};
    use anyhow::Result;
    #[test]
    fn price_scale_preserves_configured_denominator_float_mix_and_unclamped_ratios() {
        let low = PriceScale::new(0.0, 10.0, 0.75).unwrap();
        let high = PriceScale::new(17.5, 10.0, 0.75).unwrap();
        assert_eq!(low.color(), [194.0 / 255.0, 4.0 / 255.0, 4.0 / 255.0]);
        assert_eq!(high.color(), [37.0 / 255.0, 243.0 / 255.0, 7.0 / 255.0]);
        let custom = PriceScale::new(5.0, 10.0, -0.25).unwrap();
        assert_eq!(custom.maximum, 7.5);
        assert_eq!(custom.ratio, 2.0 / 3.0);
        assert!(PriceScale::new(30.0, 10.0, 0.75).unwrap().color()[0] < 0.0);
        for (price, base, deviation) in [
            (0.0, 0.0, 0.75),
            (1.0, 1.0, -1.0),
            (f64::NAN, 1.0, 0.75),
            (1.0, 1.0, f64::INFINITY),
            (1e100, 1.0, 0.0),
        ] {
            assert!(PriceScale::new(price, base, deviation).is_none());
        }
    }
    #[test]
    fn loaded_province_and_domestic_tints_preserve_asset_colors_ocean_and_market_binding()
    -> Result<()> {
        let world = World::load(&default_data_path())?;
        let mut simulation = Simulation::load(&default_data_path(), &world, 42)?;
        let id = world
            .province_at_uv(0.5 + 77.2 / 360.0, 0.5 - 28.6 / 180.0)
            .unwrap();
        let player = world.provinces[id].country;
        let pixel = (0..world.height)
            .flat_map(|y| (0..world.width).map(move |x| (x, y)))
            .find(|(x, y)| {
                world.province_at_uv(
                    *x as f32 / world.width as f32,
                    *y as f32 / world.height as f32,
                ) == Some(id)
            })
            .unwrap();
        let offset = ((pixel.1 * world.width + pixel.0) * 4) as usize;
        let colors = Mode::Province.texture(&world, &simulation, None, player, GoodId(0));
        let [r, g, b] = world.provinces[id].color.unwrap();
        assert_eq!(&colors[offset..offset + 4], &[r, g, b, 166]);
        let domestic = Mode::Domestic.texture(&world, &simulation, None, player, GoodId(0));
        assert_eq!(&domestic[offset..offset + 4], &colors[offset..offset + 4]);
        let foreign = Mode::Domestic.texture(&world, &simulation, None, None, GoodId(0));
        let [r, g, b] = world.countries[player.unwrap()].color;
        assert_eq!(&foreign[offset..offset + 4], &[r, g, b, 166]);
        let ocean = (world.height / 2 * world.width + world.width / 4) as usize * 4;
        assert_eq!(&colors[ocean..ocean + 4], &[0; 4]);
        assert!(
            Mode::Terrain
                .texture(&world, &simulation, None, player, GoodId(0))
                .iter()
                .all(|byte| *byte == 0)
        );
        let good = simulation.catalog.good("steel").unwrap();
        simulation.settlements[id].market.price[good] = 0.0;
        let before = Mode::Price.texture(&world, &simulation, None, player, good);
        simulation.settlements[id].market.price[good] = simulation.catalog.goods[good.0].price
            * (1.0 + simulation.catalog.config.market_config.base_price_deviation);
        let after = Mode::Price.texture(&world, &simulation, None, player, good);
        assert_ne!(&before[offset..offset + 4], &after[offset..offset + 4]);
        assert!(price_scale(&simulation, id, GoodId(usize::MAX)).is_none());
        Ok(())
    }
}
