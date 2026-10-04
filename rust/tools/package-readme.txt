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
Release builds use the original game's GUI subsystem: double-clicking the client
does not create a separate console. Pipe --paths/--help output when using PowerShell.
Use the supplied run-smoke.ps1 -Executable ./cqsp-client.exe for the silent,
offscreen GPU acceptance run, which waits for both dev and release builds. Captures
are written beneath target/ in the current working directory; no OS window opens.
cqsp-sim.exe --days 365 --seed 42 runs renderer-free stability checks.

The port and original game are GPL-3.0-or-later; see LICENSE.txt. Font notices are
retained under assets/core/gfx/fonts. Review dependencies and original asset notices
before redistribution; this local bundle is not a license audit or public release.
