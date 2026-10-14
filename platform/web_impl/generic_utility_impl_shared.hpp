#ifndef _WEB_GENERIC_UTILITY_IMPL_SHARED_HPP_
#define _WEB_GENERIC_UTILITY_IMPL_SHARED_HPP_
#include "../generic_utility.hpp"
#include "../module.log.hpp"

#define WEB_GU_IMPL_STR(x) #x
#define WEB_GU_IMPL_XSTR(x) WEB_GU_IMPL_STR(x)

#define WEB_GU_IMPL_ASSERT(condition, message)                                                             \
    do {                                                                                                   \
        if (!(condition)) {                                                                                \
            Platform::log("Assertion failed in file: " __FILE__ " at line: " WEB_GU_IMPL_XSTR(__LINE__) "\n"); \
            Platform::log((message));                                                                      \
            Platform::log("\n");                                                                           \
            Platform::abort();                                                                             \
        }                                                                                                  \
    } while (0) 


#endif // _WEB_GENERIC_UTILITY_IMPL_SHARED_HPP_
