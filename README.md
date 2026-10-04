# LET IT DIE PC Fixes

Small Windows x64 D3D9 wrappers for LET IT DIE Offline on Steam.

| Download | Features |
| --- | --- |
| [Borderless 0.1.1](downloads/LET-IT-DIE-Borderless-0.1.1.zip) | Borderless windowed presentation |
| [Borderless + Texture Fix 0.2.0](downloads/LET-IT-DIE-Borderless-Texture-Fix-0.2.0.zip) | Borderless presentation and automatic disabling of engine mip fading |

Install one version at a time.

## Install

1. Close the game. In Steam, choose **Browse local files**, then open `Binaries\Win64` beside `BrgGame-Steam.exe`.
2. Back up any existing `d3d9.dll` outside the game folder. Copy the DLL from your chosen download into `Binaries\Win64`.
3. Use the game's Fullscreen setting. The working configuration uses `mbFullScreen=True` in `BrgGame\Config\BrgGraphicsConfig.ini`; back up that file before editing it manually.
4. Launch normally. Actual presentation is borderless windowed, allowing desktop chat overlays to appear above the game. The combined version applies its supported texture fix automatically.

Do not edit `SteamPCRelease-BrgEngine.ini` or `SteamPCRelease-BrgGHMEngine.ini`. No launch options or F8 switch are required. Other mods that supply `d3d9.dll` cannot simply be installed alongside this one.

## Texture-fix compatibility

The texture fix disables the engine's mip-fade transition that caused repeatable blur and delayed sharpening during the tested boundary crossing. It requires this exact executable SHA-256:

`716046F09F398A61B359CC1DD08366F3FD1D867605949D6729E25C3A07A4DBAF`

The DLL also checks relevant loaded-image signatures and the engine control before changing it. On mismatch, it leaves the texture control untouched and continues providing borderless mode. A game update requires revalidation. The fix does not guarantee maximum detail for every texture or resolve unrelated streaming issues.

Both versions apply window changes only at device creation/reset. The combined version applies its texture change at those events too. There is no logging, hotkey handling, background thread, polling, per-frame enforcement or forced window stacking order. The DLL changes presentation/runtime state, not game files, saves or gameplay databases.

## Validation

Borderless 0.1.1 was confirmed working in-game with a desktop chat overlay. Manually disabling the engine mip-fade control resolved the tested texture blur without new issues noticed during initial testing. Combined 0.2.0 passed local checks; its automatic startup behavior still needs in-game confirmation. Longer play, other builds, multiple monitors and other graphics wrappers are unverified.

## Remove

Close the game, then remove this package's DLL or restore your backed-up DLL. Restore your graphics configuration if you changed it. Restarting without the combined DLL restores the engine's normal mip-fade behavior.

## Build

Sources and local tests are in `src/borderless` and `src/borderless-texture`. Run the chosen directory's `build.cmd` with Visual Studio 2022 Build Tools and the Windows SDK installed. Builds are x64 with a static C++ runtime. `SHA256SUMS.txt` lists the downloadable packages; each package also includes its DLL checksum.

## Licence and attribution

MIT; see [LICENSE](LICENSE). The borderless wrapper follows the approach in [LET IT DIE Native Borderless](https://github.com/NickLovera/LET-IT-DIE-Native-Borderless), with revised window/reset behavior. These are independent builds, not upstream releases.
