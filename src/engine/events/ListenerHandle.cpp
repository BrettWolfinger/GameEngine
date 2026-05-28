#include "ListenerHandle.h"
#include "EventDispatcher.h"

namespace Engine {

ListenerHandle::ListenerHandle(EventDispatcher* dispatcher, std::type_index type, uint64_t id)
    : m_dispatcher(dispatcher)
    , m_type(type)
    , m_id(id)
{}

ListenerHandle::ListenerHandle(ListenerHandle&& other) noexcept
    : m_dispatcher(other.m_dispatcher)
    , m_type(other.m_type)
    , m_id(other.m_id)
{
    other.m_dispatcher = nullptr;
    other.m_id         = 0;
}

ListenerHandle& ListenerHandle::operator=(ListenerHandle&& other) noexcept {
    if (this != &other) {
        reset();
        m_dispatcher       = other.m_dispatcher;
        m_type             = other.m_type;
        m_id               = other.m_id;
        other.m_dispatcher = nullptr;
        other.m_id         = 0;
    }
    return *this;
}

ListenerHandle::~ListenerHandle() {
    reset();
}

void ListenerHandle::reset() {
    if (m_id != 0 && m_dispatcher != nullptr) {
        m_dispatcher->unsubscribe(m_type, m_id);
        m_id         = 0;
        m_dispatcher = nullptr;
    }
}

} // namespace Engine
