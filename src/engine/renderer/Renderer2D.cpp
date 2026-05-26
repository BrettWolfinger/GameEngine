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
uniform vec4 u_tint;
out vec4 fragColor;
void main() {
    vec2 uv = vec2(
        u_uvRegion.x + v_uv.x * (u_uvRegion.z - u_uvRegion.x),
        u_uvRegion.y + v_uv.y * (u_uvRegion.w - u_uvRegion.y)
    );
    fragColor = texture(u_tex, uv) * u_tint;
    if (fragColor.a < 0.1) discard;
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
    // Interleaved pos(xy) + uv(xy), unit quad. In our Y-down ortho, pos(0,0) is
    // the top-left of the tile, so V=0 (image top) maps to screen top.
    float texVerts[] = {
        0.f, 0.f,  0.f, 0.f,   // top-left     in screen → top    in texture
        1.f, 0.f,  1.f, 0.f,   // top-right    in screen → top
        1.f, 1.f,  1.f, 1.f,   // bottom-right in screen → bottom in texture
        0.f, 1.f,  0.f, 1.f,   // bottom-left  in screen → bottom
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
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // Screen-space ortho: (0,0) top-left, (w,h) bottom-right
    m_proj = glm::ortho(0.f, (float)width, (float)height, 0.f, -1.f, 1.f);
    for (auto& layer : m_layers)
        layer.clear();
}

void Renderer2D::endScene() {
    for (auto& layer : m_layers) {
        for (const auto& cmd : layer) {
            std::visit([this](const auto& c) {
                using T = std::decay_t<decltype(c)>;
                if constexpr (std::is_same_v<T, RectCmd>)
                    flushRect(c);
                else
                    flushTex(c);
            }, cmd);
        }
        layer.clear();
    }
}

void Renderer2D::drawRect(float x, float y, float w, float h,
                           const glm::vec4& color, int layer) {
    m_layers[layer].push_back(RectCmd{ x, y, w, h, color });
}

void Renderer2D::drawTexturedRect(float x, float y, float w, float h,
                                   const Texture& tex,
                                   float u0, float v0, float u1, float v1,
                                   float angle,
                                   const glm::vec4& tint, int layer) {
    m_layers[layer].push_back(TexCmd{ x, y, w, h, u0, v0, u1, v1, angle, tint, &tex });
}

void Renderer2D::flushRect(const RectCmd& cmd) {
    glm::mat4 model = glm::translate(glm::mat4(1.f), glm::vec3(cmd.x, cmd.y, 0.f));
    model = glm::scale(model, glm::vec3(cmd.w, cmd.h, 1.f));

    m_shader->bind();
    m_shader->setMat4("u_mvp", m_proj * model);
    m_shader->setVec4("u_color", cmd.color);

    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

void Renderer2D::flushTex(const TexCmd& cmd) {
    glm::mat4 model = glm::translate(glm::mat4(1.f), glm::vec3(cmd.x + cmd.w * 0.5f, cmd.y + cmd.h * 0.5f, 0.f));
    if (cmd.angle != 0.f)
        model = glm::rotate(model, cmd.angle, glm::vec3(0.f, 0.f, 1.f));
    model = glm::translate(model, glm::vec3(-cmd.w * 0.5f, -cmd.h * 0.5f, 0.f));
    model = glm::scale(model, glm::vec3(cmd.w, cmd.h, 1.f));

    cmd.tex->bind(0);

    m_texShader->bind();
    m_texShader->setMat4("u_mvp", m_proj * model);
    m_texShader->setVec4("u_uvRegion", glm::vec4(cmd.u0, cmd.v0, cmd.u1, cmd.v1));
    m_texShader->setVec4("u_tint", cmd.tint);
    m_texShader->setInt("u_tex", 0);

    glBindVertexArray(m_texVao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

} // namespace Engine
