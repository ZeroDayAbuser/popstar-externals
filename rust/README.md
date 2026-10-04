<div align="center">

<img src="assets/images/logo.png" width="140" alt="rust-external logo"/>

# rust-external

**A modular external overlay for Rust (`RustClient.exe`) written in modern C++.**

Overlay UI · Hypervisor-backed memory · Aimbot · Visuals · Movement · RPC · PhysX

`Windows x64` · `C++20` · `Visual Studio 2022 (v143)`

</div>

---

## ⚠️ Disclaimer

This project is published **for educational and research purposes only** — it is a study of
external process instrumentation, overlay rendering, hypervisor-assisted memory access, and
reverse engineering of a Unity/IL2CPP title.

- Using this in an online game **violates the game's Terms of Service** and will get your
  account **permanently banned**.
- Use is entirely **at your own risk**. The authors take **no responsibility** for bans,
  hardware flags, account loss, or any other consequences.
- Do **not** use this against other players. Do **not** distribute compiled binaries.
- No warranty of any kind is provided (see [License](#license)).

If you do not agree with the above, do not use, build, or distribute this project.

---

## Overview

`rust-external` is an **external** cheat: it runs as its own process (an ImGui/D3D11 overlay
window) and never injects into the game. All game interaction happens through a **memory
backend** (driver + hypervisor) that reads and writes the target process, and an **inline-syscall**
layer used to keep the cheat's own calls off the usual user-mode hooks.

The codebase is split into a portable core (SDK, memory, features, overlay) and two interchangeable
menu frameworks, so the UI can be swapped without touching game logic.

---

## Features

### Aimbot
- Aimbot with configurable FOV, smoothing, and hitbox selection
- Silent aim / target selection, prediction, curve
- Triggerbot

### Visuals
- Player ESP: boxes, skeletons, health, names, weapons, inventory
- Per-element colors, thickness, and toggles (`player_settings`)
- World visuals, tracers, threat arrows, radar
- On-screen widgets (FPS counter, top bar)

### Misc — Movement
- Fly · Spider-Man · Speed hack · Omni-sprint · Silent walk · No fall · Walk on water · Anti-aim

### Misc — Weapon
- No spread · No recoil · Rapid fire · Automatic weapons · Instant bow · Instant Eoka
- Melee range · Thick bullet · Hitbox override

### Misc — RPC
- Instant revive · Untie crate · Instant interactions · Fast loot

### Misc — Animations / Local world
- Viewmodel swap/hide · Viewmodel no-lower · No animation · No sway · In-gesture
- FOV changer · Local chams · Debug camera
- Time-of-day / sky control · Bright night · Layer toggle · Remove water drag

### Other
- `physx` — physics-related feature set
- `bundle_icons` — Unity asset-bundle icon extraction (`lz4`)

---

## Architecture

```
 ┌─────────────┐     IPC/transport     ┌────────────────────┐
 │   Overlay   │  ◄─────────────────►  │   Memory backend   │
 │ (ImGui D3D11│                       │  driver + hypervisor│
 │   window)   │                       │  (hvre / hypercall) │
 └─────────────┘                       └─────────┬──────────┘
        ▲                                        │ read / write
        │ draw                                   ▼
 ┌──────┴───────┐                       ┌────────────────────┐
 │   Features   │ ──► SDK (entity,      │  Target process    │
 │ aim / vis /..│      bones, weapon,   │  RustClient.exe    │
 └──────────────┘      unity, gchandle)└────────────────────┘
```

- **Memory** — `src/memory/driver/` talks to a kernel component; `hypervisor/` provides a
  VMX-based backend (`vmexit.asm`, hypercalls). `camouflage.cpp`/`memory.cpp` wrap access.
- **Inline syscalls** — `ext/isyscall/` builds and issues direct syscalls
  (`initial_syscall_gadget.asm`) so sensitive calls don't go through user-mode hooks.
- **SDK** — `src/sdk/rust/` models the game's IL2CPP/Unity objects: `entity`, `bones`, `weapon`,
  `gchandle`, `unity_bundle` (with an LZ4 decoder), plus math (`matrix`, vectors).
- **Obfuscation** — per-build seed (`scripts/gen_build_seed.ps1` → `build_seed_generated.hpp`),
  compile-time string encryption (`ext/string_encryption.hpp`, `XorStr`), and VMProtect hooks
  in the `Stable` configuration.

---

## Repository layout

| Path | Purpose |
|------|---------|
| `src/main.cpp` | Entry point (both `main` and `DllMain`/`STABLE` variants) |
| `src/memory/` | Driver + hypervisor memory backends, camouflage |
| `src/sdk/` | Rust/Unity SDK, math, offsets, obfuscation seed |
| `src/game/` | Cache, features (aimbot/visuals/misc/physx), bundle icons |
| `src/menu/` | Built-in ImGui menu + widgets + tabs |
| `src/gui/`, `src/app/` | Overlay app, D3D11 backend, window |
| `src/auth/` | License / auth gate (PWFLicense) |
| `src/settings/`, `framework/` | Config + UI framework |
| `ext/gordoui/`, `ext/popstar/` | Alternate menu frameworks |
| `ext/isyscall/` | Inline-syscall engine (MASM gadget) |
| `ext/portable_executable/` | PE parsing / manual-mapping helpers |
| `assets/`, `resources/` | Fonts, icons, embedded bundle |
| `tools/physx_dumper/` | PhysX offset dumper |

---

## Requirements

- **Windows 10/11 x64**
- **Visual Studio 2022** with the **v143** toolset and **MASM** (`vcvars`/`masm` build
  customizations are imported by the project)
- **[vcpkg](https://github.com/microsoft/vcpkg)** in manifest mode — static triplet
  `x64-windows-static`. Dependencies include: `curl`, `openssl`, `zlib`, `brotli`, `bzip2`,
  `freetype`, `libpng`, `spdlog`, `zydis`, `zycore`, `asmjit`.
- A working **driver / hypervisor** backend for the memory layer (the kernel side is not part
  of this repository).
- *(Optional)* **VMProtect Professional** — required only by the `Stable` configuration.

---

## Building

1. Install vcpkg and set it up for static, x64 builds:

   ```bat
   git clone https://github.com/microsoft/vcpkg
   .\vcpkg\bootstrap-vcpkg.bat
   .\vcpkg\vcpkg integrate install
   ```

2. Open `rust-module.sln` in Visual Studio 2022.

3. Select a configuration:

   | Configuration | Type | Notes |
   |---------------|------|-------|
   | `Debug\|x64` | Application | Debug overlay, console |
   | `Release\|x64` | Application | Optimized, AVX2, CFG/CET disabled for manual mapping |
   | `Stable\|x64` | Dynamic Library | Adds VMProtect + LTO |

4. Build. The pre-build step (`scripts/gen_build_seed.ps1`) regenerates the per-build
   obfuscation seed automatically.

From the command line:

```bat
msbuild rust-module.sln /p:Configuration=Release /p:Platform=x64
```

---

## Usage

1. Launch `RustClient.exe`.
2. Start `rust-external` (pass your license key if required: `--key <KEY>`, or set `PS_KEY`).
3. Press **INSERT** to toggle the menu.
4. Press **ALT + P** to toggle the debug console.
5. Press **DELETE** to panic-clean and exit.

> The memory backend must already be loaded/available before the cheat initializes, otherwise
> `memory::is_initialized()` returns false and the process exits.

---

## Configuration

Settings and configs are managed by `src/settings/` and rendered by the settings tab. The SDK
offsets live in `src/sdk/offsets.hpp`; game-structure definitions in `src/sdk/rust/`. These are
build-specific and usually need updating after a game patch.

---

## Credits

- [Dear ImGui](https://github.com/ocornut/imgui) — overlay UI
- `gordoui`, `popstar` — bundled menu frameworks
- [Zydis](https://github.com/zyantific/zydis) / [AsmJit](https://github.com/asmjit/asmjit) — disassembly / JIT
- [spdlog](https://github.com/gabime/spdlog), [stb](https://github.com/nothings/stb) — logging & image loading
- Font Awesome, Montserrat, PixelMix — UI fonts

---

## License

No license is provided. All rights reserved by the respective authors. This repository is shared
for educational purposes; you may not use it commercially or in violation of any game's Terms of
Service. Third-party components remain under their own licenses.
