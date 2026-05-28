/// @file Events.h
/// @brief Engine event bus facade — typed publish/subscribe.
///
/// Subscribe with Engine::Events::on<MyEvent>(...) and store the returned
/// ListenerHandle as a member. Emit with Engine::Events::emit(MyEvent{...}).
/// Listeners fire synchronously and are automatically removed when the handle
/// is destroyed.
///
/// @code
/// // Define an event struct anywhere (engine or game code):
/// struct BallScored { int player; };
///
/// // Subscribe (store the handle as a member to keep the subscription alive):
/// m_scoreHandle = Engine::Events::on<BallScored>([this](const BallScored& e) {
///     addScore(e.player);
/// });
///
/// // Emit from any system:
/// Engine::Events::emit(BallScored{ 1 });
/// @endcode
#pragma once
#include <engine/events/ListenerHandle.h>
#include <functional>
#include <typeindex>

namespace Engine::Events {

namespace detail {
/// @cond INTERNAL
/// Route a typed subscription through the internal EventDispatcher.
ListenerHandle subscribeImpl(std::type_index type, std::function<void(const void*)> fn);
/// Route a typed emit through the internal EventDispatcher.
void           emitImpl(std::type_index type, const void* event);
/// @endcond
} // namespace detail

/// Subscribe to an event type.
/// @tparam EventT    The event struct to listen for.
/// @param  callback  Called synchronously whenever EventT is emitted.
/// @return A handle that auto-unsubscribes when destroyed. Store as a member.
template<typename EventT>
ListenerHandle on(std::function<void(const EventT&)> callback) {
    return detail::subscribeImpl(
        std::type_index(typeid(EventT)),
        [cb = std::move(callback)](const void* e) { cb(*static_cast<const EventT*>(e)); }
    );
}

/// Emit an event, immediately invoking all registered listeners.
/// @tparam EventT  The event struct type to emit.
/// @param  event   The event instance to broadcast.
template<typename EventT>
void emit(const EventT& event) {
    detail::emitImpl(std::type_index(typeid(EventT)), static_cast<const void*>(&event));
}

} // namespace Engine::Events
