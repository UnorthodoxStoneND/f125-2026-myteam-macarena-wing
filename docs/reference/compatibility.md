# Compatibility and testing

- Windows x64, Steam F1 25 build 26566308, official 2026 content and My Team cars.
- Swofty Launcher 1.10.30 or later retaining API v9 compatibility. The named render binding is available in the normal Steam binding set as of 1.10.30.
- Exact supported executable SHA-256 and instruction bytes are checked before registering the managed render callback. Other game builds are rejected; no performance-speed fallback is applied.
- Audi, Ferrari, Honda, Mercedes and Red Bull Ford My Team variants receive the same reverse-angle registry edit. The render effect is restricted to the marker angle and valid moving rear-wing node.

The native code writes only xmm13.low at the rear-wing render site. It does not write simulation opening fractions, opening speed, aero/engine parameters or liveries. Visual response can lag the simulation's aerodynamic response.

The live-slider bridge reads only the profile identified by this session's startup metadata. It polls at 250 ms on Swofty's frame callback, retains the last good value after malformed/partial saves and ignores other profiles. The in-game native-setting watcher remains supported.

## Evidence

The user confirmed live launcher-slider changes in 0.2.x. On Steam build 26566308, the managed test builds installed the binding and applied smoothing at 250 ms in My Team. In the sampled Shanghai runs, neither car repeated the rapid open, close and reopen observed on the earlier track. The earlier trace showed the game input reversing before smoothing, with no animation state reset in that sequence. The diagnostic logging is excluded from 1.0.0.

Automated checks exercise startup settings, native watcher changes and atomic saved-profile updates through the actual frame callback into render output. Coverage includes 0/50/150/500 ms, fresh/reset 250 ms default, mid-travel edits, paused frame delta, Unicode paths, invalid saves and recovery, pinned profiles, separate cars, render gaps, named SITE/ON_LOAD registration, mock-core dispatch and memory/register preservation.

The game logs confirm the launcher accepted the managed build's live frame callback. Live changes across the full slider range, replay behavior and Linux/Wine support have not been separately tested in-game for the managed release. Isolated launcher builds and compiled DLL fixtures are offline checks.
