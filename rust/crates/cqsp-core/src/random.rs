//! The original `StdRandom` stream with Visual Studio 2026's distribution rules.
//! Distributions are recreated per call, so the second polar sample is discarded.
#[derive(Clone, Debug)]
pub struct LegacyRandom {
    state: [u32; 624],
    index: usize,
}
impl LegacyRandom {
    /// C++ accepts an int seed and initializes the 32-bit MT19937 engine.
    pub fn new(seed: u64) -> Self {
        let mut state = [0; 624];
        state[0] = seed as u32;
        for index in 1..state.len() {
            let previous = state[index - 1];
            state[index] = 1_812_433_253_u32
                .wrapping_mul(previous ^ (previous >> 30))
                .wrapping_add(index as u32);
        }
        Self { state, index: 624 }
    }
    pub fn next_u32(&mut self) -> u32 {
        if self.index == self.state.len() {
            for index in 0..self.state.len() {
                let bits = (self.state[index] & 0x8000_0000)
                    | (self.state[(index + 1) % 624] & 0x7fff_ffff);
                self.state[index] = self.state[(index + 397) % 624]
                    ^ (bits >> 1)
                    ^ if bits & 1 == 0 { 0 } else { 0x9908_b0df };
            }
            self.index = 0;
        }
        let mut value = self.state[self.index];
        self.index += 1;
        value ^= value >> 11;
        value ^= (value << 7) & 0x9d2c_5680;
        value ^= (value << 15) & 0xefc6_0000;
        value ^ (value >> 18)
    }
    /// MSVC generate_canonical<double, 53>: low word first, discard eleven bits.
    pub fn canonical(&mut self) -> f64 {
        let low = self.next_u32() as u64 >> 11;
        let high = (self.next_u32() as u64) << 21;
        (low + high) as f64 * (1.0 / ((1_u64 << 53) as f64))
    }
    pub fn int(&mut self, min: i32, max: i32) -> i32 {
        assert!(min <= max, "Invalid random integer bounds");
        let range = (max as i64 - min as i64 + 1) as u64;
        if range == 1_u64 << 32 {
            return self.next_u32().wrapping_add(min as u32) as i32;
        }
        let threshold = (1_u64 << 32) % range;
        loop {
            let product = self.next_u32() as u64 * range;
            if (product as u32 as u64) >= threshold {
                return (min as i64 + (product >> 32) as i64) as i32;
            }
        }
    }
    pub fn normal(&mut self, mean: f64, deviation: f64) -> f64 {
        assert!(mean.is_finite() && deviation.is_finite() && deviation > 0.0);
        loop {
            let x = 2.0 * self.canonical() - 1.0;
            let y = 2.0 * self.canonical() - 1.0;
            if x * x + y * y < 1.0 && x != 0.0 && y != 0.0 {
                return Self::polar(x, y) * deviation + mean;
            }
        }
    }
    pub fn normal_int(&mut self, mean: f64, deviation: f64) -> i32 {
        self.normal(mean, deviation).round() as i32
    }
    fn polar(mut x: f64, mut y: f64) -> f64 {
        let mut squared = x * x + y * y;
        let logarithm = if squared > 1e-4 {
            squared.ln()
        } else {
            // Match MSVC's ilogb/scalbn branch instead of losing tiny polar inputs.
            let exponent = ((x.abs().max(y.abs()).to_bits() >> 52) & 0x7ff) as i32 - 1023;
            let scale = 2_f64.powi(-exponent);
            x *= scale;
            y *= scale;
            squared = x * x + y * y;
            squared.ln() + exponent as f64 * (std::f64::consts::LN_2 * 2.0)
        };
        (-2.0 * logarithm / squared).sqrt() * x
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn mt19937_known_words_and_32_bit_seed_conversion() {
        let mut random = LegacyRandom::new(5489);
        assert_eq!(
            std::array::from_fn::<_, 10, _>(|_| random.next_u32()),
            [
                3_499_211_612,
                581_869_302,
                3_890_346_734,
                3_586_334_585,
                545_404_204,
                4_161_255_391,
                3_922_919_429,
                949_333_985,
                2_715_962_298,
                1_323_567_403,
            ]
        );
        let mut low = LegacyRandom::new(42);
        let mut high = LegacyRandom::new((1_u64 << 32) + 42);
        for _ in 0..2000 {
            assert_eq!(low.next_u32(), high.next_u32());
        }
    }
    #[test]
    fn calls_consume_the_original_stream_without_caching_a_second_normal() {
        let mut random = LegacyRandom::new(42);
        let first = random.normal(0.0, 1.0);
        let second = random.normal(0.0, 1.0);
        let mut replay = LegacyRandom::new(42);
        assert_eq!(first, replay.normal(0.0, 1.0));
        assert_eq!(second, replay.normal(0.0, 1.0));
        assert_ne!(first, second);
        for _ in 0..1000 {
            assert!((0.0..1.0).contains(&random.canonical()));
            assert!((-13..=27).contains(&random.int(-13, 27)));
            assert_eq!(random.int(-5, -5), -5);
            assert!(random.normal(10.0, 0.1).is_finite());
        }
    }
    #[test]
    fn tiny_polar_inputs_use_the_scaled_branch() {
        for (x, y) in [(0.001_f64, -0.002), (2_f64.powi(-52), 2_f64.powi(-50))] {
            let squared = x * x + y * y;
            let expected = (-2.0 * squared.ln() / squared).sqrt() * x;
            assert!((LegacyRandom::polar(x, y) - expected).abs() < 1e-14);
        }
    }
}
