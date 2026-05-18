#pragma once
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <memory>
#include "Shader.h"

namespace Engine {

class Renderer2D {
public:
    Renderer2D();
    ~Renderer2D();

    // Call once per frame before any drawRect calls; clears the screen.
    void beginScene(int width, int height);

    void drawRect(float x, float y, float w, float h, const glm::vec4& color);

private:
    std::unique_ptr<Shader> m_shader;
    GLuint m_vao = 0;
    GLuint m_vbo = 0;
    GLuint m_ebo = 0;
    glm::mat4 m_proj{1.0f};
};

} // namespace Engine
