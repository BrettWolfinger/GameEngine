#include "Renderer2D.h"
#include <glm/gtc/matrix_transform.hpp>

namespace Engine {

// ---- Color-rect shaders ----

static const char* kVert = R"(
#version 410 core
layout(location = 0) in vec2 a_pos;
uniform mat4 u_mvp;
void main() { gl_Position = u_mvp * vec4(a_pos, 0.0, 1.0); }
)";

static const char* kFrag = R"(
#version 410 core
uniform vec4 u_color;
out vec4 fragColor;
void main() { fragColor = u_color; }
)";

// ---- Textured-rect shaders ----

static const char* kTexVert = R"(
#version 410 core
layout(location = 0) in vec2 a_pos;
layout(location = 1) in vec2 a_uv;
uniform mat4 u_mvp;
out vec2 v_uv;
void main() {
    v_uv = a_uv;
    gl_Position = u_mvp * vec4(a_pos, 0.0, 1.0);
}
)";

static const char* kTexFrag = R"(
#version 410 core
in vec2 v_uv;
uniform sampler2D u_tex;
uniform vec4 u_uvRegion; // (u0, v0, u1, v1)
out vec4 fragColor;
void main() {
    vec2 uv = vec2(
        u_uvRegion.x + v_uv.x * (u_uvRegion.z - u_uvRegion.x),
        u_uvRegion.y + v_uv.y * (u_uvRegion.w - u_uvRegion.y)
    );
    fragColor = texture(u_tex, uv);
}
)";

Renderer2D::Renderer2D() {
    // ---- Color-rect VAO ----
    m_shader = std::make_unique<Shader>(kVert, kFrag);

    // Unit quad — scaled/translated per drawRect call
    float vertices[] = { 0.f,0.f,  1.f,0.f,  1.f,1.f,  0.f,1.f };
    unsigned int indices[] = { 0,1,2, 2,3,0 };

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    // ---- Textured-rect VAO ----
    // Interleaved pos(xy) + uv(xy), unit quad with V-flip so Y=0 is top.
    // Layout: bl, tl, br, tr  (CCW winding pairs)
    //   pos (0,0)→uv(0,1)  pos (1,0)→uv(1,1)  pos (1,1)→uv(1,0)  pos (0,1)→uv(0,0)
    float texVerts[] = {
        0.f, 0.f,  0.f, 1.f,   // bottom-left  in screen → top    in texture (V flipped)
        1.f, 0.f,  1.f, 1.f,   // bottom-right in screen → top
        1.f, 1.f,  1.f, 0.f,   // top-right    in screen → bottom
        0.f, 1.f,  0.f, 0.f,   // top-left     in screen → bottom
    };
    unsigned int texIndices[] = { 0,1,2, 2,3,0 };

    m_texShader = std::make_unique<Shader>(kTexVert, kTexFrag);

    glGenVertexArrays(1, &m_texVao);
    glGenBuffers(1, &m_texVbo);
    glGenBuffers(1, &m_texEbo);

    glBindVertexArray(m_texVao);
    glBindBuffer(GL_ARRAY_BUFFER, m_texVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(texVerts), texVerts, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_texEbo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(texIndices), texIndices, GL_STATIC_DRAW);
    // attrib 0: position (xy)
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // attrib 1: uv (xy)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
}

Renderer2D::~Renderer2D() {
    glDeleteBuffers(1, &m_ebo);
    glDeleteBuffers(1, &m_vbo);
    glDeleteVertexArrays(1, &m_vao);

    glDeleteBuffers(1, &m_texEbo);
    glDeleteBuffers(1, &m_texVbo);
    glDeleteVertexArrays(1, &m_texVao);
}

void Renderer2D::beginScene(int width, int height) {
    glClearColor(0.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);
    // Screen-space ortho: (0,0) top-left, (w,h) bottom-right
    m_proj = glm::ortho(0.f, (float)width, (float)height, 0.f, -1.f, 1.f);
}

void Renderer2D::drawRect(float x, float y, float w, float h, const glm::vec4& color) {
    glm::mat4 model = glm::translate(glm::mat4(1.f), glm::vec3(x, y, 0.f));
    model = glm::scale(model, glm::vec3(w, h, 1.f));

    m_shader->bind();
    m_shader->setMat4("u_mvp", m_proj * model);
    m_shader->setVec4("u_color", color);

    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

void Renderer2D::drawTexturedRect(float x, float y, float w, float h,
                                   const Texture& tex,
                                   float u0, float v0, float u1, float v1) {
    glm::mat4 model = glm::translate(glm::mat4(1.f), glm::vec3(x, y, 0.f));
    model = glm::scale(model, glm::vec3(w, h, 1.f));

    tex.bind(0);

    m_texShader->bind();
    m_texShader->setMat4("u_mvp", m_proj * model);
    m_texShader->setVec4("u_uvRegion", glm::vec4(u0, v0, u1, v1));
    m_texShader->setInt("u_tex", 0);

    glBindVertexArray(m_texVao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

} // namespace Engine
