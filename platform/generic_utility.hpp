#ifndef _PLATFORM_GENERIC_UTILITY_H_
#define _PLATFORM_GENERIC_UTILITY_H_
#include "./defs.hpp"

PLATFORM_NAMESPACE_BEGIN

void *malloc(size_t byte_size);
void *realloc(void *ptr, size_t byte_size);
void free(void *ptr);

void *memcpy(void *dest, const void *src, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);
void *memmove(void *dest, const void *src, size_t n);
void *memset(void *s, int c, size_t n);

size_t strlen(const char *str);

void abort();

PLATFORM_NAMESPACE_END

#endif // _PLATFORM_GENERIC_UTILITY_H_
