#include "Events.h"
#include <engine/core/Services.h>
#include <engine/events/EventDispatcher.h>

namespace Engine::Events::detail {

ListenerHandle subscribeImpl(std::type_index type, std::function<void(const void*)> fn) {
    return Services::events().subscribeErased(type, std::move(fn));
}

void emitImpl(std::type_index type, const void* event) {
    Services::events().emitErased(type, event);
}

} // namespace Engine::Events::detail
