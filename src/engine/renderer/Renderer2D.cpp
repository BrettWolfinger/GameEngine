#include "Renderer2D.h"
#include <glm/gtc/matrix_transform.hpp>

namespace Engine {

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

Renderer2D::Renderer2D() {
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
}

Renderer2D::~Renderer2D() {
    glDeleteBuffers(1, &m_ebo);
    glDeleteBuffers(1, &m_vbo);
    glDeleteVertexArrays(1, &m_vao);
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

} // namespace Engine
