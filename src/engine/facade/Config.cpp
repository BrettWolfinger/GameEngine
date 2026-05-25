#include "Config.h"
#include <engine/core/Services.h>
#include <engine/config/ConfigWatcher.h>

namespace Engine::Config {

void watch(std::string_view path, std::function<void()> onChanged) {
#ifdef ENABLE_TOOLS
    Services::configWatcher().watch(path, std::move(onChanged));
#else
    (void)path;
    (void)onChanged;
#endif
}

} // namespace Engine::Config
