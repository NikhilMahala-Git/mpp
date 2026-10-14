#include "../module.log.hpp"
#if PROJECT_PLATFORM != PROJECT_PLATFORM_WEB

#include <iostream>

PLATFORM_NAMESPACE_BEGIN

void log(const char *message) {
    std::cout << message << std::flush;
}

PLATFORM_NAMESPACE_END
#endif // PROJECT_PLATFORM != PROJECT_PLATFORM_WEB
