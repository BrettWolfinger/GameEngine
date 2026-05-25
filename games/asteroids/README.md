# Asteroids

A single-player Asteroids clone built on the custom C++ engine.

Destroy all asteroids to advance to the next wave. You have 3 lives — earn an
extra life every 1000 points.

## Building & Running

From the repository root:

```bash
cmake -B build
cmake --build build --target asteroids
./build/games/asteroids/asteroids
```

## Controls

### Title Screen

| Key | Action |
|-----|--------|
| `↑` / `↓` | Navigate menu |
| `Enter` | Confirm |
| `Q` | Quit |

### Ship Select

| Key | Action |
|-----|--------|
| `←` / `A` | Previous ship |
| `→` / `D` | Next ship |
| `Enter` | Confirm selection |
| `Escape` | Back to title |
| `Q` | Quit |

### In-Game

| Key | Action |
|-----|--------|
| `←` / `A` | Rotate left |
| `→` / `D` | Rotate right |
| `↑` / `W` | Thrust |
| `Space` | Fire |
| `Q` | Quit |

### Game Over

| Key | Action |
|-----|--------|
| `R` | Play again (returns to ship select) |
| `Q` | Quit |

## Ships

| Ship | Speed | Handling | Fire Rate |
|------|-------|----------|-----------|
| Fighter | ★★★ | ★★★ | ★★★ |
| Scout | ★★★★★ | ★★★★ | ★★ |
| Gunship | ★★ | ★★ | ★★★★★ |
| Racer | ★★★★ | ★★★★ | ★★★★ |

## Gameplay Notes

- Waves start with 4 large asteroids spawned at a safe distance from the ship.
  Each cleared wave adds more.
- Large asteroids split into medium fragments on destruction; medium asteroids
  split into small fragments.
- A UFO appears roughly 15 seconds into each session and fires at the player.
  It respawns every 20 seconds until the game ends.
- High score is saved between sessions.
- All objects wrap around the screen edges.

## Credits

Sprites from [Asteroids Game Sprites Atlas](https://opengameart.org/content/asteroids-game-sprites-atlas)
on OpenGameArt.org.
