# My Team Macarena Wing - Red Bull Style

Version 0.2.2 — by UnorthodoxStoneNDmodding

Adds a Red Bull-style Macarena rear-wing animation to 2026 My Team cars, with reverse 160-degree rotation and adjustable visual travel time from 0 to 500 ms. Animation speed can be changed during play without affecting car performance or liveries.

Version 1.0.0 will use Swofty bindings once they are implemented.

## Compatibility

Windows, Steam F1 25 build 26566308, official 2026 season content, Swofty Launcher 1.10.21 or later. The DLL accepts only the exact supported executable and render instructions. EA and other game builds are not supported. All five My Team engine variants are targeted.

## Install and adjust

Drop the ZIP onto Swofty Launcher, then enable My Team Macarena Wing - Red Bull Style.

Visual wing travel (ms) ranges from 0 to 500 in 50 ms steps, default 250. Higher values are slower. Zero removes added smoothing and follows the game's normal render fraction. Use Reset to defaults to select 250 if you already saved another value.

The slider applies during play, normally within 250 ms, from the launcher profile that started the session. Changing another profile does not affect the current session. If a launcher update leaves repeated values beyond 500, fully exit and reopen Swofty Launcher to refresh its cached slider control.

Only visual rear-flap travel is adjusted. Simulation opening speed, aerodynamic response and performance settings are retained. No livery textures, materials or car models are supplied. Disable this addon and launch again to remove its changes.

## Credits and licenses

- Mod author: UnorthodoxStoneNDmodding.
- SafetyHook by cursey and contributors: hook library, distributed under the Boost Software License 1.0. See licenses/SafetyHook-Boost.txt and https://github.com/cursey/safetyhook.
- Zydis by Florian Bernd, Joel Hoener and contributors: instruction decoder used by SafetyHook, distributed under the MIT license. Original copyright names/notices are retained in licenses/Zydis-MIT.txt. See https://github.com/zyantific/zydis.
- Swofty Pty Ltd: SDK integration and the JSON reader used by live slider synchronization. Required Notice: Copyright Swofty Pty Ltd (ACN 665 787 244). The included JSON reader is covered by licenses/Swofty-PolyForm-Noncommercial.txt. See https://github.com/Swofty-Developments/SwoftyLauncher.
- The included wing icon is original artwork made for this mod.

Third-party components are used under their included licenses. This package contains no game executable, game archives/database, extracted livery assets or team logos. Dependency credits do not change the mod author.
