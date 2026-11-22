# CI and local checks

| Check | Evidence |
| --- | --- |
| Pinned build dependencies | SHA-256-verified sources from the locked upstream commit |
| Native animation/settings | Actual compiled DLL and frame callbacks, valid/invalid saved settings, range/default and zero endpoint |
| Hook/register contract | Real mid-hook handoff/rollback; simulation buffers and unrelated register bits preserved |
| Release ABI | API-version declaration present; fixture-only bypass exports absent; unsupported host rejected |
| Package contents | Exact allowlist, manifest references, valid PNG, compiled DLL and required notices |
| Versions and hashes | Matching manifest/versioned artifact names, ZIP-only output and SHA-256 sums |
| Package privacy | No source/build files or personal Windows paths in release text |

Run `python native/build.py`, `python source/package.py` and `python tools/checks/package_check.py` from the root. CI separates build-only and tests-only phases, as in F1 Live Telemetry, and shares the compiled outputs between jobs. CI does not run the game; see the compatibility reference for actual user test evidence and its limits.
