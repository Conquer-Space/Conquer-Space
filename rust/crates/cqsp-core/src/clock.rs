//! `src/core/components/stardate.*`: ten-minute ticks, starting on 1950-01-01.
use serde::{Deserialize, Serialize};

pub const SECONDS_PER_TICK: u64 = 600;
pub const HOUR: u64 = 6;
pub const DAY: u64 = 24 * HOUR;
pub const WEEK: u64 = 7 * DAY;

/// Original client PauseOptions/ManageTick: float millisecond intervals and one batch per frame.
#[derive(Clone, Copy, Debug)]
pub struct PlaybackSpeed(i32);
impl PlaybackSpeed {
    pub const DEFAULT_INDEX: usize = 3;
    pub const ALL: [Self; 11] = [
        Self(1000),
        Self(500),
        Self(333),
        Self(100),
        Self(50),
        Self(10),
        Self(1),
        Self(-1),
        Self(-2),
        Self(-5),
        Self(-10),
    ];
    pub fn milliseconds(self) -> i32 {
        self.0
    }
    pub fn interval(self) -> f64 {
        f64::from(self.0.max(1) as f32 / 1000.0)
    }
    pub fn batch(self) -> usize {
        if self.0 < 0 {
            self.0.unsigned_abs() as usize
        } else {
            1
        }
    }
    pub fn ticks_after(self, elapsed: f64) -> usize {
        if elapsed.is_finite() && elapsed > self.interval() {
            self.batch()
        } else {
            0
        }
    }
    pub fn label(self) -> String {
        format!(
            "{:.0} ms / {} tick{}",
            self.interval() * 1000.0,
            self.batch(),
            if self.batch() == 1 { "" } else { "s" }
        )
    }
}

#[derive(Clone, Copy, Debug, Serialize, Deserialize, PartialEq, Eq)]
pub struct StarDate {
    tick: u64,
}

impl Default for StarDate {
    fn default() -> Self {
        Self { tick: u64::MAX }
    }
}

impl StarDate {
    pub fn from_tick(tick: u64) -> Self {
        Self { tick }
    }
    pub fn tick(self) -> u64 {
        self.tick
    }
    pub fn advance(&mut self) {
        self.tick = self.tick.wrapping_add(1);
    }
    pub fn due(self, interval: u64) -> bool {
        self.tick != u64::MAX && self.tick.is_multiple_of(interval)
    }
    pub fn seconds(self) -> f64 {
        self.tick as f64 * SECONDS_PER_TICK as f64
    }
    pub fn days(self) -> f64 {
        self.tick as f64 / DAY as f64
    }
    pub fn ticks_from_seconds(seconds: f64) -> i32 {
        (seconds / SECONDS_PER_TICK as f64) as i32
    }
    pub fn hour(self, offset: f64) -> i32 {
        ((((self.tick.wrapping_mul(SECONDS_PER_TICK) / 3600) % 24) as f64 + offset) % 24.0) as i32
    }
    pub fn minute(self) -> u64 {
        (self.tick.wrapping_mul(SECONDS_PER_TICK) / 60) % 60
    }

    pub fn calendar(self) -> (u32, u32, u32) {
        let mut days = if self.tick == u64::MAX {
            0
        } else {
            self.tick / DAY
        };
        let mut year = 1950;
        // Gregorian years, not the approximate 365-day YEAR constant used by some systems.
        while days >= days_in_year(year) {
            days -= days_in_year(year);
            year += 1;
        }
        let months = [
            31,
            if leap_year(year) { 29 } else { 28 },
            31,
            30,
            31,
            30,
            31,
            31,
            30,
            31,
            30,
            31,
        ];
        let mut month = 1;
        for length in months {
            if days < length {
                break;
            }
            days -= length;
            month += 1;
        }
        (year, month, days as u32 + 1)
    }

    pub fn display(self) -> String {
        let (year, month, day) = self.calendar();
        format!("{year}-{month:02}-{day:02}")
    }
}

fn leap_year(year: u32) -> bool {
    year.is_multiple_of(4) && (!year.is_multiple_of(100) || year.is_multiple_of(400))
}
fn days_in_year(year: u32) -> u64 {
    if leap_year(year) { 366 } else { 365 }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn playback_preserves_float_threshold_strict_boundary_and_no_frame_catch_up() {
        assert_eq!(PlaybackSpeed::DEFAULT_INDEX, 3);
        for (speed, milliseconds) in PlaybackSpeed::ALL
            .into_iter()
            .zip([1000, 500, 333, 100, 50, 10, 1, -1, -2, -5, -10])
        {
            assert_eq!(speed.milliseconds(), milliseconds);
            let interval = f64::from(milliseconds.max(1) as f32 / 1000.0_f32);
            assert_eq!(speed.interval().to_bits(), interval.to_bits());
            assert_eq!(speed.ticks_after(interval), 0);
            assert_eq!(
                speed.ticks_after(f64::from_bits(interval.to_bits() + 1)),
                speed.batch()
            );
            assert_eq!(speed.ticks_after(1000.0), speed.batch());
            for elapsed in [0.0, -1.0, f64::NAN, f64::INFINITY] {
                assert_eq!(speed.ticks_after(elapsed), 0);
            }
        }
    }

    #[test]
    fn first_tick_is_zero_and_weekly_systems_are_due() {
        let mut date = StarDate::default();
        assert!(!date.due(DAY));
        date.advance();
        assert_eq!(date.tick(), 0);
        assert!(date.due(DAY) && date.due(WEEK));
        date.advance();
        assert!(!date.due(DAY));
    }

    #[test]
    fn civil_calendar_handles_leap_years_and_time() {
        assert_eq!(StarDate::from_tick(365 * DAY).calendar(), (1951, 1, 1));
        assert_eq!(
            StarDate::from_tick((365 * 2 + 59) * DAY).calendar(),
            (1952, 2, 29)
        );
        assert_eq!(
            StarDate::from_tick((365 * 2 + 60) * DAY).calendar(),
            (1952, 3, 1)
        );
        assert_eq!(StarDate::from_tick(143).hour(0.0), 23);
        assert_eq!(StarDate::from_tick(143).minute(), 50);
        assert_eq!(StarDate::from_tick(144).seconds(), 86400.0);
    }
}
