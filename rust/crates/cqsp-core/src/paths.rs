//! Packaged assets take precedence over development checkout paths, independent of cwd.
use std::path::{Path, PathBuf};

pub fn asset_directory(relative: &str, development: PathBuf) -> PathBuf {
    match std::env::current_exe() {
        Ok(exe) => resolve(&exe, relative, development),
        Err(_) => development,
    }
}

fn resolve(executable: &Path, relative: &str, development: PathBuf) -> PathBuf {
    executable
        .parent()
        .map(|directory| directory.join(relative))
        .filter(|directory| directory.is_dir())
        .unwrap_or(development)
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::{
        fs,
        time::{SystemTime, UNIX_EPOCH},
    };

    #[test]
    fn executable_relative_assets_override_the_development_checkout() {
        let parent = std::env::current_exe()
            .unwrap()
            .parent()
            .unwrap()
            .to_path_buf();
        let fixture = parent.join(format!(
            "asset-path-test-{}-{}",
            std::process::id(),
            SystemTime::now()
                .duration_since(UNIX_EPOCH)
                .unwrap()
                .as_nanos()
        ));
        let executable = fixture.join("game.exe");
        let development = fixture.join("development");
        assert_eq!(
            resolve(&executable, "assets/core", development.clone()),
            development
        );
        let packaged = fixture.join("assets/core");
        fs::create_dir_all(&packaged).unwrap();
        assert_eq!(
            resolve(&executable, "assets/core", development.clone()),
            packaged
        );
        assert_eq!(
            resolve(&executable, "assets/client", development.clone()),
            development
        );
        fs::remove_dir(packaged).unwrap();
        fs::remove_dir(fixture.join("assets")).unwrap();
        fs::remove_dir(fixture).unwrap();
    }
}
