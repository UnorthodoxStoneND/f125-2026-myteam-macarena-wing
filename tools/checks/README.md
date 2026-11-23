# CI and local checks

| Check | Evidence |
| --- | --- |
| Pinned SDK | Vendored API header/licence verified against native/SDK_VERSION.json |
| Native animation/settings | Actual compiled DLL and frame callbacks, valid/invalid saved settings, range/default and zero endpoint |
| Hook/register contract | Named SITE/ON_LOAD registration, mock-core callback dispatch, missing-binding/registration rejection; simulation buffers and unrelated register bits preserved |
| Release ABI | API-version declaration present; fixture-only bypass exports absent; unsupported host rejected |
| Package contents | Exact allowlist, manifest references, valid PNG, managed DLL, required notices and absence of bundled hook/decoder code |
| Versions and hashes | Matching manifest/versioned artifact names, ZIP-only output and SHA-256 sums |
| Package privacy | No source/build files or personal Windows paths in release text |

Run `python native/build.py`, `python source/package.py` and `python tools/checks/package_check.py` from the root. CI separates build-only and tests-only phases, as in F1 Live Telemetry, and shares the compiled outputs between jobs. CI does not run the game; see the compatibility reference for actual user test evidence and its limits.
