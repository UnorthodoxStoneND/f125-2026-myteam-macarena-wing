# My Team Macarena Wing [Swofty Add-On]

Red Bull-style reverse 160-degree rear-wing animation for 2026 My Team cars in F1 25. Version **0.2.2**, by **UnorthodoxStoneNDmodding**.

## Features

- Adjustable visual wing travel from **0 to 500 ms**, in 50 ms steps; default **250 ms**.
- Change the launching profile's slider during play, normally within 250 ms. Zero disables added smoothing.
- Applies to the five 2026 My Team engine variants while retaining existing car performance and liveries.
- Original PNG launcher icon and the publisher ID `unorthodoxstonendmodding.myteam-macarena-wing`.

Version 1.0.0 will use Swofty bindings once they are implemented.

## Install

Download the ZIP or `.swoftyaddon` from this repository's Releases page, install it in Swofty and enable **My Team Macarena Wing - Red Bull Style**. Restart the game once after updating its DLL. No separate dependency installer is needed for the bundled libraries.

If upgrading from the old `walker-racing.myteam-macarena-wing` entry, disable/remove it before enabling this corrected ID. See the [installation guide](docs/guide/installation.md) for migration, live settings and the launcher slider-cache workaround.

## Supported platforms

Windows x64, Steam F1 25 build **26566308**, the official 2026 content and Swofty Launcher **1.10.21 or later** retaining the required APIs. The DLL checks the exact game executable and instructions. Other builds and EA copies are not supported.

See [compatibility and testing](docs/reference/compatibility.md) for behavior, automated checks and the limits of recorded in-game evidence.

## Contributing

Use Python 3.10+ and the Visual Studio C++ x64 Build Tools. From the repository root:

```powershell
python native/build.py
python source/package.py
python tools/checks/package_check.py
```

See [development](docs/reference/development.md) for the Telemetry-style repository layout and pinned build dependencies, [CI checks](tools/checks/README.md) for validation, and [publishing](docs/reference/publishing.md) for the source/PR/tag/release workflow. Report in-game testing separately from offline checks.

## Licence

Original code: Copyright (c) 2026 UnorthodoxStoneNDmodding, [MIT License](LICENSE). Third-party components retain their own licences, including the Swofty JSON reader's PolyForm Noncommercial terms. See [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md) and the included notices for file/component scope.

Full source, SDK headers, build scripts and tests live in this repository. Installable archives contain only the manifest, referenced feature files and required release documentation/notices; build output and local test data are excluded.

## Credits

[Swofty Launcher](https://github.com/Swofty-Developments/SwoftyLauncher), [SafetyHook](https://github.com/cursey/safetyhook) and [Zydis](https://github.com/zyantific/zydis) supply the SDK/JSON reader and bundled hook/decoder libraries. Dependency credits do not change the mod author.
