<div align="center">

<img src="assets/images/logo.png" width="140" alt="fortnite logo"/>

# fortnite

**A driverless external overlay for Fortnite (`FortniteClient-Win64-Shipping.exe`) written in C++20.**

Driverless memory · D3D11 overlay · Aimbot · ESP · Chams · Loot

`Windows x64` · `C++20` · `Visual Studio 2022`

</div>

## Overview

This is an **external** cheat. It runs as its own process, talks to Fortnite through a
**driverless memory layer** (`impl/driverless` + `day1.lib`), and draws an ImGui/D3D11 menu + ESP
by hijacking the game window rather than injecting into it.

The overlay is branded **popstar** (`popstar.rocks` in the menu title).

---

## Features

### Aimbot
- Enable / FOV circle / FOV arrows
- Smoothing, max distance, visible check
- Primary + secondary keybinds
- DualShock / Xbox **controller support** via JoyShockLibrary

### Visuals / ESP
- Boxes, skeleton, china hat
- Player name, rank, platform
- FOV arrows
- Per-state colors (visible / hidden / downed / teammate)

### Other
- Triggerbot
- Loot / container ESP
- Chams
- Radar and customizable crosshair
- VSync toggle

---

## Architecture

```
 ┌──────────────┐   driverless R/W    ┌───────────────────────────────┐
 │  fortnite.exe│ ◄─────────────────► │  FortniteClient-Win64-Shipping│
 │  (overlay)   │   (day1.lib / CR3)  │                               │
 └──────┬───────┘                     └───────────────────────────────┘
        │ hijack HWND / Present
        ▼
 ┌──────────────┐
 │  Game window │
 └──────────────┘
```

Startup (`main.cpp`):

1. `bypass::initialize()` / `bypass::create()`
2. Attach to `FortniteClient-Win64-Shipping.exe`
3. Resolve the game HWND, hijack overlay, set up D3D11
4. Spawn workers: engine, camera, actors, cache, keybinds, guard
5. Enter `render::loop()`

---

## Repository layout

| Path | Purpose |
|------|---------|
| `main.cpp` | Entry point |
| `impl/driverless/` | Driverless memory backend (`day1.h` + `day1.lib`) |
| `impl/controller/` | Controller detection |
| `impl/utils/` | Logging / helpers |
| `workspace/core/unreal-engine/` | SDK, offsets, cache, aimbot, chams, settings |
| `workspace/render/` | Overlay hijack, ImGui menu, render loop |
| `dependencies/` | Vendored ImGui, FreeType, JoyShock, oxorany, lazy-importer |
| `assets/` | Logo, menu icons, and UI fonts |

---

## Requirements

- **Windows 10/11 x64**
- **Visual Studio 2022** with the **v143** toolset
- **C++20** (`LanguageStandard=stdcpp20` on Release x64)
- Must be launched **as Administrator** (`UACExecutionLevel=RequireAdministrator`)
- DirectX 11 runtime (the project also references the **DirectX SDK June 2010** for `d3dx11`)
- `impl/driverless/day1.lib` (prebuilt, required at link time)

Vendored third-party code lives under `dependencies/` — no vcpkg or NuGet needed for ImGui / FreeType / oxorany.

---

## Building

1. Open `fortnite.vcxproj` in Visual Studio 2022.
2. Select **Release | x64**.
3. Build.

Output is written to `..\..\build\fortnite.exe` (`OutDir=..\..\build`, `IntDir=..\..\build\int`).

From the command line:

```bat
msbuild fortnite.vcxproj /p:Configuration=Release /p:Platform=x64
```

### Path caveat

Release|x64 currently hardcodes include/lib paths from another machine:

```
C:\Users\terramog-hypervisor\Desktop\emulated\emulated\fortnite\dependencies\freetype\...
C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\...
```

If those paths do not exist on your PC, retarget them to this repo's `dependencies\freetype` and your local DirectX SDK (or the Windows SDK equivalents) before building.

---

## Usage

1. Launch `fortnite.exe` **as Administrator**.
2. Start Fortnite. The module attaches to `FortniteClient-Win64-Shipping.exe` and hijacks the window.
3. Use the in-game **popstar** menu to toggle features.

---

## Notes on offsets

Game structure offsets live in `workspace/core/unreal-engine/memory/offsets.h` (`Uworld`, `GNames`,
camera, weapon, pawn flags, etc.). These are build-specific and generally need updating after a
Fortnite patch. Decryption helpers live in `workspace/core/unreal-engine/memory/decryption/`.

---

## Credits

- [Dear ImGui](https://github.com/ocornut/imgui) — overlay UI
- [FreeType](https://freetype.org) — font rasterization
- [JoyShockLibrary](https://github.com/JibbSmart/JoyShockLibrary) — controller input
- oxorany — string obfuscation
- lazy-importer — import resolution
- Fonts in `assets/fonts/` — Inter, Space Grotesk, SST, Font Awesome

---

## License

No license is provided. All rights reserved by the respective authors. This repository is shared
for educational purposes; you may not use it commercially or in violation of any game's Terms of
Service. Third-party components remain under their own licenses.
