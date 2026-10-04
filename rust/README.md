# Conquer Space — Rust / Bevy port

This is an actively developed port, not a complete replacement for the C++ game.
It loads the original HJSON country/province definitions and 2K Earth textures, displays
terrain and political maps with province borders, supports globe navigation and picking,
and selects a player country before entering the game view. There is no Lua runtime.
The Earth economy slice loads original goods, jobs, recipes, populations and industries,
and runs daily consumption/production/markets/trade with weekly labor redistribution.
The original 11 celestial bodies and 64 satellites now load and propagate on
ten-minute ticks, with parent-centered positions and next-tick predictions. Kepler
propagation, orbital burns and state-vector conversion are C++ reference-tested.
Spacecraft now execute timed burns, trigger queues, SOI transitions, crashes and
landing commands. A native system view exposes body/ship selection, orbital paths,
inspection and basic maneuver controls. Spaceports now dispatch missions and cargo,
manufacture/launch vehicles and progress lunar colonization. The core includes 250
Moon provinces and native spaceport/vehicle/mission screens; saves and much of the
remaining UI are still being ported. See
[PARITY.md](PARITY.md) for status, evidence, compatibility gaps and acceptance gates.
The existing CMake/Visual Studio project is unchanged.

The Rust app is deliberately **silent**: no music, effects or sound-device backend
are built in. Original sound assets are left untouched. Run
`./tools/verify-silent.ps1` to audit the enabled dependency graph; pass `-Cargo`
with the full Cargo path if it is not on PATH. Audio is intentionally excluded from
the otherwise full-parity target at your request.

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

The game starts paused. Space pauses/resumes; N advances to the next economy day;
minus/equal select 1x/4x/16x/64x tick speeds. The right panel shows country/global
economic totals and the hovered province's local market. Returning to selection
pauses and preserves the in-memory simulation; it does not create a new game.

V switches Earth/system views after starting. In system view, Tab/Shift+Tab cycle
natural bodies; Q/Shift+Q cycle spacecraft at the current body; Backspace selects
the parent and E focuses Earth. Click a marker to focus it. Drag/arrows/wheel control
the camera; O toggles orbit paths and G toggles the selected spacecraft's ground track.
Select a spacecraft to use the maneuver buttons;
brackets adjust their target radius by 10%. Distances are center-to-center kilometers,
not altitude. Planet textures come from the original asset manifest and rotate with
their axial tilt/day length. The inspector reports surface latitude/longitude; the
magenta ground path projects one future spacecraft orbit onto the current planet.
Body/ship markers remain enlarged for visibility and planets are unlit; original
lighting/ship models and the complete spacecraft interface remain to be ported.

P or the **Spaceports & missions** button opens the native space screen. Design a
vehicle, select a port and manufacture it (90 daily steps), then set orbit parameters
and launch the last stored vehicle. Click fields to edit; Ctrl+A replaces, Tab moves
and Enter finishes. Scroll if the panel exceeds your viewport. Escape first ends
editing, then closes the panel without leaving the game. World shortcuts are blocked
while this screen is open; the simulation keeps running if you left it unpaused.
The lunar panel cycles all 250 provinces, begins colonization, queues stage/core
missions, dispatches from the selected port and shows delivery progress. Owned ports
also dispatch missions automatically each daily step. Native manual actions retain
the source's missing ownership check and other documented spaceport rules.

I or **Markets, industries & science** opens live native inspection. Cycle Earth/Moon,
owned/all provinces or the hovered province; inspect all 56 goods, paged factories,
11 science fields and seven technologies. Market details include all original table
fields and the latest eight Earth price samples. Advance day updates the real simulation;
Escape closes inspection without returning to selection. The panels are mutually
exclusive and do not interrupt vehicle text editing. Factory mutation controls and
interactive history graphs remain to be ported.

For fast renderer-free simulation checks:

```powershell
cargo run -p cqsp-core --bin cqsp-sim -- --days 365 --seed 42
cargo run -p cqsp-core --bin cqsp-sim -- --days 30 --market-csv target/market.csv --industries-json target/industries.json
cargo run -p cqsp-core --bin cqsp-sim -- --days 0 --science-json target/science.json
```

This checks daily economy/orbital state and prints JSON, including body positions.
The scope is explicitly partial; deterministic
Rust runs are not yet identical to the original C++ random sequence.
Daily planetary market snapshots are retained alongside population summaries.
The optional CSV keeps the legacy market logger's columns; the industry JSON keeps
its production-unit fields. Exports refuse to overwrite existing files. These are
diagnostic exports, not complete save files.
The science export contains the original 11 fields and seven technologies, including
parent/adjacent relationships, action strings and difficulty. The source has no
automatic research progression system; these definitions do not invent one.

## Verification

```powershell
cargo test --workspace
cargo clippy --workspace --all-targets -- -D warnings
./tools/verify-silent.ps1
cargo run -p cqsp-client -- --smoke
```

The GPU smoke check casts a real camera ray into Delhi, asserts that India is selected,
enters the game view, validates seven daily economy steps, navigates to the system
and a spacecraft, queues a native maneuver and advances another day. It writes
`target/selection.png`, `target/game-view.png`, `target/system-view.png` and
`target/spacecraft-view.png`, then designs/manufactures/launches a vehicle, dispatches
a lunar core mission and verifies its delivery. It also writes `target/spaceport-view.png`
and `target/launched-vehicle.png`. Finally it captures `target/market-inspector.png`,
`target/industry-inspector.png` and `target/science-inspector.png`, then exits.
Inspect the images and console for shader/rendering errors; process success alone
does not prove a correct render. The core tests cover original data ingestion, picking,
longitude wrapping, ray intersections, and selection state transitions.
Shader pipelines compile synchronously to address intermittent missing UI text in
captures; sibling labels refresh together when their panel changes. Smoke tests now
render to a fixed-size GPU image **without creating an OS window** and ignore live
input. They check glyph layouts, actual text/overlay pixels and all nine completed
captures. Background tests work while normal game windows are minimized or closed.
Focused-spacecraft captures also verify texture color variation and ground-track
pixels. The body-rotation/ground-coordinate helpers are compared against the actual
float GLM source across 165 samples (2e-6 tolerance; double comparisons remain 1e-12).

## Layout and next milestone

- `cqsp-core`: renderer-independent map, economy/history, industry modifiers, orbital math, spacecraft actions and selection rules.
- `cqsp-client`: Bevy rendering, input, native UI, and smoke verification.

Actual C++ component outputs are tested by the recorded fixture. To also compare live:

```powershell
./tools/build-reference.ps1
cargo test -p cqsp-core --test cpp_reference -- --include-ignored
```

The live reference suite also executes the original city/project loaders and SOI
ancestry helper. Cities intentionally do not consume/produce or count their unused
population fields, matching the C++ loader. Initial economic population is
360,099,455 (an earlier Rust milestone incorrectly included ignored city data).
It loads all 75 orbital definitions through the original
C++ HJSON loader and checks all 74 moving orbits at seven times spanning 50 years.
It compares actual original sources, not an independent formula replica.
The actual C++ movement/spaceport systems also execute a controlled Earth-to-Moon
colony delivery. Sampled non-degenerate states/queues and exact landing/unload ticks
match Rust; a nearly circular sample compares physical state instead of its
rounding-dependent apsis (details and tolerances in `PARITY.md`).
The reference suite now also runs the original population/reset, industry production
and launch-vehicle production systems. Controlled daily scenarios cover debt/wages,
all eight industry states, interrupted construction, taxes/subsidies and manufacturing
completion boundaries. This is subsystem evidence, not whole-economy seed parity.
Local/planetary markets and interplanetary order resolution also run against the
original systems, including retained trade, shortages, non-default pricing/access,
port stockpile depletion, repeated orders and first-port allocation.
Weekly labor, daily labor-market signals and construction-sector purchases are
compared against original systems across drift limits, wage changes, zero demand
and small/empty workforces. Undefined unemployment conversions are not accepted
as portable behavior; unordered mixed-direction drift still needs broader coverage.
Science-field and technology definitions also compare against the actual source
loaders, including defaults and directed/duplicate graph-reference handling.

Next: remaining native interfaces and broader
economy differential coverage. These components are not yet whole-game parity verified.
Original asset irregularities are reported rather than silently inventing ownership.
Build output is contained in the ignored `target/` directory and can be deleted to reclaim
space. `Cargo.lock` is tracked for reproducible application builds.

Known original-data issues: 15 provinces have no map color, two country references have
no definition, two map colors conflict, and 5,121 land pixels have no province definition.
Unmapped pixels show terrain and cannot be selected; undefined owners remain ungoverned.
For conflicting colors, the last definition in sorted filename order wins, matching the
legacy loader's overwrite rule. No original data files are rewritten to hide these issues.
