#pragma once
#include <glad/gl.h>
#include <string>

namespace Engine {

class Texture {
public:
    explicit Texture(const std::string& path); // loads via stb_image, throws on failure
    ~Texture();

    // Non-copyable; movable
    Texture(const Texture&)            = delete;
    Texture& operator=(const Texture&) = delete;
    Texture(Texture&& other) noexcept;
    Texture& operator=(Texture&& other) noexcept;

    void bind(int slot = 0) const;

    int getWidth()  const { return m_width;  }
    int getHeight() const { return m_height; }

private:
    GLuint m_id     = 0;
    int    m_width  = 0;
    int    m_height = 0;
};

} // namespace Engine
