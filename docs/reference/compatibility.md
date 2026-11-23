# Compatibility and testing

- Windows x64, Steam F1 25 build 26566308, official 2026 content and My Team cars.
- Swofty Launcher 1.10.30 or later retaining API v9 compatibility. Package installation/build checks pass on this minimum; the managed in-game test remains pending.
- Exact supported executable SHA-256 and instruction bytes are checked before registering the managed render callback. Other game builds are rejected; no performance-speed fallback is applied.
- Audi, Ferrari, Honda, Mercedes and Red Bull Ford My Team variants receive the same reverse-angle registry edit. The render effect is restricted to the marker angle and valid moving rear-wing node.

The native code writes only xmm13.low at the rear-wing render site. It does not write simulation opening fractions, opening speed, aero/engine parameters or liveries. Visual response can lag the simulation's aerodynamic response.

The live-slider bridge reads only the profile identified by this session's startup metadata. It polls at 250 ms on Swofty's frame callback, retains the last good value after malformed/partial saves and ignores other profiles. The in-game native-setting watcher remains supported.

## Evidence

The user confirmed the 0.2.x animation, live launcher-slider changes, corrected 0–500 ms range and 250 ms default. The migration retains the same addon ID, setting ID/range/default, live profile bridge, PNG and handling records. Its actual-DLL fixtures pass registration/callback tests; in-game acceptance of the new managed path is still pending.

Automated checks exercise startup settings, native watcher changes and atomic saved-profile updates through the actual frame callback into render output. Coverage includes 0/50/150/500 ms, fresh/reset 250 ms default, mid-travel edits, paused frame delta, Unicode paths, invalid saves and recovery, pinned profiles, separate cars, render gaps, named SITE/ON_LOAD registration, mock-core dispatch and memory/register preservation.

Isolated build checks passed alone and with the user's existing livery/performance combination. These are build checks, not in-game compatibility results. Specific replay/pause/both-car runtime cases and Linux/Wine support have not been separately established. Remote CI for this migration is pending until the branch is pushed.

The 1.0.0 candidate uses the launcher-provided site_Car_RearWingRenderFraction binding and has no private hook fallback. The named binding is present in the tested normal 1.10.30 Steam set. Previous in-game acceptance covers the 0.2.x runtime; the managed migration has offline coverage and still needs its own in-game confirmation.
