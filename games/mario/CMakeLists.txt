add_executable(mario
    src/main.cpp
    src/MarioGame.cpp
)

target_include_directories(mario PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(mario PRIVATE engine)

target_compile_definitions(mario PRIVATE $<$<NOT:$<CONFIG:Release>>:ENABLE_DEV_KEYS>)
target_compile_definitions(mario PRIVATE $<$<NOT:$<CONFIG:Release>>:ENABLE_TOOLS>)
