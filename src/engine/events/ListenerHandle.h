#pragma once
#include <cstdint>
#include <typeindex>

namespace Engine {

class EventDispatcher;

/// @brief RAII subscription token returned by Engine::Events::on<T>().
///
/// Store as a member of the subscribing object. When the handle is destroyed
/// (or reset() is called) the subscription is automatically removed from the
/// event bus. A default-constructed handle is a no-op sentinel.
class ListenerHandle {
public:
    /// Default constructor — creates an invalid (no-op) handle.
    ListenerHandle() = default;

    /// Non-copyable.
    ListenerHandle(const ListenerHandle&)            = delete;
    ListenerHandle& operator=(const ListenerHandle&) = delete;

    /// Movable — the moved-from handle becomes a no-op.
    ListenerHandle(ListenerHandle&& other) noexcept;
    ListenerHandle& operator=(ListenerHandle&& other) noexcept;

    /// Destructor — automatically unsubscribes if the handle is still live.
    ~ListenerHandle();

    /// Explicitly cancel the subscription early. Safe to call multiple times.
    void reset();

private:
    friend class EventDispatcher;

    /// Private constructor used by EventDispatcher when issuing a new handle.
    ListenerHandle(EventDispatcher* dispatcher, std::type_index type, uint64_t id);

    EventDispatcher* m_dispatcher = nullptr;
    std::type_index  m_type       = std::type_index(typeid(void));
    uint64_t         m_id         = 0; ///< 0 means invalid / already unsubscribed.
};

} // namespace Engine
