# Technical details

## Texture fix

Version 0.2.0 disables the engine's mip-fade transition that caused repeatable blur and delayed sharpening during the tested boundary crossing.

Supported `BrgGame-Steam.exe` SHA-256:

```text
716046F09F398A61B359CC1DD08366F3FD1D867605949D6729E25C3A07A4DBAF
```

Before changing the control, the DLL checks the executable hash, loaded-image properties and signatures, memory protection, and expected engine value. A mismatch leaves the texture control untouched. Game updates require revalidation.

Both versions change window presentation at device creation/reset. The combined version applies its texture change at those events too. There is no logging, hotkey handling, background thread, polling, per-frame enforcement, or forced window stacking order.

The DLL changes runtime state rather than game files, saves, or gameplay databases.

## Graphics configuration

Use the game's Fullscreen setting. The tested configuration uses `mbFullScreen=True` in `BrgGame\Config\BrgGraphicsConfig.ini`. Back up that file before editing it manually.

Do not edit `SteamPCRelease-BrgEngine.ini` or `SteamPCRelease-BrgGHMEngine.ini`.

## Build

Sources and local tests:

- [Borderless](../src/borderless)
- [Borderless + texture fix](../src/borderless-texture)

Install Visual Studio 2022 Build Tools with C++ tools and the Windows SDK, then run the chosen directory's `build.cmd`. The scripts expect Build Tools at its default installation path.

Builds use x64, C++17, a static C++ runtime, and warnings as errors. The scripts run unit tests, DLL export/header inspection, and a smoke-load test; the combined build also runs engine tests.

[SHA256SUMS.txt](../SHA256SUMS.txt) lists the downloadable ZIP checksums. Each package also includes its DLL checksum.

## Validation record

- Borderless 0.1.1 was confirmed in-game with a desktop chat overlay.
- Combined 0.2.0 passed the previously recorded local checks.
- On 5 October 2026, the maintainer reported testing the automatic texture fix for several hours the previous night, with the fix working and no crashes or other issues observed.
- On 5 October 2026, the maintainer downloaded and checked both updated packages and approved them for full release.
- Package documentation and attribution were refreshed; the DLLs and their internal checksums were verified unchanged. Repository ZIP checksums were updated after repacking.
- Other game builds, multi-monitor setups and other graphics wrappers remain unverified.
