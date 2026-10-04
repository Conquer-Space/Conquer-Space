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
| Country selection | Implemented | Ownership, locking, keyboard selection, start/back; remaining legacy menus and settings |
| Date / scheduling | Reference-tested components | Ten-minute ticks, initial tick zero, Gregorian leap dates, signed hour-offset behavior; time-zone UI remains |
| Wallet / PID / ledgers / production tuning | Reference-tested components | `tools/cpp-reference.cpp`, recorded and live differential tests |
| Goods / jobs / recipes / economy config | Implemented | Original 56 goods, 5 jobs, 22 recipes; defaults, units, construction-cost scaling; full loader differential corpus remains |
| Settlement / population / industry ingestion | Implemented for Earth/Moon; city loader reference-tested | 1,067 Earth + 250 Moon provinces + 24 cities; initial economic population 360,099,455 and 20 industries; zoning, power grids, resource amenability remain |
| Wallet and market resets | Implemented | Daily schedule, retained trade/prices/shortages; full tick comparison remains |
| Construction-sector labor demand | Reference-tested controlled system scenario | Actual InfrastructureSim: zero/one/ten/1,000 capacity, changing wages/taxes, preserved zero-capacity cost and consumption-only purchases; power infrastructure remains |
| Population consumption | Reference-tested controlled system scenario | Actual SysPopulationConsumption/WalletReset/MarketReset: 30 daily steps, positive/debt wallets, prior income/SOL, wages, double demand and all 56 goods; whole-economy ordering remains |
| Labor distribution / labor market | Reference-tested controlled system scenario | Actual SysLaborDistribution/SysLaborMarket: six cases and 12 weekly steps, drift caps/truncation, wages, zero demand, empty/tiny workforces, weekly hours and daily job demand; mixed-direction unordered traversal and invalid conversions remain |
| Industry state machine / production / construction | Reference-tested controlled system scenario | Actual SysProduction: nine cases spanning all eight states, 12 steps, injected noise, shortage interruption, construction completion, taxes and partial/full subsidies; broader recipe/transition corpus and original RNG sequence remain |
| Local markets / shortages / planetary trade | Reference-tested controlled system scenario | Actual SysMarket/SysPlanetaryTrade/MarketReset: three provinces, 12 daily steps, retained imports/exports, configurable local versus hard-coded planetary pricing, zero demand, shortage recovery, labor access and port depletion; whole-economy traversal remains |
| Population history | Controlled scenario reference-tested | Population/SOL/employment summaries match actual original daily history; legacy graph UI remains |
| Market history / CSV / industry export | Implemented for Earth | Daily planetary snapshots, original CSV column schema and production-unit export fields; CSV numeric/date formatting and empty-zone omission are not byte-identical, inspection graphs remain |
| Spaceports / launch vehicles / interplanetary trade | Implemented core and native screen; manufacturing/order systems reference-tested | Actual SysLaunchVehicleProduction boundaries plus SysInterplanetaryTrade: three markets, two ports each, three daily allocations, repeated orders, shortages and multiple sellers; dispatch/combined-system corpus remains |
| Natural bodies / satellite assets | Implemented, orbit loader reference-tested | All 11 celestial bodies and 64 registered HJSON satellites; parent resolution, gravity, mass/SOI, rotation, terrain/atmosphere metadata; original `LoadOrbit` comparison for all 75 entries |
| Kepler orbits / state vectors / impulses | Reference-tested components | Circular, elliptic and hyperbolic propagation, position/velocity, state recovery, local-frame burns and timing; all 74 moving asset orbits compared at seven times spanning 50 years |
| Spacecraft movement / trigger commands | Implemented; lunar delivery reference-tested | Actual original SysOrbit/SysSpacePort scenario compares sampled states, queues, landing/unload ticks and colony core/stage; broader SOI, return flights and command boundary corpus remain |
| Maneuver planning | Reference-tested components | Circularization, apsis/inclination changes, plane matching, Hohmann and coplanar interception compared to actual original functions; body-departure planner and complete Moon-transfer scenario need broader comparisons |
| Body hierarchy / movement scheduling | Implemented, ancestry helper reference-tested | Ten-minute scheduler and parent-centered/current/next-tick positions; movement is last, daily cargo unloading occurs after arrival; full original-loop comparison remains |
| Solar-system view | Implemented native view; rotation/ground coordinates reference-tested | Nine original planet textures, axial tilt/day rotation, body/ship focus, orbit overlays, surface-coordinate inspection and selected-ship ground tracks; enlarged markers, unlit lighting, ship models, complete controls and universe generation remain |
| Industry modifiers | Reference-tested components; integration implemented | Additive/multiplicative accumulation, duplicate removal, base reassignment and multiplier clamp; original modifier asset loaded, industry gain/cap wiring tested; modifier UI remains |
| Research / projects / missions | Project loader reference-tested; vehicle/mission core + UI implemented | Native province/stage/core selection, auto/explicit dispatch, five colony stages/core delivery and boundary tests; research and other project workflows remain |
| Native replacements for scripted UI/events | Partial | Country selection is native; civ info, exploration, side menu and event infrastructure remain |
| Saves / load / settings / packaging | Not ported | Persist simulation + RNG state, source-independent assets, release installation and compatibility decision |

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
- Rust currently uses seeded SmallRng/normal distributions and sorted asset/job
  iteration. C++ uses MT19937/std::normal_distribution and EnTT/unordered traversal.
  Repeated Rust runs are deterministic with the locked dependencies, but are
  **not** seed-identical to C++. Injected noise tests isolate production rules.
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
  registration. Native event machinery is still required for supported events;
  do not activate the disabled chain accidentally.
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
365 market-history records and CSV/industry export checks. There are 71 unit tests
plus 13 differential tests (12 live tests require the reference executable).
Population/wallet/market-reset scenarios compare 30 daily results; nine industry
cases compare 12 steps against actual SysProduction with controlled normal samples.
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
The cargo scenario dispatches a real Earth-to-Moon mission on ten-minute ticks,
lands on a lunar province and verifies unloading at the following daily economic
step. A second scenario runs the actual original systems with controlled parking
orbit samples, compares 41 sampled states/command counts and exact delivery state.
The reference Game alone links the existing Lua library to instantiate unchanged
legacy systems; the Rust app/core remain Lua-free and audio-free. Manufactured
vehicle launches and dynamically added render markers are tested.

The client smoke test renders to a fixed 1280×800 GPU image with Winit disabled:
no OS window is created, so minimizing/using the desktop cannot resize its viewport
or change input. It selects India through a real camera ray, enters the game,
validates seven daily steps, switches to system/spacecraft views, executes a native
maneuver and advances another day. It then designs a vehicle, manufactures it over
90 daily steps, launches it and dispatches/delivers a lunar core mission using the
native screen actions. Six verified captures include the spaceport panel and newly
launched vehicle. Text editing, Unicode deletion, shortcut isolation, one-shot
buttons, scrolling and invalid-design non-mutation have renderer-free ECS tests.
Text checks cover both glyph layouts and bright pixels inside visible text blocks;
system captures also check orbit, texture and ground-track pixels. Synchronous pipeline preparation
and sibling-label refresh address the previous missing-text captures. The previous
window-dependent center-ray failure is avoided by targeting the fixed render image.
Repeated offscreen runs have passed; live desktop input/resize still needs separate
manual acceptance. Keyboard navigation and one-shot button actions have ECS tests
which do not create native windows.

## Next acceptance gates

1. Extend the existing population/production system corpus to all recipes, larger
   FSM boundary grids, mixed-direction labor and combined tick ordering. Controlled
   labor, construction demand, shortages, taxes/subsidies and trade are covered.
2. Extend the spacecraft system harness to return flights, SOI edge ordering,
   different launch epochs and the near-circular post-burn trajectory.
3. Extend native spaceport screens with filtering/inspection detail and dispatch
   boundary corpora; manufacturing, trade orders and lunar delivery are reference-tested.
4. Replace remaining Lua-driven interfaces with native screens/actions; complete
   research, projects, modifier controls, event queues and market/history inspection.
5. Verify saves/reloads, application settings, asset packaging and input/UI flows.
   Full parity requires documented scenario-level comparisons, not file counts.
