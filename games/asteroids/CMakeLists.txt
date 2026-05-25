add_executable(asteroids
    src/main.cpp
    src/AsteroidsGame.cpp
    src/AsteroidsConfigs.cpp
    src/TitleScreen.cpp
    src/ShipSelectScreen.cpp
    src/PlayingScreen.cpp
    src/GameOverScreen.cpp
    src/Ship.cpp
    src/Bullet.cpp
    src/Asteroid.cpp
    src/UFO.cpp
)

target_include_directories(asteroids PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(asteroids PRIVATE engine)

target_compile_definitions(asteroids PRIVATE $<$<NOT:$<CONFIG:Release>>:ENABLE_DEV_KEYS>)
