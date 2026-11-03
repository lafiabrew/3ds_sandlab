# SandLab 3DS (`3ds_sandlab`)

<p align="center">
  <img src="docs/icon.png" width="72" height="72" alt="3ds_sandlab icon" />
  &nbsp;&nbsp;&nbsp;
  <img src="docs/banner.png" width="256" height="128" alt="3ds_sandlab 3DS banner" />
</p>

<p align="center">
  <img src="docs/screenshot.png" width="360" alt="3ds_sandlab dual-screen 3DS preview" />
</p>

> **Falling Sand Physics Toy (`CTR-P-SAND`, UniqueId `0x0D3E9`)** — Part of the [**lafiabrew**](https://github.com/orgs/lafiabrew/repositories) Nintendo 3DS Homebrew Suite.

60 FPS Falling-Sand cellular automata physics sandbox supporting Stone Wall, Sand, Water, Fire, Plant, Oil, and Acid reactions.

---

## Architecture & Data Flow

```mermaid
flowchart LR
    STYLUS[Bottom Stylus Brush] --> WORLD[80x48 Cellular Automata Grid]
    WORLD --> STEP[Gravity + Fluid + Fire/Acid Reactions]
    STEP --> RENDER[Top & Bottom Dual View]
```

---

## Controls & Touch Ergonomics

| Button / Input | Action |
|---|---|
| `Stylus Drag` | Paint active element onto the 80x48 physics canvas |
| `L` / `R` (or Tap Top Palette) | Select element (`Eraser`, `Wall`, `Sand`, `Water`, `Fire`, `Plant`, `Oil`, `Acid`) |
| `A` | Drop burst of particles at top center |
| `X` / `Y` | `X`: Pause/Resume physics | `Y`: Clear canvas |

---

## Build, Verify & Install

```bash
make test
make 3ds && make verify-3ds
make qr
```
