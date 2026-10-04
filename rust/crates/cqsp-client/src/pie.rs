//! Native consumption pies: source float values, double normalization, 90-degree start.
use super::chart::View;
use std::f64::consts::{FRAC_PI_2, TAU};

pub(super) const SIZE: u32 = 240;
pub(super) const BACKGROUND: [u8; 4] = [4, 15, 29, 255];
pub(super) const FIT: View = View {
    first: 0.0,
    last: 1.0,
    low: 0.0,
    high: 1.0,
};
const COLORS: [[u8; 4]; 10] = [
    [76, 114, 176, 255],
    [221, 132, 82, 255],
    [85, 168, 104, 255],
    [196, 78, 82, 255],
    [129, 114, 179, 255],
    [147, 120, 96, 255],
    [218, 139, 195, 255],
    [140, 140, 140, 255],
    [204, 185, 116, 255],
    [100, 181, 205, 255],
];
pub(super) fn color(index: usize) -> [u8; 4] {
    COLORS[index % COLORS.len()]
}
pub(super) struct Pie {
    pub fractions: Vec<f64>,
}
impl Pie {
    pub fn new(values: &[f32]) -> Option<Self> {
        let sum: f64 = values.iter().map(|&value| value as f64).sum();
        if !sum.is_finite()
            || sum <= 0.0
            || values
                .iter()
                .any(|&value| !value.is_finite() || value < 0.0)
        {
            return None;
        }
        Some(Self {
            fractions: values.iter().map(|&value| value as f64 / sum).collect(),
        })
    }
    pub fn at(&self, position: (f64, f64)) -> Option<usize> {
        let (x, y) = (position.0 - 0.5, 0.5 - position.1);
        if !x.is_finite() || !y.is_finite() || x * x + y * y > 0.4_f64.powi(2) {
            return None;
        }
        let fraction = (y.atan2(x) - FRAC_PI_2).rem_euclid(TAU) / TAU;
        let mut end = 0.0;
        self.fractions.iter().position(|&size| {
            end += size;
            size > 0.0 && fraction < end
        })
    }
}
pub(super) fn render(values: &[f32], hidden: &[bool], view: View) -> Vec<u8> {
    let pie = Pie::new(values);
    (0..SIZE * SIZE)
        .flat_map(|pixel| {
            let (x, y) = view.value_at((
                (pixel % SIZE) as f64 / SIZE as f64 + 0.5 / SIZE as f64,
                1.0 - ((pixel / SIZE) as f64 + 0.5) / SIZE as f64,
            ));
            pie.as_ref()
                .and_then(|pie| pie.at((x, 1.0 - y)))
                .filter(|&index| !hidden.get(index).copied().unwrap_or(false))
                .map_or(BACKGROUND, color)
        })
        .collect()
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn float_values_normalize_in_double_precision_and_start_counterclockwise_at_top() {
        let values = [0.1_f32, 0.2_f32, 0.7_f32];
        let pie = Pie::new(&values).unwrap();
        let sum = values.iter().map(|&value| value as f64).sum::<f64>();
        assert_eq!(pie.fractions[0], values[0] as f64 / sum);
        let pie = Pie::new(&[1.0, 3.0]).unwrap();
        assert_eq!(pie.at((0.4, 0.2)), Some(0));
        assert_eq!(pie.at((0.2, 0.6)), Some(1));
        assert_eq!(pie.at((0.8, 0.4)), Some(1));
        assert_eq!(pie.at((0.0, 0.0)), None);
        assert_eq!(pie.at((f64::NAN, 0.5)), None);
        assert_eq!(Pie::new(&[0.0, 1.0]).unwrap().at((0.4, 0.2)), Some(1));
    }
    #[test]
    fn hidden_slices_leave_their_original_angles_empty_without_renormalizing() {
        let visible = render(&[1.0, 3.0], &[], FIT);
        let hidden = render(&[1.0, 3.0], &[true, false], FIT);
        let count = |pixels: &[u8], shade: [u8; 4]| {
            pixels
                .as_chunks::<4>()
                .0
                .iter()
                .filter(|&&pixel| pixel == shade)
                .count()
        };
        assert!(count(&visible, color(0)) > 7000);
        assert_eq!(count(&hidden, color(0)), 0);
        assert_eq!(count(&visible, color(1)), count(&hidden, color(1)));
        assert_eq!(visible.len(), (SIZE * SIZE * 4) as usize);
    }
    #[test]
    fn empty_zero_negative_and_non_finite_ledgers_do_not_generate_invalid_geometry() {
        for values in [
            vec![],
            vec![0.0],
            vec![-1.0, 2.0],
            vec![f32::NAN],
            vec![f32::INFINITY],
        ] {
            assert!(Pie::new(&values).is_none());
            assert!(
                render(&values, &[], FIT)
                    .as_chunks::<4>()
                    .0
                    .iter()
                    .all(|&pixel| pixel == BACKGROUND)
            );
        }
        assert!(Pie::new(&[f32::MAX, f32::MAX]).is_some());
    }
    #[test]
    fn equal_axis_zoom_pan_and_far_offscreen_pies_stay_bounded() {
        let count = |view| {
            render(&[1.0], &[], view)
                .as_chunks::<4>()
                .0
                .iter()
                .filter(|&&pixel| pixel != BACKGROUND)
                .count()
        };
        let zoomed = FIT.zoom((0.5, 0.5), 0.5);
        assert_eq!(zoomed.last - zoomed.first, zoomed.high - zoomed.low);
        assert!(count(zoomed) > count(FIT));
        assert_eq!(count(FIT.pan((10.0, 10.0))), 0);
        assert_eq!(count(FIT.pan((f64::NAN, 0.0))), count(FIT));
    }
}
