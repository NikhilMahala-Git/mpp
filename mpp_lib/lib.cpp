#include "./lib.hpp"
#if PROJECT_PLATFORM != PROJECT_PLATFORM_WEB
#include <cstdlib>
#include <cstring>
#include "./log.hpp"

MPP_LIB_NAMESPACE_BEGIN

void *malloc(size_t byte_size) {
    auto _result =  ::malloc(byte_size);
    MPP_MACRO_ASSERT(_result != nullptr, "Call to: malloc failed");
    return _result;
}

void *realloc(void *ptr, size_t byte_size) {
     auto _result = ::realloc(ptr, byte_size);
     MPP_MACRO_ASSERT(_result != nullptr, "Call to: realloc failed");
     return _result;
}

void free(void *ptr) {
    ::free(ptr);
}

size_t strlen(const char *str) {
    MPP_MACRO_ASSERT(str != nullptr, "Call to: strlen failed. str is null");
    return ::strlen(str);
}

void *memcpy(void *dest, const void *src, size_t n) {
     auto _result = ::memcpy(dest, src, n);
     MPP_MACRO_ASSERT(_result != nullptr, "Call to: memcpy failed");
     return _result;
}

int memcmp(const void *s1, const void *s2, size_t n) {
     auto _result = ::memcmp(s1, s2, n);
     return _result;
}

void *memmove(void *dest, const void *src, size_t n) {
     auto _result = ::memmove(dest, src, n);
     MPP_MACRO_ASSERT(_result != nullptr, "Call to: memmove failed");
     return _result;
}

void *memset(void *s, int c, size_t n) {
     auto _result = ::memset(s, c, n);
     MPP_MACRO_ASSERT(_result != nullptr, "Call to: memset failed");
     return _result;
}

void abort() {
    ::abort();
}

MPP_LIB_NAMESPACE_END

#endif // PROJECT_PLATFORM != PROJECT_PLATFORM_WEB
