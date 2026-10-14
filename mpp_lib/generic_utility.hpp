#ifndef _MPP_LIB_LIB_HPP_
#define _MPP_LIB_LIB_HPP_
#include "./defs.hpp"

MPP_LIB_NAMESPACE_BEGIN

void *malloc(size_t byte_size);
void *realloc(void *ptr, size_t byte_size);
void free(void *ptr);

void *memcpy(void *dest, const void *src, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);
void *memmove(void *dest, const void *src, size_t n);
void *memset(void *s, int c, size_t n);

size_t strlen(const char *str);

void abort();

void assert(bool condition, const char *message, const char *file, const char *line);

MPP_LIB_NAMESPACE_END

#define MPP_MACRO_ASSERT(condition, message) mpp::assert((condition), (message), __FILE__, MPP_MACRO_XSTRING(__LINE__))

#endif // _MPP_LIB_LIB_HPP_
