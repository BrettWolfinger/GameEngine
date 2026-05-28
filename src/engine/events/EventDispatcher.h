#pragma once
#ifndef ENGINE_INTERNAL
#  error "engine/events/EventDispatcher.h is an engine-internal header. Include <engine/Engine.h> instead."
#endif

#include "ListenerHandle.h"
#include <cstdint>
#include <functional>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace Engine {

/// @brief Core event dispatcher — typed publish/subscribe with re-entrancy safety.
///
/// Owned by Application::Impl and exposed through Services::events().
/// Game code never touches this class directly; use the Engine::Events facade.
class EventDispatcher {
public:
    using ListenerId = uint64_t;

    /// Subscribe using a pre-type-erased callback and return a ready-to-store
    /// ListenerHandle. Used by the Events facade.
    ListenerHandle subscribeErased(std::type_index type, std::function<void(const void*)> fn);

    /// Emit using a pre-type-erased event pointer. Used by the Events facade.
    void emitErased(std::type_index type, const void* event);

    /// Cancel a subscription. If currently dispatching, marks the entry dead
    /// for deferred removal; otherwise erases it immediately.
    void unsubscribe(std::type_index type, ListenerId id);

private:
    struct Entry {
        ListenerId                    id;
        std::function<void(const void*)> callback;
    };

    std::unordered_map<std::type_index, std::vector<Entry>> m_listeners;
    ListenerId m_nextId        = 1; ///< 0 is reserved as the invalid/unsubscribed sentinel.
    int        m_dispatchDepth = 0;

    /// Remove all entries whose id has been zeroed by unsubscribe().
    void sweepDead(std::vector<Entry>& bucket);
};

} // namespace Engine
