<div align="center">

<img src="apex/assets/images/logo.png" width="140" alt="popstar logo"/>

# popstar-external

**A collection of external game overlays — driverless / hypervisor-backed memory access with ImGui + D3D11 rendering.**

Aimbot · Visuals · Chams · Loot · Radar

`Windows x64` · `C++20` · `Visual Studio 2022`

</div>

---

## ⚠️ Disclaimer

This repository is published **for educational and research purposes only** — it is a study of
external process instrumentation, driverless and hypervisor-assisted memory access, overlay window
hijacking, and reverse engineering of commercial game engines.

- Using this in an online game **violates the respective publisher's Terms of Service / EULA** and
  will get your account **permanently banned**.
- Use is entirely **at your own risk**. The authors take **no responsibility** for bans, hardware
  flags, account loss, or any other consequences.
- Do **not** use this against other players. Do **not** distribute compiled binaries.
- No warranty of any kind is provided.

If you do not agree with the above, do not use, build, or distribute this project.

---

## Projects

| Project | Target | Memory backend | Overlay |
|---------|--------|----------------|---------|
| [`fortnite/`](fortnite/README.md) | `FortniteClient-Win64-Shipping.exe` | Driverless (`day1.lib`) | D3D11 window hijack |
| [`rust/`](rust/README.md) | `RustClient.exe` | Hypervisor-backed | ImGui overlay |
| [`apex/`](apex/README.md) | `r5apex_dx12.exe` | Hypervisor (VMX hypercalls) | Swap-chain hijack |

Each project is self-contained: open its `.vcxproj`, build **Release | x64**, and read its own
README for details, features, and caveats.

---

## Shared feature set

- **Aimbot** — FOV circle/arrows, smoothing, visible check, multiple keybinds, controller support
- **Visuals / ESP** — boxes, skeleton, names, rank, platform, health/state colors
- **Chams** — via custom depth
- **Loot / item ESP** — category and rarity filters
- **Radar & crosshair** — configurable
- **Overlay UI** — Dear ImGui with FreeType, blurred background, tabbed menu

---

## Repository layout

```
popstar-external/
├── fortnite/   # Fortnite external overlay (driverless)
├── rust/       # Rust external overlay (hypervisor-backed)
└── apex/       # Apex Legends external overlay (VMX hypercalls)
```

Each folder contains its own `README.md` and `.gitignore` with build instructions specific to it.

---

## Shared dependencies

Vendored under each project's `dependencies/`:

- [Dear ImGui](https://github.com/ocornut/imgui) — overlay UI
- [FreeType](https://freetype.org) — font rasterization
- [GLM](https://github.com/g-truc/glm) — math (apex)
- [JoyShockLibrary](https://github.com/JibbSmart/JoyShockLibrary) — controller input (fortnite)
- oxorany — string obfuscation
- lazy-importer — import resolution

UI fonts and the logo live in each project's `assets/` folder.

---

## License

No license is provided. All rights reserved by the respective authors. This repository is shared
for educational purposes; you may not use it commercially or in violation of any game's Terms of
Service. Third-party components remain under their own licenses.