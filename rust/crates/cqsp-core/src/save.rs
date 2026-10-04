//! Original metadata-only saves. No economy, orbit, country or RNG snapshot exists upstream.
use crate::{clock::StarDate, random::LegacyRandom};
use anyhow::{Context, Result, bail, ensure};
use serde::Serialize;
use serde_json::Value;
use std::{
    fs,
    io::Write,
    path::{Path, PathBuf},
    time::{SystemTime, UNIX_EPOCH},
};

pub const ORIGINAL_VERSION: &str = "0.0.0";

#[derive(Clone, Debug, PartialEq, Eq, Serialize)]
pub struct Metadata {
    pub date: i64,
    pub uuid: String,
    pub version: String,
}
impl Metadata {
    pub fn new(date: StarDate, uuid: String) -> Self {
        Self {
            date: date.tick() as i64,
            uuid,
            version: ORIGINAL_VERSION.into(),
        }
    }
    pub fn parse(value: &Value) -> Result<Self> {
        ensure!(value.is_object(), "Save metadata must be an object");
        let value_date = value["date"]
            .as_number()
            .context("Save date must be numeric")?;
        let date = if let Some(date) = value_date.as_i64() {
            date
        } else {
            let date = value_date.as_f64().context("Invalid save date")?;
            ensure!(
                date >= i64::MIN as f64 && date < -(i64::MIN as f64),
                "Save date conversion is out of range"
            );
            date as i64
        };
        let uuid = match value.get("uuid") {
            None => String::new(),
            Some(Value::String(value)) => value.clone(),
            Some(Value::Null) => "null".into(),
            Some(Value::Bool(value)) => value.to_string(),
            Some(Value::Number(value)) if value.is_i64() || value.is_u64() => value.to_string(),
            _ => bail!("Invalid save UUID type"),
        };
        Ok(Self {
            date: i64::from(date as u32),
            uuid,
            version: value["version"].as_str().unwrap_or_default().into(),
        })
    }
    /// LoadMetadata changes just date and UUID; it deliberately ignores the version.
    pub fn apply(&self, date: &mut StarDate, uuid: &mut String) {
        *date = StarDate::from_tick(u64::from(self.date as u32));
        uuid.clone_from(&self.uuid);
    }
}

/// Keep identity generation separate from the simulation's deterministic random stream.
pub fn new_uuid() -> String {
    let time = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .unwrap_or_default()
        .as_nanos();
    let mut random = LegacyRandom::new(time as u64 ^ u64::from(std::process::id()));
    (0..6)
        .map(|_| b"0123456789abcdef"[random.int(0, 15) as usize] as char)
        .collect()
}

#[derive(Debug)]
pub struct Entry {
    /// The original load screen displays the whole folder name as its country label.
    pub name: String,
    pub date: i32,
    pub path: PathBuf,
}
pub struct SaveStore(pub PathBuf);
impl SaveStore {
    pub fn write(&self, country: &str, metadata: &Metadata) -> Result<PathBuf> {
        ensure!(
            safe_name(country) && safe_name(&metadata.uuid),
            "Unsafe save folder identity"
        );
        let directory = self.0.join(format!("{country}_{}", metadata.uuid));
        fs::create_dir_all(&directory).context("Create save directory")?;
        let path = directory.join("meta.hjson");
        // JSON is valid HJSON; retain only the three original metadata fields.
        let pending = directory.join(format!("meta-{}.tmp", new_uuid()));
        let mut file = fs::OpenOptions::new()
            .write(true)
            .create_new(true)
            .open(&pending)
            .context("Create pending metadata")?;
        let result = (|| -> Result<()> {
            file.write_all(serde_json::to_string_pretty(metadata)?.as_bytes())?;
            file.sync_all()?;
            drop(file);
            fs::rename(&pending, &path).context("Replace metadata")?;
            Ok(())
        })();
        if result.is_err() {
            let _ = fs::remove_file(&pending);
        }
        result.context("Write save metadata")?;
        Ok(path)
    }
    pub fn read(directory: &Path) -> Result<Metadata> {
        Metadata::parse(&read_value(&directory.join("meta.hjson"))?)
    }
    pub fn list(&self) -> Result<Vec<Entry>> {
        if !self.0.exists() {
            return Ok(Vec::new());
        }
        let mut entries = Vec::new();
        for entry in fs::read_dir(&self.0).context("List saves")? {
            let entry = entry?;
            if !entry.file_type()?.is_dir() {
                continue;
            }
            let path = entry.path();
            let metadata_path = path.join("meta.hjson");
            if !metadata_path.is_file() {
                continue;
            }
            let mut value = read_value(&metadata_path)?;
            if value.get("date").is_none() {
                value["date"] = (-1).into();
            }
            let metadata = Metadata::parse(&value)?;
            entries.push(Entry {
                name: entry.file_name().to_string_lossy().into_owned(),
                date: metadata.date as i32,
                path,
            });
        }
        entries.sort_by(|a, b| a.name.cmp(&b.name));
        Ok(entries)
    }
}
fn safe_name(value: &str) -> bool {
    !value.is_empty()
        && value
            .bytes()
            .all(|byte| byte.is_ascii_alphanumeric() || matches!(byte, b'_' | b'-'))
}
fn read_value(path: &Path) -> Result<Value> {
    serde_hjson::from_str(
        &fs::read_to_string(path).with_context(|| format!("Read {}", path.display()))?,
    )
    .context("Parse save metadata")
}

#[cfg(test)]
mod tests {
    use super::*;
    use serde_json::json;
    #[test]
    fn metadata_preserves_wrapping_defaults_and_changes_no_other_state() -> Result<()> {
        for (value, expected) in [
            (json!({"date":-1}), u32::MAX),
            (json!({"date":4294967297_u64}), 1),
            (json!({"date":143.75}), 143),
            (json!({"date":144}), 144),
        ] {
            let metadata = Metadata::parse(&value)?;
            assert_eq!(metadata.date, i64::from(expected));
            let mut date = StarDate::default();
            let mut uuid = "old".into();
            metadata.apply(&mut date, &mut uuid);
            assert_eq!(date.tick(), u64::from(expected));
            assert_eq!(uuid, "");
        }
        assert!(Metadata::parse(&json!({"date":[]})).is_err());
        for value in [
            json!({}),
            json!({"date":null}),
            json!({"date":true}),
            json!({"date":"144"}),
            json!({"date":"invalid"}),
            json!({"date":1e30}),
        ] {
            assert!(Metadata::parse(&value).is_err());
        }
        assert!(Metadata::parse(&json!({"uuid":{}})).is_err());
        let metadata =
            Metadata::parse(&json!({"date":144,"uuid":"abcdef","version":"unrecognized"}))?;
        let mut date = StarDate::default();
        let mut uuid = "old".into();
        metadata.apply(&mut date, &mut uuid);
        assert_eq!((date.tick(), uuid.as_str()), (144, "abcdef"));
        assert_eq!(Metadata::new(date, uuid).version, ORIGINAL_VERSION);
        assert_eq!(Metadata::new(StarDate::default(), "abcdef".into()).date, -1);
        assert!(new_uuid().bytes().all(|byte| byte.is_ascii_hexdigit()));
        assert_eq!(new_uuid().len(), 6);
        Ok(())
    }
    #[test]
    fn store_writes_only_metadata_and_lists_only_save_folders() -> Result<()> {
        let directory = std::env::temp_dir().join(format!(
            "cqsp-save-test-{}-{}",
            std::process::id(),
            SystemTime::now().duration_since(UNIX_EPOCH)?.as_nanos()
        ));
        let store = SaveStore(directory.clone());
        assert!(store.list()?.is_empty());
        let mut metadata = Metadata::new(StarDate::from_tick(144), "abcdef".into());
        let path = store.write("usa", &metadata)?;
        assert_eq!(fs::read_dir(path.parent().unwrap())?.count(), 1);
        assert_eq!(SaveStore::read(path.parent().unwrap())?, metadata);
        metadata.date = 288;
        store.write("usa", &metadata)?;
        assert_eq!(SaveStore::read(path.parent().unwrap())?.date, 288);
        fs::create_dir(directory.join("no_metadata"))?;
        fs::write(directory.join("not_a_directory"), "")?;
        let entries = store.list()?;
        assert_eq!(entries.len(), 1);
        assert_eq!(entries[0].name, "usa_abcdef");
        assert_eq!(entries[0].date, 288);
        assert_eq!(entries[0].path, path.parent().unwrap());
        assert!(store.write("../outside", &metadata).is_err());
        metadata.uuid = "../../outside".into();
        assert!(store.write("usa", &metadata).is_err());
        let unknown = directory.join("missing_date");
        fs::create_dir(&unknown)?;
        fs::write(unknown.join("meta.hjson"), "{}")?;
        assert!(SaveStore::read(&unknown).is_err());
        assert_eq!(
            store
                .list()?
                .iter()
                .find(|entry| entry.name == "missing_date")
                .unwrap()
                .date,
            -1
        );
        fs::remove_dir_all(&directory)?; // Unique test-owned temporary directory only.
        Ok(())
    }
}
