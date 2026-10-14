#include "../generic_utility.hpp"
#include <cstdlib>
#include <cstring>

PLATFORM_NAMESPACE_BEGIN

void *malloc(size_t byte_size) { return ::malloc(byte_size); }
void *realloc(void *ptr, size_t byte_size) { return ::realloc(ptr, byte_size); }
void free(void *ptr) { ::free(ptr); }

void *memcpy(void *dest, const void *src, size_t n) { return ::memcpy(dest, src, n); }
int memcmp(const void *s1, const void *s2, size_t n) { return ::memcmp(s1, s2, n); }
void *memmove(void *dest, const void *src, size_t n) { return ::memmove(dest, src, n); }
void *memset(void *s, int c, size_t n) { return ::memset(s, c, n); }

size_t strlen(const char *str) { return ::strlen(str); }

void abort() { ::abort(); }

PLATFORM_NAMESPACE_END
