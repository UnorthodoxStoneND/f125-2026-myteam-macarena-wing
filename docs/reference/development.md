# Development

This repository follows the F1 Live Telemetry layout, with native-only build steps for this mod.

| Path | Purpose |
| --- | --- |
| `native/` | Native animation, live profile bridge, SDK and actual-DLL host tests |
| `source/` | Installable-package builder and editable icon source |
| `tools/checks/` | SDK pin and archive validation |
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

`native/build.py` verifies the vendored SDK pin, builds an optimized static-CRT DLL into `bin/` and builds/runs the actual-DLL fixture under `.verify/native/`. It compiles against Swofty v1.10.30's API v9 header, pinned at commit `64bf599ac6232729707773a2413f51fd1fdd3614`. No dependency download or external hook library is needed. The addon resolves `site_Car_RearWingRenderFraction` and registers a SITE callback plus ON_LOAD notification; Swofty owns installation and chaining. The Steam executable/layout guard validates the named resolved site without a hardcoded address or fallback hook.

CI compiles once with `python native/build.py --build-only`, then runs `python native/build.py --tests-only` against the shared artifacts. The default command does both. No host test launches F1 25 or writes to its installation.

`source/package.py` requires a passing fixture report for the exact DLL being packaged. It writes a ZIP, its checksum and `SHA256SUMS.txt` to `dist/`. `PACKAGE_CONTENTS.json` documents every entry's release purpose outside the installable archive. `--output <directory>` selects another output folder.

Keep compiler outputs, dependency downloads, recordings, session logs, game data and machine-specific paths out of Git. Do not copy SDK headers, tests or build files into the installable archive. Changes to the icon, manifest or runtime require package validation before handoff.
