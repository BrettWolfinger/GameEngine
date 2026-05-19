add_executable(frogger
    src/main.cpp
    src/FroggerGame.cpp
    src/Frog.cpp
    src/Vehicle.cpp
)

target_include_directories(frogger PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# stb_image_write is a single header in the same stb repo — expose it to frogger
target_include_directories(frogger PRIVATE ${stb_image_SOURCE_DIR})

target_link_libraries(frogger PRIVATE engine)

target_compile_definitions(frogger PRIVATE $<$<NOT:$<CONFIG:Release>>:ENABLE_DEV_KEYS>)
