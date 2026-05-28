add_library(engine STATIC
    config/ConfigGroup.cpp
    config/ConfigLoader.cpp
    config/ConfigRegistry.cpp
    config/ConfigWatcher.cpp
    events/EventDispatcher.cpp
    events/ListenerHandle.cpp
    facade/Audio.cpp
    facade/Config.cpp
    facade/Events.cpp
    facade/Particles.cpp
    facade/Collision.cpp
    ui/Menu.cpp
    core/Application.cpp
    core/Window.cpp
    core/Input.cpp
    core/SaveData.cpp
    renderer/Shader.cpp
    renderer/Renderer2D.cpp
    renderer/Texture.cpp
    renderer/SpriteSheet.cpp
    renderer/SpriteAnimator.cpp
    renderer/SegmentFont.cpp
    renderer/PixelFont.cpp
    audio/AudioManager.cpp
    particles/ParticleSystem.cpp
    physics/CollisionWorld.cpp
    core/Services.cpp
    tilemap/Tilemap.cpp
    tilemap/TilemapRenderer.cpp
)

# Expose src/ so consumers use #include <engine/...>
target_include_directories(engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)

target_compile_definitions(engine PUBLIC  GLFW_INCLUDE_NONE)
target_compile_definitions(engine PRIVATE ENGINE_INTERNAL)
target_compile_definitions(engine PRIVATE $<$<NOT:$<CONFIG:Release>>:ENABLE_TOOLS>)

target_include_directories(engine PRIVATE ${miniaudio_SOURCE_DIR})
target_include_directories(engine PRIVATE ${stb_image_SOURCE_DIR})

target_link_libraries(engine PUBLIC glad_gl glfw glm::glm tomlplusplus::tomlplusplus imgui)

if(APPLE)
    target_link_libraries(engine PUBLIC
        "-framework OpenGL"
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreAudio"
        "-framework AudioToolbox"
    )
elseif(WIN32)
    target_link_libraries(engine PUBLIC opengl32)
endif()
