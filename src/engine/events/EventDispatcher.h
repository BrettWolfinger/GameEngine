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

    /// Subscribe a callback for events of type EventT.
    /// Returns the new listener's id; the caller is responsible for
    /// wrapping it in a ListenerHandle.
    template<typename EventT>
    ListenerId subscribe(std::function<void(const EventT&)> fn) {
        auto id = m_nextId++;
        m_listeners[std::type_index(typeid(EventT))].push_back({
            id,
            [fn = std::move(fn)](const void* e) { fn(*static_cast<const EventT*>(e)); }
        });
        return id;
    }

    /// Emit an event, invoking all registered listeners synchronously.
    /// Re-entrant calls are safe: listeners removed during dispatch are
    /// lazily swept after the outermost emit returns.
    template<typename EventT>
    void emit(const EventT& event) {
        auto it = m_listeners.find(std::type_index(typeid(EventT)));
        if (it == m_listeners.end()) return;
        ++m_dispatchDepth;
        for (auto& entry : it->second) {
            if (entry.id != 0)
                entry.callback(static_cast<const void*>(&event));
        }
        if (--m_dispatchDepth == 0)
            sweepDead(it->second);
    }

    /// Subscribe using a pre-type-erased callback and return a ready-to-store
    /// ListenerHandle. Used by the Events facade so that the facade .cpp does
    /// not need to instantiate the subscribe<T> template.
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
