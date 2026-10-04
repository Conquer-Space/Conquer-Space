//! Small native history plot rasterizer; non-finite samples leave visible gaps.
pub(super) const WIDTH: u32 = 960;
pub(super) const HEIGHT: u32 = 240;
pub(super) const LINE: [u8; 4] = [60, 220, 190, 255];
const LEFT: i32 = 24;
const TOP: i32 = 12;
const RIGHT: i32 = WIDTH as i32 - 12;
const BOTTOM: i32 = HEIGHT as i32 - 18;

#[derive(Clone, Copy, Debug)]
pub(super) struct Bounds {
    pub first: usize,
    pub last: usize,
    pub low: f64,
    pub high: f64,
}
pub(super) fn bounds(points: &[(usize, f64)]) -> Option<Bounds> {
    let mut valid = points
        .iter()
        .copied()
        .filter(|(_, value)| value.is_finite());
    let (sample, value) = valid.next()?;
    Some(valid.fold(
        Bounds {
            first: sample,
            last: sample,
            low: value,
            high: value,
        },
        |mut bounds, (sample, value)| {
            bounds.first = bounds.first.min(sample);
            bounds.last = bounds.last.max(sample);
            bounds.low = bounds.low.min(value);
            bounds.high = bounds.high.max(value);
            bounds
        },
    ))
}
pub(super) fn render(points: &[(usize, f64)]) -> Vec<u8> {
    let mut pixels = vec![[4, 15, 29, 255]; (WIDTH * HEIGHT) as usize];
    for step in 0..=4 {
        let x = LEFT + (RIGHT - LEFT) * step / 4;
        let y = TOP + (BOTTOM - TOP) * step / 4;
        line(&mut pixels, (LEFT, y), (RIGHT, y), [40, 55, 75, 255]);
        line(&mut pixels, (x, TOP), (x, BOTTOM), [40, 55, 75, 255]);
    }
    if let Some(bounds) = bounds(points) {
        let position = |(sample, value): (usize, f64)| {
            let x = if bounds.first == bounds.last {
                0.5
            } else {
                (sample - bounds.first) as f64 / (bounds.last - bounds.first) as f64
            };
            // Scaling before subtraction also handles large positive/negative finite values.
            let scale = bounds.low.abs().max(bounds.high.abs()).max(1.0);
            let y = if bounds.low == bounds.high {
                0.5
            } else {
                (value / scale - bounds.low / scale) / (bounds.high / scale - bounds.low / scale)
            };
            (
                LEFT + (x * (RIGHT - LEFT) as f64).round() as i32,
                BOTTOM - (y * (BOTTOM - TOP) as f64).round() as i32,
            )
        };
        for pair in points.windows(2) {
            if pair.iter().all(|(_, value)| value.is_finite()) {
                line(&mut pixels, position(pair[0]), position(pair[1]), LINE);
            }
        }
        for &point in points.iter().filter(|(_, value)| value.is_finite()) {
            let (x, y) = position(point);
            for dx in -1..=1 {
                for dy in -1..=1 {
                    pixel(&mut pixels, x + dx, y + dy, LINE);
                }
            }
        }
    }
    pixels.into_iter().flatten().collect()
}
fn pixel(pixels: &mut [[u8; 4]], x: i32, y: i32, color: [u8; 4]) {
    if (0..WIDTH as i32).contains(&x) && (0..HEIGHT as i32).contains(&y) {
        pixels[(y as u32 * WIDTH + x as u32) as usize] = color;
    }
}
fn line(pixels: &mut [[u8; 4]], start: (i32, i32), end: (i32, i32), color: [u8; 4]) {
    let (mut x, mut y) = start;
    let (dx, dy) = ((end.0 - x).abs(), -(end.1 - y).abs());
    let (sx, sy) = (
        if x < end.0 { 1 } else { -1 },
        if y < end.1 { 1 } else { -1 },
    );
    let mut error = dx + dy;
    loop {
        pixel(pixels, x, y, color);
        if (x, y) == end {
            break;
        }
        let doubled = 2 * error;
        if doubled >= dy {
            error += dy;
            x += sx;
        }
        if doubled <= dx {
            error += dx;
            y += sy;
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    fn count(pixels: &[u8]) -> usize {
        pixels
            .as_chunks::<4>()
            .0
            .iter()
            .filter(|pixel| **pixel == LINE)
            .count()
    }
    #[test]
    fn empty_constant_and_single_series_are_bounded_and_visible() {
        assert!(bounds(&[]).is_none());
        assert_eq!(render(&[]).len(), (WIDTH * HEIGHT * 4) as usize);
        assert_eq!(count(&render(&[])), 0);
        assert_eq!(count(&render(&[(0, 7.0)])), 9);
        let pixels = render(&[(0, 7.0), (31, 7.0)]);
        assert!(count(&pixels) >= (RIGHT - LEFT) as usize);
        assert!(
            pixels
                .as_chunks::<4>()
                .0
                .iter()
                .all(|pixel| pixel[3] == 255)
        );
    }
    #[test]
    fn gaps_are_not_bridged_and_extreme_values_remain_in_the_plot() {
        let points = [(64, -1e308), (65, 1e308), (66, f64::NAN), (67, 0.0)];
        let range = bounds(&points).unwrap();
        assert_eq!(
            (range.first, range.last, range.low, range.high),
            (64, 67, -1e308, 1e308)
        );
        assert!(count(&render(&points)) > 200);
        let gaps = render(&[(0, 0.0), (1, f64::INFINITY), (2, 0.0)]);
        assert!(count(&gaps) < 20);
        assert!(bounds(&[(0, f64::NAN), (1, f64::NEG_INFINITY)]).is_none());
    }
}
