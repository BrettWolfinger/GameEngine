add_executable(pong
    src/main.cpp
    src/PongGame.cpp
)

target_include_directories(pong PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(pong PRIVATE engine)

target_compile_definitions(pong PRIVATE $<$<NOT:$<CONFIG:Release>>:ENABLE_DEV_KEYS>)
