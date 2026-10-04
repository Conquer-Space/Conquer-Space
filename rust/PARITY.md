# Full-parity port tracker

Work lives on `rust`; `main` remains the upstream C++ branch. This is a port in
progress, not a feature-complete replacement. No Lua runtime will be introduced.
The baseline is the checked-out C++ sources and bundled assets (upstream `74b678f`).
Audio is deliberately excluded at the user's request: the Rust application has no
music, effects or enabled audio backend. `tools/verify-silent.ps1` guards that contract.

## Evidence levels

“Reference-tested” means Rust is compared to output from actual original C++
components, not a reimplementation of their formulas. “Implemented” means code
and Rust tests exist, but system-level differential verification remains open.
Successful long runs establish stability, not gameplay parity.

| Area | Status | Evidence / remaining work |
| --- | --- | --- |
| Country/province map and globe picking | Implemented | Original map fixtures, ray/UV tests, offscreen GPU smoke checks; remaining legacy overlays |
| Country selection | Implemented | Ownership, locking, keyboard selection, start/back; remaining legacy menus |
| Civilization metadata / native view | Loader reference-tested; native integration | All 199 countries' wallet/capability/research/tax fields, initial city/province lists and capital tags through actual loaders; native capital, wallet/taxes, paged governed sites/province balances and mission queue; governor and province ownership remain distinct |
| Date / scheduling | Reference-tested components | Ten-minute ticks, initial tick zero, Gregorian leap dates, signed hour-offset behavior and native spaceport local time |
| Playback / time controls | Original settings reference-tested; source-mapped native pacing | Eleven actual clientctx settings and default index 3/paused state; float millisecond intervals, strict greater-than gate, one 1/2/5/10-tick batch per frame, no backlog catch-up; native pause/slow/fast buttons and keyboard/overlay guards; not execution of full original ManageTick/Application |
| Time-zone definitions / city bindings | Loader and clock reference-tested; native display | All 107 fixed-offset definitions, duplicate last-lookup wins, city bindings, eight clock boundaries per zone and controlled conversion/default cases |
| Wallet / PID / ledgers / production tuning | Reference-tested components | `tools/cpp-reference.cpp`, recorded and live differential tests |
| Goods / jobs / zoning / recipes / economy config | Loader reference-tested | Actual original loaders compare all 56 goods, five jobs, ten zoning definitions and 22 recipes; controlled defaults, physical units, invalid-good rejection, duplicate good lookup, first default-job tag, recipe type/tag precedence, missing resources/zoning, empty-map merge and construction-cost scaling |
| Settlement / population / industry ingestion | Implemented for Earth/Moon; city loader reference-tested | 1,067 Earth + 250 Moon provinces + 24 cities; initial economic population 360,099,455 and 20 industries; full economic province-loader comparison and resource amenability remain |
| Province zoning / transport / power / grids | Bundled bindings reference-tested; native inspection | Actual power/grid/province loaders compare all 1,317 province records plus defaults, empty infrastructure, zero/negative size/cost, missing grid, unknown zoning and uint32 wrapping; plant capacity only, zero production/empty fuel and no invented city records; malformed unit/scalar conversions and nondeterministic resource amenability remain |
| Wallet and market resets | Combined controlled tick scenario reference-tested | Actual Rust tick loop against original resets/labor/consumption/production/markets/trade/history, all 22 recipes and seven weekly boundaries; full multi-site traversal remains |
| Construction-sector labor demand | Reference-tested controlled system scenario | Actual InfrastructureSim: zero/one/ten/1,000 capacity, changing wages/taxes, preserved zero-capacity cost and consumption-only purchases; power infrastructure remains |
| Population consumption | Reference-tested controlled system scenario | Actual SysPopulationConsumption/WalletReset/MarketReset: 30 daily steps, positive/debt wallets, prior income/SOL, wages, double demand and all 56 goods; whole-economy ordering remains |
| Native demographics / factory construction | Implemented; creation action reference-tested | Paged population, employment, jobs, spending/income, education, wallets and consumption toggles; two native pies per page with colored values, hover and hide/show legends; source float values, double normalization and hidden-angle behavior; all 22 recipes, cost/revenue preview and 1–1,000 factory count; 66 CreateFactory comparisons, one-shot button and invalid/non-mutating guards |
| Map construction painting | Native implementation; actual creation with source-mapped guards | Separate ConstructionMenu behavior: owned provinces only, empty-zone CreateFactory without a construction job, otherwise enlarge the last size without changing recipe/utilization/job; 528 comparisons across 22 recipes, 1/5/10 counts, ownership/type/empty cases; Left Shift wins over Ctrl, native recipe cycles/searchable selector, UI/input/close isolation |
| Labor distribution / labor market | Reference-tested controlled system scenario | Actual SysLaborDistribution/SysLaborMarket: six cases and 12 weekly steps, drift caps/truncation, wages, zero demand, empty/tiny workforces, weekly hours and daily job demand; mixed-direction unordered traversal and invalid conversions remain |
| Industry state machine / production / construction | Reference-tested controlled and seeded scenarios | Actual SysProduction: all 22 recipes, two seeds, nine cases spanning all eight states and 12 steps (4,752 seeded samples), plus injected noise, shortage interruption, construction completion, taxes/subsidies; combined tick ordering and larger FSM boundary grids remain |
| Original random generator | Reference-tested for MSVC 14.51 | MT19937, canonical doubles, normal/integer distributions, rejection and discarded spare samples; six seeds, 12,000 exact raw words and 30,000 mixed sequences; whole-game sample assignment still depends on traversal |
| Local markets / shortages / planetary trade | Reference-tested controlled system scenario | Actual SysMarket/SysPlanetaryTrade/MarketReset: three provinces, 12 daily steps, retained imports/exports, configurable local versus hard-coded planetary pricing, zero demand, shortage recovery, labor access and port depletion; whole-economy traversal remains |
| Population history | Controlled scenario reference-tested; native interactive graphs | Global population/SOL/employment/employment-rate series, retaining original employed/population denominator and daily sample-index X axis; native pan, cursor-centered zoom, readout and fit/reset |
| Market history / CSV / industry export | All-body history system reference-tested; native interactive graphs | All 11 bodies retain daily GDP/price/supply/demand/ratio snapshots; only Earth/Moon have surface sites and participate in trade; 132 controlled history samples match actual SysMarketHistory; native GDP/price/supply/demand plots with full/128/32-sample ranges, mouse pan/zoom/readout, clipped lines and non-finite gaps, table fields, CSV/export schemas; exact export formatting remains |
| Spaceports / launch vehicles / interplanetary trade | Implemented core and native screen; manufacturing/order systems reference-tested | Actual SysLaunchVehicleProduction boundaries plus SysInterplanetaryTrade: three markets, two ports each, three daily allocations, repeated orders, shortages and multiple sellers; dispatch/combined-system corpus remains |
| Natural bodies / satellite assets | Implemented, orbit loader reference-tested | All 11 celestial bodies and 64 registered HJSON satellites; parent resolution, gravity, mass/SOI, rotation, terrain/atmosphere metadata; original `LoadOrbit` comparison for all 75 entries |
| Kepler orbits / state vectors / impulses | Reference-tested components | Circular, elliptic and hyperbolic propagation, position/velocity, state recovery, local-frame burns and timing; all 74 moving asset orbits compared at seven times spanning 50 years |
| Spacecraft movement / trigger commands | Lunar delivery and boundary corpus reference-tested | Actual SysOrbit/SysSpacePort: five bit-exact seeded outbound missions; 66 movement/command cases across three epochs and 16,008 ticks include every command kind, overdue queues, blocked triggers, SOI boundaries, lunar escape and return/docking; malformed command/entity guards and broader planet transfers remain |
| Maneuver planning | Reference-tested components and command integration | Circularization, apsis/inclination changes, plane matching, Hohmann and coplanar interception compared to actual original functions; body-departure planner integrated into three original-system lunar return/docking scenarios; broader planet-transfer comparisons remain |
| Body hierarchy / movement scheduling | Implemented, ancestry helper reference-tested | Ten-minute scheduler and parent-centered/current/next-tick positions; movement is last, daily cargo unloading occurs after arrival; full original-loop comparison remains |
| Solar-system view | Implemented native view; rotation/ground coordinates reference-tested | Nine original planet textures, axial tilt/day rotation, body/ship focus, orbit overlays, surface-coordinate inspection and selected-ship ground tracks; enlarged markers, unlit lighting, ship models, complete controls and universe generation remain |
| Industry modifiers / factory controls | Reference-tested components; native integration | Original Boost Expertise action and bounded subsidy controls target a selected factory; additive/multiplicative accumulation, duplicate removal, base reassignment and multiplier clamp; original modifier asset loaded and industry gain/cap wiring tested |
| National subsidies / absolute taxes / labor view | Subsidy action reference-tested; native integration | 286 snapshots across all 22 recipes through actual ApplySubsidy, initial lists/stale ownership, fractional/full/raw finite amounts, future factories and repeated UI ledger erasure; native sliders, full lock, one-shot controls and national labor ledgers; global province-only taxes source-mapped and Rust-tested, not actual ImGui execution |
| Research / projects / missions | Project loader reference-tested; vehicle/mission core + UI implemented | Native province/stage/core selection, auto/explicit dispatch, five colony stages/core delivery and boundary tests; research and other project workflows remain |
| Science fields / technologies | Loader reference-tested; native inspection | All 11 fields and seven technologies match actual LoadFields/LoadTechnologies, plus defaults, directed/duplicate references, descriptions, actions, field sets and integer difficulty; native screens expose the loaded definitions and country research |
| Native replacements for scripted UI/events | Native event queues/modals and several interfaces implemented | Country selection and civilization/economy/history inspection are native; player queue/front-only display, actionless Ok, typed tutorial effects, hover text and full-queue clearing; tutorial stays disabled. Body/province search, side-menu mappings and further interface detail remain; exploration has no registered functional workflow upstream |
| Saves / load / settings / packaging | Metadata/default graphics reference-tested; native menus and executable-relative bundling implemented | Actual GetMetadata/LoadMetadata and empty SaveGame, numeric date/truncation/uint32 wrapping, ignored version and invalid-date rejection; CLI/native metadata browser, fresh-asset reset/tick, atomic replacement, six-hex identity separate from economy RNG; source-mapped pause menu, 15 resolution presets, fullscreen/window restoration, isolated Rust preferences. Other application settings and live native-window acceptance remain. Locked bundles exclude audio/Lua; runtime/driver prerequisites and redistribution review remain |

## Compatibility decisions and known gaps

- Preserve source behavior, including quirks: population demand is registered
  twice; income is recorded but not credited to the population wallet; zero-demand
  supply ratios are infinity even for 0/0; PID derivative uses previous minus
  current; `months` means minutes in the legacy time-unit parser; planetary price
  deviation is hard-coded to 0.75; construction duration is converted to ticks
  and then multiplied by DAY again by the industry state machine.
- C++ EnTT aggregate construction value-initializes unspecified numeric fields.
  Rust initializes these explicitly to zero; it does not invent initial income,
  cash, factories or population for settlements without those asset entries.
- The malformed Arizona revenue `2e+05.0` converts to zero in the original HJSON
  `Value::to_double`; preserve that result with a diagnostic. City population and
  recipe fields are ignored: actual `CityLoader::LoadValue` creates no economic
  Settlement/Market/IndustrialZone. Correcting the earlier Rust loader reduced
  population from 549,700,354 to 360,099,455 and industries from 22 to 20. Live
  original loader comparisons guard this distinction.
- Rust now uses the original MT19937 stream and MSVC 14.51 distribution rules.
  Each normal call discards its second polar sample like StdRandom; canonical
  doubles and integer rejection follow the installed Visual Studio headers.
  Seeds truncate to 32 bits. Six-seed live tests cover raw words, small/large
  normal scales, rounded normals, signed/full/fixed integer ranges and rejection.
  Asset/job iteration is still sorted rather than EnTT/unordered traversal, so
  whole-economy sample assignment is **not** seed-identical to C++. Seeded production
  scenarios explicitly match the fixture's reverse insertion order without
  pretending that establishes the full registry ordering. Other C++ standard-library
  distribution implementations have not been adopted as compatibility targets.
- Negative/non-finite/out-of-range unemployment float-to-unsigned conversions are
  undefined in C++; Rust records `None` and counts it in diagnostics. Employment
  totals use wide saturating arithmetic instead of overflowing the C++ `int`.
  These cases require an explicit compatibility policy before declaring parity.
- Market reset clears demand before weekly labor distribution. Construction purchases
  register consumption, not demand; they do not reopen positive job-drift gates.
  A regression test guards this ordering distinction. The controlled labor corpus
  covers order-independent flows, not every mixed-direction unordered-map traversal.
- Recipes with invalid output/job references or industry size below one fail
  clearly instead of propagating undefined entities/clamp bounds. Unit parsing
  is tested for bundled data, not every malformed legacy input.
- `scripts/events/default.lua` defines a tutorial chain but explicitly disables
  registration. Native queue/modal machinery and the two known tutorial effects
  are implemented, but normal play does not activate the chain. Arbitrary Lua
  callbacks are not retained. ExplorationWindow is not registered by UniverseScene;
  SetupContent is empty, and its colonize handler only sets tooltip text. Do not
  invent exploration/research progression to replace these unfinished source stubs.
- Science fields preserve directed parent/adjacent links; adjacency is not made
  symmetric and repeated parents are not deduplicated. Technology fields are sets,
  difficulty is an integer (default ten), and actions remain data strings. Countries
  start with zero research. CreateSystems contains no research progression system,
  so the Rust port must not invent automatic progress or unlock effects. The unused
  `scienceprojects.hjson` is not registered in the source asset manifest. Invalid
  technology field references fail clearly instead of creating default entities.
- Time zones are the source's fixed offsets, not DST/IANA rules: `isdst`, `text`
  and `utc` metadata have no loader behavior. Duplicate identifiers retain all
  definitions but the last lookup wins. GetHour uses signed remainder and truncates
  fractional hours without shifting minutes, so negative hours are preserved.
  Unknown settlement zones are diagnosed instead of keeping invalid entity references;
  non-finite offsets are rejected rather than exposing undefined integer conversion.
- Only ProvinceLoader registers the capital tag: Washington D.C.'s province is
  the USA capital; CityLoader's capital tag is unregistered and does not replace it.
  Do not invent capitals for the other countries. Governed, Province.country and
  CountryCityList are distinct: the fifth lunar delivery changes Province.country,
  not Governed or the initial city/province lists. Only the USA has space_program
  in the bundled country definitions; native actions do not add capability checks.
- Every loaded celestial body has a market/history component, including the Sun.
  Only bodies with a Settlements component participate in planetary/interplanetary
  trade. Other bodies keep base prices and unchanged ratios while daily reset clears
  their supply/demand; their zero-economic history is still recorded and inspectable.
- The original tax slider sets an absolute float amount (0–5), not a percentage,
  on all IndustrialZone markets globally. Cities and planetary markets are excluded.
  National recipe subsidies use the initial CountryCityList rather than current
  ownership. Applying affects only existing matching factories; new ones do not
  inherit the recorded policy. Deleting a policy row leaves factory values unchanged.
  Rust validates malformed IDs/non-finite amounts before mutation; finite raw core
  amounts are preserved while native sliders enforce the original UI bounds.
- The original HJSON orbit loader ignores `epoch` and its optional/copy constructor
  initializes true anomaly to mean anomaly; both are reference-tested and preserved.
  The unregistered legacy `satellites.txt` TLE file is not loaded. Signed retrograde
  rotation periods and the source's radians-as-rotation-offset behavior remain.
- A missing atmosphere adds a zero-height atmosphere in the C++ loader (Undefined
  is not Null); preserve that distinction. Unknown parents/cyclic hierarchies and
  invalid conics fail clearly rather than leaving undefined entity references.
- Body orientation follows the source's float angle/quaternion computation, including
  its double remainder after float conversion. Ground coordinates use the original
  Z-up frame and rotate into Bevy's Y-up view through an explicit axis reflection.
  Original float helpers compare within 2e-6 relative/absolute tolerance; double
  orbit/economy comparisons retain 1e-12. Zero/invalid ground positions return None
  instead of undefined angles. Texture paths come from the original manifest:
  Earth is 8K in system view and the file named `2k_moon.jpg` is actually 1K.
  The selection globe remains 2K. Planet lighting is still unlit, not shader parity.
- State recovery uses the original apsis frame even for tiny nonzero eccentricity.
  GLM's quaternion operation order and explicit Orbit copy's v=M0 behavior matter:
  the previous circular fallback changed lunar landing by six ticks (one hour).
  The controlled C++/Rust scenario now lands at tick 754 and unloads at tick 864.
  Exactly circular equatorial recovery uses a stable frame where C++ yields NaNs;
  parabolic/radial trajectories are rejected explicitly. At residual e below 1e-12,
  rounding leaves an arbitrary apsis, so the lunar test compares physical state
  within 1 m / 1 mm/s instead of w/M0/v. Other sampled elements retain the 1e-12
  tolerance. This is not bit-identical circular-frame or every-tick trajectory parity.
- The strict seeded lunar corpus now matches every sampled numeric bit on Windows
  MSVC for seeds 0, 1, 42, 5489 and UINT32_MAX, including near-circular apsis elements,
  maneuver times, queues, exact landing ticks and daily unloading. Rust's `atanh`
  uses a log1p expression unlike the original CRT; a last-bit mean-anomaly difference
  at lunar SOI entry previously amplified into a one-tick landing difference.
  Windows MSVC recovery now calls the platform CRT's pure-double math function;
  other targets use Rust's function and are not claimed bit-identical. JSON diagnostic
  decoding also needs `float_roundtrip` to avoid introducing one-bit reference errors.
  The seeded gate was strengthened to exact bits, not relaxed to a timing tolerance.
- Preserve the movement loop's late-burn epoch and starting-parent center/future
  center for the SOI-transition tick. `ForceMatchPlanes` retains the source's
  zero-burn trigger hack. The source's unimplemented Impulse/InterceptAndCircularize
  command switch cases remain no-ops; direct orbital impulses are implemented.
  Typed command payloads/non-finite validation replace malformed entity references.
  Docked ships retain diagnostic orbit/state rather than dangling removed components.
  The solar root uses its defined radius/infinite SOI instead of the source cache's
  missing/undefined root data. These undefined-edge decisions are not strict parity.
- Multiplicative modifiers add contributions to one and clamp that multiplier to
  zero; they do not multiply contributions together or clamp a negative base gain.
  Applying one modifier repeatedly creates duplicate contributions; removal erases
  only the first. Negative expertise caps are outside the valid clamp domain and
  still require a compatibility policy.
- Existing map issues (missing colors/owners, duplicate colors, unmapped land)
  remain visible. Original assets are not rewritten to conceal inconsistencies.
- A construction component can remain attached after shortages force a different
  industry state. Progress and spending occur only in the Construction state,
  matching the original switch; mere component presence must not keep billing.
  The original-system corpus caught and guards the previous Rust discrepancy.
- The original manual Construction tab creates a Steady factory and attaches
  Construction(0, recipe time, 0). The port preserves that zero-level record without
  forcing Construction state or inventing up-front purchases, ownership checks or
  zoning enforcement. It produces immediately where a population exists, while the
  attached record remains idle. Capacity inputs follow the source UI's 1–1,000 bounds.
- Spaceport delivery queues are LIFO and do not consume launch vehicles, purchase
  goods or enforce launchpad/capacity limits in the source. Manual vehicle launch
  does consume one manufactured vehicle. Valid/missing project templates both
  skip construction costs due to the original inverted component guard; project
  asset durations remain seconds advanced by ticks. Vehicle design keeps the
  source's reliability/fairing conversions even though its labels are inconsistent.
- Docking at a city port unloads stockpiles once but retains mission cargo, which
  can advance colonization again each daily tick without transferring ownership.
  Province docking consumes mission cargo once and the fifth delivery assigns
  province ownership. Colony core insertion does not invent population/capacity.
- Interplanetary orders use planetary production/consumption, not aggregate
  supply/demand (so the bundled markets normally create none). Deficits are not
  reduced across sellers, and the first port takes all orders for its body.
  The original SOI ancestor helper returns the source for equal-depth sibling
  bodies; this quirk is live reference-tested. Missing destination cities fail
  with diagnostics instead of undefined `front()` access; failed deliveries are
  still popped and missions retain their in-progress marker like the source.

## Verification

From `rust/`:

```powershell
cargo test --workspace
cargo clippy --workspace --all-targets -- -D warnings
./tools/verify-silent.ps1
cargo run -p cqsp-core --bin cqsp-sim -- --days 365 --seed 42
./tools/build-reference.ps1
cargo test -p cqsp-core --test cpp_reference -- --include-ignored
cargo run -p cqsp-client -- --smoke
```

The C++ reference build uses Visual Studio 2026's developer shell, existing
dependency headers and the checked-out HJSON submodule. Pass `-VisualStudio` or
`-DependencyInclude` for other installs.
It compiles original component/system sources into `target/cpp-reference/`; it doesn't
modify the upstream build or require the full C++ client to link. Live reference
tests are explicitly ignored in ordinary test runs so CI needs no Visual Studio.
The recorded fixture always runs. Reference outputs must be regenerated and
reviewed when the baseline changes, not blindly accepted after a mismatch. Live
orbit checks cover all 75 asset definitions and 518 propagated states, with the
same 1e-12 relative/absolute component tolerance as the component harness. The
combined economy/orbital simulation has passed 365 daily validation steps, including
365 market-history records per body and CSV/industry export checks. There are 119 unit tests
and 31 passing differential tests in debug and release. Thirty live tests require the reference executable.
Population/wallet/market-reset scenarios compare 30 daily results; nine industry
cases compare 12 steps against actual SysProduction with controlled normal samples.
An additional all-recipe corpus uses real StdRandom and the fixture's EnTT order:
22 recipes × two seeds × nine industry cases × 12 steps. The original component
and controlled lunar tolerances are unchanged; the seeded lunar gate checks exact
  numeric bits on the installed Windows MSVC target, including every flight tick.
Manufacturing compares 92 steps at zero/day/day-plus-one/90-day boundaries and the
invalid-province/template guards. These are controlled subsystem scenarios, not a
seed-identical run of the entire original economy.
Market comparisons include every ledger and deficit field before and after planetary
trade, using non-default local pricing/access and depleted port stocks. Planetary
ratios remain unchanged and supplied resources clear, matching the source. The
interplanetary fixture controls which port EnTT traverses first; it does not prove
equivalence of every possible registry order. Numeric reference output bypasses
the legacy HJSON encoder's invalid scientific `.0` suffix without changing values.
Labor/construction comparisons use six cases over 12 weekly steps, checking job
counts, hours, employment, daily demand, purchased consumption and sector cost.
Undefined unsigned unemployment conversions are excluded explicitly, not accepted
as platform-dependent reference values. Mixed-direction traversal remains a gap.
Body rotation and ground-track helpers compare 165 samples over 11 bodies and five
epochs, including retrograde and zero-rotation bodies. Renderer-free tests check
texture dimensions, axis reflection, surface path radii, escape-path rejection and
the ground-track toggle. GPU captures check actual texture color variation and
magenta ground-track pixels in the focused spacecraft's parent-body region.
Combined economy comparisons run the real Rust Simulation::tick over 665,390
ten-minute ticks: all 22 recipes × five seeded/state/capacity/subsidy cases × 43
daily samples (4,730 snapshots and 770 weekly labor boundaries). The original
reset/infrastructure/labor/consumption/production/local-market/planetary-trade/
labor-market/history systems run in source order; the weekly guard reads the actual
labor system's Interval(). Every market ledger, industry state/wallet, construction
record/cost, taxes, job demand, population/PID/history and market-history length is
compared with the existing 1e-12 tolerance. This isolates ordering with one province,
one population, one industry and one job; each original recipe retains its commodity
vectors but deliberately uses the controlled job/count. It does not establish full
asset traversal or compare spaceports, manufacturing, interplanetary shipments or
orbital state in this corpus. Undefined unemployment conversions remain excluded.
Science comparisons execute the actual loaders on the bundled definitions and a
controlled graph with duplicate/self/missing references, duplicate technology fields,
tags, fractional difficulty and defaults. `cqsp-sim --science-json NEW_PATH` exports
the loaded definitions without overwriting existing files; this is not a game save.
Economy catalog comparisons execute GoodLoader/LaborLoader/ZoningLoader/RecipeLoader
and LoadEconomyConfig on the bundled definitions plus controlled units/defaults,
invalid mass rejection, duplicate good lookup, first-tag default jobs, tag precedence,
missing references, empty-map cloning and fractional/invalid construction durations.
Empty consumption/construction maps disappear during the original Hjson merge;
the port preserves this rather than creating extra components from empty objects.
Time-zone comparisons execute TimezoneLoader on all 107 definitions plus controlled
duplicates, missing offsets, numeric strings, booleans and malformed numeric strings.
Eight clock boundaries per zone compare actual GetHour/GetMinute. The city-loader
corpus also checks its time-zone bindings after loading the original catalog.
Factory-creation comparisons execute actual CreateFactory at counts 1/27/1,000
for all 22 recipes, checking initial state, capacity/utilization, expertise/modifiers,
wallets, construction attachment, zone insertion and recipe type. The three job
arguments are copied from the original UI; this test does not execute ImGui itself.
Civilization comparisons load original country definitions plus province metadata
and original city rows through the actual loaders. Province economics/color/power
are deliberately excluded from that metadata corpus, not claimed loader parity.
All-body history comparisons execute original reset/local/trade/history systems
for two surface markets and nine inactive markets over 12 days, comparing every
ledger and newly recorded series value. Rust's full 365-day validation still does
not establish whole-game traversal or seed-assignment parity.
Infrastructure comparisons execute PowerPlantLoader/PowerGridLoader/ProvinceLoader
for all 1,317 bundled provinces plus four controlled boundary records. Other
province fields are removed from that corpus to isolate these bindings; random
resource amenability is excluded. Plants copy only scaled capacity, not recipe
fuel demand; there is no original daily power-generation system to invent.
Absent/empty infrastructure does not bind a grid. A nonempty infrastructure block
defaults to USA's grid; undefined/non-grid entity handles are represented as no
valid grid binding, not guessed grid ownership. Unknown plant types fail safely
instead of asserting on an invalid EnTT component. The grid loader ignores frequency.
The cargo scenario dispatches a real Earth-to-Moon mission on ten-minute ticks,
lands on a lunar province and verifies unloading at the following daily economic
step. A second scenario runs the actual original systems with controlled parking
orbit samples, compares 41 sampled states/command counts and exact delivery state.
The reference Game alone links the existing Lua library to instantiate unchanged
legacy systems; the Rust app/core remain Lua-free and audio-free. Manufactured
vehicle launches and dynamically added render markers are tested.
The movement boundary corpus has 66 cases at ticks 0, 144 and 10,000, totaling
16,008 compared ticks. It checks orbital elements, current/future state and centers,
entire burn queues, command order and crash/docked status with the existing 1e-12
tolerance. It covers every command kind, overdue one-per-tick processing, blocked
front triggers, natural/no-op crashes, docking, self-destruct at maneuver/entry/exit,
and both sides and the exact lunar SOI boundary. Three departure-command return
scenarios must actually reach Earth's SOI and dock, not merely compare idle queues.
This corpus caught missing ascending-node normalization and the final future-position
update after self-destruct on SOI entry; dedicated Rust regressions guard both.
Docked diagnostic orbit/state is not compared because the original removes those
components. Invalid entity/payload references remain explicit safe-port decisions.

The client smoke test renders to a fixed 1280×800 GPU image with Winit disabled:
no OS window is created, so minimizing/using the desktop cannot resize its viewport
or change input. It selects India through a real camera ray, enters the game,
validates seven daily steps, switches to system/spacecraft views, executes a native
maneuver and advances another day. It then designs a vehicle, manufactures it over
90 daily steps, launches it and dispatches/delivers a lunar core mission using the
native screen actions. Twenty-five captures include the spaceport panel, newly
launched vehicle and market/industry/science/demographics/construction/civilization/history/infrastructure inspectors.
The budget capture applies a national subsidy to a newly constructed player factory
and verifies the chosen absolute tax across all provinces before rendering. Native
input tests cover all goods/recipes, outside-slider drag capture, clamping, invalid
coordinates, the full-subsidy lock, release/close isolation, one-shot apply, ledger-only
removal and no daily advancement. National labor reads initial-list demand/supply,
not ownership-filtered aggregates. The C++ subsidy corpus executes the actual action;
ledger deletion copies the original UI erase, without claiming ImGui was executed.
The painting capture creates an owned factory, changes the selected recipe and
enlarges the existing last factory, checking size/utilization and no construction
attachment. Core tests cover all recipes, last-versus-first targeting, preserved
manual jobs and invalid/non-owned/city no-ops. Two ECS tests cover recipe cycles,
one-shot toggle, all modifier combinations (including right keys not affecting
the source's left-key-only count), UI hover, overlays/system view and cancel/reset.
Its C++ fixture uses actual CreateFactory with source-mapped ConstructionMenu
guards/size-only branch; it does not claim execution of ImGui or the whole menu.
The native recipe picker clears search on opening and filters all 22 names with
ASCII case-insensitive substrings, matching RecipeCombo for bundled names. Three
additional ECS tests cover all recipes, no/invalid matches, keyboard selection,
127-byte UTF-8-safe input, replacement/deletion, scrolling/reset, selection reset,
shortcut/painting capture and the closing frame. A GPU capture checks the filtered
picker. Native result order follows the Rust catalog, not the original EnTT order;
this is not an ImGui popup pixel or complete text-editor/caret match.
Native inspection tests cover
all good/field/technology cycles, owned/all province filters, hovered ledgers, one-shot
day advancement, live label refresh, panel exclusivity and closing-frame isolation.
Scroll layouts retain their header/navigation rows when content overflows.
Demographics pages cycle all segments and consumption modes without altering the
population. Two native consumption pies per page use source float values and double
normalization, a counterclockwise 90-degree start and colored two-decimal value legends.
Hover reports good/value/share; clicking a legend hides that slice without changing
the angular allocations or the simulation. Image assets are reused across pages,
daily updates and modes; hidden/missing segments do not leave stale hover state.
Renderer-free tests check all four value modes, float conversion, normalization,
one-shot hide/show, hover, image reuse, empty scopes and non-mutation. GPU captures
check visible slices and legend text in both cost/per-capita and count/whole-segment
modes, including one hidden entry. Empty/negative/non-finite pies retain ledger text
without undefined geometry. Pie input supports equal-axis, cursor-centered wheel zoom,
both-axis drag pan, outside-image capture and right-click/Fit reset. Daily refresh
preserves views; changing scope/page resets them. Hover uses transformed coordinates;
plot wheels cannot scroll the panel, and close/missing scopes release capture. Bounds
and pixel work remain finite even far offscreen. Renderer-free input tests check these
rules, asset reuse and unchanged simulation; an extra GPU capture verifies the
panned/zoomed pies. This is native functionality, not an ImPlot pixel match; original
unordered legend traversal and tessellation/palette details are not asserted identical.
Construction tests cover every recipe, bounded count adjustments,
one-shot creation, the original idle record and rejection without partial mutation.
Civilization pages check governor filtering independently of province ownership and
retain the initial province balance list. All eight chart series, 56 goods, 11 bodies
and range windows are reachable; cropped ranges retain original daily sample indices.
The rasterizer handles constant/single/extreme/non-finite series. Chart images reuse
one owned asset, redraw only for changed selections/new samples and hide on close.
The history GPU capture also requires actual colored series pixels in the plot region.
History controls retain a manual view while new daily samples arrive; choosing a
different series/good/body/range resets it. Left drag pans both axes, the wheel zooms
around the cursor, and right-click/Fit restores automatic bounds. Plot wheel input
does not scroll the overlay or move the game camera; drag capture continues outside
the image until release. Cursor values use the original daily sample-index axis.
Renderer-free input tests check selection reset, capture release, unchanged simulation,
asset reuse and panel-scroll isolation. Zoomed segments are clipped before rasterization
so extreme offscreen values cannot create unbounded pixel loops; non-finite gaps remain.
The extra GPU capture verifies the panned/zoomed history panel and actual curve pixels.
Text editing, Unicode deletion, shortcut isolation, one-shot
buttons, scrolling and invalid-design non-mutation have renderer-free ECS tests.
Text checks cover both glyph layouts and bright pixels inside visible text blocks;
system captures also check orbit, texture and ground-track pixels. Synchronous pipeline preparation
and sibling-label refresh address the previous missing-text captures. The previous
window-dependent center-ray failure is avoided by targeting the fixed render image.
Repeated offscreen runs have passed; live desktop input/resize still needs separate
manual acceptance. Keyboard navigation and one-shot button actions have ECS tests
which do not create native windows.
Playback settings compare the actual clientctx header's eleven-value table and
PauseOptions defaults. Native scheduling maps ManageTick's float division, strict
elapsed-time boundary and one-batch-per-frame behavior; it no longer uses invented
1x/4x/16x/64x rates or a time backlog. Unit/ECS checks cover all speed bounds,
fractional float thresholds, late frames, pause/resume, one-shot buttons, overlay
visibility/input isolation and no simulation work in country selection. This does
not execute the entire original Application/ManageTick method or assert interpolation
renderer parity. The speed-zero exactly-one-second boundary deliberately does not
tick; a 1,000-second frame at maximum speed executes ten ticks, not a backlog.

`tools/package.ps1` builds with Cargo.lock, verifies silence and writes a new bundle
under target/packages on every run; it does not overwrite an older package, install
software, change registry settings or copy audio/Lua scripts. The client and headless
simulator prefer assets/core next to their executables; the native shader uses
assets/client. Development paths are fallbacks only when those package directories
are absent. Both CLIs support --paths for non-launching resolution diagnostics.
Package verification must also run the simulator and offscreen GPU smoke from a
different working directory with no --data override. A successful local launch does
not prove CRT/graphics-driver availability on another PC; no installer or completed
redistribution-license audit is claimed.
Both dev and release bundles have passed 365 daily simulator steps and the then-current
fifteen GPU captures from a different working directory with no asset override. The optimized
C++ differential suite also passes all 24 tests, including bit-exact seeded lunar
flights. Release clients use the Windows GUI subsystem (PE subsystem 2), matching
the original release without an extra console. The supplied run-smoke.ps1 redirects
and drains both output pipes and waits for the offscreen client to exit; no native
window is created. Pipe non-launching --paths/--help output in PowerShell too.
The wrapper now also rejects missing/stale captures by checking each expected PNG's
size and write time against this run; successful rendering alone cannot conceal a
failed disk write.
The updated release bundle has also passed all seventeen captures, including both
consumption modes and panned/zoomed pies, from the external working directory with
no asset override. All 26 differential tests pass in debug and release.
The original Save::SaveGame returns an empty HJSON value, while metadata only stores
date/UUID/version. A full simulation/RNG snapshot would be a new feature, not behavior
already implemented upstream. The core/CLI/store now preserves this limited behavior;
the source-mapped pause menu and basic display settings are also implemented. Thirteen reference cases execute actual
GetMetadata/LoadMetadata, including numeric truncation, uint32 wrapping, ignored
versions, UUID conversion and invalid nonnumeric dates. The load browser defaults
missing dates to -1, whereas LoadMetadata itself rejects them. Typed validation rejects
out-of-range/invalid UUID inputs before mutation rather than exposing undefined casts
or partial writes. CLI tests saved/listed/reloaded a two-day metadata file, advanced a
fresh simulation one day and atomically replaced metadata in the same folder. Only
the agent-owned target/metadata-cli-check fixture was written; C++ saves were untouched.
The native metadata browser now saves/refreshes/pages folders, including before
country selection. Loading follows UniverseLoadingScene's fresh assets, metadata
application and one tick, then resets country selection, navigation, inspection,
ports and pause state. Extra ship markers are pruned and can be recreated without
duplicates. Four ECS tests cover modal time/shortcut capture, closing-frame isolation,
one-shot disk writes, all ten fixture folders, scroll isolation and invalid-load
non-mutation. The metadata GPU capture checks the explicit metadata-only warning;
the smoke then loads it and checks date/UUID, default ships and session reset.
Fixtures are isolated under target/; no normal Rust or original C++ saves are written.

The pause menu halts simulation work without changing playback's paused/running flag.
Resume, metadata-only Save, Options and Exit Game have native actions; original
Load Game/Exit To Menu buttons are no-ops and remain so. Three ECS tests cover
modal shortcut/closing-frame isolation, painting/editor precedence, one-shot disk
writes, no-op session preservation, all 15 presets, fullscreen/window restoration,
failed preference writes and exit signaling. Escape uses native key-press edges
rather than the source's release edges; this is not full input-phase parity.
The GPU captures exercise pause/options and isolated preference persistence.
Actual ClientOptions::GetDefaultOptions supplies the C++ graphics-default comparison;
the resolution-table test extracts all 15 entries from the original source, not
execution of ImGui. The reference harness disables original user-path access.
The unified native UI previews display changes and explicitly saves preferences;
source in-game options preview only, while its separate main-menu Submit persists.
Rust-only preferences are atomically replaced and exclude audio. Fullscreen selects
the primary monitor's current mode, restoring windowed dimensions/position (40,40)
on exit; ECS tests mutate Window components without opening an OS window. Live
fullscreen, DPI, resize and remaining MSAA/splash/icon settings require acceptance.

Native notifications are country-scoped, displaying only the player's queue front.
Choosing a named action runs its typed effect before clearing the entire queue;
actionless Ok also clears everything. Four core and three ECS tests cover invalid
choices, effectless choices, callback-appended entries, non-player queues, exact
bundled tutorial text, disabled registration, modal time/shortcut/closing-frame
capture, stale actions, missing-image fallback, tooltip labels and scroll reset.
The 48-case C++ queue corpus covers actionless/actionful, no-callback/callback,
callback-appended entries, both known tutorial stages and queue sizes 1/2/7.
The source's rocket image uses its actual aspect ratio at width 450, with a
150-pixel scrolling content area. Two GPU fixtures exercise action/Ok and check
image variation and heading pixels. Native input/modal layout are not ImGui pixel
or phase matches. The C++ reference corpus uses actual EventQueue/Event/EventResult
and sol callback types with source-mapped SysEvent acknowledgement branches; it
does not execute ImGui, FunctionEvent parsing or the disabled script registration.
Only the C++ harness links Lua; Rust's known effects are native.

The current release bundle passed all 25 fresh offscreen captures from an external
working directory, including both event dialogs and their rendered gold headings.
The workspace has 141 unit tests; all 31 differential tests pass in both debug and
release (30 live C++ comparisons and one recorded corpus). No native game window or
audio device is created by these checks. The earlier 365-day packaged stability run
uses the same simulation changes; it is not a claim of complete gameplay parity.

## Next acceptance gates

1. Extend the existing population/production system corpus to larger FSM boundary
   grids, mixed-direction labor and combined tick ordering. All recipes and seeded
   production are now covered. Controlled
   labor, construction demand, shortages, taxes/subsidies and trade are covered.
2. Extend spacecraft comparisons to broader planet transfers and malformed command
   boundaries; seeded outbound missions, lunar returns, SOI ordering and three
   launch epochs are covered.
3. Extend native spaceport screens with filtering/inspection detail and dispatch
   boundary corpora; manufacturing, trade orders and lunar delivery are reference-tested.
4. Replace remaining Lua-driven interfaces with native screens/actions; complete
   supported project/modifier controls, body/province search, side-menu mappings
   and further market/history inspection. Distinguish source stubs from implemented workflows.
5. Verify remaining menus, application settings/native-window behavior, package runtime
   prerequisites and input/UI flows. Complete-state saves would need a separate design.
   Full parity requires documented scenario-level comparisons, not file counts.
