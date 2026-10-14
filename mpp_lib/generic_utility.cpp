#include "./generic_utility.hpp"

namespace Platform {
    extern "C" void *malloc(size_t byte_size);
    extern "C" void *realloc(void *ptr, size_t byte_size);
    extern "C" void free(void *ptr);
    extern "C" void *memcpy(void *dest, const void *src, size_t n);
    extern "C" int memcmp(const void *s1, const void *s2, size_t n);
    extern "C" void *memmove(void *dest, const void *src, size_t n);
    extern "C" void *memset(void *s, int c, size_t n);
    extern "C" size_t strlen(const char *str);
    extern "C" void abort();

    extern "C" void log(const char *);
}

MPP_LIB_NAMESPACE_BEGIN

void *malloc(size_t byte_size) { return Platform::malloc(byte_size); }
void *realloc(void *ptr, size_t byte_size) { return Platform::realloc(ptr, byte_size); }
void free(void *ptr) { Platform::free(ptr); }

void *memcpy(void *dest, const void *src, size_t n) { return Platform::memcpy(dest, src, n); }
int memcmp(const void *s1, const void *s2, size_t n) { return Platform::memcmp(s1, s2, n); }
void *memmove(void *dest, const void *src, size_t n) { return Platform::memmove(dest, src, n); }
void *memset(void *s, int c, size_t n) { return Platform::memset(s, c, n); }

size_t strlen(const char *str) { return Platform::strlen(str); }

void abort() { Platform::abort(); }

void assert(bool condition, const char *message, const char *file, const char *line) {
    if (condition) {
        return;
    }
    Platform::log("Assertion failed! The next 2 line contains info about fail file location, fail line location respectively\n");
    Platform::log(file); Platform::log("\n");
    Platform::log(line); Platform::log("\nMessage is noted below:\n");
    Platform::log(message); Platform::log("\n");
}

MPP_LIB_NAMESPACE_END
