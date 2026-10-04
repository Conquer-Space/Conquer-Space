//! Source `NumberToHumanString` suffixes, rounding and fmt-style default doubles.
pub fn human_integer(value: i64) -> String {
    scaled(value as f64).unwrap_or_else(|| value.to_string())
}

pub fn human_float(value: f64) -> String {
    scaled(value).unwrap_or_else(|| float(value))
}

fn scaled(value: f64) -> Option<String> {
    const SUFFIXES: [&str; 17] = [
        "k", "M", "B", "T", "Qa", "Qn", "Sx", "Sp", "O", "N", "De", "Ud", "Dd", "Td", "Qd", "Qi",
        "Sd",
    ];
    if !value.is_finite() || value.abs() < 1000.0 {
        return None;
    }
    let exponent = (value.abs().log10() / 3.0) as usize;
    // Preserve the source's unused last suffix, not a saturating suffix lookup.
    if exponent >= SUFFIXES.len() {
        return None;
    }
    let rounded = (value / 10_f64.powi(exponent as i32 * 3) * 100.0).round() / 100.0;
    Some(format!("{} {}", float(rounded), SUFFIXES[exponent - 1]))
}

pub fn float(value: f64) -> String {
    if value.is_nan() {
        return "nan".into();
    }
    if value.is_finite() && value != 0.0 {
        let scientific = format!("{value:e}");
        let (mantissa, exponent) = scientific.split_once('e').unwrap();
        let exponent: i32 = exponent.parse().unwrap();
        if !(-4..16).contains(&exponent) {
            return format!("{mantissa}e{exponent:+03}");
        }
    }
    value.to_string()
}

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn source_suffixes_round_without_fixed_decimal_padding() {
        for (value, expected) in [
            (999, "999"),
            (1000, "1 k"),
            (1235, "1.24 k"),
            (-1235, "-1.24 k"),
            (999_999, "1000 k"),
            (1_000_000, "1 M"),
            (1_000_000_000, "1 B"),
        ] {
            assert_eq!(human_integer(value), expected);
        }
        assert_eq!(human_float(1e48), "1 Qi");
        assert_eq!(human_float(1e51), "1e+51");
        assert_eq!(human_float(0.00001), "1e-05");
        assert_eq!(human_float(0.0001), "0.0001");
        assert_eq!(human_float(-0.0), "-0");
    }
}
