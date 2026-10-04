# Conquer Space — Rust / Bevy port

This is the first runnable port milestone, not a complete replacement for the C++ game.
It loads the original HJSON country/province definitions and 2K Earth textures, displays
terrain and political maps with province borders, supports globe navigation and picking,
and selects a player country before entering the game view. There is no Lua runtime.
Economy, population, industry, trade, orbital simulation, saves, and the remaining UI
have not been ported yet. The existing CMake/Visual Studio project is unchanged.

## Build and run

Install stable Rust (MSVC) and Visual Studio's C++ tools/Windows SDK. From this directory:

```powershell
cargo run -p cqsp-client
```

If Rust is not on your PATH, use `& "$env:USERPROFILE/.cargo/bin/cargo.exe"` in place of
`cargo`. The bundled data is found independently of your working directory. To point
at another asset tree: `cargo run -p cqsp-client -- --data C:/path/to/binaries/data/core`.
The shader is loaded from this source checkout; this milestone is not a packaged release.

Right-drag or arrow keys orbit the globe; wheel zooms; left-click selects a province's
country. Tab/Shift+Tab cycle countries (including ones too small to click on the 2K map).
Enter or the sidebar button starts the game view; Escape returns to selection.
M toggles terrain/political maps, R resets the camera, F12 writes `target/conquer-space.png`.
Closing the window exits normally.

## Verification

```powershell
cargo test --workspace
cargo clippy --workspace --all-targets -- -D warnings
cargo run -p cqsp-client -- --smoke
```

The GPU smoke check casts a real camera ray into Delhi, asserts that India is selected,
enters the game view, writes `target/selection.png` and `target/game-view.png`, then exits.
Inspect both images and the console for shader/rendering errors; process success alone
does not prove a correct render. The core tests cover original data ingestion, picking,
longitude wrapping, ray intersections, and selection state transitions.

## Layout and next milestone

- `cqsp-core`: renderer-independent domain data, map ownership, globe math, selection rules.
- `cqsp-client`: Bevy rendering, input, native UI, and smoke verification.

Next: port a deterministic simulation tick and the population/goods/industry model into
`cqsp-core`, with parity fixtures against the C++ systems before adding their game UI.
Original asset irregularities are reported rather than silently inventing ownership.
Build output is contained in the ignored `target/` directory and can be deleted to reclaim
space. `Cargo.lock` is tracked for reproducible application builds.

Known original-data issues: 15 provinces have no map color, two country references have
no definition, two map colors conflict, and 5,121 land pixels have no province definition.
Unmapped pixels show terrain and cannot be selected; undefined owners remain ungoverned.
For conflicting colors, the last definition in sorted filename order wins, matching the
legacy loader's overwrite rule. No original data files are rewritten to hide these issues.
