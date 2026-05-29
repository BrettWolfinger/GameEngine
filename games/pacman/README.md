# Pac-Man

A Pac-Man clone built on the custom C++ engine.

Eat all dots to clear the stage. Avoid the ghosts — eat a power pellet to turn
the tables and hunt them down for bonus points.

## Building & Running

From the repository root:

```bash
cmake -B build
cmake --build build --target pacman
./build/games/pacman/pacman
```

## Controls

| Key | Action |
|---|---|
| Arrow keys | Move Pac-Man |
| Q | Quit |
| R | Restart (game over screen) |

## Gameplay Notes

- Eat all dots to clear the stage and advance to the next level
- Power pellets (large dots in the four corners) turn ghosts blue — chase them down for bonus points
- Eating multiple ghosts on a single pellet doubles the score each time: 200 → 400 → 800 → 1600
- Each dot is worth 10 points; each power pellet is worth 50 points
- Earn an extra life at 10,000 points
- You start with 3 lives; losing all lives ends the game

## Credits

Sound effects from [classicgaming.cc](https://classicgaming.cc/classics/pac-man/sounds).

Art assets from [Pac-Man Practice Assets](https://checkpointcafe.itch.io/pacman-practice-assets) by checkpointcafe on itch.io.
