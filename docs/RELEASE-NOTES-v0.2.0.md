# v0.2.0 — LET IT DIE Offline Fixes

Full release for LET IT DIE Offline on Steam, Windows x64. Both download packages have been checked and approved by the maintainer.

## Downloads

- **LET-IT-DIE-Borderless-Texture-Fix-0.2.0.zip** — borderless fullscreen plus the automatic texture-blur fix.
- **LET-IT-DIE-Borderless-0.1.1.zip** — borderless fullscreen only.
- **SHA256SUMS.txt** — SHA-256 checksums for both ZIPs.

Choose one package. The GitHub-generated “Source code” archives are for developers, not installation.

## Install

Close the game, extract your chosen ZIP and copy its d3d9.dll into Binaries\Win64 beside BrgGame-Steam.exe. Back up any existing DLL first. Enable Fullscreen in the game's settings and launch normally.

## Testing and compatibility

The automatic texture fix was tested for several hours in-game with no crashes or other issues observed. Borderless mode was confirmed with a desktop chat overlay.

The texture fix checks the executable and engine state before applying. On mismatch it leaves the texture setting untouched; borderless mode remains available. Game updates require revalidation. Other game builds, multi-monitor setups and other graphics wrappers are unverified.

See the repository README for removal and full compatibility details. Report problems through Issues.
