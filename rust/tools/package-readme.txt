Conquer Space - Rust / Bevy port (in progress)

Launch cqsp-client.exe. It locates assets next to the executable, independent of
the current working directory. The development checkout is not required.
This is not a full-parity release. No audio backend or Lua runtime is enabled;
music, sound and Lua scripts are intentionally not bundled.

Requirements: Windows with compatible graphics drivers and the Microsoft Visual
C++ runtime used by the MSVC Rust target. No installer is supplied. Runtime/driver
availability on another PC is not established by local package verification.
Keep the entire assets directory beside both executables.

Use cqsp-client.exe --help for controls, or --paths to inspect asset resolution.
Left Ctrl+P opens native body/province search. Enter zooms to a mapped province;
clicking a result focuses its planet. Unnamed provinces remain excluded upstream.
T opens the star-system tree/orbit filter; double-click names to focus. Imported
satellites start hidden, launched ships visible. Hide-all leaves planetary paths
and future launches unaffected; show-all overwrites individual visibility choices.
The native sidebar's Buildings route opens worldwide Production, not construction
painting. All nine summary columns and construction bars use original fields;
hover Size/Revenue for shortages/items sold, or a factory name for full details.
Max Expertise in Industries shows the modifier sum and individual contributions.
Exploration inspects every natural
body's orbit without moving the camera. Economy offers subsidy/labor tabs and an
explicit recipe picker; Budget has a separate paged absolute-tax table.
Space Access requires the source space-program tag. Projects/Situations
retain their upstream unimplemented behavior. I also opens native inspection.
Space Access now opens country-only Launch Vehicles with retained filtering/details
and a source-default design editor; use Spaceports / missions for the extended port view.
Escape opens the pause menu after starting. Options provides the original 15
resolution presets and fullscreen; audio is intentionally unavailable. Save
preferences writes only %LOCALAPPDATA%/ConquerSpaceRust/settings.hjson; --settings
FILE overrides it. Metadata saves default to ConquerSpaceRust/saves there, with
--saves DIRECTORY as an override. Original C++ settings/saves are not touched.
Release builds use the original game's GUI subsystem: double-clicking the client
does not create a separate console. Pipe --paths/--help output when using PowerShell.
Use the supplied run-smoke.ps1 -Executable ./cqsp-client.exe for the silent,
offscreen GPU acceptance run, which waits for both dev and release builds. Captures
are written beneath target/ in the current working directory; no OS window opens.
Add -Scenario Inspection for just production and the Moon/Sun exploration screens;
add -Scenario Policy for four subsidy/labor/tax screens, or -Scenario Vehicles for
four vehicle list/filter/design screens. Add -Scenario Tooltips for factory details
and modifier contributions. Add -Scenario Maneuvers for editable spacecraft controls,
rendezvous, landing and orbital/queue inspection. Add -Scenario Maps for the Earth
overlay chooser, goods paging and province/price maps. Full verifies all 53 captures.
M opens Earth overlay selection after starting: terrain, countries, provinces,
domestic provinces and selected-good prices. All 56 goods are paged; close with
M/Escape to inspect the map and hovered local price. Resource/science and other-body
overlays remain unfinished. Before starting, M retains terrain/country switching.
cqsp-sim.exe --days 365 --seed 42 runs renderer-free stability checks.

The port and original game are GPL-3.0-or-later; see LICENSE.txt. Font notices are
retained under assets/core/gfx/fonts. Review dependencies and original asset notices
before redistribution; this local bundle is not a license audit or public release.
