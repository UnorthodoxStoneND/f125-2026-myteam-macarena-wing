# Compatibility and testing

- Windows x64, Steam F1 25 build 26566308, official 2026 content and My Team cars.
- Swofty Launcher 1.10.21 or later retaining API v9 compatibility. The exact minimum was tested locally.
- Exact supported executable SHA-256 and instruction bytes are checked before installing the render hook. Other game builds are rejected; no performance-speed fallback is applied.
- Audi, Ferrari, Honda, Mercedes and Red Bull Ford My Team variants receive the same reverse-angle registry edit. The render effect is restricted to the marker angle and valid moving rear-wing node.

The native code writes only xmm13.low at the rear-wing render site. It does not write simulation opening fractions, opening speed, aero/engine parameters or liveries. Visual response can lag the simulation's aerodynamic response.

The live-slider bridge reads only the profile identified by this session's startup metadata. It polls at 250 ms on Swofty's frame callback, retains the last good value after malformed/partial saves and ignores other profiles. The in-game native-setting watcher remains supported.

## Evidence

The user confirmed working animation, live launcher-slider changes, the corrected 0–500 ms range and 250 ms default. The corrected publisher namespace has actual-DLL offline fixture coverage and isolated launcher install/build coverage. New PNG/README changes retain that DLL and handling registry byte for byte.

Automated checks exercise startup settings, native watcher changes and atomic saved-profile updates through the actual frame callback into render output. Coverage includes 0/50/150/500 ms, fresh/reset 250 ms default, mid-travel edits, paused frame delta, Unicode paths, invalid saves and recovery, pinned profiles, separate cars, render gaps, real hook handoff/rollback and memory/register preservation.

Isolated build checks passed alone and with the user's existing livery/performance combination. These are build checks, not in-game compatibility results. Specific replay/pause/both-car runtime cases and Linux/Wine support have not been separately established. Public CI is pending until this repository is pushed.

Version 1.0.0 will use Swofty bindings once they are implemented.
