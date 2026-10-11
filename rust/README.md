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
Moon provinces, native spaceport/vehicle/mission screens and an upstream-compatible
metadata save/load browser; much of the remaining UI is still being ported. See
[PARITY.md](PARITY.md) for status, evidence, compatibility gaps and acceptance gates.
The existing CMake/Visual Studio project is unchanged.

To make a local Windows bundle, run `./tools/package.ps1` from `rust/` (release),
or add `-Profile dev` for a faster development build. Each run creates a fresh
folder under `target/packages/` with both executables, original data/textures/font
and the native shader. Run `cqsp-client.exe` from that folder; assets resolve next
to the executable, not from the current working directory. `--paths` prints their
resolved locations without opening a window. `--data PATH` remains an explicit
override. Bundles intentionally omit audio/Lua and require compatible graphics
drivers and the MSVC runtime. They are not installers or full-parity releases;
see the bundled README for prerequisites and redistribution limitations.
Release clients use the Windows GUI subsystem, matching the original release:
double-clicking does not open a separate console. Pipe `--help`/`--paths` output
in PowerShell. The supplied `run-smoke.ps1 -Executable ./cqsp-client.exe` waits
for offscreen GPU checks to complete, including GUI-subsystem releases.
The default scenario verifies all 45 screens. Add `-Scenario Inspection` for a
focused three-screen production/Moon/Sun check (`--smoke-inspection` directly).
Use `-Scenario Policy` for four subsidy/labor/tax screens (`--smoke-policy`).
Use `-Scenario Vehicles` for four vehicle list/filter/design screens (`--smoke-vehicles`).
Use `-Scenario Tooltips` for factory details and modifier contributions (`--smoke-tooltips`).

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
Development builds fall back to the shader in this checkout when no packaged assets exist.

Headless metadata-only save tools are available through `cqsp-sim`:
`--save-metadata ROOT --save-country usa`, `--list-saves ROOT`, and
`--load-metadata FOLDER`. Like upstream, a save contains only `meta.hjson`
(date/UUID/version). Loading applies date/UUID to freshly loaded assets; it does
not restore economy, ships, country selection, histories or RNG state. Re-saving
updates the same country/UUID folder with an atomic metadata replacement. Paths
are explicit, and no original C++ save directories are touched by default.

The native **Save / load metadata** button is available before country selection.
It pauses simulation work while open without changing your pause setting. Save
requires a started country; Previous/Next select a folder, and Load rebuilds fresh
assets, applies date/UUID and ticks once, matching the original loading scene.
It returns to country selection, not the previously played state. The default is
`%LOCALAPPDATA%/ConquerSpaceRust/saves`; `--saves DIRECTORY` overrides it.
Neither opening the browser nor `--paths` creates folders. Offscreen smoke writes
only isolated fixtures under `target/`, never your normal save directory.

Escape opens the native pause menu after starting: Resume preserves the playback
setting, Save writes metadata, Options opens display preferences and Exit Game exits.
Load Game and Exit To Menu remain explicit no-ops like the original pause menu.
The metadata browser is available on Earth/before selection; in system view use
the pause menu for Save/Options, or V to return to Earth for the browser.
Display options include all 15 original resolution presets and fullscreen on the
primary monitor's current video mode. Leaving fullscreen restores the stored
windowed dimensions at (40,40). Defaults are 1280×720, decorated and windowed.
Changes preview immediately; Save preferences explicitly persists them to
`%LOCALAPPDATA%/ConquerSpaceRust/settings.hjson` (`--settings FILE` overrides it).
No audio preferences/backend or original C++ settings files are used. MSAA, splash
and icon preferences and live fullscreen/DPI acceptance remain unfinished.

Player event queues now have native modals with the original 450-pixel rocket image,
scrolling content, action hover text and actionless Ok. Acknowledging executes its
native effect and clears the whole queue, including notifications appended by that
effect. Pending events halt time and capture shortcuts without changing playback.
Unknown images use a visible fallback; stale/invalid buttons cannot clear another
event. The bundled tutorial remains disabled as upstream intended; smoke explicitly
queues test fixtures. Known tutorial effects are native, not arbitrary Lua callbacks.

Right-drag or arrow keys orbit the globe; wheel zooms; left-click selects a province's
country. Tab/Shift+Tab cycle countries (including ones too small to click on the 2K map).
Enter or the sidebar button starts/returns to selection; Escape opens the pause menu.
M toggles terrain/political maps, R resets the camera, F12 writes `target/conquer-space.png`.
Closing the window exits normally.

Left Ctrl+P opens native body/province search. Empty queries have no results;
ASCII case-insensitive substring matching requires the source's actual Name
component, so unnamed provinces, cities and spacecraft stay out. Up/Down clamp
selection, PageUp/PageDown move ten rows, and results scroll into view. Clicking
a province focuses its planet; Enter zooms to its map centroid at radius + 800 km.
A province with no mapped center cannot move the camera. Escape, Ctrl+P and any
outside mouse-button click close search without passing that action to other UI.
Search captures shortcuts but does not pause running time. Native key-press edges,
stable asset ordering and the bounded UTF-8 query editor are not exact RmlUi parity.

The game starts paused. Space pauses/resumes; N advances to the next economy day;
minus/equal or the native slower/faster buttons select the original eleven speeds.
The game defaults to index 3 (100 ms per tick). Positive settings wait 1,000/500/333/
100/50/10/1 ms, while the final four wait 1 ms and execute 1/2/5/10 ticks per frame.
Like upstream, a long frame executes one batch and discards excess elapsed time;
it does not accumulate a catch-up backlog. The bottom pause/play button matches
Space. These controls hide and cannot fire behind open overlays. The right panel shows country/global
economic totals and the hovered province's local market. Returning to selection
pauses and preserves the in-memory simulation; it does not create a new game.

V switches Earth/system views after starting. In system view, Tab/Shift+Tab cycle
natural bodies; Q/Shift+Q cycle spacecraft at the current body; Backspace selects
the parent and E focuses Earth. Click a marker to focus it. Drag/arrows/wheel control
the camera; O toggles orbit paths and G toggles the selected spacecraft's ground track.
T opens the native star-system tree/orbit filter. Expand arrows reveal natural bodies
before spacecraft; Previous/Next reach every row. Double-click names to focus them;
a single click does not move the camera. Imported satellites start hidden. Individual
visibility toggles control ship markers, paths and ground tracks. Hide-all changes
current ship flags, not planetary paths or future launches; show-all overwrites
individual hidden choices. Manufactured vehicles and dispatched cargo start visible.
The modal captures shortcuts/closing clicks while running time continues. This is
functional native integration, not the original always-visible 200×400 ImGui layout,
EnTT row ordering or system double-click timing; native timing is 0.5 seconds.
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

I or **Economy, construction & science** opens live native inspection. Cycle all 11 bodies,
owned/all provinces or the hovered province; inspect all 56 goods, paged factories,
11 science fields and seven technologies. Market details include all original table
fields and the latest eight selected-body price samples. Advance day updates the real simulation;
Escape closes inspection without returning to selection. The panels are mutually
exclusive and do not interrupt vehicle text editing. Select next factory identifies
the `>`-marked factory, changes its subsidy in ten-percent steps (bounded 0–100%)
or applies the original **Boost expertise** modifier. Repeated boosts retain the
original duplicate contributions. Wallet, maintenance, subsidy payment and hired
worker details are shown. Demographics pages show population, jobs, spending,
income, employment, education, wallets and resource consumption, with per-capita/
whole-segment and count/cost toggles. Construction cycles all 22 recipes and shows
their input/capital/output costs and expected profit. Select a province, adjust the
1–1,000 unit count and construct a factory. This preserves the original zero-level
construction record and Steady state; it does not invent an up-front purchase.

The native sidebar routes **Buildings** to the global, nine-column **Production**
table, not the construction tool. It includes foreign factories and ignores the
inspection scope filters. Previous/Next pages through every factory; state colors,
shortage indication, utilization delta, cash, actual employer counts and construction
bars follow the original fields. Hover Size/Revenue for shortages/items sold.
**Exploration** lists all 11 natural bodies (including Sun and Moon), showing
semi-major axis, eccentricity and reference body without moving the camera. Sun's
null parent is displayed as `Null Entity!`, matching the source name helper.
**Economy** provides subsidy and labor tabs; **Budget** opens the separate tax table.
**Space Access** requires the source space-program tag. Projects has no implemented
source window, and Situations retains the previous sidebar selection as upstream.
Sidebar/inspection layout is native, not an exact ImGui/RmlUi reproduction.
Space Access opens the launch-vehicle browser: country-only insertion-ordered
designs, eight per page, ASCII case-insensitive filtering and a separate retained
selection. Filtering can hide the selected row without hiding its details.
The source design draft starts with an empty name and zero numeric fields;
creating closes the editor without selecting the new vehicle. Draft/filter/selection
survive reopening. Names and queries allow 254 UTF-8 bytes without splitting a
character. Designs retain float input/division precision and the original display's
reliability-times-100 quirk. Manual signed/out-of-range finite inputs remain possible
as in ImGui's unclamped text entry; non-finite/float-overflow values are rejected.
The separate Spaceports / missions view retains its convenience draft/selection.

The separate map toolbar cycles all recipes and toggles **Construct / cancel (B)**.
**Search recipes** opens the native, case-insensitive picker. Opening clears its
filter; type, choose a result or use arrows/Enter, and Escape closes search before
cancelling painting. Ctrl+A replaces the query, and the result list scrolls.
Text entry captures world/playback shortcuts and cannot paint a province.
While painting, click an owned province: an empty one gets a factory with no
construction job; otherwise only its last factory's size increases, ignoring the
selected recipe and leaving utilization/job fields unchanged. Left Shift adds
five, Left Ctrl ten, and holding both adds five (the source's Shift branch wins).
Escape cancels painting without leaving the game. UI clicks and open overlays
cannot paint; entering country selection/loading cancels stale painting.
Civilization shows the player's capital, wallet/taxes, governed sites, initial province
balances and mission queue. History cycles market GDP, selected-good price/supply/demand
and global population/SOL/employment/employment rate. Next chart good and the
all/last-128/last-32 range controls redraw native plots; X uses daily sample indices,
matching the original graph despite its Tick label. Non-finite samples leave gaps.
Left-drag pans history plots, the wheel zooms around the cursor, and right-click/Fit
restores automatic bounds. Cursor values and viewport bounds use the original daily
sample indices; chart input does not scroll the panel or move the game camera.
Demographics also renders two native consumption pies per page, with colored
value legends, slice hover readouts and click-to-hide/show entries. Cost/count and
per-capita/whole-segment modes use the original float pie values and double
normalization. Hidden slices leave their original angular space empty rather than
renormalizing the remaining entries. Empty/invalid pies keep the ledger visible.
Pie plots support cursor-centered wheel zoom and left-drag pan with equal axes,
outside-image drag capture, and right-click/**Fit pies** reset. Changing scope/page
resets the view; daily data refresh retains it. Plot wheels do not scroll the panel.
Spaceport inspection
also shows loaded fixed-offset local time, retaining the source clock's quirks.

Budget exposes all 56 goods' absolute 0–5 tax sliders, eight rows per page.
Economy's Add Subsidy editor explicitly selects from all 22 recipes, with a
0–100% draft slider/full-subsidy lock and apply/remove buttons. Applying with no
selection closes without mutation. Closing/leaving the editor retains the selected
recipe, draft and full checkbox; entering Labor closes it. Taxes
affect all province markets globally, not just the player. Subsidies target existing
factories in the country's initial province list; new factories do not inherit them
and removing the policy row leaves existing factory values unchanged, like upstream.
National labor totals also use that initial list. Dragging outside a slider retains
capture until release; closing the screen releases it without advancing the game.
Infrastructure inspects province transport costs, zoning allocations and power/grid
bindings. The original Illinois coal plant and New York transport cost load directly
from the assets. Plants retain zero production and empty fuel records like the source;
grid frequency and a power-generation simulation were not implemented in the original.

For fast renderer-free simulation checks:

```powershell
cargo run -p cqsp-core --bin cqsp-sim -- --days 365 --seed 42
cargo run -p cqsp-core --bin cqsp-sim -- --days 30 --market-csv target/market.csv --industries-json target/industries.json
cargo run -p cqsp-core --bin cqsp-sim -- --days 0 --science-json target/science.json
```

This checks daily economy/orbital state and prints JSON, including body positions.
The scope is explicitly partial. Rust uses the original MT19937 stream and Visual
Studio 2026/MSVC 14.51 distribution rules, including discarded spare normal samples.
Seeds use their low 32 bits, matching the original int/engine conversion.
Whole-economy runs still differ because asset/registry traversal assigns samples
in a different order; deterministic random values alone do not establish full parity.
Daily snapshots for all 11 planetary markets are retained alongside population summaries.
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
`target/industry-inspector.png`, `target/science-inspector.png`,
`target/demographics-inspector.png`, `target/demographics-count-inspector.png`,
`target/demographics-zoomed-inspector.png`,
`target/construction-inspector.png`,
`target/civilization-inspector.png`, `target/history-inspector.png`,
`target/history-zoomed-inspector.png`, `target/infrastructure-inspector.png`,
`target/budget-inspector.png`, `target/construction-painting.png`, `target/construction-search.png`,
`target/pause-menu.png`, `target/display-options.png` and
`target/metadata-browser.png`, then loads the fixture and checks fresh-session reset.
It also captures `target/event-choice.png` and `target/event-notice.png`, checking
the original texture, action/Ok behavior and full-queue clearing without enabling
the disabled tutorial in normal play.
Three additional captures exercise body/province search and the rotating close
surface view: `target/search-body.png`, `target/search-province.png` and
`target/province-focus.png`.
Inspect the images and console for shader/rendering errors; process success alone
does not prove a correct render. The core tests cover original data ingestion, picking,
longitude wrapping, ray intersections, and selection state transitions.
Shader pipelines compile synchronously to address intermittent missing UI text in
captures; sibling labels refresh together when their panel changes. Smoke tests now
render to a fixed-size GPU image **without creating an OS window** and ignore live
input. They check glyph layouts, actual text/overlay/plot pixels and all 45 completed
captures. Background tests work while normal game windows are minimized or closed.
`tools/run-smoke.ps1` also checks that all 45 PNGs were freshly written in the
working directory, so stale images or failed disk writes cannot pass the wrapper.
Floating-card captures verify every tooltip text block, excluding overlapped table
cells from underlying-text checks. Factory name hover exposes all 21 source-mapped
details; Max Expertise hover shows the modifier sum and individual contributions.
These add `target/factory-tooltip.png` and `target/modifier-tooltip.png`.
Consumption captures verify actual slice pixels in both population pies, including
cost/per-capita and count/whole-segment modes with a hidden legend entry.
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
The full economy catalog also compares actual goods/labor/zoning/recipe/config loaders:
56 goods, five jobs, ten zoning types and 22 recipes, plus controlled physical units,
invalid-good rejection, duplicate lookup, first-tag default jobs, tag precedence,
missing references, empty-map merging and construction-time/cost defaults.
Power/grid/province infrastructure loaders compare all 1,317 province bindings plus
controlled default-grid, zero/negative capacity/cost and zoning-conversion boundaries.
Random resource amenability and malformed scalar/unit cases remain outside this corpus.
Civilization metadata and initial owned lists match actual original loaders for all
199 countries. Capital tags remain province-only, and only the USA has a bundled
space-program tag. All-body daily market history matches actual SysMarketHistory,
including nine bodies without surface settlements that do not participate in trade.
All 107 fixed-offset time zones and city bindings match the original loader;
duplicate identifiers keep the final lookup and local hours preserve the source's
signed remainder and fractional-hour truncation rather than adding DST behavior.
All 22 recipes additionally pass seeded production-system comparisons with two
seeds across all eight states. Six seeds compare 12,000 raw engine words and 30,000
mixed normal/integer distribution sequences against the actual original StdRandom.
Five seeded lunar flights now match every sampled numeric bit on Windows MSVC,
including residual circular-orbit elements, maneuver times, landing and unloading.
Recovery uses the original platform CRT's `atanh` rounding; diagnostic JSON parsing
preserves numeric bits. No timing tolerance was introduced. Other runtime targets
are not claimed bit-identical; broader flight and whole-economy comparisons remain.
An additional 66-case movement corpus compares 16,008 ticks at three epochs,
including every command kind, lunar return/docking, SOI boundaries, crash handling,
current/future centers and complete burn/command queues. It preserves the original
late self-destruct update and ascending-node angle wrapping.
The production suite also compares 63,162 daily samples at strict profit,
sustained-gain, stability, utilization, shortage and construction boundaries across
all eight states, three sizes and all 22 recipes. This does not establish arbitrary
configuration or whole-economy traversal parity.

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
