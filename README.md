# LET IT DIE Offline Fixes

Borderless fullscreen and a texture-blur fix for **LET IT DIE Offline on Steam**, Windows x64.

## Download

| Version | What it does |
| --- | --- |
| [Borderless + Texture Fix 0.2.0](https://github.com/JayKayCreep/LET-IT-DIE-Offline-fixes/raw/refs/heads/main/downloads/LET-IT-DIE-Borderless-Texture-Fix-0.2.0.zip) | Borderless fullscreen and automatic removal of the engine's mip-fade transition, addressing the tested blur and delayed sharpening. |
| [Borderless 0.1.1](https://github.com/JayKayCreep/LET-IT-DIE-Offline-fixes/raw/refs/heads/main/downloads/LET-IT-DIE-Borderless-0.1.1.zip) | Borderless fullscreen only. |

**Choose one version.** Use 0.2.0 for both fixes, or 0.1.1 if you only need borderless mode.

## Install

1. Close the game. In Steam, select **Manage → Browse local files**, then open `Binaries\Win64` (the folder containing `BrgGame-Steam.exe`).
2. Extract your chosen ZIP. If that folder already contains `d3d9.dll`, back it up outside the game folder before copying the downloaded `d3d9.dll` there.
3. Enable **Fullscreen** in the game's settings and launch normally.

The game will use borderless windowed presentation, allowing desktop chat overlays above it. Version 0.2.0 also applies the texture fix automatically on a supported executable.

No launch options or hotkeys are required. Do not edit `SteamPCRelease-BrgEngine.ini` or `SteamPCRelease-BrgGHMEngine.ini`. Other mods supplying `d3d9.dll` cannot be installed alongside this one directly.

## Compatibility and testing

- Intended for the Windows x64 Steam offline edition.
- **0.2.0:** the maintainer reports several hours of in-game testing with the automatic texture fix working and no crashes or other issues observed.
- **0.1.1:** borderless mode confirmed in-game with a desktop chat overlay.
- The texture fix checks the executable and engine state before applying. If compatibility checks fail, it leaves the texture setting untouched; borderless mode remains available.
- Game updates require revalidation. Other game builds, multi-monitor setups and other graphics wrappers are unverified.
- This addresses the tested mip-fade blur; it does not guarantee maximum detail for every texture or fix all streaming problems.

See [technical details and build instructions](docs/TECHNICAL.md) and [download checksums](SHA256SUMS.txt).

## Remove

Close the game and delete this fix's `d3d9.dll`, or restore the DLL you backed up. Restore your graphics configuration if you changed it manually. Launching without the combined DLL restores normal engine mip fading.

## Report a problem

[Open an issue](https://github.com/JayKayCreep/LET-IT-DIE-Offline-fixes/issues) with the fix version, game version, Windows version, GPU, other graphics mods, and steps to reproduce. Include a screenshot or short clip if useful.

## Licence

[MIT](LICENSE). Based on the approach used by [LET IT DIE Native Borderless](https://github.com/NickLovera/LET-IT-DIE-Native-Borderless); see [attribution](NOTICE.md). These are independent builds.
