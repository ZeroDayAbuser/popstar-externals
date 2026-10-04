<div align="center">

<img src="assets/images/logo.png" width="140" alt="apex-module logo"/>

# apex-module

**A modular external overlay for Apex Legends (`r5apex_dx12.exe`) written in modern C++.**

Hypervisor memory · Window-hijack overlay · Aimbot · ESP · Loot · Heirloom

`Windows x64` · `C++ latest` · `Visual Studio 2022`

</div>

---

## Overview

`apex-module` is an **external** cheat. It runs as its own process, talks to Apex through a
**hypervisor-backed memory layer** (VMX / hypercalls — `hvre`, `hypercall`, `vmexit.asm`), and
draws an ImGui/D3D11 menu+ESP by **hijacking the game window's swap chain** rather than injecting
into the game.

It obtains the target `CR3` from the hypervisor and reads/writes the game via the hypervisor's
memory primitives, so the overlay never touches the game process directly.

---

## Features

### Visuals / ESP
- Player boxes, health/shield bars, names, distance, indicators
- Skeleton from resolved bone data
- Snaplines / tracers, threat indicators
- Configurable colors, thickness, and per-element toggles

### Aimbot
- Target selection with configurable FOV, smoothing, and bone priority
- Prediction

### Loot
- Item ESP with category / rarity filters (`items`, `types`, `survival`)
- Customizable filter tokens and per-item colors

### Heirloom
- Dedicated heirloom feature module (`heirloom` + `heirloom_data`)

### Core / SDK
- `sdk/classes` — game object wrappers (entity, player, weapon, ...)
- `sdk/cache` — entity/player caching (`cache->tick()`)
- `sdk/offsets`, `sdk/data`, `sdk/math` (via GLM)

---

## Architecture

```
 ┌──────────────┐   hypercalls    ┌───────────────────┐
 │  apex-module │ ◄─────────────► │  Hypervisor (VMX) │
 │  (overlay)   │                 │  hvre / vmexit.asm│
 └──────┬───────┘                 └─────────┬─────────┘
        │ hook window (Present)             │ read / write / CR3
        ▼                                   ▼
 ┌──────────────┐                 ┌───────────────────┐
 │  Game window │                 │  r5apex_dx12.exe  │
 └──────────────┘                 └───────────────────┘
```

- **Driver / hypervisor** — `src/driver/` (`driver.cc`) hosts the platform glue and the `athena`
  hypervisor backend (`hvre`, `hypercall`, `hypercall_def.h`, `vmexit.asm`), plus `ia32` and
  `requests` helpers.
- **Overlay** — `src/overlay/hijack.cc` hooks the Apex window/swap chain and renders with ImGui
  (D3D11) using the vendored `dependencies/overlay` drawing + fonts.
- **Cheat** — `src/cheat/` holds the feature modules (`aimbot`, `esp`, `loot`, `heirloom`) and
  the shared `helper`.
- **Utility** — `src/utility/` (`logger`, `global`).

---

## Repository layout

| Path | Purpose |
|------|---------|
| `src/entry.cc` | Entry point — sets up hypervisor, attaches, starts feature threads, hooks overlay |
| `src/driver/` | Driver + `athena` hypervisor backend and requests |
| `src/overlay/` | Window/swap-chain hijack for the ImGui overlay |
| `src/cheat/` | Aimbot, ESP, loot, heirloom, helpers |
| `src/sdk/` | Classes, cache, offsets, data, math |
| `src/utility/` | Logger + globals |
| `dependencies/` | Vendored `glm`, `freetype`, `imgui`, `stb_image`, overlay drawing |
| `assets/` | Fonts + logo |
| `bin/` | Build output (ignored) |

---

## Requirements

- **Windows 10/11 x64**
- **Visual Studio 2022** with the **v143** toolset (Release x64) — Debug configs target `v145`
- **MASM** build customization (imported by the project; `vmexit.asm`)
- A working **hypervisor / driver** component (the VMX side is required at runtime)
- Must be launched **as Administrator**

All third-party libraries are vendored under `dependencies/` — no vcpkg or NuGet needed.

---

## Building

1. Open `apex-module.vcxproj` in Visual Studio 2022.
2. Select **Release | x64**.
3. Build.

Output is written to `bin\apex-module.exe` (`OutDir=bin\`, `IntDir=bin\objs`).

From the command line:

```bat
msbuild apex-module.vcxproj /p:Configuration=Release /p:Platform=x64
```

---

## Usage

1. Make sure the hypervisor component is loaded.
2. Launch `apex-module.exe` **as Administrator**.
3. Start Apex Legends (`r5apex_dx12.exe`). The module waits for the `Apex Legends` window, then
   attaches and hijacks it.
4. Use the in-game overlay menu (activated by the overlay's hotkey) to toggle features.

The console logs the attach status, base address, and resolved `CR3`.

---

## Notes on offsets

Game structure offsets live in `src/sdk/offsets/offsets.cuh` and `src/sdk/data.cuh`. These are
build-specific and generally need updating after an Apex patch.

---

## Credits

- [Dear ImGui](https://github.com/ocornut/imgui) — overlay UI
- [GLM](https://github.com/g-truc/glm) — math
- [FreeType](https://freetype.org) / [stb](https://github.com/nothings/stb) — fonts & image loading
- Font Awesome, Inter, Space Grotesk, SST — UI fonts

---

## License

No license is provided. All rights reserved by the respective authors. This repository is shared
for educational purposes; you may not use it commercially or in violation of any game's Terms of
Service. Third-party components remain under their own licenses.
