# Breakout

A single-player Breakout clone built on the custom C++ engine.

Clear all bricks to win. You have 3 lives.

## Building & Running

From the repository root:

```bash
cmake -B build
cmake --build build --target breakout
./build/games/breakout/breakout
```

## Controls

### Title Screen

| Key | Action |
|-----|--------|
| `Space` | Start game |
| `Q` | Quit |

### In-Game

| Key | Action |
|-----|--------|
| `←` / `→` | Move paddle left / right |
| `Space` | Launch ball (when held) |
| `Q` | Quit |

### Game Over / Win Screen

| Key | Action |
|-----|--------|
| `R` | Restart |

## Gameplay Notes

- Ball speed increases every 5 bricks destroyed, up to a maximum.
- Angle of the ball after a paddle hit is influenced by where on the paddle it lands — hitting the edges sends the ball at a sharper angle.
- Brick rows are worth different points: gray (1), green (3), orange (5), red (7).
- The first time the ball reaches the ceiling the paddle narrows — a penalty for using the ceiling to your advantage.
- High score is saved between sessions and displayed in gold at the top of the screen.
