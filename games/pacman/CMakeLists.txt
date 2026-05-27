add_executable(pacman
    src/main.cpp
    src/PacmanGame.cpp
    src/Pacman.cpp
)

target_include_directories(pacman PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(pacman PRIVATE engine)

target_compile_definitions(pacman PRIVATE $<$<NOT:$<CONFIG:Release>>:ENABLE_DEV_KEYS>)
target_compile_definitions(pacman PRIVATE $<$<NOT:$<CONFIG:Release>>:ENABLE_TOOLS>)
