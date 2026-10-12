# Mosslight

A relaxing wildlife garden game for the Game Boy Color, built with GBDK-2020.

## About

Mosslight is a cozy nature game where you tend a wildlife garden, plant native flowers, and attract animals to your sanctuary. The ROM runs on real Game Boy Color hardware or any GBC emulator.

## Features

- 6 explorable areas (Main Garden, Pond Edge, Old Grove, Moon Meadow, Gloamwick Town, Woodland Trail)
- 22 plantable native species (flowers, grasses, shrubs, trees, aquatic plants, fungi)
- 12 placeable garden objects (logs, rocks, bird baths, benches, and more)
- 28 wildlife species to discover, from bees and butterflies to owls and herons
- Day/night cycle with 4 time periods and weather system
- Data-driven wildlife spawning based on habitat tags
- Battery-backed save system
- Full Game Boy Color graphics with custom tile and sprite art
- Original chiptune music using the GBC sound chip

## Building

Requires GBDK-2020. Extract the GBDK bundle and set the path in the Makefile:

```bash
make clean && make
```

The compiled ROM will be at `build/mosslight.gb`.

## Playing

### Controls

| Key | Action |
|-----|--------|
| D-Pad | Move |
| A | Interact / Confirm |
| B | Use equipped tool |
| Start | Menu |
| Select | Field Guide |

### Emulator

The ROM runs in any Game Boy Color emulator (mGBA, SameBoy, PyBoy, BGB, etc.). A web player is included in `src/web/index.html` using EmulatorJS.

## Project Structure

```
src/
  main.c          - Game loop and state machine
  engine.c        - Overworld update/draw, camera, time, weather
  maps.c          - Area generation and tile rendering
  player.c        - Player movement and interaction
  garden.c        - Planting, tools, growth, habitat computation
  wildlife.c      - Wildlife spawning and behavior
  sprites.c       - Sprite data for player and wildlife
  tiles.c         - BG tile data and palettes
  text.c          - Font and text rendering
  audio.c         - Sound effects and music
  ui.c            - Menus, shop, field guide, tutorial, ending
  input.c         - Joypad input handling
  save.c          - SRAM save/load
  banks/bank2.c   - Data tables (names, strings)
  include/        - Shared headers and data table definitions
supabase/
  functions/      - Edge function for ROM download
  config.toml     - Supabase project config
```

## Tech

- **Language:** C (SDCC compiler via GBDK-2020)
- **Platform:** Game Boy Color (Z80-like CPU, 32KB ROM, 4 VRAM palettes)
- **Graphics:** 8x8 tiles, 4-color palettes, custom pixel art
- **Audio:** GBC sound chip (2 square waves, 1 wave, 1 noise channel)
- **Save:** Battery-backed SRAM (8KB)

## License

Personal project.
