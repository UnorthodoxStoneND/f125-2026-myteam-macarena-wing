# Publishing

Follow the same source-to-release workflow used by F1 Live Telemetry:

1. Make changes on a branch, run the native tests, package and validate the archive.
2. Open a PR against main and wait for the repository's checks before merging.
3. Tag the reviewed main commit with the version from `swofty.json`.
4. Attach the exact final ZIP and `SHA256SUMS.txt` to the GitHub release. Record automated and in-game checks separately.

The workflow builds artifacts for review; it does not publish releases or upload to Swofty automatically. Previously handed-over release archives outside this checkout are preserved. A fresh source build may have different binary/archive hashes because compiler/dependency inputs differ; identify the exact final artifact and retain its validation results before publication.

The runtime package allowlist is in `source/package.py`. Every entry must be the manifest, a referenced feature file, or required release documentation/licence notice. `tools/checks/package_check.py` rejects extra source/build content and verifies references and hashes.

For any Addon Browser submission, check the current launcher-bindings requirements and supported launcher exports as well as manifest, icon, platform, ownership and testing requirements. GitHub publication does not itself establish Browser eligibility. Version 1.0.0 uses the launcher-provided rear-wing binding.

References: https://f1.swofty.net/reference/addons/publishing.html and https://f1.swofty.net/reference/addons/manifest.html.
