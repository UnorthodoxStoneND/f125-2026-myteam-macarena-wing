# Development

This repository follows the F1 Live Telemetry layout, with native-only build steps for this mod.

| Path | Purpose |
| --- | --- |
| `native/` | Native animation, live profile bridge, SDK and actual-DLL host tests |
| `source/` | Installable-package builder and editable icon source |
| `tools/checks/` | Pinned dependency preparation and archive validation |
| `docs/guide/` | Installation and settings guidance |
| `docs/reference/` | Compatibility, development and publishing |
| `.github/workflows/` | Windows build/test/package CI |
| `swofty.json` | Addon identity, settings and runtime references |
| `registry/` | Narrow vehicle handling records |
| `bin/`, `dist/`, `.verify/`, `.deps/` | Ignored binaries, archives, test output and dependency cache |

## Build and package

Use Python 3.10+ and Visual Studio Build Tools with **Desktop development with C++** and the x64 toolchain. CI uses Python 3.14. From the repository root:

```powershell
python native/build.py
python source/package.py
python tools/checks/package_check.py
```

`native/build.py` verifies/downloads the pinned SafetyHook/Zydis sources, builds an optimized static-CRT DLL into `bin/` and builds/runs the actual-DLL fixture under `.verify/native/`. No Swofty checkout or user-specific workspace path is required. The vendored SDK and downloaded hook files come from Swofty v1.10.21, commit `70b94d885189e776e899852a4f86dc20067f2b4c`.

CI compiles once with `python native/build.py --build-only`, then runs `python native/build.py --tests-only` against the shared artifacts. The default command does both. No host test launches F1 25 or writes to its installation.

`source/package.py` requires a passing fixture report for the exact DLL being packaged. It writes a ZIP, identical `.swoftyaddon`, individual checksums and `SHA256SUMS.txt` to `dist/`. `PACKAGE_CONTENTS.json` documents every entry's release purpose outside the installable archive. `--output <directory>` selects another output folder.

Keep compiler outputs, dependency downloads, recordings, session logs, game data and machine-specific paths out of Git. Do not copy SDK headers, tests or build files into the installable archive. Changes to the icon, manifest or runtime require package validation before handoff.
