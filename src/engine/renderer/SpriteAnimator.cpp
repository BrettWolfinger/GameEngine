#include "SpriteAnimator.h"
#include <stdexcept>

namespace Engine {

SpriteAnimator::SpriteAnimator(std::shared_ptr<SpriteSheet> sheet)
    : m_sheet(std::move(sheet))
{
    if (!m_sheet) throw std::invalid_argument("SpriteAnimator: null sheet");
}

void SpriteAnimator::addClip(const std::string& name, AnimClip clip) {
    m_clips.emplace(name, std::move(clip));
}

void SpriteAnimator::setClip(const std::string& name) {
    auto it = m_clips.find(name);
    if (it == m_clips.end())
        throw std::runtime_error("SpriteAnimator: unknown clip '" + name + "'");
    m_current     = &it->second;
    m_frameIdx    = 0;
    m_accumulated = 0.f;
}

void SpriteAnimator::update(float dt) {
    if (!m_current || m_current->frames.empty()) return;

    m_accumulated += dt;
    while (m_accumulated >= m_current->frameDuration) {
        m_accumulated -= m_current->frameDuration;
        m_frameIdx++;
        const int count = static_cast<int>(m_current->frames.size());
        if (m_frameIdx >= count) {
            if (m_current->mode == PlayMode::Loop) {
                m_frameIdx = 0;
            } else {
                m_frameIdx = count - 1; // clamp on last frame for OneShot
                m_accumulated = 0.f;
                break;
            }
        }
    }
}

UVRect SpriteAnimator::currentFrameUVs() const {
    if (!m_current || m_current->frames.empty())
        return m_sheet->getFrameUVs(0);

    const int sheetFrame = m_current->frames[m_frameIdx];
    return m_sheet->getFrameUVs(sheetFrame);
}

} // namespace Engine
