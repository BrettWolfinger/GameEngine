add_executable(asteroids
    src/main.cpp
    src/AsteroidsGame.cpp
    src/Ship.cpp
)

target_include_directories(asteroids PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(asteroids PRIVATE engine)

target_compile_definitions(asteroids PRIVATE $<$<NOT:$<CONFIG:Release>>:ENABLE_DEV_KEYS>)
