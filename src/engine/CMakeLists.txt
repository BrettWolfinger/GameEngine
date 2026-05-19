add_library(engine STATIC
    core/Application.cpp
    core/Window.cpp
    core/Input.cpp
    renderer/Shader.cpp
    renderer/Renderer2D.cpp
    renderer/SegmentFont.cpp
    renderer/PixelFont.cpp
    audio/AudioManager.cpp
)

# Expose src/ so consumers use #include <engine/...>
target_include_directories(engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)

target_compile_definitions(engine PUBLIC GLFW_INCLUDE_NONE)

target_include_directories(engine PRIVATE ${miniaudio_SOURCE_DIR})

target_link_libraries(engine PUBLIC glad_gl glfw glm::glm)

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
