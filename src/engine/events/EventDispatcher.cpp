#include "EventDispatcher.h"
#include "ListenerHandle.h"
#include <algorithm>

namespace Engine {

ListenerHandle EventDispatcher::subscribeErased(
    std::type_index type, std::function<void(const void*)> fn)
{
    auto id = m_nextId++;
    m_listeners[type].push_back({ id, std::move(fn) });
    return ListenerHandle(this, type, id); // EventDispatcher is a friend of ListenerHandle
}

void EventDispatcher::emitErased(std::type_index type, const void* event) {
    auto it = m_listeners.find(type);
    if (it == m_listeners.end()) return;
    ++m_dispatchDepth;
    for (auto& entry : it->second) {
        if (entry.id != 0)
            entry.callback(event);
    }
    if (--m_dispatchDepth == 0)
        sweepDead(it->second);
}

void EventDispatcher::unsubscribe(std::type_index type, ListenerId id) {
    auto it = m_listeners.find(type);
    if (it == m_listeners.end()) return;

    if (m_dispatchDepth > 0) {
        // We're inside emit() — zero the id so the sweep pass removes it later.
        for (auto& entry : it->second) {
            if (entry.id == id) {
                entry.id = 0;
                return;
            }
        }
    } else {
        // Safe to erase immediately.
        auto& bucket = it->second;
        bucket.erase(
            std::remove_if(bucket.begin(), bucket.end(),
                           [id](const Entry& e) { return e.id == id; }),
            bucket.end());
    }
}

void EventDispatcher::sweepDead(std::vector<Entry>& bucket) {
    bucket.erase(
        std::remove_if(bucket.begin(), bucket.end(),
                       [](const Entry& e) { return e.id == 0; }),
        bucket.end());
}

} // namespace Engine
