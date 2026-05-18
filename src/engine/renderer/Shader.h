#pragma once
#include <glad/gl.h>
#include <glm/glm.hpp>

namespace Engine {

class Shader {
public:
    Shader(const char* vertSrc, const char* fragSrc);
    ~Shader();

    void bind() const;
    void setMat4(const char* name, const glm::mat4& v) const;
    void setVec4(const char* name, const glm::vec4& v) const;

private:
    GLuint m_program = 0;

    static GLuint compile(GLenum type, const char* src);
};

} // namespace Engine
