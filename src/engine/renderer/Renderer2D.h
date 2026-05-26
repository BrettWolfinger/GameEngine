#pragma once
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <array>
#include <memory>
#include <variant>
#include <vector>
#include "Shader.h"
#include "Texture.h"

namespace Engine {

class Renderer2D {
public:
    static constexpr int kMaxLayers     = 16;
    static constexpr int kParticleLayer = kMaxLayers - 1; // reserved — do not use in game code

    Renderer2D();
    ~Renderer2D();

    // Call once per frame before any draw calls; clears the screen and all layer buckets.
    void beginScene(int width, int height);

    // Flush all layer buckets in ascending layer order, then clear them.
    // Application calls this after onRender()+particles and again after onOverlayRender().
    void endScene();

    // Colored (untextured) rectangle — queued into the given layer (default 0).
    void drawRect(float x, float y, float w, float h,
                  const glm::vec4& color, int layer = 0);

    // Textured rectangle — u0/v0/u1/v1 select the UV sub-region; angle rotates around
    // the sprite center (radians); tint multiplies the sampled color (default white = no tint).
    // Queued into the given layer (default 0).
    void drawTexturedRect(float x, float y, float w, float h,
                          const Texture& tex,
                          float u0 = 0.f, float v0 = 0.f,
                          float u1 = 1.f, float v1 = 1.f,
                          float angle = 0.f,
                          const glm::vec4& tint = { 1.f, 1.f, 1.f, 1.f },
                          int layer = 0);

private:
    struct RectCmd {
        float x, y, w, h;
        glm::vec4 color;
    };
    struct TexCmd {
        float x, y, w, h;
        float u0, v0, u1, v1;
        float angle;
        glm::vec4 tint;
        const Texture* tex;
    };
    using DrawCmd = std::variant<RectCmd, TexCmd>;

    std::array<std::vector<DrawCmd>, kMaxLayers> m_layers;

    void flushRect(const RectCmd& cmd);
    void flushTex(const TexCmd& cmd);

    // --- color-rect pipeline ---
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
