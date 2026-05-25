# Engine

A C++ 2D game engine built on OpenGL 4.1, GLFW, GLM, and CMake. Designed around a
stable facade API so games program against `Engine::Audio::`, `Engine::Particles::`,
and `Engine::Collision::` rather than reaching into subsystem internals.

Games shipped on the engine: **Pong**, **Breakout**, **Frogger**, **Asteroids**.

---

## Where to start

| If you want to… | Read |
|---|---|
| Understand how the engine is structured | [Engine Facade](engine/engine-facade.md) |
| Add a new game | [Adding a New Game](engine/adding-a-new-game.md) |
| Learn the main loop and lifecycle hooks | [Application & Game Loop](engine/application-game-loop.md) |
| Set up colliders and callbacks | [Collision System](engine/collision-system.md) |
| Draw sprites | [Renderer2D](engine/renderer2d.md) |
| Handle keyboard input | [Input](engine/input.md) |
| See the full history of the project | [Devlog](devlog.md) |
| Browse the generated API reference | [API Reference](https://brettwolfinger.github.io/GameEngine/api/){target="_blank" rel="noopener noreferrer"} |
