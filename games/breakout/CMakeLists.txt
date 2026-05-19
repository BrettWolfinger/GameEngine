add_executable(breakout
    src/main.cpp
    src/BreakoutGame.cpp
)

target_include_directories(breakout PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(breakout PRIVATE engine)

target_compile_definitions(breakout PRIVATE $<$<NOT:$<CONFIG:Release>>:ENABLE_DEV_KEYS>)
