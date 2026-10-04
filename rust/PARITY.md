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
| Construction-sector labor demand | Implemented | `InfrastructureSim` demand and purchase-cost calculation; power infrastructure remains |
| Population consumption | Implemented | Previous-income/SOL ordering, debt, double demand, labor output, daily summary tests; actual C++ system harness remains |
| Labor distribution / labor market | Implemented | Weekly hours, integer-truncated drift, daily job demand; entity traversal order and invalid conversions need differential review |
| Industry state machine / production / construction | Implemented | Eight states, utilization, expertise, labor/material inputs, taxes, subsidies, construction spending; broader transition corpus and original RNG sequence remain |
| Local markets / shortages / planetary trade | Implemented | Price caps, shortages, retained trade, imports/exports, Earth/Moon markets and spaceport coupling; original system harness remains |
| Population history | Implemented | Daily domain snapshots; legacy graph UI remains |
| Market history / CSV / industry export | Implemented for Earth | Daily planetary snapshots, original CSV column schema and production-unit export fields; CSV numeric/date formatting and empty-zone omission are not byte-identical, inspection graphs remain |
| Spaceports / launch vehicles / interplanetary trade | Implemented core | LIFO shipments, design/manufacture/launch, unloading, original trade rules and missing-city diagnostics; native screens and whole-system differential tests remain |
| Natural bodies / satellite assets | Implemented, orbit loader reference-tested | All 11 celestial bodies and 64 registered HJSON satellites; parent resolution, gravity, mass/SOI, rotation, terrain/atmosphere metadata; original `LoadOrbit` comparison for all 75 entries |
| Kepler orbits / state vectors / impulses | Reference-tested components | Circular, elliptic and hyperbolic propagation, position/velocity, state recovery, local-frame burns and timing; all 74 moving asset orbits compared at seven times spanning 50 years |
| Spacecraft movement / trigger commands | Implemented | One overdue burn per tick, burn epoch, front-only trigger queue, SOI conversions, crash/dock states and lunar deorbit tests; whole original-system differential harness and cargo/docking integration remain |
| Maneuver planning | Reference-tested components | Circularization, apsis/inclination changes, plane matching, Hohmann and coplanar interception compared to actual original functions; body-departure planner and complete Moon-transfer scenario need broader comparisons |
| Body hierarchy / movement scheduling | Implemented, ancestry helper reference-tested | Ten-minute scheduler and parent-centered/current/next-tick positions; movement is last, daily cargo unloading occurs after arrival; full original-loop comparison remains |
| Solar-system view | Implemented basic native view | Body/ship focus, hierarchy navigation, precise local frames, orbit overlays, inspection, basic native maneuver buttons; simplified/enlarged markers, textured planets/ships, ground tracks, complete controls and universe generation remain |
| Industry modifiers | Reference-tested components; integration implemented | Additive/multiplicative accumulation, duplicate removal, base reassignment and multiplier clamp; original modifier asset loaded, industry gain/cap wiring tested; modifier UI remains |
| Research / projects / missions | Project loader reference-tested; vehicle/mission core implemented | Auto/explicit dispatch, five colony stages/core delivery and boundary tests; research, other project workflows and UI remain |
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
- State recovery chooses a stable circular/equatorial frame where C++ can produce
  NaNs; parabolic/radial trajectories are rejected explicitly. The orbital harness
  verifies non-degenerate states and burns, not those undefined source cases.
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
It compiles original component sources into `target/cpp-reference/`; it doesn't
modify the upstream build or require the full C++ client to link. Live reference
tests are explicitly ignored in ordinary test runs so CI needs no Visual Studio.
The recorded fixture always runs. Reference outputs must be regenerated and
reviewed when the baseline changes, not blindly accepted after a mismatch. Live
orbit checks cover all 75 asset definitions and 518 propagated states, with the
same 1e-12 relative/absolute component tolerance as the component harness. The
combined economy/orbital simulation has passed 365 daily validation steps, including
365 market-history records and CSV/industry export checks. There are 60 unit tests
plus five differential tests (four live tests require the reference executable).
The cargo scenario dispatches a real Earth-to-Moon mission on ten-minute ticks,
lands on a lunar province and verifies unloading at the following daily economic
step. Manufactured vehicle launches and dynamically added render markers are tested.

The client smoke test renders to a fixed 1280×800 GPU image with Winit disabled:
no OS window is created, so minimizing/using the desktop cannot resize its viewport
or change input. It selects India through a real camera ray, enters the game,
validates seven daily steps, switches to system/spacecraft views, executes a native
maneuver, advances another day and saves four verified captures before exiting.
Text checks cover both glyph layouts and bright pixels inside visible text blocks;
system captures also check orbit-overlay pixels. Synchronous pipeline preparation
and sibling-label refresh address the previous missing-text captures. The previous
window-dependent center-ray failure is avoided by targeting the fixed render image.
Repeated offscreen runs have passed; live desktop input/resize still needs separate
manual acceptance. Keyboard navigation and one-shot button actions have ECS tests
which do not create native windows.

## Next acceptance gates

1. Build an original-system differential harness for population, production and
   trade with controlled entity order and injected random samples. Add all FSM
   transition boundaries, construction shortages and taxation/subsidy cases.
2. Compare the spacecraft movement loop against an original-system harness,
   including SOI edge ordering and a full Earth-to-Moon transfer/landing scenario.
3. Port spaceports, launch-vehicle construction, cargo/trade routing and mission
   actions, then verify an end-to-end launch/delivery against the legacy game.
4. Replace remaining Lua-driven interfaces with native screens/actions; complete
   research, projects, modifier controls, event queues and market/history inspection.
5. Verify saves/reloads, application settings, asset packaging and input/UI flows.
   Full parity requires documented scenario-level comparisons, not file counts.
