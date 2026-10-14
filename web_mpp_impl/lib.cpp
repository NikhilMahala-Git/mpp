#include "../mpp_lib/lib.hpp"
#if PROJECT_PLATFORM == PROJECT_PLATFORM_WEB
#include "./dlmalloc.cpp"
#include "../mpp_lib/log.hpp"

MPP_LIB_NAMESPACE_BEGIN

void *malloc(size_t byte_size) {
    auto _result =  ::dlmalloc(byte_size);
    MPP_MACRO_ASSERT(_result != nullptr, "Call to: malloc failed");
    return _result;
}

void *realloc(void *ptr, size_t byte_size) {
     auto _result = ::dlrealloc(ptr, byte_size);
     MPP_MACRO_ASSERT(_result != nullptr, "Call to: realloc failed");
     return _result;
}

void free(void *ptr) {
    ::dlfree(ptr);
}

size_t strlen(const char *str) {
    MPP_MACRO_ASSERT(str != nullptr, "Call to: strlen failed. str is null");
    size_t _len = 0;
    while (*str) {
        _len++;
        ++str;
    }
    return _len;
}

void* memcpy(void* dest, const void* src, size_t n) {
    MPP_MACRO_ASSERT(dest != nullptr, "memcpy: dest cannot be null");
    MPP_MACRO_ASSERT(src != nullptr, "memcpy: src cannot be null");

    auto* d = static_cast<unsigned char*>(dest);
    const auto* s = static_cast<const unsigned char*>(src);

    // Optimization: Copy by CPU architecture word size (4 or 8 bytes at a time)
    while (n >= sizeof(size_t)) {
        *reinterpret_cast<size_t*>(d) = *reinterpret_cast<const size_t*>(s);
        d += sizeof(size_t);
        s += sizeof(size_t);
        n -= sizeof(size_t);
    }

    // Clean up remaining trailing bytes
    while (n > 0) {
        *d++ = *s++;
        n--;
    }

    return dest;
}

int memcmp(const void* s1, const void* s2, size_t n) {
    MPP_MACRO_ASSERT(s1 != nullptr, "memcmp: s1 cannot be null");
    MPP_MACRO_ASSERT(s2 != nullptr, "memcmp: s2 cannot be null");

    const auto* p1 = static_cast<const unsigned char*>(s1);
    const auto* p2 = static_cast<const unsigned char*>(s2);

    // Optimization: Fast comparison using CPU word size blocks
    while (n >= sizeof(size_t)) {
        if (*reinterpret_cast<const size_t*>(p1) != *reinterpret_cast<const size_t*>(p2)) {
            // If words mismatch, break out to find the exact byte difference
            break;
        }
        p1 += sizeof(size_t);
        p2 += sizeof(size_t);
        n -= sizeof(size_t);
    }

    // Exact byte-by-byte check for mismatches or remaining trailing bytes
    while (n > 0) {
        if (*p1 != *p2) {
            return (*p1 < *p2) ? -1 : 1;
        }
        p1++;
        p2++;
        n--;
    }

    return 0;
}

void* memmove(void* dest, const void* src, size_t n) {
    MPP_MACRO_ASSERT(dest != nullptr, "memmove: dest cannot be null");
    MPP_MACRO_ASSERT(src != nullptr, "memmove: src cannot be null");

    auto* d = static_cast<unsigned char*>(dest);
    const auto* s = static_cast<const unsigned char*>(src);

    // Handle overlapping memory regions
    if (d < s || d >= (s + n)) {
        // No forward overlap risk, route directly to our optimized forward memcpy layout
        return mpp::memcpy(dest, src, n);
    } else {
        // Critical: Source and destination overlap backwards (dest starts inside src region).
        // We must copy backwards from the end of the buffers to avoid overwriting data!
        d += n;
        s += n;

        // Optimization: Copy backwards by word chunks
        while (n >= sizeof(size_t)) {
            d -= sizeof(size_t);
            s -= sizeof(size_t);
            n -= sizeof(size_t);
            *reinterpret_cast<size_t*>(d) = *reinterpret_cast<const size_t*>(s);
        }

        // Clean up remaining trailing bytes backwards
        while (n > 0) {
            d--;
            s--;
            n--;
            *d = *s;
        }
    }

    return dest;
}

void* memset(void* s, int c, size_t n) {
    MPP_MACRO_ASSERT(s != nullptr, "memset: target buffer cannot be null");

    auto* p = static_cast<unsigned char*>(s);
    unsigned char byte_value = static_cast<unsigned char>(c);

    // Optimization: Broadcast the single byte across a full word width block
    // e.g., if c = 0xAA, word_value becomes 0xAAAAAAA...
    size_t word_value = 0;
    for (size_t i = 0; i < sizeof(size_t); ++i) {
        word_value = (word_value << 8) | byte_value;
    }

    // Fill by word chunks
    while (n >= sizeof(size_t)) {
        *reinterpret_cast<size_t*>(p) = word_value;
        p += sizeof(size_t);
        n -= sizeof(size_t);
    }

    // Fill remaining trailing bytes
    while (n > 0) {
        *p++ = byte_value;
        n--;
    }

    return s;
}

void abort() {
    __builtin_trap();
}

MPP_LIB_NAMESPACE_END

#endif // PROJECT_PLATFORM == PROJECT_PLATFORM_WEB
