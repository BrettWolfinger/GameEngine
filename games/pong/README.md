# Pong

A two-player (or single-player vs AI) Pong game built on the custom C++ engine.

First player to 7 points wins.

## Building & Running

From the repository root:

```bash
cmake -B build
cmake --build build --target pong
./build/games/pong/pong
```

## Controls

### Mode Select

| Key | Action |
|-----|--------|
| `Enter` / `1` | 1-Player (vs AI) |
| `Space` / `2` | 2-Player (local) |
| `Escape` | Quit |

### In-Game

| Key | Action |
|-----|--------|
| `W` / `S` | Move left paddle up / down |
| `↑` / `↓` | Move right paddle up / down (2P only) |
| `P` | Pause / unpause |
| `Escape` | Return to mode select |
| `Q` | Quit |

### Win Screen

| Key | Action |
|-----|--------|
| `R` | Rematch (return to mode select) |

## Gameplay Notes

- A 3-second countdown runs before each serve, giving both players time to react.
- Ball speed increases slightly with each paddle hit, up to a maximum.
- In single-player mode, the AI opponent reacts with a slight delay and imperfect aim to keep matches competitive.
