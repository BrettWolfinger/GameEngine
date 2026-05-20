# Frogger

A single-player Frogger clone built on the custom C++ engine.

Guide your frog safely home across a busy road and hazardous river. Fill all 5 home slots to win. You have 3 lives.

## Building & Running

From the repository root:

```bash
cmake -B build
cmake --build build --target frogger
./build/games/frogger/frogger
```

## Controls

### In-Game

| Key | Action |
|-----|--------|
| `↑` / `W` | Hop up |
| `↓` / `S` | Hop down |
| `←` / `A` | Hop left |
| `→` / `D` | Hop right |
| `Q` | Quit |

### Game Over / Win Screen

| Key | Action |
|-----|--------|
| `Space` | Play again |

## Gameplay Notes

- **Road zone** — avoid cars, race cars, and trucks. One hit = death.
- **River zone** — hop onto logs, turtles, or crocodiles to cross. Falling in the water = death.
- **Turtles** periodically dive underwater and become unsafe. Watch the dive animation and hop off before they fully submerge.
- **Crocodiles** open their mouths periodically. Landing on the head while the mouth is fully open = death. Body and tail are always safe.
- **Home row** — hop into one of the 5 marked slots to fill it. Landing anywhere else on the home row = death.
- Fill all 5 slots to win the round.
- A skull marks the tile where the frog last died, displayed briefly before the next attempt.
