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

#[derive(Clone, Copy, Debug, PartialEq)]
pub(super) struct View {
    pub first: f64,
    pub last: f64,
    pub low: f64,
    pub high: f64,
}
impl View {
    pub fn fit(points: &[(usize, f64)]) -> Option<Self> {
        let bounds = bounds(points)?;
        let (first, last) = if bounds.first == bounds.last {
            (bounds.first as f64 - 0.5, bounds.last as f64 + 0.5)
        } else {
            (bounds.first as f64, bounds.last as f64)
        };
        let (low, high) = if bounds.low == bounds.high {
            let margin = (bounds.low.abs() * 0.1).max(1.0);
            (
                (bounds.low - margin).max(-f64::MAX),
                (bounds.high + margin).min(f64::MAX),
            )
        } else {
            (bounds.low, bounds.high)
        };
        Some(Self {
            first,
            last,
            low,
            high,
        })
    }
    pub fn zoom(self, anchor: (f64, f64), factor: f64) -> Self {
        self.transform(
            (
                anchor.0 * (1.0 - factor),
                anchor.0 + (1.0 - anchor.0) * factor,
            ),
            (
                anchor.1 * (1.0 - factor),
                anchor.1 + (1.0 - anchor.1) * factor,
            ),
        )
    }
    pub fn pan(self, delta: (f64, f64)) -> Self {
        self.transform((-delta.0, 1.0 - delta.0), (-delta.1, 1.0 - delta.1))
    }
    pub fn value_at(self, position: (f64, f64)) -> (f64, f64) {
        (
            mix(self.first, self.last, position.0),
            mix(self.low, self.high, position.1),
        )
    }
    fn transform(self, x: (f64, f64), y: (f64, f64)) -> Self {
        let next = Self {
            first: mix(self.first, self.last, x.0),
            last: mix(self.first, self.last, x.1),
            low: mix(self.low, self.high, y.0),
            high: mix(self.low, self.high, y.1),
        };
        let scale = next.low.abs().max(next.high.abs()).max(1.0);
        if [next.first, next.last, next.low, next.high]
            .into_iter()
            .all(f64::is_finite)
            && next.last - next.first >= 1e-6
            && next.high / scale - next.low / scale >= 1e-15
        {
            next
        } else {
            self
        }
    }
    fn position(self, point: (usize, f64)) -> (f64, f64) {
        let scale = self.low.abs().max(self.high.abs()).max(1.0);
        (
            ((point.0 as f64 - self.first) / (self.last - self.first)).clamp(-1e12, 1e12),
            ((point.1 / scale - self.low / scale) / (self.high / scale - self.low / scale))
                .clamp(-1e12, 1e12),
        )
    }
}
fn mix(low: f64, high: f64, amount: f64) -> f64 {
    low * (1.0 - amount) + high * amount
}
pub(super) fn plot_position(image_position: (f64, f64)) -> (f64, f64) {
    (
        (image_position.0 * WIDTH as f64 - LEFT as f64) / (RIGHT - LEFT) as f64,
        (BOTTOM as f64 - image_position.1 * HEIGHT as f64) / (BOTTOM - TOP) as f64,
    )
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
pub(super) fn render(points: &[(usize, f64)], view: Option<View>) -> Vec<u8> {
    let mut pixels = vec![[4, 15, 29, 255]; (WIDTH * HEIGHT) as usize];
    for step in 0..=4 {
        let x = LEFT + (RIGHT - LEFT) * step / 4;
        let y = TOP + (BOTTOM - TOP) * step / 4;
        line(&mut pixels, (LEFT, y), (RIGHT, y), [40, 55, 75, 255]);
        line(&mut pixels, (x, TOP), (x, BOTTOM), [40, 55, 75, 255]);
    }
    if let Some(view) = view.or_else(|| View::fit(points)) {
        let pixel_position = |(x, y): (f64, f64)| {
            (
                LEFT + (x * (RIGHT - LEFT) as f64).round() as i32,
                BOTTOM - (y * (BOTTOM - TOP) as f64).round() as i32,
            )
        };
        for pair in points.windows(2) {
            if pair.iter().all(|(_, value)| value.is_finite())
                && let Some((start, end)) = clip(view.position(pair[0]), view.position(pair[1]))
            {
                line(
                    &mut pixels,
                    pixel_position(start),
                    pixel_position(end),
                    LINE,
                );
            }
        }
        for &point in points.iter().filter(|(_, value)| value.is_finite()) {
            let position = view.position(point);
            if !(0.0..=1.0).contains(&position.0) || !(0.0..=1.0).contains(&position.1) {
                continue;
            }
            let (x, y) = pixel_position(position);
            for dx in -1..=1 {
                for dy in -1..=1 {
                    pixel(&mut pixels, x + dx, y + dy, LINE);
                }
            }
        }
    }
    pixels.into_iter().flatten().collect()
}
fn clip(start: (f64, f64), end: (f64, f64)) -> Option<((f64, f64), (f64, f64))> {
    let (dx, dy) = (end.0 - start.0, end.1 - start.1);
    let (mut first, mut last) = (0.0_f64, 1.0_f64);
    for (direction, distance) in [
        (-dx, start.0),
        (dx, 1.0 - start.0),
        (-dy, start.1),
        (dy, 1.0 - start.1),
    ] {
        if direction == 0.0 {
            if distance < 0.0 {
                return None;
            }
            continue;
        }
        let amount = distance / direction;
        if direction < 0.0 {
            first = first.max(amount);
        } else {
            last = last.min(amount);
        }
        if first > last {
            return None;
        }
    }
    Some((
        (start.0 + dx * first, start.1 + dy * first),
        (start.0 + dx * last, start.1 + dy * last),
    ))
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
        assert_eq!(render(&[], None).len(), (WIDTH * HEIGHT * 4) as usize);
        assert_eq!(count(&render(&[], None)), 0);
        assert_eq!(count(&render(&[(0, 7.0)], None)), 9);
        let pixels = render(&[(0, 7.0), (31, 7.0)], None);
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
        assert!(count(&render(&points, None)) > 200);
        let gaps = render(&[(0, 0.0), (1, f64::INFINITY), (2, 0.0)], None);
        assert!(count(&gaps) < 20);
        assert!(bounds(&[(0, f64::NAN), (1, f64::NEG_INFINITY)]).is_none());
    }
    #[test]
    fn zoom_keeps_the_cursor_anchor_and_pan_moves_both_axes() {
        let view = View::fit(&[(0, -10.0), (100, 30.0)]).unwrap();
        let anchor = (0.25, 0.75);
        let next = view.zoom(anchor, 0.5);
        assert_eq!(next.value_at(anchor), view.value_at(anchor));
        assert_eq!((next.last - next.first, next.high - next.low), (50.0, 20.0));
        let shifted = view.pan((0.1, -0.25));
        assert_eq!(
            (shifted.first, shifted.last, shifted.low, shifted.high),
            (-10.0, 90.0, 0.0, 40.0)
        );
        let restored = shifted.pan((-0.1, 0.25));
        assert!((restored.first - view.first).abs() < 1e-12);
        assert!((restored.last - view.last).abs() < 1e-12);
        assert!((restored.low - view.low).abs() < 1e-12);
        assert!((restored.high - view.high).abs() < 1e-12);
        assert_eq!(view.zoom(anchor, 0.0), view);
        assert_eq!(view.pan((f64::NAN, 0.0)), view);
    }
    #[test]
    fn clipping_keeps_offscreen_segments_bounded_and_preserves_gaps() {
        let view = View {
            first: 10.0,
            last: 20.0,
            low: 0.0,
            high: 10.0,
        };
        assert!(count(&render(&[(0, 5.0), (30, 5.0)], Some(view))) > 900);
        assert_eq!(
            count(&render(&[(0, -1e308), (usize::MAX, -1e308)], Some(view))),
            0
        );
        assert_eq!(
            count(&render(&[(0, 5.0), (15, f64::NAN), (30, 5.0)], Some(view))),
            0
        );
        let extreme = View::fit(&[(0, -1e308), (1, 1e308)]).unwrap();
        assert_eq!(
            extreme.zoom((0.5, 0.5), 0.5).value_at((0.5, 0.5)),
            (0.5, 0.0)
        );
        assert!(clip((-1e12, 0.5), (1e12, 0.5)).is_some());
    }
}
