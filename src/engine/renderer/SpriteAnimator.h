#pragma once
#include "SpriteSheet.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace Engine {

enum class PlayMode { Loop, OneShot };

struct AnimClip {
    std::vector<int> frames;
    float            frameDuration; // seconds per frame
    PlayMode         mode = PlayMode::Loop;
};

class SpriteAnimator {
public:
    explicit SpriteAnimator(std::shared_ptr<SpriteSheet> sheet);

    void addClip(const std::string& name, AnimClip clip);
    void setClip(const std::string& name);   // switches to a named clip; resets frame/time
    void update(float dt);

    UVRect currentFrameUVs() const;
    const SpriteSheet& sheet() const { return *m_sheet; }

private:
    std::shared_ptr<SpriteSheet>              m_sheet;
    std::unordered_map<std::string, AnimClip> m_clips;
    const AnimClip*                           m_current     = nullptr;
    int                                       m_frameIdx    = 0;
    float                                     m_accumulated = 0.f;
};

} // namespace Engine
