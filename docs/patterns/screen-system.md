# Screen System (Game-Level Pattern)

## Overview

This is not an engine system — it's a structural pattern used in games until the engine scene system (issue #53) is implemented. It organises a game's distinct states (title, ship select, playing, game over, etc.) into separate screen classes that each own their own update and render logic. The game class becomes a thin router that owns shared data and delegates to the active screen.

The pattern has three parts: a **GameContext** struct, a set of **screen classes**, and a **Screen enum** that identifies which is active.

---

## GameContext

`GameContext` is a plain struct of references into the game class's member variables. It is constructed once by the game class and passed to every screen at construction time. Screens hold a `GameContext&` and access all shared state through it.

```cpp
struct GameContext {
    Engine::Renderer2D&                     renderer;
    std::shared_ptr<Engine::SpriteSheet>&   sheet;
    std::mt19937&                           rng;
    Engine::SaveData&                       saveData;
    Engine::Menu&                           titleMenu;

    std::vector<std::unique_ptr<Asteroid>>& bgAsteroids;
    std::optional<Ship>&                    ship;
    std::vector<std::unique_ptr<Bullet>>&   bullets;
    std::vector<std::unique_ptr<Asteroid>>& asteroids;

    int&   score;
    int&   highScore;
    // ... etc.
};
```

**Every field must be a reference.** If a field is a value, it captures a copy at construction time and won't reflect changes made in the game class constructor body (the crash-prone failure mode). This is the one invariant to enforce when adding new context fields.

---

## Screen Enum

A plain enum class in `GameContext.h` identifies every screen the game can be in, plus a `Quit` sentinel:

```cpp
enum class Screen { Title, ShipSelect, Playing, GameOver, Quit };
```

The game class holds the active screen as `Screen m_screen`. Screen `update()` methods return the screen they want to transition to — returning their own value means "stay here".

---

## Screen Classes

Each screen is a concrete class with a consistent interface:

```cpp
class SomeScreen {
public:
    explicit SomeScreen(GameContext& ctx);

    void   onEnter();          // called once when this screen becomes active
    void   preStep(float dt);  // optional — move objects before collision
    Screen update(float dt);   // input + logic; returns next Screen
    void   render();           // draw everything for this screen
};
```

Not all methods are required. `GameOverScreen`, for instance, has no `preStep` because nothing moves during it.

### onEnter

`onEnter()` is called by the game class exactly once each time this screen is transitioned into. It handles any setup that should happen on entry — spawning objects, resetting state, constructing game entities — rather than in the constructor, which only runs once.

```cpp
// PlayingScreen::onEnter — called every time the player starts a new game
void PlayingScreen::onEnter() {
    m_ctx.ship.emplace(m_ctx.sheet, ShipConfigs::All[m_ctx.selectedShip]);
    spawnInitialAsteroidRing();
}
```

### update

`update()` processes input and game logic, then returns a `Screen` value:

```cpp
Screen PlayingScreen::update(float dt) {
    if (Engine::Input::isKeyPressed(GLFW_KEY_Q)) return Screen::Quit;
    // ... gameplay logic ...
    if (m_ctx.lives <= 0) return Screen::GameOver;
    return Screen::Playing;  // stay on this screen
}
```

Returning a different screen signals a transition. The game class detects the change and calls `transitionTo()`.

---

## Game Class Structure

The game class owns all data, constructs the context, and routes the three engine hooks to the active screen via a switch:

```cpp
void SomeGame::onUpdate(float dt) {
    Screen next = m_screen;
    switch (m_screen) {
        case Screen::Title:    next = m_titleScreen.update(dt);    break;
        case Screen::Playing:  next = m_playingScreen.update(dt);  break;
        // ...
    }
    if (next != m_screen) transitionTo(next);
}
```

`transitionTo()` is where cross-screen state resets live (clearing bullets, resetting score, etc.) and where `onEnter()` is called on the incoming screen:

```cpp
void SomeGame::transitionTo(Screen next) {
    if (next == Screen::Quit) { quit(); return; }

    if (next == Screen::Playing && m_screen == Screen::ShipSelect)
        m_playingScreen.onEnter();

    if (next == Screen::ShipSelect && m_screen == Screen::GameOver)
        resetForRestart();

    m_screen = next;
}
```

State resets that belong to the *outgoing* screen (clearing game objects, zeroing score) stay in the game class's `resetForRestart()`-style helpers. State setup that belongs to the *incoming* screen goes in that screen's `onEnter()`.

---

## GameContext Initialisation

`GameContext` must be initialised after all the members it references. Because C++ initialises members in **declaration order**, declare all data members before `m_ctx` and the screen objects in the class definition:

```cpp
class SomeGame : public Engine::Application {
    // 1. data (initialised first by declaration order)
    Engine::Renderer2D                     m_renderer;
    std::shared_ptr<Engine::SpriteSheet>   m_sheet;
    int                                    m_score = 0;
    // ...

    // 2. context (references the data above — must come after)
    GameContext   m_ctx;

    // 3. screens (hold a GameContext& — must come after m_ctx)
    TitleScreen   m_titleScreen;
    PlayingScreen m_playingScreen;
};
```

Use a `makeContext()` helper in the constructor's member-initialiser list to keep the initialisation readable:

```cpp
SomeGame::SomeGame()
    : Engine::Application("Game", W, H)
    , m_ctx(makeContext(m_renderer, m_sheet, m_score, ...))
    , m_titleScreen(m_ctx)
    , m_playingScreen(m_ctx)
{
    // m_sheet assigned here — safe because m_ctx.sheet is a reference
    m_sheet = std::make_shared<Engine::SpriteSheet>(...);
    m_titleScreen.onEnter();
}
```

---

## Adding a New Screen

1. Add a value to the `Screen` enum in `GameContext.h`.
2. Create `NewScreen.h` / `NewScreen.cpp` with the interface above.
3. Add any new shared state the screen needs to `GameContext` as a reference.
4. Add a `NewScreen m_newScreen` member to the game class (after `m_ctx`).
5. Pass `m_ctx` to it in the constructor's member-initialiser list.
6. Add a `case Screen::NewScreen:` to the `preStep`, `onUpdate`, and `onRender` switches.
7. Add transition logic in `transitionTo()` if entry or exit requires setup/teardown.
8. Register the new `.cpp` in the game's `CMakeLists.txt`.

---

## When to Migrate to the Engine Scene System

Once two or more games are using this pattern, the repetition across games (the `Screen` enum, the switch dispatch, the `GameContext` boilerplate) becomes the signal that an engine-level scene system has earned its complexity. At that point, each screen class maps directly to a `Scene` subclass and the game class dispatch loop moves into the engine. See issue #53.
