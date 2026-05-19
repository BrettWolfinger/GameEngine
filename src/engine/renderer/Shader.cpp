#include "Shader.h"
#include <glm/gtc/type_ptr.hpp>
#include <stdexcept>
#include <string>

namespace Engine {

Shader::Shader(const char* vertSrc, const char* fragSrc) {
    GLuint vert = compile(GL_VERTEX_SHADER,   vertSrc);
    GLuint frag = compile(GL_FRAGMENT_SHADER, fragSrc);

    m_program = glCreateProgram();
    glAttachShader(m_program, vert);
    glAttachShader(m_program, frag);
    glLinkProgram(m_program);
    glDeleteShader(vert);
    glDeleteShader(frag);

    GLint ok;
    glGetProgramiv(m_program, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[512];
        glGetProgramInfoLog(m_program, 512, nullptr, log);
        throw std::runtime_error(std::string("Shader link error: ") + log);
    }
}

Shader::~Shader() { glDeleteProgram(m_program); }
void Shader::bind() const { glUseProgram(m_program); }

void Shader::setMat4(const char* name, const glm::mat4& v) const {
    glUniformMatrix4fv(glGetUniformLocation(m_program, name), 1, GL_FALSE, glm::value_ptr(v));
}
void Shader::setVec4(const char* name, const glm::vec4& v) const {
    glUniform4fv(glGetUniformLocation(m_program, name), 1, glm::value_ptr(v));
}
void Shader::setInt(const char* name, int v) const {
    glUniform1i(glGetUniformLocation(m_program, name), v);
}

GLuint Shader::compile(GLenum type, const char* src) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint ok;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[512];
        glGetShaderInfoLog(shader, 512, nullptr, log);
        glDeleteShader(shader);
        throw std::runtime_error(std::string("Shader compile error: ") + log);
    }
    return shader;
}

} // namespace Engine
