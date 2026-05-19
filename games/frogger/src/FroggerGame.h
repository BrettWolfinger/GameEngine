#pragma once
#include <engine/core/Application.h>
#include <engine/renderer/Renderer2D.h>
#include <engine/renderer/SpriteAnimator.h>
#include <memory>
#include <optional>
#include <string>

class FroggerGame : public Engine::Application {
public:
    FroggerGame();

protected:
    void onUpdate(float dt) override;
    void onRender()         override;

private:
    // Generates a placeholder 4-frame horizontal strip PNG if it doesn't exist.
    // Each 32x32 frame is a solid color: red, green, blue, yellow.
    static void ensurePlaceholderAsset(const std::string& path);

    Engine::Renderer2D                      m_renderer;
    std::shared_ptr<Engine::SpriteSheet>    m_sheet;
    std::optional<Engine::SpriteAnimator>   m_animator;
};
