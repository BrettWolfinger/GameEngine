#pragma once
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <memory>
#include "Shader.h"
#include "Texture.h"

namespace Engine {

class Renderer2D {
public:
    Renderer2D();
    ~Renderer2D();

    // Call once per frame before any draw calls; clears the screen.
    void beginScene(int width, int height);

    // Colored (untextured) rectangle
    void drawRect(float x, float y, float w, float h, const glm::vec4& color);

    // Textured rectangle — u0/v0/u1/v1 select the UV sub-region; angle rotates around the sprite center (radians)
    void drawTexturedRect(float x, float y, float w, float h,
                          const Texture& tex,
                          float u0 = 0.f, float v0 = 0.f,
                          float u1 = 1.f, float v1 = 1.f,
                          float angle = 0.f);

private:
    // --- color-rect pipeline (unchanged) ---
    std::unique_ptr<Shader> m_shader;
    GLuint m_vao = 0;
    GLuint m_vbo = 0;
    GLuint m_ebo = 0;

    // --- textured-rect pipeline ---
    std::unique_ptr<Shader> m_texShader;
    GLuint m_texVao = 0;
    GLuint m_texVbo = 0;
    GLuint m_texEbo = 0;

    glm::mat4 m_proj{1.0f};
};

} // namespace Engine
